#include "format.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace {
	constexpr int PRICE_SIGNIFICANT_DIGITS = 4; // Digits kept for prices below $1 (0.08123 instead of 0.08)
	constexpr int MAX_PRICE_DECIMALS = 8;
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

// Choose the decimals for a price: 2 from $1 up, more below so small prices keep their significant digits
int getPriceDecimals(double value) {
	const double magnitude = fabs(value);
	if (magnitude >= 1.0 || magnitude == 0.0) return 2;

	// Count the zeros right after the decimal point (0.08 has one), then add the significant digits
	const int leadingZeroCount = -static_cast<int>(floor(log10(magnitude))) - 1;
	const int decimals = leadingZeroCount + PRICE_SIGNIFICANT_DIGITS;
	return decimals > MAX_PRICE_DECIMALS ? MAX_PRICE_DECIMALS : decimals;
}

// Format a number with the US (1,234.56) or EU (1.234,56) separators
void formatNumber(double value, int decimals, formatNum formatType, char* output, size_t length) {
	// Nothing to write into
	if (length == 0) return;

	// Format with the separators of the requested format and copy the result (cut off safely if too long)
	const bool isUsFormat = formatType == FORMAT_US;
	snprintf(output, length, "%s", addThousandsSeparators(value, decimals, isUsFormat ? '.' : ',', isUsFormat ? ',' : '.'));
}