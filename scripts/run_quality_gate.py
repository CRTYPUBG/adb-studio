#!/usr/bin/env python3
"""Run the complete local milestone gate repeatedly and record evidence."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import os
import pathlib
import shutil
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "build/windows-msvc-release"
QT_BIN = pathlib.Path(os.environ.get("QTDIR", "C:/Qt/6.8.1/msvc2022_64")) / "bin"


def run(args: list[str | pathlib.Path], environment: dict[str, str] | None = None) -> None:
    command = [str(value) for value in args]
    print("+", subprocess.list2cmdline(command), flush=True)
    subprocess.run(command, cwd=ROOT, env=environment, check=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runs", type=int, default=3)
    options = parser.parse_args()
    if options.runs < 1:
        raise ValueError("--runs must be positive")
    evidence = ROOT / "dist/evidence"
    evidence.mkdir(parents=True, exist_ok=True)
    validators = (
        "validate_resources.py", "validate_version.py", "validate_architecture.py",
        "validate_qml_policy.py", "validate_diagnostics_docs.py",
        "validate_no_placeholders.py", "validate_translations.py", "validate_ui_connections.py",
    )
    sources = sorted(path for directory in (ROOT / "apps", ROOT / "core", ROOT / "tests")
                     for suffix in ("*.cpp", "*.hpp") for path in directory.rglob(suffix))
    qml = sorted((ROOT / "apps/studio/qml").rglob("*.qml"))
    environment = os.environ.copy()
    environment["PATH"] = str(QT_BIN) + os.pathsep + environment.get("PATH", "")
    environment["QT_FORCE_STDERR_LOGGING"] = "1"
    completed = []
    for number in range(1, options.runs + 1):
        print(f"QUALITY_GATE_RUN={number}/{options.runs}", flush=True)
        for validator in validators:
            run([sys.executable, ROOT / "scripts" / validator, ROOT])
        run([shutil.which("clang-format") or "clang-format", "--dry-run", "--Werror", *sources])
        run(["cmake", "--build", BUILD, "--config", "Release", "--target",
             "adb_studio_core_qmltyperegistration"])
        run([QT_BIN / "qmllint.exe", "-I", BUILD / "core", *qml])
        run(["cmake", "--build", "--preset", "windows-msvc-release", "--parallel"])
        run(["ctest", "--test-dir", BUILD, "-C", "Release", "--output-on-failure"])
        run([sys.executable, "-m", "unittest", "discover", "-s", "tests/python", "-p", "test_*.py"])
        executable = BUILD / "Release/adb-studio.exe"
        for language in ("en", "tr"):
            screenshot = evidence / f"quality-gate-{number}-{language}.png"
            run([executable, "--language", language, "--workspace", "mirror",
                 "--render-screenshot", screenshot], environment)
            if not screenshot.is_file() or screenshot.stat().st_size == 0:
                raise RuntimeError(f"runtime screenshot was not produced: {screenshot}")
        completed.append({"run": number, "passed": True})
    report = {
        "schemaVersion": 1,
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "runs": completed,
        "passed": len(completed) == options.runs,
    }
    (evidence / "quality-gate.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
