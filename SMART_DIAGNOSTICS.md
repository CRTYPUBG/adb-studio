# Smart Diagnostics

Each check returns exactly one of `PASS`, `WARNING`, `FAIL`, `UNSUPPORTED` or `UNKNOWN`. Results also
contain what was measured, why it matters, a corrective action, difficulty, estimated time and
whether an automatic repair exists.

Implemented probes cover ADB installation/server/version compatibility, authorization, separate
USB and Windows Android/MTP driver health, measured USB mode, wireless debugging, screen-record
command availability, AVC codec availability, display refresh evidence, root, Fastboot and
Recovery. Audio and battery optimization remain `UNKNOWN` until a device-specific capability probe
measures them.

For one authorized, verified USB transport, the probe runs two bounded 4 MiB device-to-host samples
and records their mean throughput. If both complete, the lower sample must be at least 70 percent of
the higher sample to pass the transport-stability check. This is only a repeatability proxy; it is
not proof of the physical cable's quality. Failed, wireless or unverifiable measurements remain
`UNKNOWN`.

Automatic repair is deliberately narrow: rescan Windows PnP devices, stop the stale ADB server,
start one ADB server, request bounded reconnects for offline/device transports, and rescan. ADB
Studio never auto-approves RSA authorization, disables OEM security controls, changes battery
policy or claims to repair a physical cable.
