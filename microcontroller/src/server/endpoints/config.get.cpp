#include "../server.h"
#include <ArduinoJson.h>
#include <gizmo/wifi_setup.h>
#include "../settings_json.h"
#include "../../utils/utils.h"

namespace {
	// Build the JSON response for the full device config snapshot
	JsonDocument buildConfigData(const DeviceConfig& config) {
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();

		// Add the display settings
		writeSettingsToJson(data, config);

		// Add the saved network (stored by gizmo-common) and the masked API key
		char ssid[gizmo::WIFI_SSID_SIZE];
		gizmo::getSavedWifiSsid(ssid, sizeof(ssid));
		data["ssid"] = ssid;
		data["apiKey"] = maskInternalChars(config.apiKey);

		// Return the built JSON document
		return doc;
	}
}

// Get the full current device config
void setupConfigGetRoute() {
	server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Get the current config send the JSON responses
		const DeviceConfig& config = getDeviceConfig();
		JsonDocument doc = buildConfigData(config);
		sendSuccessResponse(request, 200, &doc);
	});
}