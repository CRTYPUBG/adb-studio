# Review

## Build 6 Signing Review

Certificate selection is explicit and deterministic. The private key remains in the Windows
certificate store, while GitHub retains temporary secret-backed PFX support. The application is
signed before portable/MSIX/Inno packaging; package containers are then signed and every signature
is verified. Third-party binaries are excluded by basename allow-list. Timestamp verification was
confirmed against DigiCert's signed RFC 3161 chain.

## Enterprise Banner Review

The banner uses an ultra-wide two-column hierarchy with restrained Fluent glass, high-contrast
copy, vector device presentation and sufficient negative space. All dense product information is
contained in aligned panels rather than floating decoration. The final 8K render remains legible at
README scale while retaining detailed dashboard content for store and presentation use.

## Build 5 Three-Stage Review

### Architecture review

The change preserves `QML -> ViewModel -> Services -> Infrastructure`. QML has no process, ADB or
settings logic. scrcpy remains isolated and per-serial session ownership is explicit. ADR-0005 and
the mirroring sequence/state diagrams describe the accepted change; dependency validation is
acyclic.

### Independent quality and security review

Every visible module has a service-backed action path. Failure, retry, diagnostics, logs, copy and
issue-report actions are connected. Process shutdown, duplicate terminal signals, asynchronous
ViewModel teardown, settings persistence, translations, resource declaration and third-party
signing scope were reviewed. A deterministic child-process test replaced external ADB timing
dependence.

### Final review

Three consecutive local gates passed after the lifecycle fix, including both locale renders. The
module is production-ready for software-only validation and unsigned packaging. Release signing and
physical-device compatibility certification remain explicit external gates; no public release is
approved without them.

## Review 1: Architecture

The initial target dependency is acyclic: application presentation depends on core presentation;
core does not depend on the application or QML.

## Review 2: Quality and Security

Resource traversal and integrity are validated. QML is presentation-only. Third-party processes are
not executed in this milestone.

## Final Review

The foundation milestone passed clean configure, warnings-as-errors build, unit tests, formatting,
QML lint, manifest checks, architecture checks, runtime smoke rendering and startup measurement.
It is ready for the ADB/Device Manager milestone, but public distribution still requires production
signing credentials and clean-machine installer/MSIX certification.

Repository hardening review confirmed a linear remote base, Apache-2.0 project license, reusable CI
policy, secret-free signing definitions, portable dependency verification and synchronized installer
metadata. Server-side GitHub settings still require an authenticated repository administrator.

Release automation review confirmed one version source, environment-only secrets, explicit
first-party signing scope, timestamped signature verification, signed-content-first packaging and
complete evidence generation. Ordinary `package` remains intentionally unsigned for developer
validation; production commands cannot bypass signing.

Smart onboarding review confirmed deterministic classification tests, explicit unknown states,
authorized-property OEM matching, HTTPS guide links, bounded recovery and presentation-only QML.
The engine does not infer driver, cable, codec or wireless capabilities from Android version alone.
USB speed/stability and AVC codec results now require successful authorized-device measurements;
Windows-driver and Recovery checks have independent diagnostic records.

The earlier unavailable-capability presentation policy is superseded by ADR-0005; absent backends
are no longer visible.
