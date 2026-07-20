# Dependency Report

The current first-party application directly depends on Qt 6.8.1. scrcpy 4.0 and its packaged ADB,
FFmpeg, SDL and libusb components are retained as isolated third-party inputs and are not linked into
the ADB Studio executable. CI generates SPDX/CycloneDX evidence and rejects unapproved licenses or
known critical vulnerabilities before release.
