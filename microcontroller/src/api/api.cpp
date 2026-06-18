#include "api.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <StreamUtils.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../config/config.h"
#include "../matrix/matrix.h"
#include "../serial/serial.h"
#include "../utils/utils.h"

/*
The main loop never performs the blocking HTTP request directly.
Instead it schedules a refresh task, keeps rendering the last cached snapshot,
and only surfaces an error when no cached market data is available yet.
*/

const char marketApiUrlTemplate[] = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd&ids=%s&price_change_percentage=24h"; // Market API URL template
const char apiKeyHeaderName[] = "x-cg-demo-api-key"; // CoinGecko demo key header

namespace {
	constexpr unsigned long API_REFRESH_INTERVAL = 360000;
	constexpr unsigned long API_RETRY_INTERVAL = 30000;
	constexpr uint32_t API_TASK_STACK_SIZE = 8192;

	// Shared market-data struct
	struct MarketDataRuntimeState {
		MarketTickerData cachedData = { "BTC", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		bool hasCachedData = false;
		bool fetchInProgress = false;
		bool hasPendingError = false;
		unsigned long nextFetchAt = 0;
		char pendingErrorMessage[BUF_SIZE] = "";
		portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	};

	// Create the shared market data state instance
	MarketDataRuntimeState marketState;

	// Check whether the cache is stale enough that a refresh should be scheduled
	bool shouldScheduleMarketDataRefresh(unsigned long currentMillis) {
		portENTER_CRITICAL(&marketState.mux);
		const bool refreshDue = !marketState.fetchInProgress && (timestampStockData == 0 || currentMillis >= marketState.nextFetchAt);
		portEXIT_CRITICAL(&marketState.mux);
		return refreshDue;
	}

	// Return the last cached snapshot, if one is available
	bool readCachedMarketData(MarketTickerData& marketData) {
		portENTER_CRITICAL(&marketState.mux);

		// If cached data is available, copy it to the output parameter
		const bool dataAvailable = marketState.hasCachedData;
		if (dataAvailable) {
			marketData = marketState.cachedData;
		}

		// Return whether cached data was available
		portEXIT_CRITICAL(&marketState.mux);
		return dataAvailable;
	}

	// Consume the latest background refresh error so the main loop can decide whether to surface it
	bool takePendingRefreshError(char* message, size_t messageLength) {
		portENTER_CRITICAL(&marketState.mux);

		// If no error is pending, return false
		if (!marketState.hasPendingError) {
			portEXIT_CRITICAL(&marketState.mux);
			return false;
		}

		// Otherwise, copy the error message to the output buffer and return true
		stringCopy(message, marketState.pendingErrorMessage, messageLength);
		marketState.hasPendingError = false;
		portEXIT_CRITICAL(&marketState.mux);
		return true;
	}

	// Store a successful refresh result and schedule the next periodic update
	void completeMarketDataRefreshSuccess(const MarketTickerData& marketData, unsigned long refreshedAt) {
		portENTER_CRITICAL(&marketState.mux);
		marketState.cachedData = marketData;
		marketState.hasCachedData = true;
		marketState.fetchInProgress = false;
		marketState.hasPendingError = false;
		marketState.nextFetchAt = refreshedAt + API_REFRESH_INTERVAL;
		timestampStockData = refreshedAt;
		portEXIT_CRITICAL(&marketState.mux);
	}

