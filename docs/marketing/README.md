# Marketing Artwork

## Enterprise Banner

- Raster deliverable: `assets/marketing/adb-studio-enterprise-banner.png`
- Editable vector source: `assets/marketing/adb-studio-enterprise-banner.svg`
- Canvas: 7680 × 4320 pixels, 16:9
- Palette: `#050A16`, `#08172F`, `#0A2545`, `#2563EB`, `#06B6D4`, `#22C55E`
- Typography: Segoe UI Variable with Segoe UI and Inter fallbacks

The artwork is an original ADB Studio composition. The former banner was inspected only as a
layout-quality reference; no element was traced or recreated pixel-by-pixel. All brand marks,
device silhouettes, interface panels, charts and icons in the new banner are custom SVG geometry.
No Android robot, Google, Samsung or other third-party branding is present.

The source is maintained as SVG so website, README, documentation, installer, store, splash,
social, press-kit and presentation variants can be exported without quality loss. The canonical
PNG is rendered from the SVG at one device pixel per SVG unit and verified at exactly 7680 × 4320.

When updating the banner:

1. Preserve the exact 16:9 view box and original-artwork policy.
2. Keep all visible text as SVG text rather than rasterized text.
3. Render at 7680 × 4320 with hidden scrollbars and device scale factor 1.
4. Visually inspect the complete composition and validate output dimensions.
5. Regenerate and validate `resources/res.crty`.
