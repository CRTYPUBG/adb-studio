# Security Review

## Milestone 0.1.0

- QML has no direct filesystem, process, network or device access.
- Theme and dashboard objects are owned by the main thread and exposed read-only except explicit
  commands.
- Resource paths reject absolute paths and traversal; resource content is SHA-256 verified.
- scrcpy and ADB binaries are not launched by this milestone.
- No update, plugin, FFI or telemetry boundary is implemented yet; those features remain blocked
  until their dedicated threat review and contract tests exist.

- Authenticode release automation reads certificate material only from environment/GitHub Secrets,
  redacts passwords from logs, timestamps and verifies first-party/package signatures, and deletes
  the temporary CI certificate even after failure.
- Trivy, dependency review, CodeQL and deterministic dependency/license auditing are mandatory
  workflow gates.

Residual risk: production signing credentials and clean-machine installer/MSIX certification are
required before public redistribution. This development milestone is not a public release.
