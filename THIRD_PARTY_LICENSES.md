# Third-Party Licenses

| Component | Version | License | Role |
|---|---:|---|---|
| Qt | 6.8.1 | LGPL-3.0-only / commercial | Application and FluentWinUI3 UI runtime |
| Microsoft Visual C++ Runtime | 14.44 | Microsoft Visual C++ Redistributable terms | Packaged runtime |
| scrcpy | 4.0 | Apache-2.0 | Planned isolated process; not distributed in milestone 0.1.0 |
| Android platform-tools / ADB | Not distributed | Android SDK license | Planned device transport |
| FFmpeg libraries | Not distributed | Component-specific LGPL/GPL configuration | Planned scrcpy media runtime |
| SDL | Not distributed | Zlib | Planned scrcpy window/input runtime |
| libusb | Not distributed | LGPL-2.1-or-later | Planned scrcpy USB runtime |

Release automation generates complete notices and SBOM evidence for files actually distributed.
Unknown, unresolved or incompatible licenses block release.
