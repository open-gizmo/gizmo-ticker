#include <unity.h>
#include <math.h>

#include "api/market_data.h"

namespace {
	// Parse a CoinGecko reply into a document
	JsonDocument parseReply(const char* jsonText) {
		JsonDocument doc;
		TEST_ASSERT_FALSE(deserializeJson(doc, jsonText));
		return doc;
	}

	// Compare a value with what the reply said (ArduinoJson keeps short numbers as float, so allow its precision)
	void assertSameValue(double expected, double actual) {
		TEST_ASSERT_DOUBLE_WITHIN(fabs(expected) * 1e-6, expected, actual);
	}
}

// Nothing to prepare before each test
void setUp() {}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Coins ********************************/

// Each coin has its CoinGecko ID and its ticker label
void test_coins_have_their_api_id_and_label() {
	TEST_ASSERT_EQUAL_STRING("bitcoin", getCryptoApiId("bitcoin"));
	TEST_ASSERT_EQUAL_STRING("BTC", getCryptoTickerLabel("bitcoin"));
	TEST_ASSERT_EQUAL_STRING("kaspa", getCryptoApiId("kaspa"));
	TEST_ASSERT_EQUAL_STRING("KAS", getCryptoTickerLabel("kaspa"));
}

// An unknown coin falls back to Bitcoin, for the ID and the label alike
void test_unknown_coin_falls_back_to_bitcoin() {
	TEST_ASSERT_EQUAL_STRING("bitcoin", getCryptoApiId("dogecoin"));
	TEST_ASSERT_EQUAL_STRING("BTC", getCryptoTickerLabel("dogecoin"));
}

/******************************** Market data ********************************/

// Every field is read from the reply, and the open price is derived from the 24 h change
void test_reply_fills_every_field() {
	const JsonDocument doc = parseReply(R"([{"current_price":0.09,"price_change_percentage_24h_in_currency":-2.5,"price_change_24h":-0.002,"market_cap":2300000000,"high_24h":0.095,"low_24h":0.088,"total_volume":45000000,"ath":0.207,"atl":0.0002}])");
	MarketTickerData marketData = {};

	// Parse the reply
	TEST_ASSERT_TRUE(parseMarketData(doc.as<JsonVariantConst>(), "kaspa", marketData));

	// Expect each field, and the label of the coin
	TEST_ASSERT_EQUAL_STRING("KAS", marketData.tickerLabel);
	assertSameValue(0.09, marketData.currentPrice);
	assertSameValue(-2.5, marketData.priceChangePercentage24h);
	assertSameValue(-0.002, marketData.priceChange24h);
	assertSameValue(2300000000.0, marketData.marketCap);
	assertSameValue(0.095, marketData.dailyHigh);
	assertSameValue(0.088, marketData.dailyLow);
	assertSameValue(0.207, marketData.yearHigh);
	assertSameValue(0.0002, marketData.yearLow);
	assertSameValue(0.092, marketData.openPrice);
	assertSameValue(45000000.0, marketData.volume);
}

// Integer values in the reply are read as numbers too
void test_integer_values_are_read() {
	const JsonDocument doc = parseReply(R"([{"current_price":64000,"price_change_24h":1000}])");
	MarketTickerData marketData = {};

	// Parse the reply
	TEST_ASSERT_TRUE(parseMarketData(doc.as<JsonVariantConst>(), "bitcoin", marketData));

	// Expect the price and the derived open price
	assertSameValue(64000.0, marketData.currentPrice);
	assertSameValue(63000.0, marketData.openPrice);
}

// An empty reply (unknown coin on CoinGecko) is not market data
void test_empty_reply_is_rejected() {
	const JsonDocument emptyArray = parseReply("[]");
	const JsonDocument errorObject = parseReply(R"({"status":{"error_code":429,"error_message":"rate limited"}})");
	MarketTickerData marketData = {};

	// Expect both to be rejected
	TEST_ASSERT_FALSE(parseMarketData(emptyArray.as<JsonVariantConst>(), "bitcoin", marketData));
	TEST_ASSERT_FALSE(parseMarketData(errorObject.as<JsonVariantConst>(), "bitcoin", marketData));
}

// Run all the market data tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_coins_have_their_api_id_and_label);
	RUN_TEST(test_unknown_coin_falls_back_to_bitcoin);
	RUN_TEST(test_reply_fills_every_field);
	RUN_TEST(test_integer_values_are_read);
	RUN_TEST(test_empty_reply_is_rejected);
	return UNITY_END();
}