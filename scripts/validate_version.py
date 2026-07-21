#!/usr/bin/env python3
import json
import os
import pathlib
import re
import sys


SEMVER = re.compile(r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$")
REQUIRED = {
    "name": str,
    "company": str,
    "version": str,
    "build": int,
    "channel": str,
    "qt": str,
    "compiler": str,
    "minimumWindows": str,
    "architecture": str,
}


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    try:
        manifest = json.loads((root / "version.crty").read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"version validation: {error}", file=sys.stderr)
        return 1
    errors = []
    for field, expected_type in REQUIRED.items():
        if not isinstance(manifest.get(field), expected_type):
            errors.append(f"{field} must be {expected_type.__name__}")
    version = manifest.get("version", "")
    if not SEMVER.fullmatch(version):
        errors.append("version must be canonical semantic version MAJOR.MINOR.PATCH")
    if isinstance(manifest.get("build"), int) and manifest["build"] < 1:
        errors.append("build must be positive")
    tag = os.environ.get("GITHUB_REF_NAME", "")
    if tag.startswith("v") and tag[1:] != version:
        errors.append(f"Git tag {tag} does not match version {version}")
    if errors:
        print("\n".join(f"version validation: {error}" for error in errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
