#include "../server.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../config/config.h"
#include "../../eeprom/eeprom.h"

// Clear all saved settings from EEPROM and restart
void setupResetSettingsPostRoute() {
	// POST to clear the settings (a GET could be triggered by any page on the network, for example with an image tag)
	server.on("/api/reset-settings", HTTP_POST, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Clear EEPROM
		if (!clearEEPROM()) {
			sendErrorResponse(request, 500, "clear_settings_failed", "Failed to clear EEPROM");
			return;
		}

		// Send response and restart from the main loop (so the response can leave before the reboot)
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();
		data["message"] = "EEPROM cleared. Restarting...";
		sendSuccessResponse(request, 200, &doc);
		scheduleRestart();
	});

	// OPTIONS request for preflighted requests
	server.on("/api/reset-settings", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
		sendCorsPreflightResponse(request, "POST, OPTIONS");
	});
}