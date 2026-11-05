#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

String maskInternalChars(const char *text, size_t visibleEdgeChars = 3);
void formatCurrency(double value, char *output, const byte length);
void formatPrice(double value, char *output, const byte length);

#endif