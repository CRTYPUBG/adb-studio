#!/usr/bin/env python3
"""Fail when a supported Qt translation is incomplete or breaks placeholders."""

from __future__ import annotations

import pathlib
import re
import sys
import xml.etree.ElementTree as ElementTree


PLACEHOLDER = re.compile(r"%\d+|%n")


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    errors: list[str] = []
    for path in sorted((root / "translations").glob("adb_studio_*.ts")):
        tree = ElementTree.parse(path)
        for message in tree.findall(".//message"):
            source = message.findtext("source", default="")
            translation = message.find("translation")
            if translation is None or translation.get("type") == "unfinished" or not translation.text:
                errors.append(f"{path.relative_to(root)}: missing translation: {source}")
                continue
            if sorted(PLACEHOLDER.findall(source)) != sorted(PLACEHOLDER.findall(translation.text)):
                errors.append(f"{path.relative_to(root)}: placeholder mismatch: {source}")
    if not list((root / "translations").glob("adb_studio_*.ts")):
        errors.append("translations: no application catalogs found")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
