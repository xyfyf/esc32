#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SKIP_DIRS = {
    ".git", ".venv", "dist", "build", "__pycache__",
    ".pytest_cache", "node_modules", "build_pyinstaller",
}
SKIP_EXT = {
    ".exe", ".elf", ".hex", ".map", ".bin", ".pkg", ".pyz",
    ".pyc", ".zip", ".toc", ".html", ".o",
}


def skip(p: Path) -> bool:
    if any(d in SKIP_DIRS for d in p.parts):
        return True
    return p.suffix.lower() in SKIP_EXT


def classify(path: Path) -> str:
    if path.name in ("Makefile", "CMakeLists.txt"):
        return "Makefile/CMake"
    ext = path.suffix.lower()
    return {
        ".c": "C",
        ".h": "H",
        ".py": "Python",
        ".ps1": "PowerShell",
        ".md": "Markdown",
        ".json": "JSON",
        ".yml": "YAML",
        ".yaml": "YAML",
    }.get(ext, "Other")


def main() -> None:
    fc: dict[str, list[int]] = {}
    dir_src: dict[str, int] = {}

    for path in ROOT.rglob("*"):
        if not path.is_file() or skip(path):
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        n = len(text.splitlines()) if text.strip() else 0
        key = classify(path)
        fc.setdefault(key, [0, 0])
        fc[key][0] += 1
        fc[key][1] += n

        if path.suffix.lower() in (".c", ".h", ".py", ".ps1") or path.name == "Makefile":
            top = path.relative_to(ROOT).parts[0]
            dir_src[top] = dir_src.get(top, 0) + n

    src_keys = ["C", "H", "Python", "PowerShell", "Makefile/CMake"]
    print(f"Root: {ROOT}\n")
    print(f"{'Category':<18} {'Files':>6} {'Lines':>8}")
    print("-" * 36)
    for k in [
        "C", "H", "Python", "PowerShell", "Makefile/CMake",
        "JSON", "Markdown", "YAML", "Other",
    ]:
        if k in fc:
            print(f"{k:<18} {fc[k][0]:>6} {fc[k][1]:>8}")
    print("-" * 36)
    sf = sum(fc[k][0] for k in src_keys if k in fc)
    sl = sum(fc[k][1] for k in src_keys if k in fc)
    tf = sum(v[0] for v in fc.values())
    tl = sum(v[1] for v in fc.values())
    print(f"{'Source code':<18} {sf:>6} {sl:>8}")
    print(f"{'Incl. docs/config':<18} {tf:>6} {tl:>8}")
    print("\nSource by top-level directory:")
    for d, n in sorted(dir_src.items(), key=lambda x: -x[1]):
        print(f"  {d:<14} {n:>6}")


if __name__ == "__main__":
    main()
