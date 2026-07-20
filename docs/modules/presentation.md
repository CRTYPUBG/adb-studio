# Presentation Module

The presentation module contains FluentWinUI3 QML and C++ ViewModels. QML may bind observable
properties, format localized display values and emit user commands. It may not execute processes,
access files, perform device I/O or implement domain decisions.

`ThemeService` owns system/light/dark preference and semantic palette tokens. The dashboard
ViewModel currently exposes the disconnected state and refresh command contract; the Device Manager
milestone will provide its asynchronous data source.

All components require accessible names, keyboard behavior, responsive layouts, translated visible
strings and registration in `resources/res.crty`.

Visual baselines are generated from the production QML tree with
`adb-studio --render-screenshot <path>`. The command performs no device operations and exits after
the first stable frame.
