#!/usr/bin/env python3
"""Merge all project .md files into docs/esc32-完整文档.md

注意：分散的 .md 已合并删除后，请直接编辑 docs/esc32-完整文档.md；
勿重复运行本脚本（会覆盖合订本）。仅当恢复多文件文档后再合并。
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
    seen = set()

    for rel in ORDER:
        p = ROOT / rel.replace("/", "\\") if "\\" in str(ROOT) else ROOT / rel
        p = ROOT / rel
        if p.exists() and p not in seen:
            ordered.append(p)
            seen.add(p)

    for p in all_md:
        if p not in seen:
            ordered.append(p)
            seen.add(p)

    parts = [
        "# esc32 完整文档\n",
        "> 由仓库内全部 Markdown 自动合并生成。源文件仍保留在各目录，"
        "以 `scripts/merge-md.py` 重新生成。\n",
        f"> 生成时间：自动合并 · 共 **{len(ordered)}** 个源文件\n",
        "## 目录\n",
    ]

    for i, p in enumerate(ordered, 1):
        rel = p.relative_to(ROOT).as_posix()
        anchor = rel.replace("/", "-").replace(".", "").replace(" ", "-")
        title = rel
        parts.append(f"{i}. [{title}](#{anchor})\n")

    parts.append("\n" + SEP)

    for p in ordered:
        rel = p.relative_to(ROOT).as_posix()
        text = p.read_text(encoding="utf-8")
        # Demote top-level # to ## to avoid breaking doc structure
        lines = text.splitlines()
        if lines and lines[0].startswith("# "):
            lines[0] = "## " + lines[0][2:]
        body = "\n".join(lines)
        parts.append(f"<!-- 源文件: {rel} -->\n\n")
        parts.append(f"## 文档：{rel}\n\n")
        parts.append(body.strip())
        parts.append(SEP)

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("".join(parts).rstrip() + "\n", encoding="utf-8")
    line_count = len(OUT.read_text(encoding="utf-8").splitlines())
    print(f"Wrote {OUT} ({line_count} lines, {len(ordered)} sources)")


if __name__ == "__main__":
    main()
