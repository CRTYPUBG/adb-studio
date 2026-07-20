#!/usr/bin/env python3
import pathlib
import re
import sys


REQUIRED_IMPORTS = (
    "import QtQuick",
    "import QtQuick.Layouts",
    "import QtQuick.Controls",
    "import QtQuick.Controls.FluentWinUI3",
)
FORBIDDEN = ("QtQuick.Controls.Material", "QtQuick.Controls.Universal", "Canvas {")


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    errors = []
    for path in sorted((root / "apps" / "studio" / "qml").rglob("*.qml")):
        text = path.read_text(encoding="utf-8")
        relative = path.relative_to(root)
        for required in REQUIRED_IMPORTS:
            if required not in text:
                errors.append(f"{relative}: missing {required}")
        for forbidden in FORBIDDEN:
            if forbidden in text:
                errors.append(f"{relative}: forbidden UI construct {forbidden}")
        if re.search(r"\b(x|y)\s*:\s*[0-9]", text):
            errors.append(f"{relative}: fixed positioning is forbidden")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
