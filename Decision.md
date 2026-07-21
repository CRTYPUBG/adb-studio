# Decisions

## 2026-07-21

- Selected SVG as the canonical enterprise-banner source to guarantee exact product copy,
  resolution-independent export and auditable original geometry; the checked-in PNG is its 8K
  distribution render.
- Extended ADR-0002 implementation with an explicit Windows `CurrentUser/My` certificate-store
  source. PFX and thumbprint selection are mutually exclusive; certificate auto-selection remains
  prohibited.
- Corrected the default RFC 3161 timestamp endpoint to DigiCert's operational HTTP URL. Integrity
  comes from the verified signed timestamp token; SHA-256 remains mandatory.
- Accepted ADR-0005: visible modules require complete service-backed vertical slices; the registry
  now contains Dashboard, Devices, Screen Mirroring and Device Guide only.
- Kept mirroring within the existing dependency direction through `MirrorViewModel`,
  `MirrorService`, isolated scrcpy 4.0 child processes and the bundled ADB adapter.
- Selected live Qt translator replacement plus `QQmlApplicationEngine::retranslate()` and explicit
  C++ translation contexts for restart-free English/Turkish switching.
- Selected a compiled failing child-process fixture for deterministic lifecycle tests; it is a real
  process boundary test and not a mock service.
- Third-party scrcpy/ADB runtime files remain outside the first-party signing allow-list.

## 2026-07-20

- Accepted ADR-0001: layered C++/QML architecture and FluentWinUI3-only presentation.
- Accepted ADR-0002: single-entry build, signing, packaging and publication automation.
- Accepted ADR-0003: tri-state evidence, non-guessing connection classification, bounded ADB/PnP
  recovery and verified OEM-guide selection.
- Accepted ADR-0004: C++ sidebar workspace registry, explicit capability availability and
  presentation-only reusable module pages.
- `version.crty` is the version source of truth.
- CMake, application, Windows FileVersion and ProductVersion are generated from `version.crty`;
  tagged release automation updates commit, branch, build number and build date before packaging.
- The canonical PNG generates a multi-resolution ICO; the same icon is embedded in the PE executable
  and Inno Setup installer.
- GitHub workflows share one reusable Windows/MSVC/Qt pipeline to avoid duplicated build policy.
- `resources/res.crty` is generated deterministically and validated during CMake configuration.
- scrcpy remains an isolated third-party process; its source tree is not linked into first-party
  targets.
