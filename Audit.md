# Audit

## Milestone 0.1.0 Build 5: Real Mirroring and Localization

- Removed eleven unsupported workspaces and every unavailable-backend presentation path; only four
  implemented modules remain visible.
- Verified all visible interactive controls with the UI-connection validator; no placeholder,
  dummy, mock-service, fake-button or backend-required text remains.
- Added real `AdbService`, `DeviceManagerService`, `MirrorService` and `MirrorViewModel` paths for
  bundled scrcpy 4.0 and ADB.
- Audited scrcpy lifecycle ownership. Destruction now clears the session registry and disconnects
  callbacks before killing child processes, preventing callback mutation during container
  iteration; duplicate finished/error handling is guarded by process identity.
- Replaced external-device-dependent failure testing with a compiled deterministic child process;
  50 repeated CTest runs passed.
- Added 394 finished Turkish translations, live translator replacement, persistent locale/theme and
  English/Turkish runtime screenshot checks.
- Corrected local clang-tidy selection/include argument handling and included the services headers;
  changed mirroring source passed clang-tidy with warnings as errors.
- Third-party executables are packaged under `tools/scrcpy` and remain excluded from first-party
  signing.
- No authorized physical device was available; hardware behavior is not claimed as certified.

## Milestone 0.1.0 Build 6: Authenticode Repair

- Found that the signing implementation accepted only PFX files although valid private-key
  `CN=CRTYPUBG` certificates were installed in `CurrentUser/My`.
- Added explicit thumbprint selection without automatic certificate guessing; ambiguous PFX/store
  configurations fail closed.
- Identified that DigiCert rejects the HTTPS variant of its RFC 3161 endpoint in SignTool; the
  documented operational HTTP endpoint returned a signed DigiCert timestamp token that passed
  `/pa /all` verification.
- Signing remains limited to reviewed first-party binaries and package containers. Qt, MSVC,
  scrcpy and ADB binaries remain untouched.

## Enterprise Marketing Banner

- Existing artwork was used only to assess layout quality and was not traced or copied.
- The new composition uses only repository-authored SVG geometry, typography, gradients and icons.
- No Android robot, Google, Samsung or other third-party logo is embedded.
- SVG and PNG are stored together so every derived marketing format has an editable source.

## Milestone 0.1.0 Foundation

- Repository scan found no existing ADB Studio implementation to reuse.
- Existing scrcpy 4.0 source is Apache-2.0 and remains unmodified.
- Installed Qt 6.8.1 provides the official FluentWinUI3 QML style module.
- First-party targets enable strict compiler warnings and warnings-as-errors.
- Resource and QML policy validators are part of configure/CI.
- CMake dependency edges match the machine-readable layer policy and are cycle-free.
- `build.py` stages output atomically, confines cleanup to `dist`, runs `windeployqt`, copies the
  redistributable MSVC runtime and rejects incomplete DLL/plugin deployments.
- GitHub hardening includes templates, Dependabot, release categorization, separated workflow
  surfaces, CodeQL, signing readiness and documented branch/repository settings.
- Git repository governance is initialized on the `main` branch; no release commit or remote exists.
- Public distribution remains blocked until production certificate signing and clean-machine
  installer/MSIX certification complete; planned scrcpy/ADB inputs are not included in 0.1.0.
- The release pipeline signs only the explicit first-party allow-list before packaging, then signs
  and verifies Inno/MSIX artifacts. Certificate paths/passwords are environment-only and absent
  credentials block release/nightly/publish operations.
- SPDX and CycloneDX documents, package hashes, artifact manifest, release notes, symbols, license
  bundle and governance evidence are emitted for every complete package run.
- Device onboarding classifies only tri-state measured facts. Windows PnP, ADB and Fastboot probes
  run outside the UI thread; unavailable cable/capability evidence stays `UNKNOWN`.
- Recovery is bounded to PnP refresh, ADB restart, offline/device reconnect requests and rescan. RSA
  approval, OEM security controls and physical cable actions are never automated or claimed without
  verification.
- Authorized USB transports use two bounded synthetic transfer samples; the reported cable check is
  explicitly a transport-stability proxy. Codec support is based on successful `media.codec`
  output, while failed probes remain `UNKNOWN`.
- Earlier sidebar findings are superseded by ADR-0005: unsupported modules are now absent.
