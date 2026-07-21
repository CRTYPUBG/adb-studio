<div align="center">

<img src="https://github.com/user-attachments/assets/2f38a5f8-43a8-4269-9dd4-f218a8491b38" width="100%">

# ADB Studio

### Modern Android Device Management Platform

Windows-first • C++20 • Qt 6.8 • Fluent Design • Screen Mirroring • Developer Tools

<p>

<img src="https://img.shields.io/badge/Windows-11-0078D4?style=for-the-badge&logo=windows11&logoColor=white">

<img src="https://img.shields.io/badge/C++20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white">

<img src="https://img.shields.io/badge/Qt-6.8-41CD52?style=for-the-badge&logo=qt&logoColor=white">

<img src="https://img.shields.io/github/license/CRTYPUBG/adb-studio?style=for-the-badge">

<img src="https://img.shields.io/github/actions/workflow/status/CRTYPUBG/adb-studio/build.yml?style=for-the-badge">

<img src="https://img.shields.io/github/v/release/CRTYPUBG/adb-studio?style=for-the-badge">

</p>

</div>

---

# Overview

ADB Studio is a modern, high-performance Android device management platform built with **C++20**, **Qt 6.8**, **Qt Quick**, **QML**, and a **Windows 11 Fluent Design** interface.

The project is designed for:

- Android Developers
- QA Engineers
- Power Users
- Mobile Gamers
- Device Repair Technicians
- Reverse Engineers
- Software Engineers

ADB Studio combines Android device management, screen mirroring, file transfer, diagnostics, developer tools, and performance analysis into a single native desktop application.

---

# Highlights

- Modern Fluent Design UI
- Screen Mirroring
- Device Dashboard
- Wireless ADB
- USB Device Manager
- APK Manager
- File Explorer
- Logcat Viewer
- Fastboot Tools
- ADB Shell
- Performance Monitor
- Plugin System
- AI Assistant (Planned)

---

# Current Development Status

| Module | Status |
|----------|:------:|
| Repository Foundation | ✅ |
| Build System | ✅ |
| Fluent UI Shell | ✅ |
| Theme System | ✅ |
| Resource System | ✅ |
| Build Pipeline | ✅ |
| Device Discovery | 🚧 |
| Screen Mirroring | 🚧 |
| File Manager | 🚧 |
| APK Manager | 🚧 |
| Wireless ADB | 🚧 |
| Plugin SDK | 📅 |
| AI Assistant | 📅 |

---

# Screenshots

## Dashboard

<p align="center">

<img src="docs/screenshots/foundation-shell.png" width="90%">

</p>

More screenshots will be added as development progresses.

---

# Build

## Requirements

- Windows 11
- Visual Studio 2022
- MSVC x64
- Qt 6.8.1
- CMake 3.25+
- Python 3
- Android Platform Tools

---

## Automatic Build

ADB Studio includes a complete build automation system.

Simply run:

```powershell
python Build.py
```

The build pipeline automatically performs:

- Resource Validation
- Version Validation
- CMake Configure
- CMake Build
- Unit Tests
- Qt Deployment
- DLL Verification
- Plugin Verification
- Release Packaging
- Installer Generation
- Digital Signing (optional)
- SHA256 Generation

---

## Manual Build

```powershell
cmake --preset windows-msvc-debug

cmake --build --preset windows-msvc-debug

ctest --preset windows-msvc-debug -C Debug
```

---

## Resource Validation

```powershell
python scripts/update_resources.py

python scripts/validate_resources.py

python scripts/validate_qml_policy.py
```

---

# Repository Structure

```
apps/
core/
engine/
plugins/
sdk/
resources/
themes/
translations/
tests/
benchmarks/
tools/
scripts/
docs/
cmake/
.github/
```

---

# Technology Stack

| Category | Technology |
|------------|------------|
| Language | C++20 |
| UI | Qt Quick |
| Framework | Qt 6.8 |
| Build | CMake |
| IDE | Visual Studio 2022 |
| Installer | Inno Setup |
| Package | MSIX |
| CI/CD | GitHub Actions |
| Testing | GoogleTest |
| Documentation | Markdown |

---

# Roadmap

- Repository Foundation
- Fluent UI
- Device Dashboard
- Screen Mirroring
- File Manager
- APK Manager
- Wireless ADB
- Fastboot
- Plugin SDK
- AI Assistant
- Stable Release v1.0

---

# Documentation

Documentation is available in:

- Docs
- Wiki
- Architecture
- Contributing
- Governance

---

# Contributing

Contributions are welcome.

Before submitting a Pull Request please read:

- CONTRIBUTING.md
- SECURITY.md
- CODE_OF_CONDUCT.md
- GOVERNANCE.md

---

# License

Apache License 2.0

---

<div align="center">

Built with ❤️ by **CRTYPUBG**

ADB Studio © 2026

</div>
