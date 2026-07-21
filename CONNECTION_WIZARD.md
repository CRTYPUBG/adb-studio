# Smart Connection Wizard

1. Inventory Android/MTP interfaces and their Windows driver status.
2. Locate the active `adb` and `fastboot` executables.
3. Measure ADB version/server/device transport state.
4. If authorized, read identity and capability properties from that exact serial.
5. Classify the connection only from positive evidence.
6. If transport detection fails, refresh PnP, restart ADB once and repeat the scan.
7. Present a friendly state, verified diagnostics, health score and OEM guide.

The no-device dashboard is the wizard, not an empty device grid. It provides scan, automatic repair
and OEM-guide actions, an animated reduced-motion-aware USB illustration, progress state and
accessible diagnostic results. Long-running process and CIM calls never run on the UI thread.
