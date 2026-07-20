# Threat Model

## Assets and Boundaries

Protected assets include device access, user files, recordings, credentials, update trust and plugin
permissions. Trust boundaries exist at QML/C++, ADB process, scrcpy process, plugin IPC, filesystem,
network and release/update interfaces.

## STRIDE Controls

| Threat | Primary controls |
|---|---|
| Spoofing | Device identity confirmation; signed updates and plugins |
| Tampering | SHA-256 manifests, signatures, atomic settings and provenance |
| Repudiation | Structured security audit events without sensitive payloads |
| Information disclosure | Opt-in diagnostics, redaction, capability-scoped plugins |
| Denial of service | Timeouts, cancellation, quotas, backoff and process supervision |
| Elevation of privilege | Least privilege, no shell interpolation, deny-by-default capabilities |

Each new external input or trust boundary requires an ADR-linked threat-model update and failure
tests before merge.
