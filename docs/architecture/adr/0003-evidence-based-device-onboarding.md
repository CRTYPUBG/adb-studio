# ADR 0003: Evidence-based device onboarding and recovery

- Status: Accepted
- Date: 2026-07-20

## Context

ADB cannot distinguish every USB, driver, OEM and authorization failure by itself. Guessing a cause
can send users to unsafe or irrelevant settings and conflicts with the diagnostics requirements.

## Decision

Represent every probe input as `Yes`, `No` or `Unknown`. The diagnostics engine classifies a
connection only from positive evidence and keeps `UNKNOWN` when the available probes cannot isolate
one cause. ADB, Fastboot and Windows PnP/CIM probing runs off the UI thread. One bounded recovery
attempt may refresh PnP and restart ADB before the final report; physical, authorization and OEM
security actions remain user-controlled.

OEM selection uses verified manufacturer, brand, model and measured skin properties from an
authorized serial. Guides are never selected from USB VID/PID guesses or marketing names alone.
Raw tool errors remain internal and are translated into stable, localized ViewModel state.

## Consequences

Some results remain `UNKNOWN` until an authorized device or benchmark supplies evidence. The UI
shows evidence confidence alongside the 0–100 health score. All state/OEM documentation is checked
against the implementation during configure and CI.
