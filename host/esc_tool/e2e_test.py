"""Closed-loop end-to-end self-test (requires firmware/esc32_sim.exe to be running)."""
from __future__ import annotations

import struct
import sys
import time
import zlib

from .client import EscClient
from .protocol import Cmd, pack_frame
from .transport import UdpTransport

FAULT_NAMES = {
    0: "NONE",
    1: "OVER_VOLTAGE",
    2: "UNDER_VOLTAGE",
    3: "OVER_CURRENT",
    4: "OVER_TEMP_MOS",
    9: "THROTTLE_LOST",
}


def run_e2e(host: str = "127.0.0.1", port: int = 7777) -> int:
    tp = UdpTransport(host, port)
    c = EscClient(tp)
    failed = 0

    def check(name: str, cond: bool) -> None:
        nonlocal failed
        if cond:
            print(f"  [OK] {name}")
        else:
            print(f"  [FAIL] {name}")
            failed += 1

    print("=== esc32 E2E ===")
    try:
        ver = c.ping()
        check("PING", ver >= 1)

        info = c.get_info()
        check("GET_INFO name", len(info.name) > 0)
        check("GET_INFO product", info.product_id in (0x60, 0x80, 0x120))
        if info.proto_version >= 2:
            check("GET_INFO target_id", info.target_id != 0)
            check("GET_INFO mcu_id", info.mcu_id == 0)  # SIM

        c.load_defaults()
        check("LOAD_DEFAULTS", True)

        pl = struct.pack("<HH", 0, 200)
        assert c._exchange(Cmd.SET_PARAM, pl)[0] == 0, "SET motor_kv"
        pl = struct.pack("<HH", 15, 31)  # node_id index in s_desc
        c._exchange(Cmd.SET_PARAM, pl)
        c.save_params()
        name, kv = c.list_param(0)
        check("SET/GET_PARAM motor_kv", name == "motor_kv" and abs(kv - 200.0) < 0.1)

        c.arm()
        c.set_throttle_us(1500)
        time.sleep(0.15)
        t = c.get_telem()
        check("ARM+THROTTLE state", t.state >= 2)
        check("TELEM vbus", t.vbus_mv > 10000)
        c.disarm()
        c.set_throttle_us(1000)

        # OTA: small image
        img = bytes([0xEC, 0x32, 0x01, 0x02, 0x03] * 20)
        c.fw_erase()
        c.fw_write(0, img[:64])
        crc = zlib.crc32(img) & 0xFFFFFFFF
        c.fw_crc(crc)
        check("OTA pipeline", True)

        logs = c.get_fault_log(0, 4)
        check("FAULT_LOG read", isinstance(logs, list))

        print(f"\nResult: {failed} failed")
        return 1 if failed else 0
    except Exception as e:
        print(f"E2E error: {e}")
        return 1
    finally:
        tp.close()


def main() -> None:
    host = "127.0.0.1"
    port = 7777
    if len(sys.argv) >= 2:
        h, _, p = sys.argv[1].partition(":")
        host = h or host
        port = int(p or "7777")
    sys.exit(run_e2e(host, port))


if __name__ == "__main__":
    main()
