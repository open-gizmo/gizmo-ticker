#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <ArduinoJson.h>

// Market data shown on the matrix, read from the CoinGecko reply (kept free of the hardware so the native tests can check it)

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

const char* getCryptoApiId(const char* cryptoCoin);
const char* getCryptoTickerLabel(const char* cryptoCoin);
bool parseMarketData(JsonVariantConst marketDoc, const char* cryptoCoin, MarketTickerData& marketData);

#endif