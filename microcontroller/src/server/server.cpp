#include "server.h"
#include "../config/config.h"

namespace {
	constexpr const char* DEFAULT_CORS_HEADERS = "Content-Type";
	constexpr const char* DEFAULT_CORS_METHODS = "GET, POST, OPTIONS";

	// Get the value of a specific header
	String getHeaderValue(AsyncWebServerRequest* request, const char* headerName) {
		if (!request->hasHeader(headerName)) return String();
		return request->getHeader(headerName)->value();
	}

	// Validate if the CORS origin is allowed
	bool isAllowedCorsOrigin(AsyncWebServerRequest* request) {
		const String origin = getHeaderValue(request, "Origin");

		// Allow if no Origin header is present (standard for same-origin requests)
		if (origin.length() == 0) {
			return true;
		}

		// Allow if Origin matches the Host header (same-origin)
		if (origin == "null") {
			return false;
		}

		// Allow if Origin matches the Host header (same-origin)
		const String host = getHeaderValue(request, "Host");
		if (host.length() != 0 && (origin == "http://" + host || origin == "https://" + host)) {
			return true;
		}

		// Otherwise, block
		return false;
	}

	// Append CORS headers to the response
	void appendCorsHeaders(AsyncWebServerRequest* request, AsyncWebServerResponse* response, const char* allowedMethods) {
		// Only append CORS headers if the Origin is allowed
		const String origin = getHeaderValue(request, "Origin");
		if (origin.length() == 0 || !isAllowedCorsOrigin(request)) {
			return;
		}

		// Append the appropriate CORS headers
		response->addHeader("Access-Control-Allow-Origin", origin);
		response->addHeader("Vary", "Origin");
		response->addHeader("Access-Control-Allow-Methods", allowedMethods);
		response->addHeader("Access-Control-Allow-Headers", DEFAULT_CORS_HEADERS);
		response->addHeader("Access-Control-Max-Age", "600");
	}

	// Initialize the standard response structure
	void initializeResponse(JsonDocument& doc) {
		doc.clear();
		doc["errorId"] = nullptr;
		doc["errorMessage"] = nullptr;
		doc["data"] = nullptr;
	}

	// Serialize the JSON document and send the response
	void sendResponse(AsyncWebServerRequest* request, uint16_t statusCode, JsonDocument& doc) {
		String jsonResponse;
		serializeJson(doc, jsonResponse);
		AsyncWebServerResponse* response = request->beginResponse(statusCode, "application/json", jsonResponse);
		appendCorsHeaders(request, response, DEFAULT_CORS_METHODS);
		request->send(response);
	}
}

// Validate CORS and send error response if not allowed
bool ensureCorsAllowed(AsyncWebServerRequest* request) {
	// Check if CORS is allowed for this request
	if (isAllowedCorsOrigin(request)) {
		return true;
	}

	// Send error response for disallowed CORS origin
	sendErrorResponse(request, 403, "cors_origin_not_allowed", "CORS origin not allowed");
	return false;
}

// Handle CORS preflight requests
void sendCorsPreflightResponse(AsyncWebServerRequest* request, const char* allowedMethods) {
	// Check if CORS is allowed for this request
	if (!isAllowedCorsOrigin(request)) {
		sendErrorResponse(request, 403, "cors_origin_not_allowed", "CORS origin not allowed");
		return;
	}

	// Send a 204 No Content response with the appropriate CORS headers
	AsyncWebServerResponse* response = request->beginResponse(204);
	appendCorsHeaders(request, response, allowedMethods);
	request->send(response);
}

// Send a success response with optional data
void sendSuccessResponse(AsyncWebServerRequest* request, uint16_t statusCode, const JsonDocument* doc) {
	// Initialize the response JSON document
	JsonDocument responseDoc;
	initializeResponse(responseDoc);

	// Add the data object if provided
	if (doc != nullptr) {
		responseDoc["data"].set(doc->as<JsonVariantConst>());
	}

	// Send the response
	sendResponse(request, statusCode, responseDoc);
}

// Send an error response
void sendErrorResponse(AsyncWebServerRequest* request, uint16_t statusCode, const char* errorId, const char* errorMessage) {
	// Initialize the response JSON document
	JsonDocument doc;
	initializeResponse(doc);

	// Populate the error details
	doc["errorId"] = errorId;
	doc["errorMessage"] = errorMessage;

	// Send the response
	sendResponse(request, statusCode, doc);
}

// Setup routes
void setupRoutes() {
	// Setup the routes
	setupConnectPostRoute();
	setupCheckConnectionGetRoute();
	setupNetworksGetRoute();
	setupApiKeyPostRoute();
	setupConfigGetRoute();
	setupSettingsPostRoute();
	setupResetSettingsGetRoute();
	setupCaptivePortalRoute();
}

// Setup server
void setupServer() {
	setupRoutes(); // Setup routes
	server.begin(); // Start the server
}