# Changelog

What changed in each release. New notes go under **Unreleased** while the work is done; a release moves them under its version, and the release workflow publishes that section as the GitHub release notes (it refuses to release a version without them).

## Unreleased

- The **Device** tab shows the address that opens the device (`gizmo-ticker.local`). It showed the Wi-Fi's default name (`esp32s3-XXXXXX.local`), which doesn't open it.
- The router lists the device by its name (`gizmo-ticker`) instead of `esp32s3-XXXXXX`.
- gizmo-common v2.5.2.

## v1.2.0 - 2026-11-13

- A new Gizmo Ticker logo, a ₿ lit on an LED matrix, as the favicon and the phone's home-screen icon. The header shows it in both themes, whatever the coin (the coin still sets the accent color).
- The web app opens on the new **Ticker** page, with what the matrix shows (the visible values, the coin and number format, brightness and speed), instead of the WiFi page.
- The tabs follow the same order as every OpenGizmo app: Ticker, API Key, Device, WiFi.
- The **Device** tab (it was Settings) has the device details, the network, Restart, Forget network and Reset saved settings. Its first group is now **Info**.
- gizmo-common v2.5.1 and ui-vintage 0.9.51.

Flash it again from the website to get the new web app: the saved settings and Wi-Fi are kept.

## Earlier versions

See the [GitHub releases](https://github.com/open-gizmo/gizmo-ticker/releases) up to v1.1.0.