	// Store a refresh failure so the main loop can retry later without blocking
	void completeMarketDataRefreshFailure(const char* message, unsigned long nextRetryAt) {
		portENTER_CRITICAL(&marketState.mux);
		marketState.fetchInProgress = false;
		marketState.hasPendingError = true;
		marketState.nextFetchAt = nextRetryAt;
		stringCopy(marketState.pendingErrorMessage, message, sizeof(marketState.pendingErrorMessage));
		portEXIT_CRITICAL(&marketState.mux);
	}
}

// Get the selected crypto coin API ID
static const char* getSelectedCryptoApiId() {
	const DeviceConfig& config = getDeviceConfig();
	return strcmp(config.cryptoCoin, "kaspa") == 0 ? "kaspa" : "bitcoin";
}

// Get the selected crypto coin ticker label
static const char* getSelectedCryptoTickerLabel() {
	const DeviceConfig& config = getDeviceConfig();
	return strcmp(config.cryptoCoin, "kaspa") == 0 ? "KAS" : "BTC";
}

// Get the main market data used by the ticker
static bool fetchMarketData(JsonDocument &doc) {
	char url[220];
	WiFiClientSecure clientSecure;
	HTTPClient http;

	// Filter just the needed data to save memory and speed up parsing
	JsonDocument filter;
	filter[0]["current_price"] = true;
	filter[0]["price_change_percentage_24h_in_currency"] = true;
	filter[0]["price_change_24h"] = true;
	filter[0]["market_cap"] = true;
	filter[0]["high_24h"] = true;
	filter[0]["low_24h"] = true;
	filter[0]["total_volume"] = true;
	filter[0]["ath"] = true;
	filter[0]["atl"] = true;

	// Prepare the HTTP request
	snprintf(url, sizeof(url), marketApiUrlTemplate, getSelectedCryptoApiId());
	printLogfln("Requesting URL: %s", url);
	clientSecure.setInsecure();
	http.begin(clientSecure, url);
	http.setTimeout(5000);
	http.addHeader("Accept", "application/json");
	http.addHeader(apiKeyHeaderName, getDeviceConfig().apiKey);

	// Send the request
	const int httpCode = http.GET();
	if (httpCode != HTTP_CODE_OK) {
		printLogfln("HTTP call error: %d", httpCode);
		http.end();
		return false;
	}

	// CoinGecko can reply using chunked transfer encoding, decode it before JSON parsing
	ChunkDecodingStream decodedStream(http.getStream());
	DeserializationError error = deserializeJson(doc, decodedStream, DeserializationOption::Filter(filter));
	if (error) {
		printLogfln("Error while parsing the JSON: %s", error.c_str());
		http.end();
		return false;
	}

	// End the HTTP connection
	http.end();
	return true;
}

// Setup the web client
void setupWebClient() {
	marketState.nextFetchAt = 0;
}

// Parse the fetched market data into the runtime model
static bool parseMarketData(JsonDocument& marketDoc, MarketTickerData& marketData) {
	JsonVariant marketDataJson = marketDoc[0];

	// Check if the market data is present
	if (marketDataJson.isNull()) {
		printLogfln("Missing CoinGecko market data item");
		return false;
	}

	// Parse the market data into the output model
	marketData.tickerLabel = getSelectedCryptoTickerLabel();
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

// Fetch and parse the market data
static bool getStockDataAPI(MarketTickerData& marketData) {
	JsonDocument marketDoc;

	// Fetch the market data
	if (!fetchMarketData(marketDoc)) {
		return false;
	}

	// Parse the result into the output model
	return parseMarketData(marketDoc, marketData);
}

// Worker task that performs the blocking HTTP request away from the main loop
static void marketDataRefreshWorker(void* parameter) {
	MarketTickerData refreshedMarketData;
	const bool fetchSucceeded = getStockDataAPI(refreshedMarketData);
	const unsigned long currentMillis = millis();

	// Store the result and schedule the next update
	if (fetchSucceeded) {
		completeMarketDataRefreshSuccess(refreshedMarketData, currentMillis);
	} else {
		const char errorMessageServer[] = "Error while calling the API. Retrying...";
		completeMarketDataRefreshFailure(errorMessageServer, currentMillis + API_RETRY_INTERVAL);
	}

	// Delete the worker task when done
	vTaskDelete(nullptr);
}

// Start a background refresh if one is not already running
static bool scheduleMarketDataRefresh() {
	portENTER_CRITICAL(&marketState.mux);

	// If refresh already in progress, return true
	if (marketState.fetchInProgress) {
		portEXIT_CRITICAL(&marketState.mux);
		return true;
	}

	// Otherwise, mark that a refresh is now in progress
	marketState.fetchInProgress = true;
	portEXIT_CRITICAL(&marketState.mux);

	// Start the background worker task
	if (xTaskCreate(marketDataRefreshWorker, "marketDataFetch", API_TASK_STACK_SIZE, nullptr, 1, nullptr) != pdPASS) {
		const char errorMessageSchedule[] = "Unable to refresh market data. Retrying...";
		completeMarketDataRefreshFailure(errorMessageSchedule, millis() + API_RETRY_INTERVAL);
		return false;
	}

	// Task started successfully
	return true;
}

// Call the API to get the data
bool callAPI(MarketTickerData& marketData) {
	const unsigned long currentMillis = millis();
	char errorMessage[BUF_SIZE];

	// Check if the API key is present
	if (getDeviceConfig().apiKey[0] == '\0') {
		const char errorMessageApi[] = "API key is not present. Use the web page to insert the key and try again.";
		printLogfln(errorMessageApi);
		printOnLedMatrix(errorMessageApi, sizeof(errorMessageApi)); // Print the message on the matrix
		return false; // If error, return false
	}

	// Start a background refresh whenever the cache is empty or stale
	if (shouldScheduleMarketDataRefresh(currentMillis)) {
		printLogfln("Scheduling market data refresh...");
		scheduleMarketDataRefresh();
	}

	// Show a fetch error only when there is no cached data to display yet
	if (takePendingRefreshError(errorMessage, sizeof(errorMessage)) && !readCachedMarketData(marketData)) {
		printLogfln(errorMessage);
		printOnLedMatrix(errorMessage, strlen(errorMessage) + 1);
		return false;
	}

	// If no cached data, return false
	if (!readCachedMarketData(marketData)) {
		return false;
	}

	// If no error, return true
	return true;
}