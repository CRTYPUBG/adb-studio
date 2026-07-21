#!/usr/bin/env python3
"""Validate that every exposed workspace and interactive control has a real command path."""

from __future__ import annotations

import pathlib
import re
import sys


CONTROL_EVENTS = {
    "Button": "onClicked:",
    "PrimaryButton": "onClicked:",
    "Switch": "onToggled:",
    "MenuItem": "onTriggered:",
    "FileDialog": "onAccepted:",
    "DropArea": "onDropped:",
}


def blocks(text: str, control: str):
    pattern = re.compile(rf"(?<![A-Za-z0-9_])(?:Components\.)?{control}\s*\{{")
    for match in pattern.finditer(text):
        start = match.start()
        cursor = match.end()
        depth = 1
        while cursor < len(text) and depth:
            if text[cursor] == "{":
                depth += 1
            elif text[cursor] == "}":
                depth -= 1
            cursor += 1
        yield start, text[start:cursor]


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    errors: list[str] = []
    qml_paths = [root / "apps/studio/qml/Main.qml", *sorted((root / "apps/studio/qml/pages").glob("*.qml")),
                 root / "apps/studio/qml/components/ConnectionWizard.qml"]
    for path in qml_paths:
        text = path.read_text(encoding="utf-8")
        for control, event in CONTROL_EVENTS.items():
            for offset, block in blocks(text, control):
                if event not in block:
                    line = text.count("\n", 0, offset) + 1
                    errors.append(f"{path.relative_to(root)}:{line}: {control} has no {event}")

    workspace = (root / "core/presentation/src/workspace_view_model.cpp").read_text(encoding="utf-8")
    keys = re.findall(r'module\("([a-z-]+)"', workspace)
    if keys != ["dashboard", "devices", "mirror", "guide"]:
        errors.append(f"workspace registry exposes unverified modules: {keys}")

    required_symbols = {
        "core/services/include/adb_studio/services/mirror_service.hpp":
            ("class AdbService", "class DeviceManagerService", "class MirrorService"),
        "core/presentation/include/adb_studio/presentation/mirror_view_model.hpp":
            ("class MirrorViewModel", "Q_INVOKABLE void start", "Q_INVOKABLE void pause",
             "Q_INVOKABLE void takeScreenshot", "Q_INVOKABLE void installApk"),
    }
    for relative, symbols in required_symbols.items():
        text = (root / relative).read_text(encoding="utf-8")
        for symbol in symbols:
            if symbol not in text:
                errors.append(f"{relative}: missing connected service symbol {symbol}")

    for relative in ("scrcpy/adb.exe", "scrcpy/scrcpy.exe", "scrcpy/scrcpy-server"):
        if not (root / relative).is_file():
            errors.append(f"missing verified mirror runtime: {relative}")

    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
