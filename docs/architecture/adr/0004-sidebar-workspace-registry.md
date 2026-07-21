# ADR-0004: Sidebar Workspace Registry and Capability Availability

- Status: Superseded by ADR-0005
- Date: 2026-07-20

## Context

The application shell had disabled sidebar entries. Several requested workspaces depend on service
milestones that do not yet exist, including the isolated scrcpy supervisor, filesystem repository,
package service, plugin host and telemetry collectors. Enabling controls without those backends
would create fake actions and violate evidence-based behavior.

## Decision

Use one C++ `WorkspaceViewModel` as the presentation-facing registry for sidebar navigation,
capability search/filtering, loading, error, notification and undo state. The registry is the sole
authority for whether a capability has an implemented command. QML renders state and emits commands
only. The application composition root connects available commands to existing ViewModels.

This original decision allowed unsupported capabilities to remain visible. ADR-0005 replaces that
policy: only complete service-backed vertical slices may appear in the registry.

## Consequences

- Disabled placeholder navigation is eliminated.
- A visible tile cannot silently claim that an absent backend exists.
- Module services remain independently deliverable behind application contracts.
- Registry and dispatch behavior are deterministic and unit tested.
