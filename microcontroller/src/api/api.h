#ifndef API_H
#define API_H

struct MarketTickerData {
	const char* tickerLabel;
	double currentPrice;
	double priceChangePercentage24h;
	double priceChange24h;
	double marketCap;
	double dailyHigh;
	double dailyLow;
	double yearHigh;
	double yearLow;
	double openPrice;
	double volume;
};

void setupWebClient();
bool callAPI(MarketTickerData& marketData);

#endif