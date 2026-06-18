<div align="center">

# 💰 Gizmo Ticker

### A real-time crypto display for ESP32 and MAX7219

Gizmo Ticker is a two-part project for building a WiFi-connected cryptocurrency display with an ESP32 microcontroller, a MAX7219 LED matrix, and a Nuxt 4 web app for configuration.

<p>
  <a href="https://github.com/open-gizmo/gizmo-ticker"><img alt="GitHub Repo" src="https://img.shields.io/badge/github-open--gizmo%2Fgizmo--ticker-111111?logo=github"></a>
  <img alt="ESP32" src="https://img.shields.io/badge/esp32-ESP32-E7352C?logo=espressif&logoColor=white">
  <img alt="PlatformIO" src="https://img.shields.io/badge/platformio-PlatformIO-FF6B35?logo=platformio&logoColor=white">
  <img alt="Nuxt 4" src="https://img.shields.io/badge/nuxt-4-00DC82?logo=nuxt&logoColor=white">
  <img alt="License MIT" src="https://img.shields.io/badge/license-MIT-2563EB">
</p>

<p>
  📡 WiFi-connected ticker • 📺 MAX7219 LED matrix output • 🎛️ Web-based setup and settings • 🌍 EN/IT UI • ⚡ Lightweight Nuxt UI
</p>

<p>
  <a href="#features"><strong>Features</strong></a> •
  <a href="#architecture"><strong>Architecture</strong></a> •
  <a href="#quick-start"><strong>Quick Start</strong></a> •
  <a href="#firmware-overview"><strong>Firmware Overview</strong></a> •
  <a href="#web-app-overview"><strong>Web App Overview</strong></a> •
  <a href="#build-and-deploy"><strong>Build and Deploy</strong></a>
</p>

</div>

Gizmo Ticker is designed for a simple hardware-to-web workflow: the ESP32 fetches cryptocurrency market data, drives the LED matrix, and exposes a local configuration interface that stays aligned with the firmware settings model.

<a id="features"></a>
## ✨ Features

- Live cryptocurrency data on a MAX7219 LED matrix.
- ESP32 firmware with WiFi, EEPROM-backed settings, and embedded HTTP configuration.
- Web-based configuration for WiFi credentials, API key, and display settings.
- Adjustable scroll speed, selected metrics, and matrix behavior.
- Nuxt 4 interface that mirrors the firmware configuration flow.
- `@brumaombra/ui-vintage` UI system shared across layout, forms, dialogs, and overlays.
- English and Italian localization with browser language detection.
- Captive-portal setup flow in AP mode and `ticker.local` hostname access after joining WiFi.

<a id="architecture"></a>
## 🏗️ Architecture

Gizmo Ticker is built around three cooperating parts:

- 🔌 **ESP32 firmware** handles WiFi, data fetching, settings storage, and the embedded HTTP server.
- 📺 **MAX7219 LED matrix** displays the live crypto ticker output.
- 🌐 **Nuxt web app** provides the browser-based setup and configuration interface with theme, language, and shared UI-system support.

### 🔄 Data Flow

1. The web app sends configuration changes to the ESP32 over HTTP.
2. The firmware stores settings and fetches cryptocurrency market data from external APIs.
3. The matrix renders the current values as scrolling text.

<a id="quick-start"></a>
## 🚀 Quick Start

### 📋 Requirements

- ESP32 board, such as an ESP32 Dev Module or NodeMCU-32S.
- MAX7219 LED matrix module.
- Jumper wires and a stable power supply.
- VS Code with PlatformIO installed.
- A WiFi network and a cryptocurrency market data API key.

### 🔧 Initial Setup

1. Open the repository in VS Code.
2. Install the PlatformIO extension if it is not already installed.
3. Connect the ESP32 and MAX7219 matrix according to the pinout used in the firmware.
4. Build and upload the firmware from the `microcontroller/` project.
5. Connect to its setup access point when it starts for the first time.
6. Enter WiFi credentials and the API key through the web interface.

### 🔄 Configuration Flow

