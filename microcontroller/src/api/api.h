#ifndef API_H
#define API_H

#include <stddef.h>
#include "market_data.h"

void setupWebClient();
bool callAPI(MarketTickerData& marketData);
void requestMarketDataRefresh();
bool getStaleDataWarning(char* message, size_t messageLength);

#endif