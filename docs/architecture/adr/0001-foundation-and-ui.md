# ADR-0001: Foundation and FluentWinUI3 UI

- Status: Accepted
- Date: 2026-07-20

## Context

ADB Studio begins from scrcpy source and binary distributions without an existing application
architecture. The product requires a Windows 11-native UI while preserving Qt portability.

## Decision

Use C++20, Qt 6.8+, CMake and a strict layered architecture. QML is presentation-only and uses
`QtQuick.Controls.FluentWinUI3` as the sole controls style. Theme and device state enter QML through
C++ ViewModels/services. scrcpy and future third-party plugins run as supervised child processes.

## Consequences

The UI remains testable without device commands in QML. Windows-specific Mica integration stays
behind a platform adapter. Fluent style deployment, Qt licensing and process supervision become
mandatory release concerns.
