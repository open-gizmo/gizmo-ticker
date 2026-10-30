#ifndef API_H
#define API_H

#include "market_data.h"

void setupWebClient();
bool callAPI(MarketTickerData& marketData);
void requestMarketDataRefresh();

#endif