#include "market_refresh.h"

// Start with a fetch right away
void resetMarketRefreshSchedule(MarketRefreshSchedule& schedule) {
	schedule.isFetchInProgress = false;
	schedule.isRefreshRequested = true;
	schedule.nextFetchScheduledAt = 0;
	schedule.nextFetchDelay = 0;
}

// Check whether a fetch should start now (the elapsed-time check stays correct across millis() rollover)
bool isMarketRefreshDue(const MarketRefreshSchedule& schedule, uint32_t currentMillis) {
	if (schedule.isFetchInProgress) return false;
	return schedule.isRefreshRequested || currentMillis - schedule.nextFetchScheduledAt >= schedule.nextFetchDelay;
}

// Mark a fetch as started (it covers any refresh asked for until now)
void startMarketRefresh(MarketRefreshSchedule& schedule) {
	schedule.isFetchInProgress = true;
	schedule.isRefreshRequested = false;
}

// Schedule the next fetch: the normal interval after a success, the retry interval after a failure
void completeMarketRefresh(MarketRefreshSchedule& schedule, bool isSuccessful, uint32_t completedAt) {
	schedule.isFetchInProgress = false;
	schedule.nextFetchScheduledAt = completedAt;
	schedule.nextFetchDelay = isSuccessful ? MARKET_REFRESH_INTERVAL_MS : MARKET_RETRY_INTERVAL_MS;
}

// Ask for a fetch right away (if one is running, the next one starts as soon as it ends)
void requestMarketRefresh(MarketRefreshSchedule& schedule) {
	schedule.isRefreshRequested = true;
}