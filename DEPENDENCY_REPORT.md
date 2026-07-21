# Dependency Report

The current first-party application directly depends on Qt 6.8.1 and distributes the Microsoft
Visual C++ 14.44 runtime. scrcpy 4.0, ADB, FFmpeg, SDL and libusb are planned isolated inputs and are
not shipped in milestone 0.1.0. CI generates SPDX/CycloneDX evidence and rejects unresolved licenses,
tracked certificate/private-key material or known critical vulnerabilities before release.
