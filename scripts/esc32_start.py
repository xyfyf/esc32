"""一键启动：仿真固件 + 上位机 GUI（打包为 esc32_start.exe 后双击即可）"""
from __future__ import annotations

import os
import sys
import time
import subprocess
from pathlib import Path


def _base_dir() -> Path:
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parents[1]


def main() -> None:
    base = _base_dir()
    sim = base / "esc32_sim.exe"
    tool = base / "esc_tool.exe"

    if not sim.is_file():
        _msg(f"未找到仿真固件：\n{sim}\n\n请先运行 scripts\\build-release.ps1 生成发布包。")
        sys.exit(1)

    flags = 0
    if os.name == "nt":
        flags = subprocess.CREATE_NEW_CONSOLE  # 保留仿真器控制台输出

    subprocess.Popen([str(sim)], cwd=str(base), creationflags=flags)

    time.sleep(0.8)

    if tool.is_file():
        subprocess.Popen([str(tool)], cwd=str(base))
    else:
        host_py = base.parent / "host" / ".venv" / "Scripts" / "python.exe"
        if host_py.is_file():
            subprocess.Popen(
                [str(host_py), "-m", "esc_tool", "--gui"],
                cwd=str(base.parent / "host"),
            )
        else:
            _msg(f"未找到 esc_tool.exe，且开发环境不可用。\n请运行 build-release.ps1。")
            sys.exit(1)


def _msg(text: str) -> None:
    if os.name == "nt":
        try:
            import ctypes

            ctypes.windll.user32.MessageBoxW(0, text, "esc32", 0x10)
            return
        except Exception:
            pass
    print(text, file=sys.stderr)


if __name__ == "__main__":
    main()
