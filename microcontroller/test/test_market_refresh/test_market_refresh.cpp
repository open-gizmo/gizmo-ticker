#include <unity.h>

#include "api/market_refresh.h"

namespace {
	// Schedule under test
	MarketRefreshSchedule schedule;

	// Run one fetch at a time: start it and end it right away
	void runFetch(bool isSuccessful, uint32_t currentMillis) {
		TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, currentMillis));
		startMarketRefresh(schedule);
		completeMarketRefresh(schedule, isSuccessful, currentMillis);
	}
}

// Start each test from a freshly booted device
void setUp() {
	resetMarketRefreshSchedule(schedule);
}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Normal refresh ********************************/

// The first fetch starts right after boot
void test_first_fetch_is_due_at_boot() {
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 0));
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 123456));
}

// No second fetch starts while one is running
void test_no_fetch_while_one_is_running() {
	startMarketRefresh(schedule);
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 1000000));
}

// After a success, the next fetch waits for the refresh interval
void test_success_waits_for_the_refresh_interval() {
	runFetch(true, 1000);
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 1000 + MARKET_REFRESH_INTERVAL_MS - 1));
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 1000 + MARKET_REFRESH_INTERVAL_MS));
}

/******************************** Failures ********************************/

// A failure before any success still waits for the retry interval (it used to retry on every matrix cycle)
void test_first_failure_waits_for_the_retry_interval() {
	runFetch(false, 5000);
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 5000 + 1));
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 5000 + MARKET_RETRY_INTERVAL_MS - 1));
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 5000 + MARKET_RETRY_INTERVAL_MS));
}

// Repeated failures keep waiting for the retry interval each time
void test_repeated_failures_keep_the_retry_interval() {
	uint32_t currentMillis = 1000;
	for (int attempt = 0; attempt < 5; attempt += 1) {
		runFetch(false, currentMillis);
		TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, currentMillis + MARKET_RETRY_INTERVAL_MS / 2));
		currentMillis += MARKET_RETRY_INTERVAL_MS;
	}
}

// A failure after a success waits for the retry interval, not the refresh interval
void test_failure_after_success_retries_sooner() {
	runFetch(true, 1000);
	runFetch(false, 1000 + MARKET_REFRESH_INTERVAL_MS);
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 1000 + MARKET_REFRESH_INTERVAL_MS + MARKET_RETRY_INTERVAL_MS));
}

/******************************** Requested refresh ********************************/

// A requested refresh skips the wait (for example after a coin change)
void test_requested_refresh_skips_the_wait() {
	runFetch(true, 1000);
	requestMarketRefresh(schedule);
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 2000));
}

// A requested refresh also skips the retry wait
void test_requested_refresh_skips_the_retry_wait() {
	runFetch(false, 1000);
	requestMarketRefresh(schedule);
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 1001));
}

// A refresh requested during a fetch starts as soon as that fetch ends (its data may be for the old coin)
void test_refresh_requested_during_a_fetch_runs_after_it() {
	startMarketRefresh(schedule);
	requestMarketRefresh(schedule);
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 2000));

	// Expect another fetch right after the running one ends, then the normal wait again
	completeMarketRefresh(schedule, true, 3000);
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 3000));
	runFetch(true, 3000);
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 3001));
}

// A fetch started after a request covers it
void test_starting_a_fetch_clears_the_request() {
	requestMarketRefresh(schedule);
	runFetch(true, 1000);
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 1001));
}

/******************************** Clock rollover ********************************/

// The waits work across the millis() rollover
void test_waits_work_across_clock_rollover() {
	// Fail right before the clock wraps around
	const uint32_t failedAt = UINT32_MAX - 1000;
	runFetch(false, failedAt);

	// Expect the retry wait to run through the wrap
	TEST_ASSERT_FALSE(isMarketRefreshDue(schedule, 500));
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, failedAt + MARKET_RETRY_INTERVAL_MS));
}

// Run all the market refresh tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_first_fetch_is_due_at_boot);
	RUN_TEST(test_no_fetch_while_one_is_running);
	RUN_TEST(test_success_waits_for_the_refresh_interval);
	RUN_TEST(test_first_failure_waits_for_the_retry_interval);
	RUN_TEST(test_repeated_failures_keep_the_retry_interval);
	RUN_TEST(test_failure_after_success_retries_sooner);
	RUN_TEST(test_requested_refresh_skips_the_wait);
	RUN_TEST(test_requested_refresh_skips_the_retry_wait);
	RUN_TEST(test_refresh_requested_during_a_fetch_runs_after_it);
	RUN_TEST(test_starting_a_fetch_clears_the_request);
	RUN_TEST(test_waits_work_across_clock_rollover);
	return UNITY_END();
}