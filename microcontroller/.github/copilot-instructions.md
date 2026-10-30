# Project Overview

This repository contains ESP32 firmware for a Bitcoin ticker built around an LED matrix display and an embedded web UI.

## Structure

- `src/main.cpp`: entry point; initializes serial logging, settings storage, Wi-Fi, the web server, the HTTP client, and the LED matrix.
- `src/config`: shared hardware constants, global buffers, enums, and runtime state.
- `src/api`: fetches BTC market data from the external API and formats the scrolling ticker messages.
- `src/wifi`: passes the ticker's names to the gizmo-common Wi-Fi setup (connection, access point fallback, captive portal) and shows its status on the matrix.
- `src/server`: exposes the AsyncWebServer routes for API key updates, settings, and embedded assets (the Wi-Fi setup routes come from gizmo-common).
- `src/storage`: persists the display settings and API key with `Preferences` (gizmo-common stores the Wi-Fi credentials).
- `src/matrix`: controls the MD_MAX72xx / MD_Parola LED matrix and cycles the visible ticker sections.
- `src/serial`: serial debug logging helpers.
- `src/utils`: shared helpers for formatting and string handling.
- `src/webui`: generated assets for the embedded front-end.

## Runtime Flow

The firmware boots, loads saved settings, starts the server, connects to Wi-Fi or enables access point mode, fetches ticker data on a timed interval, and scrolls the enabled market sections across the matrix.