#!/usr/bin/env python3
"""Quick interactive demo: arm the sim ESC, ramp throttle, print telemetry.

Run from the host directory while esc32_sim.exe is running:

    cd host
    python ..\\scripts\\sim-demo.py
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

# Make `import esc_tool` work whether run from repo root or host/.
HOST_DIR = Path(__file__).resolve().parent.parent / "host"
if str(HOST_DIR) not in sys.path:
    sys.path.insert(0, str(HOST_DIR))

from esc_tool.client import EscClient
from esc_tool.transport import UdpTransport


def main() -> None:
    tp = UdpTransport("127.0.0.1", 7777)
    c = EscClient(tp)
    try:
        info = c.get_info()
        print(f"Connected: {info.name}  fw={info.fw_version >> 8}.{info.fw_version & 0xFF}  product=0x{info.product_id:02X}")
        c.set_param("motor_sound_enable", 0.0)
        c.save_params()
        time.sleep(0.6)  # let any boot melody finish before we start

        print("\n[1] disarm + telem snapshot")
        c.disarm()
        time.sleep(0.1)
        t = c.get_telem()
        print(f"    state={t.state} V={t.vbus_mv/1000:.1f}V RPM={t.rpm} thr={t.throttle_us}us")

        print("\n[2] arm + ramp throttle 1100 -> 1700us")
        c.arm()
        time.sleep(0.1)

        for thr in (1100, 1300, 1500, 1700, 1500, 1100):
            c.set_throttle_us(thr)
            for _ in range(8):
                time.sleep(0.05)
                t = c.get_telem()
            print(f"    thr={thr:4d}us  V={t.vbus_mv/1000:5.2f}V  I={t.ibus_ma:5d}mA  iq={t.iq_ma:5d}mA  RPM={t.rpm:6d}  state={t.state}")

        print("\n[3] disarm + final telem")
        c.disarm()
        time.sleep(0.1)
        t = c.get_telem()
        print(f"    state={t.state} V={t.vbus_mv/1000:.1f}V RPM={t.rpm}")

        print("\n[4] fault log (last 4)")
        for e in c.get_fault_log(0, 4):
            print(f"    t={e.timestamp_ms}ms code=0x{e.code:02X} V={e.vbus_mv/1000:.1f}V RPM={e.rpm} state={e.state}")
        print("\nDemo complete.")
    finally:
        tp.close()


if __name__ == "__main__":
    main()
