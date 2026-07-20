# Audit

## Milestone 0.1.0 Foundation

- Repository scan found no existing ADB Studio implementation to reuse.
- Existing scrcpy 4.0 source is Apache-2.0 and remains unmodified.
- Installed Qt 6.8.1 provides the official FluentWinUI3 QML style module.
- First-party targets enable strict compiler warnings and warnings-as-errors.
- Resource and QML policy validators are part of configure/CI.
- CMake dependency edges match the machine-readable layer policy and are cycle-free.
- `Build.py` stages output atomically, confines cleanup to `dist`, runs `windeployqt`, copies the
  redistributable MSVC runtime and rejects incomplete DLL/plugin deployments.
- GitHub hardening includes templates, Dependabot, release categorization, separated workflow
  surfaces, CodeQL, signing readiness and documented branch/repository settings.
- Git repository governance is initialized on the `main` branch; no release commit or remote exists.
- Distribution remains blocked until third-party binary versions and license provenance are fully
  resolved.
