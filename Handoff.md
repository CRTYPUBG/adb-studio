# Handoff

## Current Milestone

The service-backed screen-mirroring, live-localization and repaired Authenticode pipeline are
complete at `0.1.0-dev`, build 6. The visible sidebar contains only Dashboard, Devices, Screen Mirroring and
Device Guide. `MirrorPage.qml` is connected through `MirrorViewModel` to real ADB/device/mirror
services and bundled scrcpy 4.0 child processes. English and Turkish switch live and persist.

The canonical enterprise marketing banner is available as an editable SVG and a verified
7680 × 4320 PNG under `assets/marketing`; README consumes the PNG directly. Artwork provenance and
export requirements are documented in `docs/marketing/README.md`.

Three consecutive complete local gates passed before this evidence update. Each gate included
resource/version/architecture/QML/translation/UI-connection validation, clang-format, qmllint,
warnings-as-errors Release build, 6/6 C++ tests, 3/3 Python tests and English/Turkish runtime
screenshots. The deterministic mirror failure test passed 50 repeated CTest runs. See
`dist/evidence/quality-gate.json` and `docs/screenshots/mirror-service*.png`.

`python build.py package` remains the unsigned developer-package entry point. Local signing uses an
explicit `SIGN_CERT_THUMBPRINT`; CI uses temporary PFX credentials. Both paths timestamp and verify
every first-party/package signature and fail closed.

## Next Milestone

Certify mirroring with authorized physical USB and wireless devices across the compatibility matrix,
including audio, codecs, recording, pause/resume, multi-device, sustained reconnect and long-duration
memory/FPS behavior. Then deliver filesystem/package services as complete vertical slices before
adding their modules to the registry. Production signing and clean-machine installer/MSIX
certification still require external credentials and reference machines.
