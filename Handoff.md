# Handoff

## Current Milestone

The repository foundation and FluentWinUI3 shell are implemented at `0.1.0-dev`. The shell starts in
an honest disconnected state and emits a refresh request through the dashboard ViewModel. Build,
unit tests, QML lint, resource/version/architecture validation, SBOM generation and runtime visual
smoke testing pass. The current visual baseline is `docs/screenshots/foundation-shell.png`.
`python Build.py` is the supported portable-build entry point and places all required runtime DLLs
next to `dist/ADB-Studio/adb-studio.exe`.
Release builds also generate the versioned Inno Setup installer under `dist/installer`. The `origin`
remote targets `https://github.com/CRTYPUBG/adb-studio.git`; repository settings documented under
`docs/github` require an authenticated administrator session.

## Next Milestone

Implement `IAdbBackend`, safe process execution and the typed Device Manager state machine. Connect
the dashboard only after unit, parser, cancellation, timeout and fake-process contract tests pass.
