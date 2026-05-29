"""python -m esc_tool"""
import sys

from .cli import main as cli_main
from .gui import run_gui


def main() -> None:
    if "--gui" in sys.argv:
        sys.argv.remove("--gui")
        run_gui()
    else:
        cli_main()


if __name__ == "__main__":
    main()
