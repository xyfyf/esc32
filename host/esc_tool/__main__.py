"""python -m esc_tool"""
import sys


def main() -> None:
    if "--gui" in sys.argv:
        sys.argv.remove("--gui")
        from .gui import run_gui
        run_gui()
    else:
        from .cli import main as cli_main
        cli_main()


if __name__ == "__main__":
    main()
