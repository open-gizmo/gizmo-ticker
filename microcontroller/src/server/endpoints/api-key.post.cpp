#include "../server.h"
#include <ArduinoJson.h>
#include <cstring>
#include "../../config/config.h"
#include "../../utils/utils.h"
#include "../../eeprom/eeprom.h"
#include "../../serial/serial.h"

namespace {
	constexpr size_t MAX_API_KEY_LENGTH = API_KEY_SIZE - 1;

	// Validate the apiKey field
	bool validateApiKeyField(AsyncWebServerRequest* request, JsonVariantConst value) {
		// Null is not allowed
		if (value.isNull()) {
			sendErrorResponse(request, 400, "missing_api_key", "Missing apiKey field");
			return false;
		}

		// Must be a string
		if (!value.is<const char*>()) {
			printLogfln("Invalid apiKey type");
			sendErrorResponse(request, 400, "invalid_api_key", "Invalid apiKey field");
			return false;
		}

		// If string, must not exceed max length
		const char* apiKeyValue = value.as<const char*>();
		if (strlen(apiKeyValue) > MAX_API_KEY_LENGTH) {
			printLogfln("apiKey is too long: %d", strlen(apiKeyValue));
			sendErrorResponse(request, 400, "invalid_api_key", "apiKey is too long");
			return false;
		}

		// Valid apiKey value
		return true;
	}
}

// Save the API key
void setupApiKeyPostRoute() {
	// POST to update the API key
	server.on("/api/api-key", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
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

		// Validate the apiKey field
		if (!validateApiKeyField(request, doc["apiKey"])) {
			return;
		}

		// Save the new API key and write to EEPROM
		DeviceConfig config = getDeviceConfig();
		stringCopy(config.apiKey, doc["apiKey"].as<const char*>(), sizeof(config.apiKey));
		setDeviceConfig(config);
		writeEEPROM();

		// Send success response
		sendSuccessResponse(request);
		printLogfln("API key changed");
	});

    // OPTIONS request for preflighted requests
	server.on("/api/api-key", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
		sendCorsPreflightResponse(request, "POST, OPTIONS");
	});
}