# Validation

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
- Unit tests: 2/2 passed.
- clang-format dry run: passed.
- qmllint: passed without diagnostics.
- Resource, version, QML policy and architecture validators: passed.
- Runtime QML/FluentWinUI3 screenshot smoke test: exit 0, empty stderr.
- Time to first QML root object: 800 ms on the local reference machine.
- SPDX 2.3 and CycloneDX 1.6 SBOM generation: passed.
- Windows FileVersion and ProductVersion: both `0.1.0.1`, derived from `version.crty`.
- `Build.py` Release packaging: passed; portable output contains Qt, FluentWinUI3, Windows platform
  and MSVC runtime dependencies.
- Portable-folder runtime smoke test: exit 0 with empty stderr and no Qt development PATH required.
- Multi-resolution ICO generation and resource hash validation: passed.
- PE executable and installer associated-icon extraction: passed.
- Inno Setup 6.6.1 compilation: passed.
- GitHub YAML parse validation: 24 files passed.
- Visual baseline: `docs/screenshots/foundation-shell.png`.

Cppcheck, clang-tidy, include-what-you-use, CodeQL and sanitizer executions are configured as
mandatory CI jobs; local Cppcheck/IWYU executables are not installed. Hardware, device throughput,
stress and long-duration results begin with the Device Manager milestone because this milestone has
no device transport implementation.
