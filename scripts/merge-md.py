#!/usr/bin/env python3
"""Merge all project .md files into the consolidated documentation file.

Note: once the per-section .md files have been merged and removed, edit the
consolidated documents (`docs/esc32-完整文档.md` and `docs/esc32-full-document-en.md`)
directly. Do not re-run this script unless the per-section files are restored,
otherwise it will overwrite the merged doc.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "docs" / "esc32-完整文档.md"
SKIP_DIRS = {".git", ".venv", "dist", "build", "__pycache__", "build_pyinstaller", "node_modules"}

# Logical reading order (relative to ROOT)
ORDER = [
    "README.md",
    "docs/项目特点.md",
    "docs/需求实现状态.md",
    "docs/可行性分析与技术方案.md",
    "docs/系统闭环.md",
    "docs/ROADMAP.md",
    "docs/BUILD.md",
    "docs/命名与系列规范.md",
    "docs/MCU移植与多平台架构.md",
    "docs/UAVCAN协议栈.md",
    "docs/生产与标定流程.md",
    "docs/台架验收清单.md",
    "docs/hardware/ESC-80硬件原理图.md",
    "docs/hardware/原理图-三相桥详图.md",
    "docs/hardware/STM32G474引脚与接口.md",
    "docs/hardware/BOM-ESC-80.md",
    "docs/hardware/ESC-60硬件概要.md",
    "docs/hardware/ESC-120硬件概要.md",
    "docs/hardware/IPX6结构与灌封工艺.md",
    "hardware/kicad/ESC-80/README.md",
    "shared/defaults/README.md",
    "firmware/boards/targets/README.md",
    "firmware/boards/mcu/stm32g474/README.md",
    "firmware/boards/mcu/stm32g431/README.md",
    "firmware/boards/mcu/stm32h743/README.md",
    "firmware/boards/mcu/at32f415/README.md",
    "firmware/comm/cyphal/README.md",
]

SEP = "\n\n---\n\n"


def should_skip(p: Path) -> bool:
    return any(d in SKIP_DIRS for d in p.parts)


def collect_all_md() -> list[Path]:
    found = sorted(
        p for p in ROOT.rglob("*.md")
        if p.is_file() and not should_skip(p) and p != OUT
    )
    return found


def main() -> None:
    all_md = collect_all_md()
    ordered: list[Path] = []
    seen: set[Path] = set()

    for rel in ORDER:
        p = ROOT / rel
        if p.exists() and p not in seen:
            ordered.append(p)
            seen.add(p)

    for p in all_md:
        if p not in seen:
            ordered.append(p)
            seen.add(p)

    parts = [
        "# esc32 Complete Documentation\n",
        "> Auto-merged from every Markdown file in the repository. The source\n"
        "> files remain in place under their original directories and can be\n"
        "> regenerated with `scripts/merge-md.py`.\n",
        f"> Generated: auto-merge \u00b7 **{len(ordered)}** source files\n",
        "## Table of Contents\n",
    ]

    for i, p in enumerate(ordered, 1):
        rel = p.relative_to(ROOT).as_posix()
        anchor = rel.replace("/", "-").replace(".", "").replace(" ", "-")
        parts.append(f"{i}. [{rel}](#{anchor})\n")

    parts.append("\n" + SEP)

    for p in ordered:
        rel = p.relative_to(ROOT).as_posix()
        text = p.read_text(encoding="utf-8")
        # Demote top-level # to ## so that the merged doc keeps a single H1.
        lines = text.splitlines()
        if lines and lines[0].startswith("# "):
            lines[0] = "## " + lines[0][2:]
        body = "\n".join(lines)
        parts.append(f"<!-- Source: {rel} -->\n\n")
        parts.append(f"## Document: {rel}\n\n")
        parts.append(body.strip())
        parts.append(SEP)

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("".join(parts).rstrip() + "\n", encoding="utf-8")
    line_count = len(OUT.read_text(encoding="utf-8").splitlines())
    print(f"Wrote {OUT} ({line_count} lines, {len(ordered)} sources)")


if __name__ == "__main__":
    main()
