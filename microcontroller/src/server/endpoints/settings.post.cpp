#include "../server.h"
#include <ArduinoJson.h>
#include <cstring>
#include "../../config/config.h"
#include "../../utils/utils.h"
#include "../../eeprom/eeprom.h"
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
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Accumulate the request body
		const RequestBodyResult body = accumulateRequestBody(request, data, len, index, total);
		if (body.state == RequestBodyState::InProgress) {
			return;
		}

		// If error, send error response
		if (body.state == RequestBodyState::Error) {
			sendErrorResponse(request, 500, "request_body_read_failed", "Failed to read request body");
			return;
		}
		
		// Parse the JSON object and release the request body memory
		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, body.data, body.length + 1);
		releaseRequestBody(request);

		// If error, send error response
		if (error) {
			sendErrorResponse(request, 400, "invalid_json", "Invalid JSON");
			return;
		}

		// Validate that the root is a JSON object
		if (!doc.is<JsonObject>()) {
			sendErrorResponse(request, 400, "invalid_json", "Invalid JSON object");
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
		if (!doc["currentPrice"].isNull())
			config.currentPriceVisible = doc["currentPrice"].as<bool>();
		if (!doc["priceChange"].isNull())
			config.priceChangeVisible = doc["priceChange"].as<bool>();
		if (!doc["marketCap"].isNull())
			config.marketCapVisible = doc["marketCap"].as<bool>();
		if (!doc["dailyHighLow"].isNull())
			config.dailyHighLowVisible = doc["dailyHighLow"].as<bool>();
		if (!doc["yearHighLow"].isNull())
			config.yearHighLowVisible = doc["yearHighLow"].as<bool>();
		if (!doc["openPrice"].isNull())
			config.openPriceVisible = doc["openPrice"].as<bool>();
		if (!doc["volume"].isNull())
			config.volumeVisible = doc["volume"].as<bool>();
		if (!doc["cryptoCoin"].isNull())
			stringCopy(config.cryptoCoin, doc["cryptoCoin"].as<const char*>(), sizeof(config.cryptoCoin));
		if (!doc["formatType"].isNull())
			config.formatType = strcmp(doc["formatType"].as<const char*>(), "US") == 0 ? FORMAT_US : FORMAT_EU;
		if (!doc["matrixIntensity"].isNull())
			config.matrixIntensity = doc["matrixIntensity"].as<uint8_t>();
		if (!doc["scrollSpeed"].isNull())
			config.scrollSpeed = doc["scrollSpeed"].as<uint8_t>();

		// Persist the changes before mutating runtime state
		if (!saveDeviceConfigToEEPROM(config)) {
			sendErrorResponse(request, 500, "settings_save_failed", "Failed to save settings");
			return;
		}

		// Apply changes
		setDeviceConfig(config);
		timestampStockData = 0;
		setMatrixIntensity(config.matrixIntensity);
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