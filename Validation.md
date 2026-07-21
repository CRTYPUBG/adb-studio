# Validation

## Milestone 0.1.0 Build 6

- Certificate-store signing probe: passed with explicit `CN=CRTYPUBG` thumbprint.
- SHA-256 Authenticode file digest: verified.
- DigiCert RFC 3161 signed timestamp: verified.
- `signtool verify /pa /all /v`: passed with zero warnings and zero errors.
- PowerShell `Get-AuthenticodeSignature`: `Valid`.
- Build automation tests: 5/5 passed, including store selection and ambiguous-source rejection.
- Portable archive root contract test: passed; EXE and adjacent runtime DLLs extract under the
  stable `ADB-Studio` directory regardless of staging name.

## Enterprise Marketing Banner

- SVG source parsed and rendered successfully.
- PNG dimensions: 7680 × 4320, 16:9.
- PNG pixel format: 24-bit RGB; output size approximately 7.2 MB.
- Visual inspection confirmed title, subtitle, tagline, six feature cards, enterprise info bar,
  monitor dashboard, companion phone, analytics panels and bottom product metadata.
- Original-artwork and prohibited-branding review: passed.

## Milestone 0.1.0 Build 5

Results on Windows 11, MSVC, Qt 6.8.1 and scrcpy 4.0:

- Three consecutive complete quality-gate runs: passed; machine-readable evidence contains exactly
  three successful runs.
- Warnings-as-errors Release build: passed.
- C++ unit/integration tests: 6/6 passed per gate; Python build tests: 3/3 passed per gate.
- Mirror integration repetition: 50/50 passed with a deterministic real child process failure.
- clang-format and qmllint: passed without diagnostics.
- Changed `mirror_service.cpp` clang-tidy: passed with warnings as errors.
- Resource, version, architecture, QML policy, documentation-sync, no-placeholder, translation and
  UI-connection validators: passed.
- English and Turkish mirror-page runtime rendering: passed in every gate; generated screenshots
  were non-empty and emitted no QML warnings.
- Translation catalog: 394/394 Turkish messages finished.
- Bundled runtime discovery: `scrcpy 4.0`, `adb.exe` and `scrcpy-server` verified.
- Physical-device USB/wireless video, audio, recording, codecs, pause/resume, multiple-device,
  stress and long-duration certification: not run because no authorized device was attached.
- Cppcheck, include-what-you-use, CodeQL and sanitizers remain mandatory CI gates; Cppcheck/IWYU are
  unavailable in this local environment.

## Milestone 0.1.0

Validation commands:

```powershell
python scripts/update_resources.py .
python scripts/validate_resources.py .
python scripts/validate_qml_policy.py .
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug -C Debug
```

Results on Windows 11, MSVC 19.44 and Qt 6.8.1:

- Clean configure: passed.
- Warnings-as-errors Debug build: passed without compiler or linker warnings.
- C++ unit/integration tests: 5/5 passed, including deterministic diagnostics, workspace navigation
  and command availability, plus the real system probe integration contract.
- Build-automation unit tests: 3/3 passed.
- clang-format dry run: passed.
- qmllint: passed without diagnostics.
- Resource, version, QML policy and architecture validators: passed.
- Runtime QML/FluentWinUI3 screenshot smoke test: exit 0, empty stderr.
- Median time to first QML root object: 562 ms across three final local reference runs; the 2,000 ms
  absolute budget passed.
- SPDX 2.3 and CycloneDX 1.6 SBOM generation: passed.
- Executable and installer Windows FileVersion/ProductVersion: `0.1.0.4`, derived from
  `version.crty`.
- `build.py package` Release packaging: passed; portable output contains Qt, FluentWinUI3, Windows platform
  and MSVC runtime dependencies.
- Portable-folder runtime smoke test: exit 0 with empty stderr and no Qt development PATH required.
- Multi-resolution ICO generation and resource hash validation: passed.
- PE executable and installer associated-icon extraction: passed.
- Inno Setup 6.6.1 compilation: passed.
- Windows SDK MakeAppx MSIX compilation: passed.
- Portable ZIP, symbol ZIP, license bundle, SBOM and artifact-manifest generation: passed.
- `build.py release` without signing credentials: expected fail-closed behavior passed.
- GitHub YAML parse validation: 24 files passed.
- Visual baseline: `docs/screenshots/foundation-shell.png`.
- Smart onboarding baseline: `docs/screenshots/connection-wizard.png`.
- Sidebar baseline: `docs/screenshots/sidebar-workspaces.png`; Devices workspace baseline:
  `docs/screenshots/sidebar-devices-workspace.png`.
- Real Windows/ADB/Fastboot probe integration: passed in 0.47 seconds with a 20-second hard timeout.
- Diagnostics documentation synchronization validator: passed for all state keys, OEM catalog keys
  and seven required operational guides.
- Deterministic diagnostics tests cover USB transport stability, Windows-driver presence and normal
  Recovery-mode evidence; authorized hardware sampling remains bounded and fail-unknown.
- Final Release package validation passed through a repository-local alternate output after Windows
  held the previous output directory open. The validated portable tree was synchronized back to
  `dist/ADB-Studio`; its EXE has 40 root runtime DLLs beside it, and Inno/MSIX/ZIP artifacts were
  regenerated with `0.1.0.4` file/product versions and fresh SHA-256 manifests.
- The earlier 15-workspace validation is superseded by ADR-0005 and the four implemented-module
  registry.
- clang-tidy passed for the changed workspace, dashboard and application composition sources.

Cppcheck, include-what-you-use, CodeQL and sanitizer executions are configured as mandatory CI jobs;
local Cppcheck/IWYU executables are not installed. Physical-device throughput, stress and
long-duration results still require reference hardware and an authorized Android transport.
