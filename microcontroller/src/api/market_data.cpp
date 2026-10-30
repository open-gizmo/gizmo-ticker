#include "market_data.h"
#include <cstring>

// Get the CoinGecko API ID of a crypto coin
const char* getCryptoApiId(const char* cryptoCoin) {
	return strcmp(cryptoCoin, "kaspa") == 0 ? "kaspa" : "bitcoin";
}

// Get the ticker label of a crypto coin
const char* getCryptoTickerLabel(const char* cryptoCoin) {
	return strcmp(cryptoCoin, "kaspa") == 0 ? "KAS" : "BTC";
}

// Parse the CoinGecko markets reply (an array with one item for the coin) into the runtime model
bool parseMarketData(JsonVariantConst marketDoc, const char* cryptoCoin, MarketTickerData& marketData) {
	JsonVariantConst marketDataJson = marketDoc[0];

	// Check if the market data is present
	if (marketDataJson.isNull()) {
		return false;
	}

	// Parse the market data into the output model
	marketData.tickerLabel = getCryptoTickerLabel(cryptoCoin);
	marketData.currentPrice = marketDataJson["current_price"].as<double>();
	marketData.priceChangePercentage24h = marketDataJson["price_change_percentage_24h_in_currency"].as<double>();
	marketData.priceChange24h = marketDataJson["price_change_24h"].as<double>();
	marketData.marketCap = marketDataJson["market_cap"].as<double>();
	marketData.dailyHigh = marketDataJson["high_24h"].as<double>();
	marketData.dailyLow = marketDataJson["low_24h"].as<double>();
	marketData.yearHigh = marketDataJson["ath"].as<double>();
	marketData.yearLow = marketDataJson["atl"].as<double>();
	marketData.openPrice = marketData.currentPrice - marketData.priceChange24h;
	marketData.volume = marketDataJson["total_volume"].as<double>();

	// Mark as successful
	return true;
}