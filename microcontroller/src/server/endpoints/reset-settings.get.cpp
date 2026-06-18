#include "../server.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../config/config.h"
#include "../../eeprom/eeprom.h"

// Clear all saved settings from EEPROM and restart
void setupResetSettingsGetRoute() {
	server.on("/api/reset-settings", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Clear EEPROM
		if (!clearEEPROM()) {
			sendErrorResponse(request, 500, "clear_settings_failed", "Failed to clear EEPROM");
			return;
		}

		// Send response and restart
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();
		data["message"] = "EEPROM cleared. Restarting...";
		sendSuccessResponse(request, 200, &doc);
		delay(500);
		ESP.restart();
	});
}