# Architecture

ADB Studio uses `QML Presentation -> C++ ViewModel -> Application -> Core -> Domain`, with
Infrastructure implementing inward-facing ports. QML contains no business logic and plugins run out
of process. The authoritative diagrams, ADRs and machine-readable dependency matrix are in
[`docs/architecture`](docs/architecture/README.md).
