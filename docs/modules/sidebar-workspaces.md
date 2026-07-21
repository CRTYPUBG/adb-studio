# Sidebar Workspaces

ADB Studio exposes four FluentWinUI3 workspaces whose vertical slices are implemented: Dashboard,
Devices, Screen Mirroring and Device Guide. Modules without real services are absent from the UI.

`WorkspaceViewModel` owns the module catalog and all navigation state. It supplies translated
translated titles, descriptions, search results, filters, loading/error/empty states, notifications
and undo. `MirrorPage.qml` owns mirroring presentation; reusable QML contains no process,
filesystem or device logic.

## Command availability

Device scan, bounded ADB recovery, OEM/wireless guides and the mirroring operations documented in
`screen-mirroring.md` dispatch real commands. Filesystem, package, benchmark, plugin and other future
modules are not visible until their complete service-backed vertical slices pass the milestone gate.

## Interaction contract

- `Ctrl+F` focuses workspace search.
- `F5` refreshes the current capability snapshot.
- `Ctrl+Z` returns to the previous workspace when navigation undo is available.
- Right-clicking a capability opens its Fluent context menu.
- Search and category filters are evaluated in C++, not QML.
- Loading, empty, error, ready and notification states have accessible announcements.

Each backend milestone must add its own application service, repository/adapter, ViewModel tests,
contract tests and documentation before marking the corresponding registry capability available.

The reviewed Devices workspace baseline is `docs/screenshots/sidebar-devices-workspace.png`.
