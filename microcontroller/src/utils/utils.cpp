#include "utils.h"
#include <Arduino.h>
#include <math.h>
#include <gizmo/format.h>
#include "../config/config.h"

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


// Get the separator style of the configured number format
static gizmo::NumberFormat getConfiguredNumberFormat() {
	return getDeviceConfig().formatType == FORMAT_US ? gizmo::NumberFormat::US : gizmo::NumberFormat::EU;
}

// Format currency with the configured separators (always 2 decimals, for large amounts and percentages)
void formatCurrency(double value, char* output, const byte length) {
	gizmo::formatNumber(value, 2, getConfiguredNumberFormat(), output, length);
}

// Format a price with the configured separators (2 decimals from $1 up, more below so coins worth cents stay readable)
void formatPrice(double value, char* output, const byte length) {
	gizmo::formatNumber(value, gizmo::getSignificantDecimals(value), getConfiguredNumberFormat(), output, length);
}