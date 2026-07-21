#!/usr/bin/env python3
"""Reject user-visible placeholder language and unfinished production markers."""

from __future__ import annotations

import pathlib
import re
import sys


FORBIDDEN_TEXT = re.compile(
    r'(?i)["\'](?:coming soon|not implemented|backend required|dummy data|mock service|stub implementation|todo)["\']'
)
FORBIDDEN_MARKER = re.compile(r"(?i)\b(?:TODO|FIXME)\b")


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    errors: list[str] = []
    for base in (root / "apps", root / "core"):
        for path in sorted(base.rglob("*")):
            if path.suffix.lower() not in {".cpp", ".hpp", ".qml"}:
                continue
            text = path.read_text(encoding="utf-8")
            relative = path.relative_to(root)
            for number, line in enumerate(text.splitlines(), 1):
                if FORBIDDEN_TEXT.search(line) or FORBIDDEN_MARKER.search(line):
                    errors.append(f"{relative}:{number}: unfinished production marker")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
