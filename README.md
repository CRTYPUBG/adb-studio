# ADB Studio

ADB Studio is a Windows-first, cross-platform-ready Android device management application built
with C++20, Qt 6.8, Qt Quick, QML and the FluentWinUI3 Qt Quick Controls style.

The current `0.1.0-dev` milestone provides the governed repository foundation, FluentWinUI3
application shell, centralized theme service, typed dashboard ViewModel, resource validation and
unit-test infrastructure. ADB discovery and device transport are the next milestone.

## Build

Prerequisites are Visual Studio 2022, CMake 3.25+, Python 3 and Qt 6.8.1 MSVC x64.

For a complete portable folder with every required DLL beside the executable, run:

```powershell
python Build.py
```

The release output is `dist/ADB-Studio/adb-studio.exe`. `Build.py` runs validation, CMake, tests and
`windeployqt`, then verifies the Qt runtime, Windows platform plugin and FluentWinUI3 plugin. Use
`python Build.py --config Debug` for a debug deployment.

Release builds also invoke Inno Setup 6 using [`setup.iss`](setup.iss) and create
`dist/installer/ADB-Studio-Setup-<version>-x64.exe`. Set `ISCC` only when Inno Setup is installed in
a non-standard location.

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug -C Debug
```

Update and validate resources after QML changes:

```powershell
python scripts/update_resources.py .
python scripts/validate_resources.py .
python scripts/validate_qml_policy.py .
```

See `CONTRIBUTING.md`, `GOVERNANCE.md`, and `docs/architecture/README.md` before changing code.

## Current UI

![ADB Studio FluentWinUI3 foundation shell](docs/screenshots/foundation-shell.png)
