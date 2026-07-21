#!/usr/bin/env python3
"""Environment-backed configuration for the ADB Studio build orchestrator."""

from __future__ import annotations

from dataclasses import dataclass
import json
import os
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent


def _env(name: str, default: str) -> str:
    value = os.environ.get(name)
    return value.strip() if value and value.strip() else default


@dataclass(frozen=True)
class BuildConfig:
    root: Path
    app_name: str
    company: str
    version: str
    build_number: int
    build_type: str
    target: str
    qt_version: str
    qt_dir: Path
    cmake_generator: str
    output_directory: Path
    installer_script: Path
    sign_cert_path: Path | None
    sign_cert_password: str | None
    sign_cert_thumbprint: str | None
    sign_timestamp_url: str
    sign_hash_algorithm: str
    msix_publisher: str
    github_repository: str

    @property
    def preset(self) -> str:
        return f"windows-msvc-{self.build_type.lower()}"

    @property
    def build_directory(self) -> Path:
        return self.root / "build" / self.preset

    @property
    def executable(self) -> Path:
        return self.build_directory / self.build_type / "adb-studio.exe"

    @property
    def dist(self) -> Path:
        return self.output_directory.parent

    @property
    def full_version(self) -> str:
        return f"{self.version}.{self.build_number}"

    @property
    def artifact_stem(self) -> str:
        return f"ADB-Studio-{self.version}-{self.target}"


def load_config(build_type: str | None = None) -> BuildConfig:
    manifest = json.loads((ROOT / "version.crty").read_text(encoding="utf-8"))
    configuration = build_type or _env("BUILD_TYPE", "Release")
    if configuration not in {"Debug", "Release"}:
        raise ValueError("BUILD_TYPE must be Debug or Release")
    output = Path(_env("OUTPUT_DIRECTORY", str(ROOT / "dist" / "ADB-Studio")))
    if not output.is_absolute():
        output = ROOT / output
    certificate = os.environ.get("SIGN_CERT_PATH")
    configured_version = _env("VERSION", manifest["version"])
    configured_build = int(_env("BUILD_NUMBER", str(manifest["build"])))
    if configured_version != manifest["version"] or configured_build != manifest["build"]:
        raise ValueError(
            "VERSION and BUILD_NUMBER must match version.crty; update the sole version source first"
        )
    company = _env("COMPANY", manifest["company"])
    publisher = _env("MSIX_PUBLISHER", "CN=CRTY")
    if not re.fullmatch(r"[A-Za-z0-9 .,&'()-]{1,64}", company):
        raise ValueError("COMPANY contains unsupported packaging characters")
    if not re.fullmatch(r"CN=[A-Za-z0-9 .,&'()-]{1,128}", publisher):
        raise ValueError("MSIX_PUBLISHER must be a valid CN= certificate subject")
    return BuildConfig(
        root=ROOT,
        app_name=_env("APP_NAME", manifest["name"]),
        company=company,
        version=configured_version,
        build_number=configured_build,
        build_type=configuration,
        target=_env("TARGET", manifest.get("architecture", "x64")),
        qt_version=_env("QT_VERSION", manifest.get("qt", "6.8.1")),
        qt_dir=Path(_env("QTDIR", "C:/Qt/6.8.1/msvc2022_64")),
        cmake_generator=_env("CMAKE_GENERATOR", "Visual Studio 17 2022"),
        output_directory=output.resolve(),
        installer_script=ROOT / _env("INSTALLER_SCRIPT", "setup.iss"),
        sign_cert_path=Path(certificate).resolve() if certificate else None,
        sign_cert_password=os.environ.get("SIGN_CERT_PASSWORD"),
        sign_cert_thumbprint=os.environ.get("SIGN_CERT_THUMBPRINT"),
        sign_timestamp_url=_env("SIGN_TIMESTAMP_URL", "http://timestamp.digicert.com"),
        sign_hash_algorithm=_env("SIGN_HASH_ALGORITHM", "SHA256").upper(),
        msix_publisher=publisher,
        github_repository=_env("GITHUB_REPOSITORY", "CRTYPUBG/adb-studio"),
    )
