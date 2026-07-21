#!/usr/bin/env python3
"""Fail-closed dependency, license and secret-material audit for release inputs."""

from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import sys


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    output = Path(sys.argv[2] if len(sys.argv) > 2 else root / "dist/evidence")
    output.mkdir(parents=True, exist_ok=True)
    errors: list[str] = []
    version = json.loads((root / "version.crty").read_text(encoding="utf-8"))
    notices = (root / "THIRD_PARTY_LICENSES.md").read_text(encoding="utf-8")
    lowered = notices.lower()
    for marker in ("pending verification", "unknown license", "todo"):
        if marker in lowered:
            errors.append(f"third-party notices contain unresolved marker: {marker}")
    presets = (root / "CMakePresets.json").read_text(encoding="utf-8")
    if version["qt"] not in presets:
        errors.append("Qt version is not synchronized between version.crty and CMakePresets.json")
    tracked = subprocess.run(["git", "ls-files"], cwd=root, text=True,
                             capture_output=True, check=True).stdout.splitlines()
    forbidden_extensions = {".pfx", ".p12", ".p8", ".pem", ".key"}
    leaked = [name for name in tracked if Path(name).suffix.lower() in forbidden_extensions]
    if leaked:
        errors.append("private key/certificate material is tracked: " + ", ".join(leaked))
    report = {
        "schemaVersion": 1,
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "version": version["version"],
        "checks": {
            "resolvedLicenseNotices": "passed" if not any("notices" in e for e in errors) else "failed",
            "qtVersionSynchronization": "passed" if not any("Qt version" in e for e in errors) else "failed",
            "trackedSecretMaterial": "passed" if not leaked else "failed",
        },
        "errors": errors,
        "passed": not errors,
    }
    (output / "dependency-license-audit.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if errors:
        print("\n".join(f"supply-chain audit: {error}" for error in errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
