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

/******************************** Cancelled fetch ********************************/

// A fetch dropped after a coin change is followed by another one right away
void test_cancelled_fetch_fetches_again_right_away() {
	runFetch(true, 1000);
	startMarketRefresh(schedule);
	cancelMarketRefresh(schedule);
	TEST_ASSERT_TRUE(isMarketRefreshDue(schedule, 2000));
}

// A dropped fetch doesn't make the data on screen look newer (it used to go through the success path)
void test_cancelled_fetch_keeps_the_age_of_the_data() {
	runFetch(true, 1000);
	startMarketRefresh(schedule);
	cancelMarketRefresh(schedule);
	TEST_ASSERT_TRUE(isMarketDataStale(schedule, 1000 + MARKET_STALE_AFTER_MS));
}

/******************************** Stale data ********************************/

// Before any success there is no data to warn about (the matrix shows the error itself)
void test_no_stale_warning_before_any_success() {
	runFetch(false, 1000);
	TEST_ASSERT_FALSE(isMarketDataStale(schedule, 1000 + 10 * MARKET_STALE_AFTER_MS));
}

// The data turns stale after two refresh intervals without a success
void test_data_turns_stale_without_a_success() {
	// Succeed once, then fail every retry
	runFetch(true, 1000);
	runFetch(false, 1000 + MARKET_REFRESH_INTERVAL_MS);

	// Expect fresh data until the limit, then stale data
	TEST_ASSERT_FALSE(isMarketDataStale(schedule, 1000 + MARKET_STALE_AFTER_MS - 1));
	TEST_ASSERT_TRUE(isMarketDataStale(schedule, 1000 + MARKET_STALE_AFTER_MS));
}

// A success makes the data fresh again
void test_success_clears_the_stale_warning() {
	runFetch(true, 1000);
	TEST_ASSERT_TRUE(isMarketDataStale(schedule, 1000 + MARKET_STALE_AFTER_MS));
	runFetch(true, 1000 + MARKET_STALE_AFTER_MS);
	TEST_ASSERT_FALSE(isMarketDataStale(schedule, 1000 + MARKET_STALE_AFTER_MS + 1));
}

// The age of the data is in whole minutes
void test_data_age_is_in_whole_minutes() {
	runFetch(true, 1000);
	TEST_ASSERT_EQUAL_UINT32(0, getMarketDataAgeMinutes(schedule, 1000 + 59999));
	TEST_ASSERT_EQUAL_UINT32(12, getMarketDataAgeMinutes(schedule, 1000 + 12 * 60000UL + 30000));
}

// The age works across the millis() rollover
void test_data_age_works_across_clock_rollover() {
	const uint32_t fetchedAt = UINT32_MAX - 60000;
	runFetch(true, fetchedAt);
	TEST_ASSERT_EQUAL_UINT32(13, getMarketDataAgeMinutes(schedule, fetchedAt + 13 * 60000UL));
	TEST_ASSERT_TRUE(isMarketDataStale(schedule, fetchedAt + MARKET_STALE_AFTER_MS));
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
	RUN_TEST(test_cancelled_fetch_fetches_again_right_away);
	RUN_TEST(test_cancelled_fetch_keeps_the_age_of_the_data);
	RUN_TEST(test_no_stale_warning_before_any_success);
	RUN_TEST(test_data_turns_stale_without_a_success);
	RUN_TEST(test_success_clears_the_stale_warning);
	RUN_TEST(test_data_age_is_in_whole_minutes);
	RUN_TEST(test_data_age_works_across_clock_rollover);
	return UNITY_END();
}