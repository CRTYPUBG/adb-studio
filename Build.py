#!/usr/bin/env python3
"""Configure, build, test and create a self-contained ADB Studio folder."""

from __future__ import annotations

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
import json


ROOT = pathlib.Path(__file__).resolve().parent
DEFAULT_QT = pathlib.Path("C:/Qt/6.8.1/msvc2022_64")


def run(command: list[str], *, environment: dict[str, str] | None = None) -> None:
    print("+", subprocess.list2cmdline(command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True, env=environment)


def safe_remove_tree(path: pathlib.Path, allowed_parent: pathlib.Path) -> None:
    resolved = path.resolve()
    parent = allowed_parent.resolve()
    if resolved == parent or parent not in resolved.parents:
        raise RuntimeError(f"Refusing to remove unsafe output path: {resolved}")
    if resolved.exists():
        shutil.rmtree(resolved)


def locate_qt() -> pathlib.Path:
    candidate = pathlib.Path(os.environ.get("QTDIR", DEFAULT_QT))
    deploy_tool = candidate / "bin" / "windeployqt.exe"
    if not deploy_tool.is_file():
        raise FileNotFoundError(
            f"Qt deployment tool was not found at {deploy_tool}. Set QTDIR to a Qt 6.8+ MSVC kit."
        )
    return candidate.resolve()


def locate_inno_setup() -> pathlib.Path:
    configured = os.environ.get("ISCC")
    candidates = [
        pathlib.Path(configured) if configured else pathlib.Path(),
        pathlib.Path("C:/Program Files (x86)/Inno Setup 6/ISCC.exe"),
        pathlib.Path("C:/Program Files/Inno Setup 6/ISCC.exe"),
    ]
    for candidate in candidates:
        if candidate and candidate.is_file():
            return candidate.resolve()
    raise FileNotFoundError("Inno Setup 6 Compiler (ISCC.exe) was not found; set ISCC")


def deployment_environment() -> dict[str, str]:
    environment = os.environ.copy()
    if environment.get("VCINSTALLDIR"):
        return environment
    visual_studio = pathlib.Path("C:/Program Files/Microsoft Visual Studio/2022")
    for edition in ("Community", "BuildTools", "Professional", "Enterprise"):
        vc_directory = visual_studio / edition / "VC"
        if vc_directory.is_dir():
            environment["VCINSTALLDIR"] = str(vc_directory)
            return environment
    raise FileNotFoundError("Visual Studio 2022 VC runtime directory was not found")


def copy_msvc_runtime(directory: pathlib.Path, environment: dict[str, str]) -> None:
    redist_root = pathlib.Path(environment["VCINSTALLDIR"]) / "Redist" / "MSVC"
    candidates = sorted(
        redist_root.glob("*/x64/Microsoft.VC143.CRT"),
        key=lambda path: path.parent.parent.name,
        reverse=True,
    )
    if not candidates:
        raise FileNotFoundError(f"MSVC x64 redistributable DLL directory was not found under {redist_root}")
    runtime_files = list(candidates[0].glob("*.dll"))
    if not runtime_files:
        raise FileNotFoundError(f"No redistributable DLLs were found in {candidates[0]}")
    for runtime in runtime_files:
        shutil.copy2(runtime, directory / runtime.name)


def verify_distribution(directory: pathlib.Path, configuration: str) -> None:
    debug_suffix = "d" if configuration == "Debug" else ""
    required = (
        directory / "adb-studio.exe",
        directory / f"Qt6Core{debug_suffix}.dll",
        directory / "platforms" / f"qwindows{debug_suffix}.dll",
        directory / "version.crty",
        directory / "resources" / "res.crty",
    )
    missing = [str(path.relative_to(directory)) for path in required if not path.is_file()]
    fluent_plugins = list(
        (directory / "qml" / "QtQuick" / "Controls" / "FluentWinUI3").glob(
            "qtquickcontrols2fluentwinui3styleplugin*.dll"
        )
    )
    if not fluent_plugins:
        missing.append("qml/QtQuick/Controls/FluentWinUI3/*styleplugin*.dll")
    if configuration == "Release":
        for runtime in ("vcruntime140.dll", "msvcp140.dll"):
            if not (directory / runtime).is_file():
                missing.append(runtime)
    if missing:
        raise RuntimeError("Incomplete deployment; missing: " + ", ".join(missing))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config", choices=("Debug", "Release"), default="Release", help="Build configuration"
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=ROOT / "dist" / "ADB-Studio",
        help="Self-contained output directory",
    )
    parser.add_argument("--skip-tests", action="store_true", help="Skip CTest for local iteration")
    parser.add_argument("--skip-installer", action="store_true", help="Create only the portable folder")
    arguments = parser.parse_args()

    configuration = arguments.config
    preset = f"windows-msvc-{configuration.lower()}"
    build_directory = ROOT / "build" / preset
    executable = build_directory / configuration / "adb-studio.exe"
    output = arguments.output.resolve()
    distribution_root = (ROOT / "dist").resolve()
    if output != distribution_root and distribution_root not in output.parents:
        raise RuntimeError("--output must stay inside the repository dist directory")

    qt = locate_qt()
    run([sys.executable, "scripts/generate_windows_icon.py", "."])
    run([sys.executable, "scripts/update_resources.py", "."])
    run(["python", "scripts/validate_resources.py", "."])
    run(["python", "scripts/validate_version.py", "."])
    run(["python", "scripts/validate_architecture.py", "."])
    run(["python", "scripts/validate_qml_policy.py", "."])
    run(["cmake", "--preset", preset])
    run(["cmake", "--build", "--preset", preset, "--parallel"])
    if not arguments.skip_tests:
        run(["ctest", "--test-dir", str(build_directory), "-C", configuration, "--output-on-failure"])
    if not executable.is_file():
        raise FileNotFoundError(f"Build succeeded but executable was not found: {executable}")

    staging = distribution_root / ".ADB-Studio.staging"
    safe_remove_tree(staging, distribution_root)
    staging.mkdir(parents=True)
    shutil.copy2(executable, staging / "adb-studio.exe")
    pdb = executable.with_suffix(".pdb")
    if configuration == "Debug" and pdb.is_file():
        shutil.copy2(pdb, staging / pdb.name)

    deploy = qt / "bin" / "windeployqt.exe"
    deploy_mode = "--debug" if configuration == "Debug" else "--release"
    deploy_environment = deployment_environment()
    run(
        [
            str(deploy),
            deploy_mode,
            "--compiler-runtime",
            "--no-system-dxc-compiler",
            "--verbose",
            "0",
            "--qmldir",
            str(ROOT / "apps" / "studio" / "qml"),
            "--dir",
            str(staging),
            str(staging / "adb-studio.exe"),
        ],
        environment=deploy_environment,
    )
    if configuration == "Release":
        copy_msvc_runtime(staging, deploy_environment)
    shutil.copy2(ROOT / "version.crty", staging / "version.crty")
    (staging / "resources").mkdir()
    shutil.copy2(ROOT / "resources" / "res.crty", staging / "resources" / "res.crty")
    shutil.copy2(ROOT / "THIRD_PARTY_LICENSES.md", staging / "THIRD_PARTY_LICENSES.md")
    verify_distribution(staging, configuration)

    safe_remove_tree(output, distribution_root)
    staging.replace(output)
    installer = None
    if configuration == "Release" and not arguments.skip_installer:
        manifest = json.loads((ROOT / "version.crty").read_text(encoding="utf-8"))
        installer_directory = distribution_root / "installer"
        installer_directory.mkdir(parents=True, exist_ok=True)
        iscc = locate_inno_setup()
        run(
            [
                str(iscc),
                f"/DMyAppVersion={manifest['version']}",
                f"/DMyAppBuild={manifest['build']}",
                f"/DSourceDir={output}",
                f"/DOutputDir={installer_directory}",
                f"/DSetupIcon={ROOT / 'assets' / 'app-icon-adb.ico'}",
                str(ROOT / "setup.iss"),
            ]
        )
        installer = installer_directory / f"ADB-Studio-Setup-{manifest['version']}-x64.exe"
        if not installer.is_file():
            raise FileNotFoundError(f"Inno Setup did not create the expected installer: {installer}")
    print(f"\nADB Studio is ready: {output / 'adb-studio.exe'}")
    print("Qt DLLs and QML plugins are deployed next to the executable.")
    if installer is not None:
        print(f"Installer is ready: {installer}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"Build failed: {error}", file=sys.stderr)
        raise SystemExit(1) from error
