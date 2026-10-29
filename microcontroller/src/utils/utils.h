#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class AsyncWebServerRequest;

enum class RequestBodyState {
	InProgress,
	Ready,
	Error,
	TooLarge,
	Ignored // A later chunk of a body that was already rejected (its error response was already sent)
};

struct RequestBodyResult {
	RequestBodyState state;
	const char* data;
	size_t length;
};

void stringCopy(char* destination, const char* text, size_t length);
String maskInternalChars(const char* text, size_t visibleEdgeChars = 3);
void formatCurrency(double value, char* output, const byte length);
void formatPrice(double value, char* output, const byte length);
RequestBodyResult accumulateRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void releaseRequestBody(AsyncWebServerRequest *request);

#endif