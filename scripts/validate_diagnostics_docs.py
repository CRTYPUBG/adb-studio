#!/usr/bin/env python3
"""Reject connection/OEM implementation changes that leave required documentation stale."""

from __future__ import annotations

from pathlib import Path
import re
import sys


REQUIRED_DOCUMENTS = (
    "OEM_GUIDES.md",
    "DEVICE_DETECTION.md",
    "SMART_DIAGNOSTICS.md",
    "CONNECTION_WIZARD.md",
    "USB_RECOVERY.md",
    "WIRELESS_SETUP.md",
    "DEVICE_HEALTH_SCORE.md",
)


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    errors: list[str] = []
    documents: dict[str, str] = {}
    for name in REQUIRED_DOCUMENTS:
        path = root / name
        if not path.is_file():
            errors.append(f"missing required diagnostics document: {name}")
            continue
        documents[name] = path.read_text(encoding="utf-8")
    source_path = root / "core/diagnostics/src/device_diagnostics.cpp"
    if not source_path.is_file():
        errors.append("missing diagnostics engine implementation")
    else:
        source = source_path.read_text(encoding="utf-8")
        state_keys = set(re.findall(r'm_stateKey|stateKey = QStringLiteral\("([A-Z_]+)"\)', source))
        # The first alternative is retained for compatibility with generated sources; remove empties.
        state_keys.discard("")
        detection = documents.get("DEVICE_DETECTION.md", "")
        for key in sorted(state_keys):
            if f"`{key}`" not in detection:
                errors.append(f"DEVICE_DETECTION.md does not document state {key}")
        catalog_keys = set(re.findall(r'\{"([a-z]+)", "[^"]+", "https://', source))
        catalog_keys.add("other")
        guides = documents.get("OEM_GUIDES.md", "")
        for key in sorted(catalog_keys):
            if f"`{key}`" not in guides:
                errors.append(f"OEM_GUIDES.md does not document catalog key {key}")
    if errors:
        print("\n".join(f"diagnostics documentation: {error}" for error in errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
