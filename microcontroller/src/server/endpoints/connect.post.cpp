#include "../server.h"
#include <ArduinoJson.h>
#include <cstring>
#include "../../config/config.h"
#include "../../wifi/wifi.h"
#include "../../utils/utils.h"
#include "../../serial/serial.h"

namespace {
	constexpr size_t MAX_WIFI_SSID_LENGTH = WIFI_SSID_SIZE - 1;
	constexpr size_t MAX_WIFI_PASSWORD_LENGTH = WIFI_PASSWORD_SIZE - 1;

	// Validate the credential parameters
	bool validateCredentialField(AsyncWebServerRequest* request, const char* fieldName, JsonVariantConst value, size_t maxLength, bool allowEmpty) {
		// Field must be present
		if (value.isNull()) {
			printLogfln("Missing required %s value", fieldName);
			sendErrorResponse(request, 400, "missing_required_fields", fieldName);
			return false;
		}

		// Field must be a string
		if (!value.is<const char*>()) {
			printLogfln("Invalid %s type", fieldName);
			sendErrorResponse(request, 400, "invalid_connection_field", fieldName);
			return false;
		}

		// If string, must not exceed max length
		const char* resolvedValue = value.as<const char*>();

		// Null or empty check
		if (!allowEmpty && resolvedValue[0] == '\0') {
			printLogfln("Missing required %s value", fieldName);
			sendErrorResponse(request, 400, "missing_required_fields", fieldName);
			return false;
		}

		// Length check
		const size_t valueLength = strlen(resolvedValue);
		if (valueLength > maxLength) {
			printLogfln("%s is too long: %d", fieldName, valueLength);
			sendErrorResponse(request, 400, "invalid_connection_field", fieldName);
			return false;
		}

		// Valid parameter
		return true;
	}
}

// Connect to WiFi with the provided credentials
void setupConnectPostRoute() {
	server.on("/api/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
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

		// Parse the JSON body
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

		// Validate the credential fields
		if (!validateCredentialField(request, "ssid", doc["ssid"], MAX_WIFI_SSID_LENGTH, false)
			|| !validateCredentialField(request, "password", doc["password"], MAX_WIFI_PASSWORD_LENGTH, true)) {
			return;
		}

		// Save the credentials to the config and trigger Wi-Fi connection attempt
		DeviceConfig config = getDeviceConfig();
		stringCopy(config.ssid, doc["ssid"].as<const char*>(), sizeof(config.ssid));
		stringCopy(config.password, doc["password"].as<const char*>(), sizeof(config.password));
		setDeviceConfig(config);
		queueWiFiConnectionAttempt();

		// Send the response with the updated connection status
		JsonDocument responseDoc;
		JsonObject responseData = responseDoc.to<JsonObject>();
		responseData["connectionStatus"] = static_cast<int>(wiFiConnectionStatus);
		sendSuccessResponse(request, 200, &responseDoc);
	});

	// OPTIONS request for preflighted requests
	server.on("/api/connect", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
		sendCorsPreflightResponse(request, "POST, OPTIONS");
	});
}