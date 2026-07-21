# USB Recovery

The automatic recovery sequence is bounded and idempotent:

1. Preserve the initial measured facts.
2. Run Windows `pnputil /scan-devices` when available.
3. Run `adb kill-server`, clearing stale transports.
4. Run `adb start-server` with a timeout.
5. Run bounded `adb reconnect offline` and `adb reconnect device` requests. These refresh stale
   transports but never approve the device's RSA prompt.
6. Re-enumerate ADB and Fastboot transports.
7. Replace the report only with newly measured results.

Physical reconnect, cable replacement, RSA approval, driver installation and USB-mode changes
require the user. The wizard explains those actions rather than pretending they were completed.