1. The device creates a temporary access point on first boot.
2. Connect to the AP and open the device IP address in a browser.
3. Save WiFi credentials so the ESP32 can join your local network.
4. Save the API key used for market data requests.
5. Adjust display settings such as visible metrics and scroll speed.
6. Reboot or let the device refresh settings as needed.

<a id="firmware-overview"></a>
## 🔌 Firmware Overview

The firmware is responsible for the runtime behavior of the physical ticker.

- Connects the ESP32 to WiFi.
- Persists settings in EEPROM-backed local storage.
- Hosts the embedded HTTP server.
- Fetches and parses cryptocurrency market data.
- Renders scrolling text to the MAX7219 matrix.
- Serves the embedded web UI assets.
- Keeps AP setup available during onboarding and exposes `ticker.local` on supported networks after joining WiFi.

Key firmware areas live under `microcontroller/src/`:

- `api/` for data fetching and parsing.
- `config/` for settings and validation.
- `matrix/` for LED output.
- `server/` for HTTP routes.
- `eeprom/` for EEPROM persistence.
- `wifi/` for network connection logic.

<a id="web-app-overview"></a>
## 🌐 Web App Overview

The Nuxt app provides the browser-based device configuration interface.

- Mirrors the firmware settings model.
- Lets users configure WiFi and API credentials.
- Controls display behavior and matrix preferences.
- Supports English and Italian localization.
- Uses the `@brumaombra/ui-vintage` package for the shared design system.
- Includes library-managed busy, confirm, and message flows for critical actions.
- Keeps device API helpers, theme state, and crypto-accent logic in `webapp/app/composables/`.
- Uses static generation for the embedded web UI bundle that is packaged into firmware assets.

### 🎨 UI System

The web app now splits UI responsibilities between the shared package and the app itself.

- `@brumaombra/ui-vintage` provides the reusable UI system.
- `webapp/app/components/` contains app-specific pieces such as branding, crypto logos, language/theme selectors, and page intro wrappers.
- `webapp/app/layouts/private.vue` composes the shared `DashboardShell` with app-defined navigation, branding, and toolbar controls.
- `webapp/app/assets/main.css` imports the shared library stylesheet and then layers the app-specific crypto accent tokens on top.
- `webapp/app/composables/useUtils.js` and `webapp/app/composables/stores/useGlobalStore.js` keep theme mode, crypto coin selection, and device-backed UI state local to the app.

Important web app areas:

- `webapp/app/pages/` for the main setup screens.
- `@brumaombra/ui-vintage` for shared layout, form, dialog, and overlay primitives.
- `webapp/app/components/` for app-specific presentation components and selectors.
- `webapp/app/assets/main.css` for shared-style import and app-level design tokens.
- `webapp/app/layouts/private.vue` for the dashboard shell adapter.
- `webapp/app/composables/useDeviceApi.js` for device communication.
- `webapp/app/composables/useUtils.js` for theme, crypto accent, dialog, and app helpers.
- `webapp/app/composables/stores/useGlobalStore.js` for shared device/UI state.
- `webapp/i18n/locales/` for translations.

<a id="build-and-deploy"></a>
## 🛠️ Build and Deploy

### 🔌 Firmware

- Run `npm run build:embedded` from `webapp/` when the embedded UI changes.
- Open `microcontroller/` in PlatformIO.
- Build and upload the firmware.

### 🌐 Web App

- Open `webapp/` as the Nuxt app workspace.
- Install dependencies with `npm install`.
- Run `npm run dev` for local work.
- Run `npm run build` for a production static build.
- Run `npm run build:embedded` to export the embedded web UI bundle used by the firmware.
- The web app consumes the `@brumaombra/ui-vintage` package.

### 📦 Embedded Web UI

The repository includes an embedded UI export path so the web app can be packaged into firmware assets when needed.

- Source: `webapp/`.
- Embedded docs: `docs/embedded-webui.md`.
- Export script: `webapp/scripts/export-embedded-static.js`.

## 📄 License

MIT