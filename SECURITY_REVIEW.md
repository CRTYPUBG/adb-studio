# Security Review

## Milestone 0.1.0

- QML has no direct filesystem, process, network or device access.
- Theme and dashboard objects are owned by the main thread and exposed read-only except explicit
  commands.
- Resource paths reject absolute paths and traversal; resource content is SHA-256 verified.
- scrcpy and ADB binaries are not launched by this milestone.
- No update, plugin, FFI or telemetry boundary is implemented yet; those features remain blocked
  until their dedicated threat review and contract tests exist.

Residual risk: bundled third-party binary versions and provenance require verification before any
redistribution. This development milestone is not a public release.
