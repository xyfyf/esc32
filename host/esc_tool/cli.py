"""Command-line host tool (minimal P0 feature set)."""
from __future__ import annotations

import argparse
import sys
import time

from .client import EscClient
from .transport import SerialTransport, UdpTransport

PARAM_NAMES = [
    "motor_kv",
    "motor_pole_pairs",
    "ibus_max_current",
    "power_limit",
    "node_id",
    "normal_pwm_start",
    "normal_pwm_end",
]


def main() -> None:
    ap = argparse.ArgumentParser(description="esc32 ESC tuning tool")
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--sim-udp", metavar="HOST:PORT", help="Connect to the simulator over UDP, e.g. 127.0.0.1:7777")
    g.add_argument("--serial", metavar="PORT", help="Serial port, e.g. COM3")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--gui", action="store_true", help="Launch the graphical UI")
    ap.add_argument("command", nargs="?", default="shell",
                    choices=["ping", "info", "telem", "params", "defaults", "save",
                             "faults", "shell"])
    args = ap.parse_args()

    if args.gui:
        from .gui import run_gui
        run_gui()
        return

    if args.sim_udp:
        host, _, port = args.sim_udp.partition(":")
        tp = UdpTransport(host, int(port or "7777"))
    else:
        tp = SerialTransport(args.serial, args.baud)

    client = EscClient(tp)

    try:
        if args.command == "ping":
            print("PONG version", client.ping())
        elif args.command == "info":
            i = client.get_info()
            print(f"Device: {i.name}  FW={i.fw_version>>8}.{i.fw_version&0xFF}  board=0x{i.board_id:02X}")
        elif args.command == "telem":
            t = client.get_telem()
            print(f"state={t.state} fault={t.fault_code} V={t.vbus_mv/1000:.1f}V "
                  f"I={t.ibus_ma}mA RPM={t.rpm} thr={t.throttle_us}us")
        elif args.command == "params":
            for idx in range(32):
                try:
                    name, val = client.list_param(idx)
                    print(f"  [{idx:2d}] {name:24s} = {val}")
                except TimeoutError:
                    break
        elif args.command == "defaults":
            client.load_defaults()
            print("Defaults loaded")
        elif args.command == "save":
            client.save_params()
            print("Params saved")
        elif args.command == "faults":
            for e in client.get_fault_log(0, 16):
                print(e)
        else:
            run_shell(client)
    finally:
        tp.close()


def run_shell(client: EscClient) -> None:
    print("esc_tool shell. Commands: ping | info | telem | params | arm | disarm | thr <us> | set <name> <val> | quit")
    while True:
        try:
            line = input("esc> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break
        if not line:
            continue
        parts = line.split()
        cmd = parts[0].lower()
        try:
            if cmd in ("q", "quit", "exit"):
                break
            if cmd == "ping":
                print("proto", client.ping())
            elif cmd == "info":
                print(client.get_info())
            elif cmd == "telem":
                print(client.get_telem())
            elif cmd == "params":
                for idx in range(len(PARAM_NAMES)):
                    try:
                        print(client.list_param(idx))
                    except TimeoutError:
                        break
            elif cmd == "arm":
                client.arm()
                print("armed")
            elif cmd == "disarm":
                client.disarm()
                print("disarmed")
            elif cmd == "thr" and len(parts) >= 2:
                client.set_throttle_us(int(parts[1]))
            elif cmd == "set" and len(parts) >= 3:
                ok = client.set_param(parts[1], float(parts[2]))
                print("ok" if ok else "failed")
            elif cmd == "watch":
                while True:
                    t = client.get_telem()
                    sys.stdout.write(
                        f"\r V={t.vbus_mv/1000:.1f}V I={t.ibus_ma:5d}mA RPM={t.rpm:6d} st={t.state}   "
                    )
                    sys.stdout.flush()
                    time.sleep(0.1)
            else:
                print("unknown command")
        except Exception as e:
            print("error:", e)


if __name__ == "__main__":
    main()
