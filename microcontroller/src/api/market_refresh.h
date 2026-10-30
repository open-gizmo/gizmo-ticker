#ifndef MARKET_REFRESH_H
#define MARKET_REFRESH_H

#include <stdint.h>

// When the next market data fetch is due, kept free of the hardware so the native tests can drive it (times are millis(), 32-bit like on the ESP32)

constexpr uint32_t MARKET_REFRESH_INTERVAL_MS = 360000; // Time between two successful fetches
constexpr uint32_t MARKET_RETRY_INTERVAL_MS = 30000; // Wait after a failed fetch (also while no fetch has worked yet)
constexpr uint32_t MARKET_STALE_AFTER_MS = 2 * MARKET_REFRESH_INTERVAL_MS; // Data this old means the refreshes keep failing, so the matrix warns about it

struct MarketRefreshSchedule {
	bool isFetchInProgress;
	bool isRefreshRequested; // A fetch asked for right away (for example after a coin change)
	uint32_t nextFetchScheduledAt; // When the wait for the next fetch started
	uint32_t nextFetchDelay; // How long to wait before the next fetch
	bool hasSucceeded; // A fetch has worked since boot, so there is data to show
	uint32_t lastSuccessAt; // When the data on screen was fetched
};

void resetMarketRefreshSchedule(MarketRefreshSchedule& schedule);
bool isMarketRefreshDue(const MarketRefreshSchedule& schedule, uint32_t currentMillis);
void startMarketRefresh(MarketRefreshSchedule& schedule);
void completeMarketRefresh(MarketRefreshSchedule& schedule, bool isSuccessful, uint32_t completedAt);
void cancelMarketRefresh(MarketRefreshSchedule& schedule);
void requestMarketRefresh(MarketRefreshSchedule& schedule);
bool isMarketDataStale(const MarketRefreshSchedule& schedule, uint32_t currentMillis);
uint32_t getMarketDataAgeMinutes(const MarketRefreshSchedule& schedule, uint32_t currentMillis);

#endif