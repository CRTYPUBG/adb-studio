#!/usr/bin/env python3
"""Generate the multi-resolution Windows icon from the canonical PNG asset."""

from __future__ import annotations

import pathlib
import sys

from PIL import Image


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    source = root / "assets" / "app-icon-adb.png"
    output = root / "assets" / "app-icon-adb.ico"
    with Image.open(source) as image:
        rgba = image.convert("RGBA")
        rgba.save(
            output,
            format="ICO",
            sizes=((16, 16), (20, 20), (24, 24), (32, 32), (40, 40), (48, 48),
                   (64, 64), (128, 128), (256, 256)),
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
