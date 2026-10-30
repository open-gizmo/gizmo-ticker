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
#include "market_refresh.h"
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
	constexpr uint32_t API_TASK_STACK_SIZE = 8192;

	// Shared market-data struct
	struct MarketDataRuntimeState {
		MarketTickerData cachedData = { "BTC", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		bool hasCachedData = false;
		bool hasPendingError = false;
		MarketRefreshSchedule refreshSchedule = { false, true, 0, 0, false, 0 }; // A fetch right away
		char pendingErrorMessage[BUF_SIZE] = "";
		char lastErrorMessage[BUF_SIZE] = ""; // Error of the last failed fetch, kept for the stale data warning (cleared by a success)
		portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	};

	// Create the shared market data state instance
	MarketDataRuntimeState marketState;

	// Check whether the cache is stale enough that a refresh should be scheduled
	bool shouldScheduleMarketDataRefresh() {
		portENTER_CRITICAL(&marketState.mux);
		const unsigned long currentMillis = millis(); // Read inside the lock so it is never older than a time stored by the worker
		const bool refreshDue = isMarketRefreshDue(marketState.refreshSchedule, currentMillis);
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
		marketState.hasPendingError = false;
		marketState.lastErrorMessage[0] = '\0';
		completeMarketRefresh(marketState.refreshSchedule, true, refreshedAt);
		portEXIT_CRITICAL(&marketState.mux);
	}

	// Drop an outdated refresh result and schedule a new fetch right away (the data on screen keeps its age)
	void discardMarketDataRefresh() {
		portENTER_CRITICAL(&marketState.mux);
		cancelMarketRefresh(marketState.refreshSchedule);
		portEXIT_CRITICAL(&marketState.mux);
	}

	// Store a refresh failure so the main loop can retry later without blocking
	void completeMarketDataRefreshFailure(const char* message, unsigned long failedAt) {
		portENTER_CRITICAL(&marketState.mux);
		marketState.hasPendingError = true;
		completeMarketRefresh(marketState.refreshSchedule, false, failedAt);
		stringCopy(marketState.pendingErrorMessage, message, sizeof(marketState.pendingErrorMessage));
		stringCopy(marketState.lastErrorMessage, message, sizeof(marketState.lastErrorMessage));
		portEXIT_CRITICAL(&marketState.mux);
	}
}

// Get the main market data used by the ticker
static bool fetchMarketData(JsonDocument &doc, const char* cryptoCoin) {
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
	snprintf(url, sizeof(url), marketApiUrlTemplate, getCryptoApiId(cryptoCoin));
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
	portENTER_CRITICAL(&marketState.mux);
	resetMarketRefreshSchedule(marketState.refreshSchedule);
	portEXIT_CRITICAL(&marketState.mux);
}

// Fetch the market data again as soon as possible (for example after a coin or API key change)
void requestMarketDataRefresh() {
	portENTER_CRITICAL(&marketState.mux);
	requestMarketRefresh(marketState.refreshSchedule);
	portEXIT_CRITICAL(&marketState.mux);
}

// Build the warning shown before old data (the refreshes keep failing), and return whether there is one
bool getStaleDataWarning(char* message, size_t messageLength) {
	char lastErrorMessage[BUF_SIZE];

	// Read the age of the data and the last error while holding the lock
	portENTER_CRITICAL(&marketState.mux);
	const unsigned long currentMillis = millis();
	const bool isStale = isMarketDataStale(marketState.refreshSchedule, currentMillis);
	const uint32_t ageMinutes = getMarketDataAgeMinutes(marketState.refreshSchedule, currentMillis);
	stringCopy(lastErrorMessage, marketState.lastErrorMessage, sizeof(lastErrorMessage));
	portEXIT_CRITICAL(&marketState.mux);

	// Nothing to warn about while the data is fresh
	if (!isStale) return false;

	// Say how old the prices are, and why
	snprintf(message, messageLength, "Prices from %lu min ago. %s", static_cast<unsigned long>(ageMinutes), lastErrorMessage[0] != '\0' ? lastErrorMessage : "Retrying...");
	return true;
}

// Fetch and parse the market data
static bool getStockDataAPI(MarketTickerData& marketData, const char* cryptoCoin) {
	JsonDocument marketDoc;

	// Fetch the market data
	if (!fetchMarketData(marketDoc, cryptoCoin)) {
		return false;
	}

	// Parse the result into the output model
	if (!parseMarketData(marketDoc.as<JsonVariantConst>(), cryptoCoin, marketData)) {
		printLogfln("Missing CoinGecko market data item");
		return false;
	}
	return true;
}

// Worker task that performs the blocking HTTP request away from the main loop
static void marketDataRefreshWorker(void* parameter) {
	MarketTickerData refreshedMarketData;

	// Read the selected coin once, so the URL and the label always match
	char cryptoCoin[CRYPTO_COIN_SIZE];
	stringCopy(cryptoCoin, getDeviceConfig().cryptoCoin, sizeof(cryptoCoin));
	const bool fetchSucceeded = getStockDataAPI(refreshedMarketData, cryptoCoin);
	const unsigned long currentMillis = millis();

	// Discard the result if the coin changed during the fetch, and fetch the new coin right away
	if (strcmp(cryptoCoin, getDeviceConfig().cryptoCoin) != 0) {
		printLogfln("Coin changed during the fetch, discarding %s data", cryptoCoin);
		discardMarketDataRefresh();
		vTaskDelete(nullptr);
		return;
	}

	// Store the result and schedule the next update
	if (fetchSucceeded) {
		completeMarketDataRefreshSuccess(refreshedMarketData, currentMillis);
	} else {
		const char errorMessageServer[] = "Error while calling the API. Retrying...";
		completeMarketDataRefreshFailure(errorMessageServer, currentMillis);
	}

	// Delete the worker task when done
	vTaskDelete(nullptr);
}

// Start a background refresh if one is not already running
static bool scheduleMarketDataRefresh() {
	portENTER_CRITICAL(&marketState.mux);

	// If refresh already in progress, return true
	if (marketState.refreshSchedule.isFetchInProgress) {
		portEXIT_CRITICAL(&marketState.mux);
		return true;
	}

	// Otherwise, mark that a refresh is now in progress
	startMarketRefresh(marketState.refreshSchedule);
	portEXIT_CRITICAL(&marketState.mux);

	// Start the background worker task
	if (xTaskCreate(marketDataRefreshWorker, "marketDataFetch", API_TASK_STACK_SIZE, nullptr, 1, nullptr) != pdPASS) {
		const char errorMessageSchedule[] = "Unable to refresh market data. Retrying...";
		completeMarketDataRefreshFailure(errorMessageSchedule, millis());
		return false;
	}

	// Task started successfully
	return true;
}

// Call the API to get the data
bool callAPI(MarketTickerData& marketData) {
	char errorMessage[BUF_SIZE];

	// Check if the API key is present
	if (getDeviceConfig().apiKey[0] == '\0') {
		const char errorMessageApi[] = "API key is not present. Use the web page to insert the key and try again.";
		printLogfln(errorMessageApi);
		printOnLedMatrix(errorMessageApi, sizeof(errorMessageApi)); // Print the message on the matrix
		return false; // If error, return false
	}

	// Start a background refresh whenever the cache is empty or stale
	if (shouldScheduleMarketDataRefresh()) {
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