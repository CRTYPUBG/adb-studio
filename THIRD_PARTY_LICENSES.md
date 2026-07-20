# Third-Party Licenses

| Component | Version | License | Role |
|---|---:|---|---|
| Qt | 6.8.1 | LGPL-3.0-only / commercial | Application and FluentWinUI3 UI runtime |
| scrcpy | 4.0 | Apache-2.0 | Isolated mirroring process and source reference |
| Android platform-tools / ADB | bundled version pending verification | Android SDK license | Device transport executable |
| FFmpeg libraries | bundled with scrcpy | Component-specific LGPL/GPL configuration | scrcpy media runtime |
| SDL | bundled with scrcpy | Zlib | scrcpy window/input runtime |
| libusb | bundled with scrcpy | LGPL-2.1-or-later | scrcpy USB runtime |

Release automation must replace unresolved bundled versions with inspected binary metadata and
generate complete notices before distribution. Unknown or incompatible licenses block release.
