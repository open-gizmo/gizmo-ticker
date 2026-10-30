#include "wifi.h"
#include <Arduino.h>
#include <gizmo/wifi_setup.h>
#include "../config/config.h"
#include "../matrix/matrix.h"
#include "../serial/serial.h"

/*
The Wi-Fi connection, the setup hotspot and its web API come from gizmo-common.
This module only gives them the ticker's names and shows the status on the matrix.
*/

namespace {
	// Wi-Fi setup options of the ticker
	const gizmo::WifiSetupOptions WIFI_SETUP_OPTIONS = {
		"Gizmo-Ticker", // Setup hotspot name
		"ticker", // mDNS hostname (http://ticker.local)
		"/ap-setup", // Setup page of the web app
		"/_nuxt/" // Files of the web app, never redirected
	};

	// Choose the matrix message for the current Wi-Fi state (string literals, so the pointer stays valid)
	const char* getWiFiStatusMessage() {
		if (gizmo::getWifiStatus() == gizmo::WifiStatus::Connecting) return "Connecting to Wi-Fi...";
		return "Not connected to Wi-Fi. Use the 'Gizmo-Ticker' access point to enter the Wi-Fi credentials.";
	}
}

// Start the Wi-Fi setup and register its web API (call before the web server starts)
void setupWiFi() {
	gizmo::setupWifi(server, WIFI_SETUP_OPTIONS);
}

// Manage the Wi-Fi connection and the setup hotspot
void manageWiFiConnection() {
	gizmo::manageWifi();
}

// Check if connected to Wi-Fi, showing the status on the matrix otherwise
bool checkWifiConnection() {
	// If not connected, show a notice on the matrix
	if (!gizmo::isWifiConnected()) {
		const char* errorMessage = getWiFiStatusMessage();
		printLogfln("%s", errorMessage);
		printOnLedMatrix(errorMessage, strlen(errorMessage) + 1); // Print the message on the matrix

		// Not connected
		return false;
	}

	// Connected
	return true;
}