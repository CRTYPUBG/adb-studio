#!/usr/bin/env python3
"""Single entry point for ADB Studio build, test, package and release operations."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess
import sys
import time
import zipfile
import xml.etree.ElementTree as ElementTree
from urllib.parse import urlparse
from xml.sax.saxutils import escape

from build_config import BuildConfig, ROOT, load_config


class BuildError(RuntimeError):
    """An actionable build or release gate failure."""


def run(command: list[str | Path], *, env: dict[str, str] | None = None,
        capture: bool = False) -> subprocess.CompletedProcess[str]:
    argv = [str(item) for item in command]
    display = argv.copy()
    for secret_name in ("SIGN_CERT_PASSWORD", "GH_TOKEN", "GITHUB_TOKEN"):
        secret = os.environ.get(secret_name)
        if secret:
            display = [item.replace(secret, "***") for item in display]
    print("+", subprocess.list2cmdline(display), flush=True)
    return subprocess.run(argv, cwd=ROOT, env=env, check=True, text=True,
                          capture_output=capture)


def find_tool(name: str, candidates: list[Path] | None = None) -> Path:
    for candidate in candidates or []:
        if candidate.is_file():
            return candidate
    located = shutil.which(name)
    if located:
        return Path(located)
    raise BuildError(f"Required tool was not found: {name}")


def windows_sdk_tool(name: str) -> Path:
    configured = os.environ.get(name.upper().replace(".", "_"))
    candidates = [Path(configured)] if configured else []
    for kits in (Path("C:/Program Files (x86)/Windows Kits/10/bin"),
                 Path("C:/Program Files/Windows Kits/10/bin")):
        if kits.is_dir():
            candidates.extend(sorted(kits.glob(f"*/x64/{name}"), reverse=True))
    return find_tool(name, candidates)


def safe_remove_tree(path: Path, allowed_parent: Path) -> None:
    resolved, parent = path.resolve(), allowed_parent.resolve()
    if resolved == parent or parent not in resolved.parents:
        raise BuildError(f"Refusing unsafe recursive removal: {resolved}")
    if resolved.exists():
        shutil.rmtree(resolved)


def validate_output(config: BuildConfig) -> None:
    dist = (ROOT / "dist").resolve()
    if config.output_directory != dist and dist not in config.output_directory.parents:
        raise BuildError("OUTPUT_DIRECTORY must remain inside the repository dist directory")


def validate_repository() -> None:
    sync_readme_badge()
    run([sys.executable, "scripts/generate_windows_icon.py", "."])
    run([sys.executable, "scripts/update_resources.py", "."])
    for validator in ("validate_resources.py", "validate_version.py",
                      "validate_architecture.py", "validate_qml_policy.py",
                      "validate_diagnostics_docs.py", "validate_no_placeholders.py",
                      "validate_translations.py", "validate_ui_connections.py"):
        run([sys.executable, f"scripts/{validator}", "."])


def configure(config: BuildConfig) -> None:
    validate_repository()
    run(["cmake", "--preset", config.preset, f"-DCMAKE_PREFIX_PATH={config.qt_dir}",
         f"-DADB_STUDIO_COMPANY={config.company}"])


def build(config: BuildConfig) -> None:
    configure(config)
    run(["cmake", "--build", "--preset", config.preset, "--parallel"])
    run(["cmake", "--build", config.build_directory, "--config", config.build_type,
         "--target", "adb_studio_core_qmltyperegistration"])
    run(["cmake", "--build", "--preset", config.preset, "--parallel"])
    if not config.executable.is_file():
        raise BuildError(f"Expected executable was not produced: {config.executable}")


def clean(config: BuildConfig) -> None:
    safe_remove_tree(ROOT / "build", ROOT)
    if config.dist.exists():
        safe_remove_tree(config.dist, ROOT)


def test(config: BuildConfig, ensure_build: bool = True) -> None:
    if ensure_build:
        build(config)
    run(["ctest", "--test-dir", config.build_directory, "-C", config.build_type,
         "--output-on-failure"])
    run([sys.executable, "-m", "unittest", "discover", "-s", "tests/python",
         "-p", "test_*.py"])


def source_files() -> list[Path]:
    result: list[Path] = []
    for directory in (ROOT / "apps", ROOT / "core", ROOT / "tests"):
        for suffix in ("*.cpp", "*.hpp"):
            result.extend(directory.rglob(suffix))
    return sorted(result)


def lint(config: BuildConfig) -> None:
    configure(config)
    run(["cmake", "--build", config.build_directory, "--config", config.build_type,
         "--target", "adb_studio_core_qmltyperegistration"])
    files = source_files()
    run([find_tool("clang-format"), "--dry-run", "--Werror", *files])
    qml = sorted((ROOT / "apps" / "studio" / "qml").rglob("*.qml"))
    qmllint = find_tool("qmllint.exe", [config.qt_dir / "bin" / "qmllint.exe"])
    run([qmllint, "-I", config.build_directory / "core", *qml])
    cppcheck = find_tool("cppcheck")
    run([cppcheck, "--enable=warning,style,performance,portability", "--error-exitcode=1",
         "--std=c++20", "--inline-suppr", "--suppress=missingIncludeSystem",
         f"-I{ROOT / 'core/diagnostics/include'}", f"-I{ROOT / 'core/presentation/include'}",
         f"-I{ROOT / 'core/services/include'}",
         f"-I{config.qt_dir / 'include'}",
         ROOT / "apps", ROOT / "core"])
    llvm_roots = [Path("C:/Program Files/Microsoft Visual Studio/2022") / edition /
                  "VC/Tools/Llvm/x64/bin/clang-tidy.exe"
                  for edition in ("Community", "BuildTools", "Professional", "Enterprise")]
    clang_tidy = find_tool("clang-tidy", llvm_roots)
    analysis_includes = [ROOT / "core/diagnostics/include", ROOT / "core/presentation/include",
                         ROOT / "core/services/include",
                         config.build_directory / "generated", config.qt_dir / "include"]
    analysis_includes.extend(config.qt_dir / "include" / component for component in
                             ("QtCore", "QtGui", "QtQml", "QtQuick", "QtQuickControls2", "QtTest"))
    analysis_includes.append(config.qt_dir / "include" / "QtConcurrent")
    analysis_args = ["-std=c++20", "-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH"]
    for path in (*analysis_includes, config.qt_dir / "mkspecs/win32-msvc"):
        analysis_args.extend(("-I", str(path)))
    analysis_files = [item for item in files
                      if item.suffix == ".cpp" and (ROOT / "tests") not in item.parents]
    for path in analysis_files:
        run([clang_tidy, path, "--", *analysis_args])
    iwyu = find_tool("include-what-you-use", [Path(p) for p in (
        os.environ.get("IWYU", ""),) if p])
    for path in analysis_files:
        run([iwyu, *analysis_args, path])


def deployment_environment() -> dict[str, str]:
    environment = os.environ.copy()
    if environment.get("VCINSTALLDIR"):
        return environment
    root = Path("C:/Program Files/Microsoft Visual Studio/2022")
    for edition in ("Community", "BuildTools", "Professional", "Enterprise"):
        candidate = root / edition / "VC"
        if candidate.is_dir():
            environment["VCINSTALLDIR"] = str(candidate)
            return environment
    raise BuildError("Visual Studio 2022 VC runtime directory was not found")


def copy_msvc_runtime(destination: Path, environment: dict[str, str]) -> None:
    root = Path(environment["VCINSTALLDIR"]) / "Redist" / "MSVC"
    candidates = sorted(root.glob("*/x64/Microsoft.VC143.CRT"), reverse=True)
    if not candidates:
        raise BuildError(f"MSVC x64 redistributable runtime not found under {root}")
    for runtime in candidates[0].glob("*.dll"):
        shutil.copy2(runtime, destination / runtime.name)


def stage_portable(config: BuildConfig, ensure_build: bool = True) -> None:
    validate_output(config)
    if ensure_build:
        build(config)
        test(config, ensure_build=False)
    qt_deploy = find_tool("windeployqt.exe", [config.qt_dir / "bin" / "windeployqt.exe"])
    staging = config.dist / ".ADB-Studio.staging"
    safe_remove_tree(staging, config.dist)
    staging.mkdir(parents=True)
    shutil.copy2(config.executable, staging / "adb-studio.exe")
    deploy_env = deployment_environment()
    run([qt_deploy, "--debug" if config.build_type == "Debug" else "--release",
         "--compiler-runtime", "--no-system-dxc-compiler", "--verbose", "0",
         "--qmldir", ROOT / "apps/studio/qml", "--dir", staging,
         staging / "adb-studio.exe"], env=deploy_env)
    if config.build_type == "Release":
        copy_msvc_runtime(staging, deploy_env)
    scrcpy_target = staging / "tools" / "scrcpy"
    shutil.copytree(ROOT / "scrcpy", scrcpy_target)
    shutil.copy2(ROOT / "scrcpy-master" / "LICENSE", scrcpy_target / "LICENSE")
    shutil.copy2(ROOT / "version.crty", staging / "version.crty")
    (staging / "resources").mkdir()
    shutil.copy2(ROOT / "resources/res.crty", staging / "resources/res.crty")
    shutil.copy2(ROOT / "THIRD_PARTY_LICENSES.md", staging / "THIRD_PARTY_LICENSES.md")
    verify_portable(staging, config)
    safe_remove_tree(config.output_directory, config.dist)
    staging.replace(config.output_directory)


def verify_portable(directory: Path, config: BuildConfig) -> None:
    suffix = "d" if config.build_type == "Debug" else ""
    required = [directory / "adb-studio.exe", directory / f"Qt6Core{suffix}.dll",
                directory / "platforms" / f"qwindows{suffix}.dll",
                directory / "version.crty", directory / "resources/res.crty",
                directory / "tools/scrcpy/scrcpy.exe",
                directory / "tools/scrcpy/scrcpy-server",
                directory / "tools/scrcpy/adb.exe"]
    fluent = list((directory / "qml/QtQuick/Controls/FluentWinUI3").glob(
        "qtquickcontrols2fluentwinui3styleplugin*.dll"))
    missing = [str(path.relative_to(directory)) for path in required if not path.is_file()]
    if not fluent:
        missing.append("qml/QtQuick/Controls/FluentWinUI3/*styleplugin*.dll")
    if missing:
        raise BuildError("Portable deployment is incomplete: " + ", ".join(missing))


def write_msix_assets(config: BuildConfig, destination: Path) -> None:
    try:
        from PIL import Image
    except ImportError as error:
        raise BuildError("Pillow is required to generate MSIX assets") from error
    source = Image.open(ROOT / "assets/app-icon-adb.png").convert("RGBA")
    assets = destination / "Assets"
    assets.mkdir(parents=True)
    for name, size in (("StoreLogo.png", 50), ("Square44x44Logo.png", 44),
                       ("Square150x150Logo.png", 150)):
        source.resize((size, size), Image.Resampling.LANCZOS).save(assets / name)
    app_name = escape(config.app_name, {'"': "&quot;"})
    company = escape(config.company, {'"': "&quot;"})
    publisher = escape(config.msix_publisher, {'"': "&quot;"})
    manifest = f'''<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
 xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
 xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities">
 <Identity Name="CRTY.ADBStudio" Publisher="{publisher}" Version="{config.full_version}" ProcessorArchitecture="x64" />
 <Properties><DisplayName>{app_name}</DisplayName><PublisherDisplayName>{company}</PublisherDisplayName>
  <Logo>Assets\\StoreLogo.png</Logo></Properties>
 <Resources><Resource Language="en-us" /><Resource Language="tr-tr" /></Resources>
 <Dependencies><TargetDeviceFamily Name="Windows.Desktop" MinVersion="10.0.17763.0" MaxVersionTested="10.0.26100.0" /></Dependencies>
 <Applications><Application Id="ADBStudio" Executable="adb-studio.exe" EntryPoint="Windows.FullTrustApplication">
  <uap:VisualElements DisplayName="{app_name}" Description="Android device management studio"
   BackgroundColor="transparent" Square150x150Logo="Assets\\Square150x150Logo.png" Square44x44Logo="Assets\\Square44x44Logo.png" />
 </Application></Applications><Capabilities><rescap:Capability Name="runFullTrust" /></Capabilities>
</Package>'''
    (destination / "AppxManifest.xml").write_text(manifest + "\n", encoding="utf-8")


def locate_inno() -> Path:
    configured = os.environ.get("ISCC")
    return find_tool("ISCC.exe", [Path(configured)] if configured else [
        Path("C:/Program Files (x86)/Inno Setup 6/ISCC.exe"),
        Path("C:/Program Files/Inno Setup 6/ISCC.exe")])


def create_zip(source: Path, destination: Path, root_name: str | None = None) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.unlink(missing_ok=True)
    with zipfile.ZipFile(destination, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(source.rglob("*")):
            if path.is_file():
                archive.write(path, Path(root_name or source.name) / path.relative_to(source))
    with zipfile.ZipFile(destination) as archive:
        corrupt = archive.testzip()
    if corrupt is not None:
        raise BuildError(f"ZIP integrity verification failed at {corrupt}")


def create_packages(config: BuildConfig) -> list[Path]:
    artifacts = config.dist / "artifacts"
    artifacts.mkdir(parents=True, exist_ok=True)
    portable_zip = artifacts / f"{config.artifact_stem}-portable.zip"
    create_zip(config.output_directory, portable_zip, "ADB-Studio")
    symbols_dir = config.dist / ".symbols"
    safe_remove_tree(symbols_dir, config.dist)
    symbols_dir.mkdir()
    for pdb in config.build_directory.rglob("*.pdb"):
        relative = pdb.relative_to(config.build_directory)
        destination = symbols_dir / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(pdb, destination)
    if not any(symbols_dir.rglob("*.pdb")):
        raise BuildError("No debug symbols were produced; Release builds must emit PDB files")
    symbols_zip = artifacts / f"{config.artifact_stem}-symbols.zip"
    create_zip(symbols_dir, symbols_zip, "ADB-Studio-symbols")
    safe_remove_tree(symbols_dir, config.dist)

    installer_dir = artifacts
    run([locate_inno(), f"/DMyAppVersion={config.version}",
         f"/DMyAppBuild={config.build_number}", f"/DSourceDir={config.output_directory}",
         f"/DOutputDir={installer_dir}", f"/DSetupIcon={ROOT / 'assets/app-icon-adb.ico'}",
         f"/DMyAppCompany={config.company}",
         config.installer_script])
    installer = installer_dir / f"ADB-Studio-Setup-{config.version}-{config.target}.exe"
    if not installer.is_file():
        raise BuildError(f"Inno Setup did not create {installer}")

    msix_stage = config.dist / ".msix"
    safe_remove_tree(msix_stage, config.dist)
    shutil.copytree(config.output_directory, msix_stage)
    write_msix_assets(config, msix_stage)
    msix = artifacts / f"{config.artifact_stem}.msix"
    msix.unlink(missing_ok=True)
    run([windows_sdk_tool("makeappx.exe"), "pack", "/o", "/d", msix_stage, "/p", msix])
    safe_remove_tree(msix_stage, config.dist)
    return [portable_zip, installer, msix, symbols_zip]


def verify_artifact_versions(config: BuildConfig, artifacts: list[Path]) -> None:
    installer = next((path for path in artifacts if path.name.startswith("ADB-Studio-Setup-")), None)
    msix = next((path for path in artifacts if path.suffix.lower() == ".msix"), None)
    if installer is None or msix is None:
        raise BuildError("Installer or MSIX is missing from version validation")
    expected = config.full_version
    script = (
        "& { param($a,$b) "
        "$x=(Get-Item -LiteralPath $a).VersionInfo; "
        "$y=(Get-Item -LiteralPath $b).VersionInfo; "
        "@($x.FileVersion,$x.ProductVersion,$y.FileVersion,$y.ProductVersion) | "
        "ForEach-Object { $_.Trim() } }"
    )
    result = run(["powershell", "-NoProfile", "-Command", script,
                  config.output_directory / "adb-studio.exe", installer], capture=True)
    versions = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    if len(versions) != 4 or any(version != expected for version in versions):
        raise BuildError(f"PE version mismatch: expected {expected}, found {versions}")
    with zipfile.ZipFile(msix) as archive:
        manifest = ElementTree.fromstring(archive.read("AppxManifest.xml"))
    identity = manifest.find("{http://schemas.microsoft.com/appx/manifest/foundation/windows10}Identity")
    msix_version = identity.get("Version") if identity is not None else None
    if msix_version != expected:
        raise BuildError(f"MSIX version mismatch: expected {expected}, found {msix_version}")


def signing_credentials(config: BuildConfig) -> list[str | Path]:
    has_file = config.sign_cert_path is not None
    has_store = bool(config.sign_cert_thumbprint)
    if has_file == has_store:
        raise BuildError(
            "Signing requires exactly one certificate source: SIGN_CERT_PATH with "
            "SIGN_CERT_PASSWORD, or SIGN_CERT_THUMBPRINT from CurrentUser/My"
        )
    if has_file:
        if not config.sign_cert_password:
            raise BuildError("SIGN_CERT_PASSWORD is required with SIGN_CERT_PATH")
        if not config.sign_cert_path or not config.sign_cert_path.is_file():
            raise BuildError(f"Signing certificate does not exist: {config.sign_cert_path}")
        certificate_arguments: list[str | Path] = [
            "/f", config.sign_cert_path, "/p", config.sign_cert_password
        ]
    else:
        thumbprint = re.sub(r"\s+", "", config.sign_cert_thumbprint or "").upper()
        if not re.fullmatch(r"[0-9A-F]{40}", thumbprint):
            raise BuildError("SIGN_CERT_THUMBPRINT must contain exactly 40 hexadecimal characters")
        certificate_arguments = ["/s", "My", "/sha1", thumbprint]
    if config.sign_hash_algorithm not in {"SHA256", "SHA384", "SHA512"}:
        raise BuildError("SIGN_HASH_ALGORITHM must be SHA256, SHA384 or SHA512")
    timestamp = urlparse(config.sign_timestamp_url)
    if timestamp.scheme not in {"http", "https"} or not timestamp.netloc:
        raise BuildError("SIGN_TIMESTAMP_URL must be an HTTP(S) RFC 3161 endpoint")
    return certificate_arguments


def first_party_binaries(config: BuildConfig) -> list[Path]:
    # Qt, MSVC, graphics and other deployed DLLs are third-party and intentionally excluded.
    names = {"adb-studio.exe"}
    manifest = ROOT / "config/first-party-binaries.txt"
    if manifest.is_file():
        names.update(line.strip() for line in manifest.read_text(encoding="utf-8").splitlines()
                     if line.strip() and not line.lstrip().startswith("#"))
    found = [path for path in config.output_directory.rglob("*")
             if path.is_file() and path.name in names and path.suffix.lower() in {".exe", ".dll"}]
    if not found:
        raise BuildError("No first-party binaries were found to sign")
    return sorted(found)


def sign_files(config: BuildConfig, files: list[Path]) -> None:
    certificate_arguments = signing_credentials(config)
    signtool = windows_sdk_tool("signtool.exe")
    for path in files:
        run([signtool, "sign", "/fd", config.sign_hash_algorithm, "/td",
             config.sign_hash_algorithm, "/tr", config.sign_timestamp_url,
             *certificate_arguments, path])
        run([signtool, "verify", "/pa", "/all", "/v", path])


def generate_sbom(config: BuildConfig) -> list[Path]:
    output = config.dist / "evidence" / "sbom"
    run([sys.executable, "scripts/generate_sbom.py", ROOT, output])
    return [output / "bom.spdx.json", output / "bom.cdx.json"]


def audit_supply_chain(config: BuildConfig) -> Path:
    output = config.dist / "evidence"
    run([sys.executable, "scripts/audit_supply_chain.py", ROOT, output])
    return output / "dependency-license-audit.json"


def git_output(*args: str, required: bool = True) -> str:
    result = run(["git", *args], capture=True)
    value = result.stdout.strip()
    if required and not value:
        raise BuildError(f"git {' '.join(args)} returned no output")
    return value


def commits_since_release() -> list[tuple[str, str]]:
    tags = subprocess.run(["git", "describe", "--tags", "--abbrev=0"], cwd=ROOT,
                          text=True, capture_output=True)
    revision = f"{tags.stdout.strip()}..HEAD" if tags.returncode == 0 else "HEAD"
    log = git_output("log", revision, "--pretty=format:%h%x09%s", required=False)
    return [tuple(line.split("\t", 1)) for line in log.splitlines() if "\t" in line]


def changed_paths_since_release() -> list[str]:
    tags = subprocess.run(["git", "describe", "--tags", "--abbrev=0"], cwd=ROOT,
                          text=True, capture_output=True)
    if tags.returncode == 0:
        output = git_output("diff", "--name-only", f"{tags.stdout.strip()}..HEAD", required=False)
    else:
        output = git_output("ls-files", required=False)
    return sorted(line for line in output.splitlines() if line)


def release_notes(config: BuildConfig) -> str:
    groups = {"Breaking Changes": [], "Features": [], "Fixes and Closed Bugs": [],
              "Merged Pull Requests": [], "Other Changes": []}
    for commit, subject in commits_since_release():
        lowered = subject.lower()
        pull_request = re.search(r"(?:merge pull request |\()#(\d+)", lowered)
        if "breaking change" in lowered or re.match(r"\w+!:", lowered):
            group = "Breaking Changes"
        elif lowered.startswith(("feat:", "feat(")):
            group = "Features"
        elif lowered.startswith(("fix:", "fix(")):
            group = "Fixes and Closed Bugs"
        elif pull_request:
            group = "Merged Pull Requests"
        else:
            group = "Other Changes"
        entry = f"- {subject} (`{commit}`)"
        groups[group].append(entry)
        if pull_request and group != "Merged Pull Requests":
            groups["Merged Pull Requests"].append(entry)
    lines = [f"# {config.app_name} {config.version}", ""]
    for heading, entries in groups.items():
        if entries:
            lines.extend([f"## {heading}", "", *entries, ""])
    if len(lines) == 2:
        lines.extend(["No source changes since the previous release.", ""])
    return "\n".join(lines)


def update_changelog(config: BuildConfig, notes: str) -> None:
    path = ROOT / "CHANGELOG.md"
    existing = path.read_text(encoding="utf-8")
    heading = f"## {config.version} -"
    if heading in existing:
        return
    body = notes.replace(f"# {config.app_name} {config.version}\n\n", "")
    date = datetime.now(timezone.utc).date().isoformat()
    content = f"# Changelog\n\n## {config.version} - {date}\n\n{body}"
    if existing.startswith("# Changelog"):
        existing = existing[len("# Changelog"):].lstrip("\r\n")
    path.write_text(content.rstrip() + "\n\n" + existing, encoding="utf-8")


def benchmark(config: BuildConfig, ensure_build: bool = True) -> Path:
    if ensure_build:
        build(config)
    output = config.dist / "evidence"
    output.mkdir(parents=True, exist_ok=True)
    screenshot = output / "benchmark-shell.png"
    environment = os.environ.copy()
    environment.setdefault("QT_QUICK_BACKEND", "software")
    startup_report = output / "startup-ms.txt"
    startup_report.unlink(missing_ok=True)
    environment["ADB_STUDIO_STARTUP_REPORT"] = str(startup_report)
    executable = config.executable if ensure_build else config.output_directory / "adb-studio.exe"
    if executable == config.executable:
        environment["PATH"] = str(config.qt_dir / "bin") + os.pathsep + environment.get("PATH", "")
    if not executable.is_file():
        raise BuildError(f"Benchmark executable does not exist: {executable}")
    startup_samples: list[int] = []
    wall_samples: list[float] = []
    for _ in range(3):
        startup_report.unlink(missing_ok=True)
        started = time.perf_counter()
        result = subprocess.run([executable, "--report-startup", "--render-screenshot", screenshot],
                                cwd=executable.parent, env=environment, text=True,
                                capture_output=True, timeout=30)
        wall_samples.append(round((time.perf_counter() - started) * 1000, 2))
        if result.returncode != 0:
            raise BuildError(f"Startup benchmark failed: {result.stderr.strip()}")
        match = re.search(r"ADB_STUDIO_STARTUP_MS=(\d+)", result.stderr + result.stdout)
        sample = int(startup_report.read_text(encoding="utf-8").strip()) \
            if startup_report.is_file() else (int(match.group(1)) if match else None)
        if sample is None:
            raise BuildError("Startup benchmark produced no metric")
        startup_samples.append(sample)
    startup_report.unlink(missing_ok=True)
    startup_ms = int(statistics.median(startup_samples))
    wall_ms = round(statistics.median(wall_samples), 2)
    report = {"schemaVersion": 1, "version": config.version,
              "generatedAt": datetime.now(timezone.utc).isoformat(),
              "startupQmlMs": startup_ms, "smokeWallMs": wall_ms,
              "startupSamplesMs": startup_samples, "smokeWallSamplesMs": wall_samples,
              "executableBytes": executable.stat().st_size,
              "budget": {"startupQmlMs": 2000},
              "passed": startup_ms is not None and startup_ms < 2000}
    baseline_path = ROOT / "benchmarks/baseline.json"
    if baseline_path.is_file():
        baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
        approved = baseline.get("startupQmlMs")
        tolerance = float(baseline.get("allowedRegressionPercent", 10))
        if approved is not None and startup_ms is not None:
            limit = approved * (1 + tolerance / 100)
            report["previousApproved"] = baseline
            report["regressionLimitMs"] = round(limit, 2)
            report["passed"] = report["passed"] and startup_ms <= limit
    path = output / "benchmark-report.json"
    path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if not report["passed"]:
        raise BuildError("Startup benchmark exceeded its budget/regression limit or produced no metric")
    return path


def copy_evidence(config: BuildConfig) -> list[Path]:
    evidence = config.dist / "evidence"
    evidence.mkdir(parents=True, exist_ok=True)
    result = []
    for name in ("Audit.md", "Validation.md", "Review.md", "Handoff.md", "Decision.md",
                 "THIRD_PARTY_LICENSES.md", "SECURITY_REVIEW.md"):
        target = evidence / name
        shutil.copy2(ROOT / name, target)
        result.append(target)
    return result


def create_license_bundle(config: BuildConfig) -> Path:
    destination = config.dist / "artifacts" / f"{config.artifact_stem}-licenses.zip"
    destination.unlink(missing_ok=True)
    with zipfile.ZipFile(destination, "w", zipfile.ZIP_DEFLATED) as archive:
        for name in ("LICENSE", "THIRD_PARTY_LICENSES.md", "DEPENDENCY_REPORT.md", "SBOM.md"):
            archive.write(ROOT / name, name)
    return destination


def finalize_artifacts(config: BuildConfig, artifacts: list[Path], *, signed: bool) -> list[Path]:
    artifacts = sorted({path.resolve() for path in artifacts if path.is_file()})
    manifest = {"schemaVersion": 1, "application": config.app_name,
                "version": config.version, "build": config.build_number,
                "commit": git_output("rev-parse", "HEAD"),
                "dirty": bool(git_output("status", "--porcelain", required=False)),
                "changedPathsSincePreviousRelease": changed_paths_since_release(),
                "generatedAt": datetime.now(timezone.utc).isoformat(), "artifacts": []}
    for path in artifacts:
        signable = path.suffix.lower() in {".exe", ".msix"}
        manifest["artifacts"].append({"path": path.relative_to(config.dist).as_posix(),
                                      "size": path.stat().st_size,
                                      "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                                      "signature": ("verified" if signed else "unsigned")
                                      if signable else "not-applicable"})
    artifact_manifest = config.dist / "artifacts" / "artifact-manifest.json"
    artifact_manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    all_files = artifacts + [artifact_manifest]
    checksums = config.dist / "artifacts" / "SHA256SUMS.txt"
    checksums.write_text("".join(
        f"{hashlib.sha256(path.read_bytes()).hexdigest()}  "
        f"{path.relative_to(config.dist).as_posix()}\n" for path in all_files),
        encoding="utf-8")
    return all_files + [checksums]


def generate_provenance(config: BuildConfig, *, signed: bool) -> Path:
    path = config.dist / "evidence/provenance.json"
    statement = {
        "_type": "https://in-toto.io/Statement/v1",
        "subject": [{"name": "version.crty",
                     "digest": {"sha256": hashlib.sha256((ROOT / "version.crty").read_bytes()).hexdigest()}},
                    {"name": "resources/res.crty",
                     "digest": {"sha256": hashlib.sha256((ROOT / "resources/res.crty").read_bytes()).hexdigest()}}],
        "predicateType": "https://slsa.dev/provenance/v1",
        "predicate": {"buildDefinition": {"buildType": "https://adb-studio.dev/build.py/v1",
                                            "externalParameters": {"configuration": config.build_type,
                                                                   "target": config.target}},
                      "runDetails": {"builder": {"id": "build.py"},
                                     "metadata": {"invocationId": os.environ.get("GITHUB_RUN_ID", "local"),
                                                  "signaturesRequired": signed}}},
    }
    path.write_text(json.dumps(statement, indent=2) + "\n", encoding="utf-8")
    return path


def approve_benchmark_baseline(config: BuildConfig) -> None:
    report_path = config.dist / "evidence/benchmark-report.json"
    report = json.loads(report_path.read_text(encoding="utf-8"))
    if not report.get("passed") or report.get("startupQmlMs") is None:
        raise BuildError("Only a passing measured benchmark can become the release baseline")
    baseline = {"schemaVersion": 1, "approvedVersion": config.version,
                "startupQmlMs": report["startupQmlMs"], "allowedRegressionPercent": 10,
                "approvedCommit": git_output("rev-parse", "HEAD")}
    path = ROOT / "benchmarks/baseline.json"
    path.write_text(json.dumps(baseline, indent=2) + "\n", encoding="utf-8")


def package(config: BuildConfig, *, ensure_build: bool = True, signed: bool = False) -> list[Path]:
    if signed:
        signing_credentials(config)
    stage_portable(config, ensure_build=ensure_build)
    for generated in (config.dist / "artifacts", config.dist / "evidence", config.dist / "installer"):
        safe_remove_tree(generated, config.dist)
    if signed:
        sign_files(config, first_party_binaries(config))
    artifacts = create_packages(config)
    verify_artifact_versions(config, artifacts)
    if signed:
        sign_files(config, [path for path in artifacts if path.suffix.lower() in {".exe", ".msix"}])
    notes_path = config.dist / "artifacts" / "RELEASE_NOTES.md"
    notes_path.write_text(release_notes(config), encoding="utf-8")
    artifacts.extend([notes_path, create_license_bundle(config), audit_supply_chain(config),
                      *generate_sbom(config), *copy_evidence(config)])
    artifacts.append(generate_provenance(config, signed=signed))
    benchmark_path = benchmark(config, ensure_build=False)
    artifacts.extend([benchmark_path, benchmark_path.with_name("benchmark-shell.png")])
    return finalize_artifacts(config, artifacts, signed=signed)


def release(config: BuildConfig) -> list[Path]:
    if config.build_type != "Release":
        raise BuildError("release requires --config Release")
    signing_credentials(config)
    if git_output("status", "--porcelain", required=False):
        raise BuildError("release requires a clean working tree")
    validate_repository()
    lint(config)
    test(config)
    notes = release_notes(config)
    if os.environ.get("GITHUB_REF_TYPE") == "tag":
        changelog = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8")
        if f"## {config.version} -" not in changelog:
            raise BuildError("Tagged releases require the generated CHANGELOG entry to be committed")
    update_changelog(config, notes)
    return package(config, ensure_build=False, signed=True)


def publish(config: BuildConfig) -> None:
    if git_output("status", "--porcelain", required=False):
        raise BuildError("publish requires a clean working tree before release generation")
    artifacts = release(config)
    approve_benchmark_baseline(config)
    gh = find_tool("gh")
    tag = f"v{config.version}"
    run(["git", "add", "CHANGELOG.md", "version.crty", "benchmarks/baseline.json"])
    staged = subprocess.run(["git", "diff", "--cached", "--quiet"], cwd=ROOT)
    if staged.returncode != 0:
        run(["git", "commit", "-m", f"chore(release): {config.version}"])
        run(["git", "push", "origin", "HEAD"])
    existing = subprocess.run(["git", "tag", "--list", tag], cwd=ROOT,
                              text=True, capture_output=True, check=True).stdout.strip()
    if not existing:
        run(["git", "tag", "-a", tag, "-m", f"{config.app_name} {config.version}"])
        run(["git", "push", "origin", tag])
    notes = config.dist / "artifacts/RELEASE_NOTES.md"
    run([gh, "release", "create", tag, "--repo", config.github_repository,
         "--verify-tag", "--title", f"{config.app_name} {config.version}",
         "--notes-file", notes, *artifacts])


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("configure", "clean", "build", "rebuild", "test",
                        "benchmark", "lint", "package", "sign", "release", "publish", "nightly"))
    parser.add_argument("--config", choices=("Debug", "Release"), default=None)
    parser.add_argument("--version", help="Update the version.crty semantic version before running")
    parser.add_argument("--build-number", type=int,
                        help="Update the positive version.crty build number before running")
    return parser.parse_args()


def update_version_source(version: str | None, build_number: int | None) -> None:
    if version is None and build_number is None:
        return
    if version is not None and not re.fullmatch(r"(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)", version):
        raise BuildError("--version must be canonical MAJOR.MINOR.PATCH")
    if build_number is not None and build_number < 1:
        raise BuildError("--build-number must be positive")
    path = ROOT / "version.crty"
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if version is not None:
        manifest["version"] = version
    if build_number is not None:
        manifest["build"] = build_number
    path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    sync_readme_badge()


def sync_readme_badge() -> None:
    manifest = json.loads((ROOT / "version.crty").read_text(encoding="utf-8"))
    path = ROOT / "README.md"
    content = path.read_text(encoding="utf-8")
    updated, count = re.subn(
        r"(https://img\.shields\.io/badge/version-)[0-9]+\.[0-9]+\.[0-9]+(-2563EB)",
        rf"\g<1>{manifest['version']}\g<2>", content)
    if count != 1:
        raise BuildError("README must contain exactly one managed version badge")
    if updated != content:
        path.write_text(updated, encoding="utf-8")


def main() -> int:
    args = parse_args()
    update_version_source(args.version, args.build_number)
    config = load_config(args.config)
    commands = {
        "configure": lambda: configure(config), "clean": lambda: clean(config),
        "build": lambda: build(config),
        "rebuild": lambda: (clean(config), build(config)),
        "test": lambda: test(config), "benchmark": lambda: benchmark(config),
        "lint": lambda: lint(config), "package": lambda: package(config),
        "sign": lambda: package(config, signed=True),
        "release": lambda: release(config), "publish": lambda: publish(config),
        "nightly": lambda: release(config),
    }
    commands[args.command]()
    print(f"ADB Studio '{args.command}' completed successfully.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (BuildError, OSError, ValueError, subprocess.CalledProcessError,
            subprocess.TimeoutExpired) as error:
        print(f"Build failed: {error}", file=sys.stderr)
        raise SystemExit(1) from error
