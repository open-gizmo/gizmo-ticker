# Project Overview

This repository contains ESP32 firmware for a Bitcoin ticker built around an LED matrix display and an embedded web UI.

## Structure

- `src/main.cpp`: entry point; initializes serial logging, EEPROM storage, Wi-Fi, the web server, the HTTP client, and the LED matrix.
- `src/config`: shared hardware constants, global buffers, enums, and runtime state.
- `src/api`: fetches BTC market data from the external API and formats the scrolling ticker messages.
- `src/wifi`: manages Wi-Fi connection attempts, access point fallback, and connection status checks.
- `src/server`: exposes the AsyncWebServer routes for Wi-Fi setup, API key updates, settings, and embedded assets.
- `src/storage`: persists credentials and display settings in EEPROM.
- `src/matrix`: controls the MD_MAX72xx / MD_Parola LED matrix and cycles the visible ticker sections.
- `src/serial`: serial debug logging helpers.
- `src/utils`: shared helpers for formatting and string handling.
- `src/webui`: generated assets for the embedded front-end.

## Runtime Flow

The firmware boots, loads saved settings, starts the server, connects to Wi-Fi or enables access point mode, fetches ticker data on a timed interval, and scrolls the enabled market sections across the matrix.