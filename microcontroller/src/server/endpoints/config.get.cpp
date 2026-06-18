#include "../server.h"
#include <ArduinoJson.h>
#include "../../eeprom/eeprom.h"
#include "../../utils/utils.h"

namespace {
	// Build the JSON response for the full device config snapshot
	JsonDocument buildConfigData(const DeviceConfig& config) {
        // Build the JSON response
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();

        // Add all config fields to the response
		data["currentPrice"] = config.currentPriceVisible;
		data["priceChange"] = config.priceChangeVisible;
		data["marketCap"] = config.marketCapVisible;
		data["dailyHighLow"] = config.dailyHighLowVisible;
		data["yearHighLow"] = config.yearHighLowVisible;
		data["openPrice"] = config.openPriceVisible;
		data["volume"] = config.volumeVisible;
		data["cryptoCoin"] = config.cryptoCoin;
		data["formatType"] = config.formatType == FORMAT_US ? "US" : "EU";
		data["matrixIntensity"] = config.matrixIntensity;
		data["scrollSpeed"] = config.scrollSpeed;
		data["ssid"] = config.ssid;
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