#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class AsyncWebServerRequest;

enum class RequestBodyState {
	InProgress,
	Ready,
	Error
};

struct RequestBodyResult {
	RequestBodyState state;
	const char* data;
	size_t length;
};

void stringCopy(char* destination, const char* text, size_t length);
String maskInternalChars(const char* text, size_t visibleEdgeChars = 3);
char* addThousandsSeparators(double value, int decimals, char decimalSeparator, char thousandSeparator, char symbol = ' ');
void formatCurrency(double value, char* output, const byte length);
RequestBodyResult accumulateRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void releaseRequestBody(AsyncWebServerRequest *request);

#endif