# Changelog

## 0.1.0 - 2026-07-21

- Established the C++20, Qt 6.8 and CMake repository foundation.
- Added the FluentWinUI3 application shell, theme service and device dashboard ViewModel.
- Added resource integrity checks, unit tests, CI policy and enterprise governance documents.
- Recorded architecture and initial STRIDE security decisions.
- Added the initial build entry point to produce a verified self-contained Windows folder with Qt
  DLLs and QML plugins.
- Embedded a multi-resolution Windows icon into the executable and Inno Setup installer.
- Added `setup.iss`, automated installer compilation and enterprise GitHub repository configuration.
- Replaced the legacy one-shot build script with lowercase `build.py`, the single command-based
  entry point for local and GitHub Actions build, test, benchmark, lint, packaging and publication.
- Added fail-closed Authenticode signing and verification for first-party binaries, Inno installer
  and MSIX without signing third-party runtime files.
- Added Portable ZIP, MSIX, symbol archive, license bundle, SPDX/CycloneDX SBOM, release notes,
  SHA-256 checksums, artifact manifest and benchmark evidence generation.
- Added evidence-based ADB/Fastboot/Windows PnP device detection, bounded automatic recovery and
  human-readable connection states without exposing raw ADB errors.
- Added typed smart diagnostics, evidence confidence, weighted device health scoring and dedicated
  OEM setup guidance for Samsung, Pixel, Xiaomi-family, Huawei/Honor, BBK-family and other OEMs.
- Added the FluentWinUI3 connection wizard so a missing device never produces an empty dashboard.
- Added bounded USB throughput and transport-stability sampling, verified AVC codec probing, and
  explicit Windows-driver and Recovery-mode diagnostics without converting missing evidence into
  assumptions.
- Extended automatic recovery with bounded offline/device reconnect requests after restarting ADB.
- Replaced disabled sidebar placeholders with 15 navigable FluentWinUI3 workspaces backed by a
  tested C++ `WorkspaceViewModel`, responsive capability pages, search, filtering, keyboard
  shortcuts, context menus, loading/error/empty states, notifications and navigation undo.
- Added fail-honest capability dispatch: only composed scan, recovery and guide commands can run;
  absent scrcpy, filesystem, package, telemetry and plugin backends cannot emit fake actions.
- Replaced the 15-workspace catalog with four fully implemented modules so unsupported features are
  not visible as placeholders.
- Added real multi-device scrcpy 4.0 supervision with USB/wireless serials, audio, recording,
  screenshots, clipboard options, APK drop/install, fullscreen, rotation, resolution, FPS, bitrate,
  codecs, latency, performance overlay, pause/resume, reconnect and actionable failure recovery.
- Added `AdbService`, `DeviceManagerService`, `MirrorService` and `MirrorViewModel` with deterministic
  child-process lifecycle integration coverage.
- Added persistent live English/Turkish switching with 394 completed translations and runtime
  rendering validation in both locales.
- Added no-placeholder, translation-completeness and UI-action-connection validators plus an
  automated three-run milestone quality gate.
- Fixed mirror-session teardown mutation and duplicate terminal-signal risks, and made the failure
  integration test independent of external ADB/device timing.
- Fixed Windows signing so local releases can explicitly use a private-key certificate from
  `CurrentUser/My` through `SIGN_CERT_THUMBPRINT`, while retaining secret-backed PFX signing in CI.
- Corrected the DigiCert RFC 3161 timestamp endpoint and kept mandatory post-sign verification for
  the application, Inno installer and MSIX package.
- Stabilized portable and symbol archive root names so temporary staging/output directory names
  never leak into distributed ZIP layouts.
- Added a completely original 8K enterprise marketing banner with an editable SVG source, Fluent
  dark visual language, custom device/dashboard artwork, feature cards and exact product metadata.
