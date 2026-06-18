#include "../server.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include "../../config/config.h"

namespace {
	// Create the JSON payload for the check connection endpoint
	JsonDocument getCheckConnectionData() {
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();

		// Add the Wi-Fi connection status to the JSON response
		data["connectionStatus"] = static_cast<int>(wiFiConnectionStatus);

		// If connected, add the SSID and IP address to the JSON response
		if (wiFiConnectionStatus == WIFI_OK && WiFi.status() == WL_CONNECTED) {
			data["ssid"] = WiFi.SSID();
			data["ip"] = WiFi.localIP().toString();
			data["hostname"] = String(mdnsHostname) + ".local";
		}

		// Return the JSON document
		return doc;
	}
}

// Check Wi-Fi connection status
void setupCheckConnectionGetRoute() {
	server.on("/api/check-connection", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Create the JSON payload
		JsonDocument doc = getCheckConnectionData();

		// Once connected, mark the access point for shutdown
		if (wiFiConnectionStatus == WIFI_OK) {
			disableAccessPoint = true;
		}

		// Send the JSON response
		sendSuccessResponse(request, 200, &doc);
	});
}