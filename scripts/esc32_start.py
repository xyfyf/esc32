"""One-click launcher: simulation firmware + host GUI (packaged as esc32_start.exe)."""
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
        _msg(f"Simulation firmware not found:\n{sim}\n\nRun scripts\\build-release.ps1 first to build the release package.")
        sys.exit(1)

    flags = 0
    if os.name == "nt":
        flags = subprocess.CREATE_NEW_CONSOLE  # keep the simulator's console output visible

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
            _msg("esc_tool.exe not found and the development environment is unavailable.\nRun build-release.ps1.")
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
