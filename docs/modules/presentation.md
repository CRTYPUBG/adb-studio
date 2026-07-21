# Presentation Module

`DeviceDashboardViewModel` adapts immutable `ConnectionReport` values into QML properties. It owns a
future watcher but no process implementation; worker tasks call the diagnostics probe and apply the
result on the owning UI thread. The connection wizard binds state, diagnostics, health confidence
and OEM steps, and emits only ViewModel commands.

The presentation module contains FluentWinUI3 QML and C++ ViewModels. QML may bind observable
properties, format localized display values and emit user commands. It may not execute processes,
access files, perform device I/O or implement domain decisions.

`ThemeService` owns persisted system/light/dark preference and semantic palette tokens.
`LanguageService` owns persisted English/Turkish selection, Qt translator installation and live QML
retranslation. `MirrorViewModel` owns mirroring presentation state and delegates every operation to
the device/mirror services.

`WorkspaceViewModel` is the sidebar registry and state owner. It contains only the four implemented
modules, implements navigation undo, search, category filtering, loading, empty, error and
notification states, and retranslates its C++-generated catalog live.

All components require accessible names, keyboard behavior, responsive layouts, translated visible
strings and registration in `resources/res.crty`.

Visual baselines are generated from the production QML tree with
`adb-studio --render-screenshot <path>`. The command performs no device operations and exits after
the first stable frame.
