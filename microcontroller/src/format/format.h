#ifndef FORMAT_H
#define FORMAT_H

#include <stddef.h>
#include "../config/device_config.h"

// Number formatting for the matrix, free of hardware headers so it can be tested on the host
char* addThousandsSeparators(double value, int decimals, char decimalSeparator, char thousandSeparator, char symbol = ' ');
int getPriceDecimals(double value);
void formatNumber(double value, int decimals, formatNum formatType, char* output, size_t length);

#endif