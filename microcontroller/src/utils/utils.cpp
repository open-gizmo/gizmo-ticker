#include "utils.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <math.h>
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

// Format a numeric value with thousands separators and optional decimals
char* addThousandsSeparators(double value, int decimals, char decimalSeparator, char thousandSeparator, char symbol) {
	// Prepare the output buffers and normalize the input
	static char formatted[48];
	char rawDigits[24];
	const int safeDecimals = decimals < 0 ? 0 : (decimals > 9 ? 9 : decimals);
	uint64_t scaleFactor = 1;
	for (int index = 0; index < safeDecimals; index++) {
		scaleFactor *= 10;
	}
	const int64_t roundedValue = llround(value * static_cast<double>(scaleFactor));
	const bool negative = roundedValue < 0;
	const uint64_t magnitude = negative
		? static_cast<uint64_t>(-(roundedValue + 1)) + 1
		: static_cast<uint64_t>(roundedValue);

	// Convert the rounded absolute value into a raw digit string
	snprintf(rawDigits, sizeof(rawDigits), "%llu", static_cast<unsigned long long>(magnitude));

	// Determine how many digits belong to the integer and decimal parts
	const size_t rawLength = strlen(rawDigits);
	const size_t integerDigits = rawLength > static_cast<size_t>(safeDecimals)
		? rawLength - static_cast<size_t>(safeDecimals)
		: 0;
	size_t writeIndex = 0;

	// Append a character only when there is enough room left in the output buffer
	auto appendChar = [&](char character) {
		if (writeIndex + 1 >= sizeof(formatted)) {
			return;
		}
		formatted[writeIndex++] = character;
	};

	// Add the optional currency symbol prefix
	if (symbol != ' ') {
		appendChar(symbol);
		appendChar(' ');
	}

	// Add the negative sign when the original value was below zero
	if (negative) {
		appendChar('-');
	}

	// Write the integer portion with thousands separators
	if (integerDigits == 0) {
		appendChar('0');
	} else {
		for (size_t index = 0; index < integerDigits; index++) {
			// Append the next digit
			appendChar(rawDigits[index]);

			// Append a thousands separator when appropriate
			const size_t digitsRemaining = integerDigits - index - 1;
			if (digitsRemaining > 0 && digitsRemaining % 3 == 0) {
				appendChar(thousandSeparator);
			}
		}
	}

	// Write the decimal portion, padding with zeros when needed
	if (safeDecimals > 0) {
		// Append the decimal separator
		appendChar(decimalSeparator);

		// Append the decimal digits, adding leading zeros if the raw value has fewer decimal digits than requested
		const size_t zeroPadding = rawLength < static_cast<size_t>(safeDecimals)
			? static_cast<size_t>(safeDecimals) - rawLength
			: 0;
		for (size_t index = 0; index < zeroPadding; index++) {
			appendChar('0');
		}

		// Append the existing decimal digits from the raw value
		const size_t decimalStart = rawLength > static_cast<size_t>(safeDecimals)
			? rawLength - static_cast<size_t>(safeDecimals)
			: 0;
		for (size_t index = decimalStart; index < rawLength; index++) {
			appendChar(rawDigits[index]);
		}
	}

	// Null-terminate the buffer and return the formatted string
	formatted[writeIndex] = '\0';
	return formatted;
}

/* Format a currency - Inspired by the Currency library made by RobTillaart
char* addThousandsSeparators(double value, int decimals, char decimalSeparator, char thousandSeparator, char symbol) {
	static char tmp[30]; // Temporary buffer to store the formatted string
	uint8_t index = 0; // Index for placing characters in tmp
	int64_t v = (int64_t)value; // Convert the value to an integer
	bool negative = v < 0; // Flag for negative numbers
	if (negative) v = -v; // Make v positive for processing
	int pos = -decimals; // Tracks the position relative to the decimal point
	while ((pos < 1) || (v > 0)) { // Loop until we've processed all digits
		if ((pos == 0) && (decimals > 0)) tmp[index++] = decimalSeparator; // Add decimal separator
		if ((pos > 0) && (pos % 3 == 0)) tmp[index++] = thousandSeparator; // Add thousand separator
		pos++;
		tmp[index++] = (v % 10) + '0'; // Extract and store the last digit of v
		v /= 10; // Remove the last digit from v
	}
	if (negative) tmp[index++] = '-'; // Add negative sign if necessary
	if (symbol != ' ') {
		tmp[index++] = ' '; // Add space
		tmp[index++] = symbol; // Add the currency symbol if there is
	}
	tmp[index] = '\0'; // Null-terminate the string
	for (uint8_t i = 0, j = index - 1; i < index / 2; i++, j--) { // Reverse the string since it was built backwards
		char c = tmp[i];
		tmp[i] = tmp[j];
		tmp[j] = c;
	}
	return tmp; // Return the pointer to the formatted string
}
*/

// Format currency
void formatCurrency(double value, char* output, const byte length) {
	const DeviceConfig& config = getDeviceConfig();
	if (config.formatType == FORMAT_US) {
		stringCopy(output, addThousandsSeparators(value, 2, '.', ','), length);
	} else {
		stringCopy(output, addThousandsSeparators(value, 2, ',', '.'), length);
	}
}

// Accumulate the request body for POST requests
RequestBodyResult accumulateRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
	// If it's the first chunk, allocate memory for the entire body
	if (index == 0) {
		releaseRequestBody(request);
		request->_tempObject = malloc(total + 1);
		if (request->_tempObject == NULL) {
			return { RequestBodyState::Error, NULL, 0 };
		}
	}

	// If memory allocation failed, return an error
	if (request->_tempObject == NULL) {
		return { RequestBodyState::Error, NULL, 0 };
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