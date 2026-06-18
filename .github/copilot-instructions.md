# Copilot Instructions

## What This Project Is

Bitcoin Ticker is a two-part project for an ESP8266-based LED matrix Bitcoin display.

- `microcontroller/` contains the PlatformIO firmware for a NodeMCU ESP8266. It manages WiFi, local storage, an embedded HTTP server, API requests, and MAX7219 LED matrix output.
- `webapp/` contains the Nuxt 4 configuration UI used to set WiFi credentials, API key, and display settings for the device.

The firmware is the runtime for the physical ticker. The webapp is a separate frontend that mirrors the device configuration flow and should stay aligned with the firmware API and settings model.

## Core Concepts

- Firmware code is organized by responsibility under `microcontroller/src/` (`api`, `config`, `matrix`, `serial`, `server`, `storage`, `utils`, `wifi`).
- The ESP8266 uses Arduino framework libraries plus LittleFS-backed persistence and an async web server.
- The web interface is a Nuxt app under `webapp/src/` with file-based pages, shared components, and composables for device API access and UI state.
- Changes to configuration fields or API payloads usually require coordinated updates in both firmware and webapp.

## Working Rules

1. Keep the split between firmware and webapp clear. Do not mix embedded-device logic into the frontend or frontend-specific behavior into firmware modules.
2. Preserve existing module boundaries in `microcontroller/src/`; prefer extending the relevant subsystem instead of adding cross-cutting logic to `main.cpp`.
3. When changing settings, routes, payloads, or validation rules, verify the firmware handlers and the Nuxt UI still match.
4. Keep the webapp lightweight and compatible with the current Nuxt 4 stack already in the repo.
5. Prefer minimal, hardware-aware changes in firmware: avoid unnecessary dynamic allocation, blocking behavior, and broad refactors unless they are required.