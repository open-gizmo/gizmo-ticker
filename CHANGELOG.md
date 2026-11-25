# Changelog

What changed in each release. New notes go under **Unreleased** while the work is done; a release moves them under its version, and the release workflow publishes that section as the GitHub release notes (it refuses to release a version without them).

## Unreleased

## v1.3.1 - 2026-11-25

- gizmo-common v2.7.0: when the device sends an answer the web app can't read, it now shows an error ("The device sent an answer the app could not read.") instead of carrying on with empty values. Flash the new firmware to get it.
- gizmo-common v2.7.1: the web app writes "Wi-Fi" instead of "WiFi", on the tab and on the Wi-Fi setup page. Flash the new firmware to get it.

## v1.3.0 - 2026-11-18

- The ticker describes itself to AI agents: a Markdown guide at `http://gizmo-ticker.local/llms.txt` explains what it shows and how to read its settings, with its address filled in. Flash the new firmware to get it.
- New enclosure look: round screw ears at the corners that flow into the body with soft curves, like the other OpenGizmo gadgets, and a thinner 6 mm frame around the LEDs. It's a bit bigger (157.5 x 57.5 mm instead of 153.6 x 53.6) and stands on its two bottom ears instead of the rails. The four M3 x 8 screws go through the ears, outside the body, and sit flush in the back plate. The parts inside and the wiring are the same, so there's nothing to flash or rewire: print the new shell and back plate only if you want the new look.
- gizmo-common v2.6.0: the Wi-Fi setup page can no longer report a wrong password by mistake right after you send one, and the Wi-Fi status stops saying "connected" as soon as the network drops. Flash the new firmware to get it.

## v1.2.1 - 2026-11-14

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
