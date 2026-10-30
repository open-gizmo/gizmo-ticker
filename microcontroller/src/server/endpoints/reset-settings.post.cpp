#include "../server.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <gizmo/wifi_setup.h>
#include "../../config/config.h"
#include "../../storage/storage.h"

// Clear all saved settings and restart
void setupResetSettingsPostRoute() {
	// POST to clear the settings (a GET could be triggered by any page on the network, for example with an image tag)
	server.on("/api/reset-settings", HTTP_POST, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Clear the settings and the saved network
		const bool isSettingsCleared = clearStoredDeviceConfig();
		const bool isWifiCleared = gizmo::clearWifiCredentials();
		if (!isSettingsCleared || !isWifiCleared) {
			sendErrorResponse(request, 500, "clear_settings_failed", "Failed to clear the settings");
			return;
		}

		// Send response and restart from the main loop (so the response can leave before the reboot)
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();
		data["message"] = "Settings cleared. Restarting...";
		sendSuccessResponse(request, 200, &doc);
		scheduleRestart();
	});

	// OPTIONS request for preflighted requests
	server.on("/api/reset-settings", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
		sendCorsPreflightResponse(request, "POST, OPTIONS");
	});
}