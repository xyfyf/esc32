"""将 shared/defaults/*.json 预设写入已连接的 ESC"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from .client import EscClient
from .transport import SerialTransport, UdpTransport

# JSON 键 -> 固件参数名
KEY_MAP = {
    "motor_kv": "motor_kv",
    "motor_pole_pairs": "motor_pole_pairs",
    "motor_ld_uh": "motor_ld",
    "motor_lq_uh": "motor_lq",
    "motor_rs_mohm": "motor_rs",
    "motor_max_current_a": "motor_max_current",
    "motor_max_rpm": "motor_max_rpm",
    "observer_type": "observer_type",
    "ibus_max_current_a": "ibus_max_current",
    "power_limit_w": "power_limit",
    "normal_pwm_start_us": "normal_pwm_start",
    "normal_pwm_end_us": "normal_pwm_end",
    "node_id": "node_id",
    "motor_sound_enable": "motor_sound_enable",
    "motor_sound_volume": "motor_sound_volume",
}


def apply_preset(client: EscClient, preset: dict) -> None:
    for jk, pname in KEY_MAP.items():
        if jk in preset:
            client.set_param(pname, float(preset[jk]))
    client.save_params()


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("preset_json", type=Path)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--sim-udp", default="127.0.0.1:7777")
    g.add_argument("--serial", metavar="COM")
    args = ap.parse_args()

    data = json.loads(args.preset_json.read_text(encoding="utf-8"))
    if args.serial:
        tp = SerialTransport(args.serial, 115200)
    else:
        h, _, p = args.sim_udp.partition(":")
        tp = UdpTransport(h, int(p or "7777"))

    c = EscClient(tp)
    try:
        apply_preset(c, data)
        print(f"Applied: {data.get('config_name', args.preset_json.name)}")
    finally:
        tp.close()


if __name__ == "__main__":
    main()
