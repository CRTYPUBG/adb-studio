# ADR-0005: Real Mirroring, Live Localization and Visible Module Policy

- Status: Accepted
- Date: 2026-07-21
- Supersedes: ADR-0004 statements that permitted unavailable capabilities to remain visible

## Context

The workspace catalog exposed modules whose application services did not exist. Even when disabled,
those surfaces behaved as product promises rather than working features. Screen mirroring also
needed a process boundary that could supervise the bundled scrcpy 4.0 runtime without putting
device commands or process logic in QML. English and Turkish had to switch live for both QML and
dynamically generated C++ presentation text.

## Decision

The visible registry contains only Dashboard, Devices, Screen Mirroring and Device Guide. A module
may enter the registry only after its service, ViewModel, real commands, tests, translations and
documentation exist.

`AdbService`, `DeviceManagerService` and `MirrorService` remain C++ services below the presentation
layer. `MirrorService` supervises one isolated scrcpy child process per device serial and owns
session lifecycle, pause/resume, recording, screenshots, APK installation, clipboard options,
latency and process output. `MirrorViewModel` publishes typed observable state and asynchronous
refresh/latency work; QML binds that state and emits commands only. Third-party scrcpy and ADB
binaries stay outside first-party signing scope.

`LanguageService` installs the selected Qt translator, persists the locale and calls
`QQmlApplicationEngine::retranslate()`. C++-generated workspace text is rebuilt through explicit
translation contexts. Both language and theme preferences persist through `QSettings` and apply
without restarting.

Process-failure integration tests use a compiled deterministic child-process fixture. They do not
depend on an attached device or simulate a service. Physical-device certification remains a
separate hardware gate.

## Consequences

- No unavailable backend, fake action or placeholder module is visible.
- Multiple USB or wireless serials can own independent scrcpy sessions.
- A faulty or failed scrcpy process cannot crash the application and produces actionable state.
- English/Turkish presentation text changes live and is completeness-validated.
- New visible modules require complete vertical slices and a new or amended ADR when architecture
  changes.
