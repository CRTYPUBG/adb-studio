# Architecture

ADB Studio enforces this dependency direction:

`Presentation -> ViewModel -> Application -> Core -> Domain <- Infrastructure`

Infrastructure implements domain-facing interfaces and is composed at the application boundary.
QML imports presentation types only. Plugins run outside the application process. See ADR-0001 and
the diagrams in this directory before changing dependencies.

## Diagrams

```mermaid
C4Context
title ADB Studio Context
Person(user, "Device operator")
System(adbstudio, "ADB Studio", "Manages Android devices")
System_Ext(device, "Android device", "ADB and media endpoint")
System_Ext(releases, "Signed release service", "Updates and plugin packages")
Rel(user, adbstudio, "Operates")
Rel(adbstudio, device, "ADB, USB or TCP/IP")
Rel(adbstudio, releases, "Verifies signed artifacts")
```

```mermaid
C4Container
title ADB Studio Containers
Container(ui, "Qt Quick UI", "QML / FluentWinUI3", "Presentation only")
Container(app, "Application", "C++20", "Use cases and ViewModels")
Container(core, "Core and Domain", "C++20", "Contracts and device state")
Container(infra, "Infrastructure", "C++20", "ADB, filesystem and platform adapters")
Container_Ext(scrcpy, "scrcpy 4.0", "Isolated process", "Mirroring")
Container_Ext(plugin, "Plugin host", "Isolated process", "Capability-limited extensions")
Rel(ui, app, "Commands and observable state")
Rel(app, core, "Uses")
Rel(infra, core, "Implements ports")
Rel(infra, scrcpy, "Supervises")
Rel(app, plugin, "Versioned IPC")
```

```mermaid
flowchart LR
  QML["QML Presentation"] --> VM["C++ ViewModels"]
  VM --> APP["Application Services"]
  APP --> CORE["Core Services"]
  CORE --> DOMAIN["Domain"]
  INFRA["Infrastructure Adapters"] --> DOMAIN
  PLUGIN["Out-of-process Plugin Host"] -. "IPC" .-> APP
```

```mermaid
sequenceDiagram
  actor User
  participant QML
  participant Dashboard as Dashboard ViewModel
  participant Device as Device Manager
  participant ADB as ADB Backend
  User->>QML: Refresh
  QML->>Dashboard: refresh()
  Dashboard->>Device: scan(cancellation)
  Device->>ADB: enumerate(timeout)
  ADB-->>Device: typed devices/result
  Device-->>Dashboard: observable snapshot
  Dashboard-->>QML: property notifications
```

```mermaid
stateDiagram-v2
  [*] --> Disconnected
  Disconnected --> Discovering: scan
  Discovering --> Unauthorized: authorization required
  Discovering --> Connected: transport ready
  Discovering --> Offline: device reported offline
  Unauthorized --> Discovering: authorization changed
  Offline --> Discovering: retry with backoff
  Connected --> Recovering: transport lost
  Recovering --> Connected: transport restored
  Recovering --> Disconnected: retry budget exhausted
```

## Module Dependency Matrix

| Consumer | Presentation | ViewModel | Application | Core | Domain | Infrastructure |
|---|---:|---:|---:|---:|---:|---:|
| Presentation | allowed | allowed | denied | denied | denied | denied |
| ViewModel | denied | allowed | allowed | denied | denied | denied |
| Application | denied | denied | allowed | allowed | allowed | denied |
| Core | denied | denied | denied | allowed | allowed | denied |
| Domain | denied | denied | denied | denied | allowed | denied |
| Infrastructure | denied | denied | denied | allowed | allowed | allowed |

`module-dependencies.json` is the reviewed machine-readable dependency policy and mirrors CMake
target metadata. CI rejects cycles, unknown modules and matrix violations.
