#!/usr/bin/env python3
import datetime
import json
import os
import pathlib
import subprocess
import sys


def git_value(root: pathlib.Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", *arguments], cwd=root, text=True, capture_output=True, check=False
    )
    return result.stdout.strip() if result.returncode == 0 else ""


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    path = root / "version.crty"
    manifest = json.loads(path.read_text(encoding="utf-8"))
    manifest["commit"] = os.environ.get("GITHUB_SHA") or git_value(root, "rev-parse", "HEAD")
    manifest["branch"] = os.environ.get("GITHUB_REF_NAME") or git_value(
        root, "branch", "--show-current"
    )
    manifest["buildDate"] = datetime.datetime.now(datetime.timezone.utc).replace(
        microsecond=0
    ).isoformat().replace("+00:00", "Z")
    run_number = os.environ.get("GITHUB_RUN_NUMBER")
    if run_number:
        manifest["build"] = int(run_number)
    path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
