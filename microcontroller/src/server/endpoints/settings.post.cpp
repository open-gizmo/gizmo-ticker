#include "../server.h"
#include <ArduinoJson.h>
#include <cstring>
#include "../../config/config.h"
#include "../../storage/storage.h"
#include "../settings_json.h"
#include "../../matrix/matrix.h"
#include "../../serial/serial.h"

namespace {
	constexpr uint8_t MIN_SETTINGS_LEVEL = 0;
	constexpr uint8_t MAX_SETTINGS_LEVEL = 15;

	// Validate the boolean fields
	bool validateBooleanField(AsyncWebServerRequest* request, JsonVariantConst value, const char* fieldName) {
		// Null is allowed
		if (value.isNull()) {
			return true;
		}

		// If not null, must be a boolean
		if (!value.is<bool>()) {
			printLogfln("Invalid boolean value for %s", fieldName);
			sendErrorResponse(request, 400, "invalid_settings_field", fieldName);
			return false;
		}

		// Valid boolean value
		return true;
	}

	// Validate the numeric fields
	bool validateLevelField(AsyncWebServerRequest* request, JsonVariantConst value, const char* fieldName) {
		// Null is allowed
		if (value.isNull()) {
			return true;
		}

		// If not null, must be an integer
		if (!value.is<int>()) {
			printLogfln("Invalid numeric value for %s", fieldName);
			sendErrorResponse(request, 400, "invalid_settings_field", fieldName);
			return false;
		}

		// If integer, must be within the allowed range
		const int resolvedValue = value.as<int>();
		if (resolvedValue < MIN_SETTINGS_LEVEL || resolvedValue > MAX_SETTINGS_LEVEL) {
			printLogfln("Out-of-range value for %s: %d", fieldName, resolvedValue);
			sendErrorResponse(request, 400, "invalid_settings_field", fieldName);
			return false;
		}

		// Valid numeric value
		return true;
	}

	// Validate the cryptoCoin field
	bool validateCryptoCoinField(AsyncWebServerRequest* request, JsonVariantConst value) {
		// Null is allowed
		if (value.isNull()) {
			return true;
		}

		// If not null, must be a string
		if (!value.is<const char*>()) {
			printLogfln("Invalid cryptoCoin type");
			sendErrorResponse(request, 400, "invalid_settings_field", "cryptoCoin");
			return false;
		}

		// If string, must be either "bitcoin" or "kaspa"
		const char* cryptoCoinValue = value.as<const char*>();
		if (strcmp(cryptoCoinValue, "bitcoin") != 0 && strcmp(cryptoCoinValue, "kaspa") != 0) {
			printLogfln("Unsupported cryptoCoin: %s", cryptoCoinValue);
			sendErrorResponse(request, 400, "invalid_settings_field", "cryptoCoin");
			return false;
		}

		// Valid cryptoCoin value
		return true;
	}

	// Validate the formatType field
	bool validateFormatTypeField(AsyncWebServerRequest* request, JsonVariantConst value) {
		// Null is allowed
		if (value.isNull()) {
			return true;
		}

		// If not null, must be a string
		if (!value.is<const char*>()) {
			printLogfln("Invalid formatType type");
			sendErrorResponse(request, 400, "invalid_settings_field", "formatType");
			return false;
		}

		// If string, must be either "US" or "EU"
		const char* formatTypeValue = value.as<const char*>();
		if (strcmp(formatTypeValue, "US") != 0 && strcmp(formatTypeValue, "EU") != 0) {
			printLogfln("Unsupported formatType: %s", formatTypeValue);
			sendErrorResponse(request, 400, "invalid_settings_field", "formatType");
			return false;
		}

		// Valid formatType value
		return true;
	}
}

// Save the settings
void setupSettingsPostRoute() {
	// POST to update the settings
	server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		// Read the JSON body
		JsonDocument doc;
		if (!gizmo::readJsonBody(request, data, len, index, total, doc)) {
			return;
		}

		// Validate each field
		if (!validateBooleanField(request, doc["currentPrice"], "currentPrice")
			|| !validateBooleanField(request, doc["priceChange"], "priceChange")
			|| !validateBooleanField(request, doc["marketCap"], "marketCap")
			|| !validateBooleanField(request, doc["dailyHighLow"], "dailyHighLow")
			|| !validateBooleanField(request, doc["yearHighLow"], "yearHighLow")
			|| !validateBooleanField(request, doc["openPrice"], "openPrice")
			|| !validateBooleanField(request, doc["volume"], "volume")
			|| !validateCryptoCoinField(request, doc["cryptoCoin"])
			|| !validateFormatTypeField(request, doc["formatType"])
			|| !validateLevelField(request, doc["matrixIntensity"], "matrixIntensity")
			|| !validateLevelField(request, doc["scrollSpeed"], "scrollSpeed")) {
			return;
		}

		// Get the current config
		DeviceConfig config = getDeviceConfig();

		// Update the settings with the new values
		applySettingsFromJson(doc.as<JsonObjectConst>(), config);

		// Persist the changes before mutating runtime state
		if (!saveDeviceConfigToStorage(config)) {
			sendErrorResponse(request, 500, "settings_save_failed", "Failed to save settings");
			return;
		}

		// Apply changes
		setDeviceConfig(config);
		timestampStockData = 0;
		queueMatrixIntensityChange(); // Applied by the main loop, which owns the matrix
		setMatrixSpeed(config.scrollSpeed);

		// Send success response
		sendSuccessResponse(request);
		printLogfln("Values settings changed");
	});

	// OPTIONS request for preflighted requests
	server.on("/api/settings", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
		sendCorsPreflightResponse(request, "POST, OPTIONS");
	});
}