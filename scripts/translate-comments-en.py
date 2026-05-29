#!/usr/bin/env python3
"""Replace Chinese text in source comments/strings with English (code comments only)."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SKIP_DIRS = {".git", ".venv", "dist", "build", "__pycache__", "build_pyinstaller", "node_modules"}

# Longer phrases first (order matters for overlapping matches)
REPLACEMENTS: list[tuple[str, str]] = [
    # --- Documentation / scripts ---
    ("Merge all project .md files into docs/esc32-Complete Documentation.md (Chinese edition)",
     "Merge all project .md files into docs/esc32-Complete Documentation.md (Chinese edition) (Chinese edition)"),
    ("Note: after scattered .md files were merged and removed, edit docs/esc32-Complete Documentation.md directly;",
     "Note: after scattered .md files were merged and removed, edit docs/esc32-Complete Documentation.md directly;"),
    ("Do not re-run this script (it overwrites the combined doc). Re-merge only after restoring split docs.",
     "Do not re-run this script (it overwrites the combined doc). Re-merge only after restoring split docs."),
    ("Auto-merged from all Markdown in the repo. Source files remain in their directories;",
     "Auto-merged from all Markdown in the repo. Source files remain in their directories;"),
    ("regenerate with `scripts/merge-md.py`.",
     "regenerate with `scripts/merge-md.py`."),
    ("Generated: auto-merge · **",
     "Generated: auto-merge · **"),
    (" source files", " source files"),
    ("Complete Documentation", "Complete Documentation"),
    ("Source:", "Source:"),
    ("Document:", "Document:"),
    # --- mcu_catalog.h ---
    ("MCU family catalog (common 32-bit ESC MCUs + FOC extension tiers)",
     "MCU family catalog (common 32-bit ESC MCUs + FOC extension tiers)"),
    ("Family ID: high byte = vendor, low byte = series index; used by host filtering and OTA packaging.",
     "Family ID: high byte = vendor, low byte = series index; used by host filtering and OTA packaging."),
    ("New MCU: register here -> boards/mcu/<family>/ -> boards/targets/<TARGET>/.",
     "New MCU: register here -> boards/mcu/<family>/ -> boards/targets/<TARGET>/."),
    ("Vendor domain", "Vendor domain"),
    ("integrated predriver", "integrated predriver"),
    ("ESC-60, G4 series priority port", "ESC-60, G4 series priority port"),
    ("ESC-80 first production target", "ESC-80 first production target"),
    ("high-power tier", "high-power tier"),
    ("mid/small ESC, dedicated HAL", "mid/small ESC, dedicated HAL"),
    ("CKS/WCH (not for production; compatibility register only)", "CKS/WCH (not for production; compatibility register only)"),
    # --- motor beep ---
    ("Motor winding audio: boot melody / link melody / signal-loss beeps",
     "Motor winding audio: boot melody / link melody / signal-loss beeps"),
    ("Fixed electrical angle + audio-frequency three-phase duty modulation (no FOC)",
     "Fixed electrical angle + audio-frequency three-phase duty modulation (no FOC)"),
    ("Full boot/link melody (three-note ascending, typical ESC style)",
     "Full boot/link melody (three-note ascending, typical ESC style)"),
    ("Loss alarm: short-pause-short", "Loss alarm: short-pause-short"),
    ("Called from app slow loop: advance notes and alarm cycle",
     "Called from app slow loop: advance notes and alarm cycle"),
    ("Optional P3 feature toggles", "Optional P3 feature toggles"),
    ("see comm/cyphal/README.md", "see comm/cyphal/README.md"),
    ("Motor beeps (power-on / link established / loss alarm)",
     "Motor beeps (power-on / link established / loss alarm)"),
    # --- protocol ---
    ("esc32 host <-> firmware debug/tuning protocol (UART or UDP transport)",
     "esc32 host <-> firmware debug/tuning protocol (UART or UDP transport)"),
    ("Protocol version; must match firmware ESC_FW_PROTO_VERSION",
     "Protocol version; must match firmware ESC_FW_PROTO_VERSION"),
    ("see firmware/include/mcu_catalog.h", "see firmware/include/mcu_catalog.h"),
    ("PCB revision", "PCB revision"),
    ("Firmware name / target short name", "Firmware name / target short name"),
    ("bit0 motor beep, bit1 UAVCAN", "bit0 motor beep, bit1 UAVCAN"),
    ("followed by `returned` fault_log_entry_t records (18 bytes each; see fault_log.h)",
     "followed by `returned` fault_log_entry_t records (18 bytes each; see fault_log.h)"),
    ("Pack frame into out; returns total length; buffer needs header+payload+crc",
     "Pack frame into out; returns total length; buffer needs header+payload+crc"),
    ("Parser state machine: feed bytes; callback(cmd,payload,len) on complete frame",
     "Parser state machine: feed bytes; callback(cmd,payload,len) on complete frame"),
    ("@return 0 continue, 1 frame done, <0 error (reset parser)",
     "@return 0 continue, 1 frame done, <0 error (reset parser)"),
    # --- mcu_port ---
    ("MCU capability descriptor (VESC-style hw_*.h macros + build-time selection)",
     "MCU capability descriptor (VESC-style hw_*.h macros + build-time selection)"),
    ("Core algorithms (FOC/protection/protocol) depend only on hal.h;",
     "Core algorithms (FOC/protection/protocol) depend only on hal.h;"),
    ("This module exposes dual ADC / op-amp / recommended product tier for defaults and host UI.",
     "This module exposes dual ADC / op-amp / recommended product tier for defaults and host UI."),
    ("2 or 3", "2 or 3"),
    # --- params ---
    ("ESC parameter block (motor / FOC / protection / comm)", "ESC parameter block (motor / FOC / protection / comm)"),
    ("Motor", "Motor"),
    ("Observer / FOC", "Observer / FOC"),
    ("Speed loop", "Speed loop"),
    ("Position loop (reserved)", "Position loop (reserved)"),
    ("Throttle / PWM", "Throttle / PWM"),
    ("Accel / decel", "Accel / decel"),
    ("Power / current limits", "Power / current limits"),
    ("Protection", "Protection"),
    ("Communication", "Communication"),
    ("Filtering", "Filtering"),
    ("21-point curve", "21-point curve"),
    ("Misc", "Misc"),
    ("Parameter descriptors: host read/write by index", "Parameter descriptors: host read/write by index"),
    # --- product / target / board ---
    ("产品 ID 与默认参数、Protection阈值", "Product ID and default params / protection thresholds"),
    ("Target metadata (build-time)", "Target metadata (build-time)"),
    ("Board: load product defaults, pins, HAL", "Board: load product defaults, pins, HAL"),
    ("Simulation platform HAL", "Simulation platform HAL"),
    ("G474 production HAL (skeleton/stub)", "G474 production HAL (skeleton/stub)"),
    ("G431 stub HAL", "G431 stub HAL"),
    ("H743 stub HAL", "H743 stub HAL"),
    ("AT32F415 stub HAL", "AT32F415 stub HAL"),
    ("Inherits common G4 configuration", "Inherits common G4 configuration"),
    ("Pin map (pending hardware finalization)", "Pin map (pending hardware finalization)"),
    ("Stub: no real peripherals connected", "Stub: no real peripherals connected"),
    # --- comm / uavcan ---
    ("DroneCAN application layer (built on comm/uavcan)", "DroneCAN application layer (built on comm/uavcan)"),
    ("UAVCAN v0 transport layer", "UAVCAN v0 transport layer"),
    ("Multi-frame transfer, CRC, tail byte", "Multi-frame transfer, CRC, tail byte"),
    ("DSDL encode/decode (ESC RawCommand/Status, Node)", "DSDL encode/decode (ESC RawCommand/Status, Node)"),
    ("CRC-16-CCITT-FALSE", "CRC-16-CCITT-FALSE"),
    ("PWM input and loss detection", "PWM input and loss detection"),
    ("Loss only after a valid link was established", "Loss only after a valid link was established"),
    # --- foc / motor / fault ---
    ("Sensorless observer interface", "Sensorless observer interface"),
    ("Motor状态机与 FOC 封装", "Motor state machine and FOC wrapper"),
    ("Fault black-box entry", "Fault black-box entry"),
    # --- app ---
    ("Play full melody immediately after power-on", "Play full melody immediately after power-on"),
    ("Play again on first PWM link", "Play again on first PWM link"),
    ("Periodic alarm after signal loss", "Periodic alarm after signal loss"),
    # --- Makefile ---
    ("Simulation target", "Simulation target"),
    ("Hardware ELF (stub HAL)", "Hardware ELF (stub HAL)"),
    ("G474 production HAL target", "G474 production HAL target"),
    ("Verify: build all targets", "Verify: build all targets"),
    # --- host python (comments only - partial) ---
    ("ESC debug protocol client", "ESC debug protocol client"),
    ("UDP transport", "UDP transport"),
    ("End-to-end test", "End-to-end test"),
    ("Motor标定向导", "Motor tuning wizard"),
    ("Apply JSON preset to sim/device", "Apply JSON preset to sim/device"),
    ("Graphical UI", "Graphical UI"),
    # --- shared defaults json ---
    ("Agricultural drone default", "Agricultural drone default"),
    ("Heavy-lift default", "Heavy-lift default"),
    ("Light load default", "Light load default"),
    ("High power default", "High power default"),
]

EXTS = {".c", ".h", ".py", ".ps1", ".json"}


def translate_text(text: str) -> str:
    for zh, en in REPLACEMENTS:
        text = text.replace(zh, en)
    return text


def process_file(path: Path) -> bool:
    try:
        original = path.read_text(encoding="utf-8")
    except OSError:
        return False
    updated = translate_text(original)
    if updated == original:
        return False
    path.write_text(updated, encoding="utf-8", newline="\n")
    return True


def main() -> None:
    changed: list[str] = []
    remaining: dict[str, int] = {}
    cjk = re.compile(r"[\u4e00-\u9fff]")

    for p in sorted(ROOT.rglob("*")):
        if not p.is_file():
            continue
        if any(d in p.parts for d in SKIP_DIRS):
            continue
        if p.suffix.lower() not in EXTS:
            continue
        if "esc32-Complete Documentation" in p.name or "esc32-full-document" in p.name:
            continue
        if process_file(p):
            changed.append(p.relative_to(ROOT).as_posix())
        n = len(cjk.findall(p.read_text(encoding="utf-8", errors="replace")))
        if n:
            remaining[p.relative_to(ROOT).as_posix()] = n

    print(f"Updated {len(changed)} files")
    for f in changed:
        print(f"  {f}")
    if remaining:
        print(f"\nRemaining CJK ({len(remaining)} files):")
        for f, n in sorted(remaining.items(), key=lambda x: -x[1]):
            print(f"  {n:4}  {f}")
    else:
        print("\nNo remaining CJK in scanned extensions.")


if __name__ == "__main__":
    main()
