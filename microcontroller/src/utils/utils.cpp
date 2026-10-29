#include "utils.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <math.h>
#include "../config/config.h"
#include "../format/format.h"

namespace {
	constexpr size_t MAX_REQUEST_BODY_SIZE = 2048; // Largest API payload is well under 1 KB, even with escaped Wi-Fi credentials
}

// Custom string copy function
void stringCopy(char* destination, const char* text, size_t length) {
	if (length <= 0) return; // If length is 0, do nothing
    strncpy(destination, text, length - 1); // Copy the string
    destination[length - 1] = '\0'; // Add the terminating character
}

// Mask the internal portion of a string while leaving the edges visible
String maskInternalChars(const char* text, size_t visibleEdgeChars) {
	// If the input text is null, return an empty string
	if (text == nullptr) {
		return String();
	}

	// Determine how many characters to show
	const size_t length = strlen(text);
	const size_t visibleChars = visibleEdgeChars * 2;
	if (length <= visibleChars) {
		return String(text);
	}

	// Build the masked string
	String masked;
	masked.reserve(length);

	// Add the starting visible edge characters
	for (size_t index = 0; index < visibleEdgeChars; index++) {
		masked += text[index];
	}

	// Add the masked portion
	for (size_t index = visibleEdgeChars; index < length - visibleEdgeChars; index++) {
		masked += '*';
	}

	// Add the ending visible edge characters
	for (size_t index = length - visibleEdgeChars; index < length; index++) {
		masked += text[index];
	}

	// Return the masked string
	return masked;
}


// Format currency with the configured separators (always 2 decimals, for large amounts and percentages)
void formatCurrency(double value, char* output, const byte length) {
	formatNumber(value, 2, getDeviceConfig().formatType, output, length);
}

// Format a price with the configured separators (2 decimals from $1 up, more below so coins worth cents stay readable)
void formatPrice(double value, char* output, const byte length) {
	formatNumber(value, getPriceDecimals(value), getDeviceConfig().formatType, output, length);
}

// Accumulate the request body for POST requests
RequestBodyResult accumulateRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
	// If it's the first chunk, allocate memory for the entire body
	if (index == 0) {
		releaseRequestBody(request);

		// Reject bodies larger than any valid API payload before allocating them
		if (total > MAX_REQUEST_BODY_SIZE) {
			return { RequestBodyState::TooLarge, NULL, 0 };
		}

		// Allocate the body buffer
		request->_tempObject = malloc(total + 1);
		if (request->_tempObject == NULL) {
			return { RequestBodyState::Error, NULL, 0 };
		}
	}

	// If the first chunk was rejected, ignore the rest of the body (the error response was already sent)
	if (request->_tempObject == NULL) {
		return { RequestBodyState::Ignored, NULL, 0 };
	}

	// Copy the current chunk into the allocated memory
	memcpy(static_cast<uint8_t*>(request->_tempObject) + index, data, len);

	// If still receiving data, wait for the next call
	if (index + len != total) {
		return { RequestBodyState::InProgress, NULL, 0 };
	}

	// All data received, return the complete body
	char* body = static_cast<char*>(request->_tempObject);
	body[total] = '\0';
	return { RequestBodyState::Ready, body, total };
}

// Release the memory allocated for the request body
void releaseRequestBody(AsyncWebServerRequest *request) {
	if (request->_tempObject == NULL) return;
	free(request->_tempObject);
	request->_tempObject = NULL;
}