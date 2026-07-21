# Screen Mirroring Module

The module is a complete `MirrorPage.qml -> MirrorViewModel -> MirrorService -> scrcpy/ADB` vertical
slice. QML contains no process or device logic.

## Working operations

- Enumerate authorized USB and wireless ADB serials asynchronously.
- Start, stop and reconnect independent scrcpy 4.0 sessions for multiple devices.
- Configure audio, fullscreen, max resolution, FPS, bitrate, H264/H265/AV1, orientation,
  clipboard synchronization and recording before launch.
- Capture PNG screenshots with `adb exec-out screencap -p`.
- Install dropped APK files with `adb install -r` after extension/path validation.
- Read scrcpy FPS output and measure bounded ADB round-trip latency.
- Suspend/resume a running child process on Windows and apply automatic reconnect policy.
- Surface cause, solution and technical details with retry/repair, diagnostics, logs, copy-error and
  report-issue actions.

The runtime is discovered from `tools/scrcpy` in a deployed package and from the repository's
`scrcpy` directory during development. Packaging requires `scrcpy.exe`, `scrcpy-server`, `adb.exe`
and the complete runtime dependency set. These are third-party files and are not Authenticode-signed
by ADB Studio.

Settings are persisted with `QSettings`. Device enumeration, engine-version discovery and latency
sampling do not block the UI thread. Destruction cancels and waits for outstanding ViewModel work;
service destruction disconnects callbacks before terminating remaining child processes.

Automated validation covers bundled tool discovery/version, deterministic process failure,
settings validation, session cleanup, UI command connections, both locales and repeated runtime
rendering. Hardware-only audio, video, pause/resume and sustained reconnect behavior are not marked
certified until an authorized physical device is available.
