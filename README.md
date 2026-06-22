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
	<a href="#overview"><strong>Overview</strong></a> •
	<a href="#features"><strong>Features</strong></a> •
	<a href="#architecture"><strong>Architecture</strong></a> •
	<a href="#quick-start"><strong>Quick Start</strong></a> •
	<a href="#hardware"><strong>Hardware</strong></a> •
	<a href="#firmware-overview"><strong>Firmware Overview</strong></a> •
	<a href="#configuration"><strong>Configuration</strong></a> •
	<a href="#build-and-deploy"><strong>Build and Deploy</strong></a> •
	<a href="#troubleshooting"><strong>Troubleshooting</strong></a> •
	<a href="#license"><strong>License</strong></a>
</p>

</div>

<a id="overview"></a>
## 📘 Overview

Gizmo Ticker is built for a very specific kind of maker project: a small always-on crypto display that feels more like a compact appliance than a demo. The ESP32 keeps pulling live market data, the MAX7219 matrix turns it into something visible and immediate, and the Nuxt app gives you a clean place to configure the device without needing to reflash firmware for every adjustment.

The result is a practical two-part system that is easy to set up, easy to tweak, and easy to leave running on a desk, shelf, or workbench. You can use it as a daily crypto glance board, a weekend hardware project, or a template for building other connected displays with the same firmware-plus-web-app workflow.

Behind the scenes, the firmware and web app stay aligned through the same settings model, so changes made in the browser match what the ESP32 expects at runtime. That keeps the experience predictable: the web UI configures the device, the firmware fetches and renders the values, and the matrix shows the current data in a simple, readable format.

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

<a id="hardware"></a>
## 🔌 Hardware

### Pinout

Current firmware pin assignments:

- MAX7219 CLK: `GPIO18`
- MAX7219 DATA: `GPIO23`
- MAX7219 CS: `GPIO5`

### Display

- Type: `MAX7219 LED matrix`
- Hardware layout: `FC16_HW`
- Maximum modules: `16`

<a id="firmware-overview"></a>
## 🔍 Firmware Overview

### ⚙️ General

The firmware is responsible for the runtime behavior of the physical ticker.

- Connects the ESP32 to WiFi.
- Persists settings in EEPROM-backed local storage.
- Hosts the embedded HTTP server.
- Fetches and parses cryptocurrency market data.
- Renders scrolling text to the MAX7219 matrix.
- Serves the embedded web UI assets.
- Keeps AP setup available during onboarding and exposes `ticker.local` on supported networks after joining WiFi.

### 🧩 Firmware Layers

The firmware is split into focused modules under `microcontroller/src/`:

- 📡 `api/` handles market data fetching and parsing.
- ⚙️ `config/` centralizes settings, defaults, and validation.
- 📺 `matrix/` manages LED output and scrolling text.
- 🌐 `server/` exposes the embedded HTTP routes.
- 💾 `eeprom/` stores settings in persistent memory.
- 🔌 `wifi/` handles network connection logic.

### 🌐 Web App Overview

The Nuxt app provides the browser-based device configuration interface.

- Mirrors the firmware settings model.
- Lets users configure WiFi and API credentials.
- Controls display behavior and matrix preferences.
- Supports English and Italian localization.
- Uses the `@brumaombra/ui-vintage` package for the shared design system.
- Includes library-managed busy, confirm, and message flows for critical actions.
- Keeps device API helpers, theme state, and crypto-accent logic in `webapp/app/composables/`.
- Uses static generation for the embedded web UI bundle that is packaged into firmware assets.

#### 🎨 UI System

The web app now splits UI responsibilities between the shared package and the app itself.

- `@brumaombra/ui-vintage` provides the reusable UI system.
- `webapp/app/components/` contains app-specific pieces such as branding, crypto logos, language/theme selectors, and page intro wrappers.
- `webapp/app/layouts/private.vue` composes the shared `DashboardShell` with app-defined navigation, branding, and toolbar controls.
- `webapp/app/assets/main.css` imports the shared library stylesheet and then layers the app-specific crypto accent tokens on top.
- `webapp/app/composables/useUtils.js` and `webapp/app/composables/stores/useGlobalStore.js` keep theme mode, crypto coin selection, and device-backed UI state local to the app.

<a id="configuration"></a>
## ⚙️ Configuration

### Device Settings

The ticker keeps its runtime settings in a shared `DeviceConfig` model defined in `microcontroller/src/config/config.h` and initialized in `microcontroller/src/config/config.cpp`.

The main adjustable values are:

- which market sections are visible on the matrix
- the selected crypto coin
- the number formatting style (`US` or `EU`)
- matrix intensity
- scroll speed
- WiFi credentials
- API key for market data requests

### Firmware Defaults

The firmware starts with a practical default configuration:

- all market sections enabled
- `bitcoin` as the default coin
- US-style number formatting
- a moderate matrix intensity
- a readable scroll speed for the LED matrix

### Runtime Flow

The device is configured through its embedded web UI and local network flow.

1. The ESP32 starts in access point mode on first boot.
2. The web app lets you edit the ticker settings in the browser.
3. The firmware stores the settings locally and reuses them after reboot.
4. The device can then be reached on the local network using `ticker.local` on supported setups.

### Key Files

- `microcontroller/src/config/config.h` for the device config structure and hardware constants.
- `microcontroller/src/config/config.cpp` for defaults and runtime state.
- `webapp/app/composables/stores/useGlobalStore.js` for shared UI and device-backed state.
- `webapp/app/composables/useDeviceApi.js` for browser-to-device communication.

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

<a id="troubleshooting"></a>
## 🧰 Troubleshooting

- The LED matrix powers on but shows no text: verify the MAX7219 wiring, confirm the correct `GPIO18` / `GPIO23` / `GPIO5` connections, and check that the matrix is using the `FC16_HW` layout.
- The web UI does not open after joining WiFi: make sure the ESP32 is connected to the same network and try the `ticker.local` hostname if your router supports mDNS.
- The ticker does not update with new prices: confirm the WiFi credentials and the market data API key are saved correctly in the setup flow.
- The display scrolls too fast or too slow: adjust the scroll speed in the web configuration and rebuild the embedded assets if needed.
- The AP setup flow never appears: power-cycle the device and check the firmware logs for WiFi connection errors or saved configuration issues.

<a id="license"></a>
## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

<p align="center">
	<img src="https://raw.githubusercontent.com/open-gizmo/.github/main/profile/images/open-gizmo-logo-with-text.min.png" alt="OpenGizmo" width="200">
	<br/>
	<strong>Real projects. Real hardware. Real fun.</strong>
	<br/>
	<sub>A small corner of the internet for ESP32 builds that are useful, remixable, and actually fun to use.</sub>
</p>