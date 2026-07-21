# API

## Smart device onboarding

- `diagnostics::SystemDeviceProbe::probe()` measures ADB, Fastboot, authorized properties and
  Windows Android/MTP PnP evidence without exposing raw errors.
- `SystemDeviceProbe::probeWithRecovery()` performs one bounded PnP/ADB recovery and re-probes.
- `ConnectionDiagnosticsEngine::analyze(DeviceFacts)` is deterministic and side-effect free; it
  produces a connection state, typed diagnostics and weighted health score.
- `OemGuideCatalog::detect(DeviceFacts)` selects a guide only from verified identity properties.
- `DeviceDashboardViewModel` exposes asynchronous scan/recovery commands and presentation-safe
  observable state. QML never executes ADB or platform commands.

## Mirroring and localization

- `services::AdbService` locates the bundled ADB executable and performs bounded process calls.
- `services::DeviceManagerService` enumerates device serial/state/model values, restarts ADB,
  measures latency and installs validated APK paths.
- `services::MirrorService` supervises isolated per-serial scrcpy processes and exposes session,
  failure, log, FPS, pause, screenshot and APK operations.
- `MirrorViewModel` exposes selectable devices, persisted mirror settings, session state, metrics,
  logs and actionable error/recovery commands to QML.
- `LanguageService` persists `en`/`tr`, installs the translator and triggers live engine and
  workspace retranslation.

The versioned `ADBStudio.Presentation` QML module exposes uncreatable `ThemeService`,
`LanguageService`, `DeviceDashboardViewModel`, `MirrorViewModel` and `WorkspaceViewModel` types
supplied by the composition root. Exceptions do not cross the QML or child-process boundary.
API/ABI compatibility policy follows semantic versioning and requires contract tests.
