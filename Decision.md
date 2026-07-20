# Decisions

## 2026-07-20

- Accepted ADR-0001: layered C++/QML architecture and FluentWinUI3-only presentation.
- `version.crty` is the version source of truth.
- CMake, application, Windows FileVersion and ProductVersion are generated from `version.crty`;
  tagged release automation updates commit, branch, build number and build date before packaging.
- The canonical PNG generates a multi-resolution ICO; the same icon is embedded in the PE executable
  and Inno Setup installer.
- GitHub workflows share one reusable Windows/MSVC/Qt pipeline to avoid duplicated build policy.
- `resources/res.crty` is generated deterministically and validated during CMake configuration.
- scrcpy remains an isolated third-party process; its source tree is not linked into first-party
  targets.
