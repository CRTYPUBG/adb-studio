# Device Detection

ADB Studio uses measured evidence from Android platform-tools, Fastboot and Windows PnP/CIM. A
missing measurement remains `UNKNOWN`; absence from `adb devices` alone is never treated as proof
that USB debugging or a driver is disabled.

| State key | Meaning | Primary next action |
|---|---|---|
| `NO_USB_DEVICE` | Hardware inventory positively reports no Android USB device | Connect an unlocked device with a data cable |
| `USB_CHARGING_ONLY` | Measured USB configuration exposes no data function | Select File Transfer/recommended USB mode |
| `USB_FILE_TRANSFER` | MTP data path exists without verified ADB authorization | Enable debugging and authorize |
| `USB_DEBUGGING_DISABLED` | Measured device USB configuration omits ADB | Enable USB debugging |
| `AUTHORIZATION_PENDING` | An authorization decision is pending on-device | Unlock and review the RSA fingerprint |
| `UNAUTHORIZED` | ADB explicitly returned `unauthorized` | Approve the fingerprint prompt |
| `OFFLINE` | ADB explicitly returned `offline` | Reconnect, recover ADB, rescan |
| `ADB_SERVER_NOT_RUNNING` | Platform-tools are absent or the server failed | Install or restart official platform-tools |
| `ADB_VERSION_MISMATCH` | Client/server mismatch text was measured | Remove stale ADB processes/installations |
| `DEVICE_BUSY` | A measured operation owns the required device resource | Wait, then retry |
| `DRIVER_MISSING` | Windows sees an Android interface with an unhealthy driver | Open the OEM/Windows driver guide |
| `MTP_DRIVER_MISSING` | Windows sees an unhealthy MTP interface | Repair MTP and reconnect |
| `FASTBOOT_MODE` | `fastboot devices` positively identifies the device | Boot Android unless Fastboot is intentional |
| `RECOVERY_MODE` | ADB explicitly reports Recovery | Restart into Android |
| `UNSUPPORTED_DEVICE` | A required capability is positively unavailable | Review compatibility results |
| `WIRELESS_AVAILABLE` | Wireless debugging is measured as enabled | Pair the device |
| `WIRELESS_CONNECTED` | The active ADB serial is a network endpoint | Benchmark latency/throughput |
| `MULTIPLE_DEVICES` | More than one ADB transport is returned | Select a serial explicitly |
| `CONNECTED` | One authorized `device` transport is verified | Review health/recommendations |
| `UNKNOWN` | Evidence cannot isolate one cause | Follow individual diagnostics; do not guess |

Detection runs on a worker thread. Authorized-device properties populate manufacturer, brand,
model, Android version, API level, build number and measured OEM skin properties. Raw ADB stderr is
never shown in the UI.
