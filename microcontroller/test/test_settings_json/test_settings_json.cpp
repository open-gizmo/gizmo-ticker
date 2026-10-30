#include <unity.h>
#include <string.h>

#include "server/settings_json.h"

namespace {
	// Build the firmware's default config
	DeviceConfig buildDefaultConfig() {
		DeviceConfig config = {};
		config.currentPriceVisible = true;
		config.priceChangeVisible = true;
		config.marketCapVisible = true;
		config.dailyHighLowVisible = true;
		config.yearHighLowVisible = true;
		config.openPriceVisible = true;
		config.volumeVisible = true;
		strcpy(config.cryptoCoin, "bitcoin");
		config.formatType = FORMAT_US;
		config.matrixIntensity = 1;
		config.scrollSpeed = 1;
		return config;
	}

	// Apply a JSON text to a default config, like POST /api/settings does
	DeviceConfig applyJsonText(const char* jsonText) {
		JsonDocument doc;
		deserializeJson(doc, jsonText);
		DeviceConfig config = buildDefaultConfig();
		applySettingsFromJson(doc.as<JsonObjectConst>(), config);
		return config;
	}
}

// Nothing to prepare before each test
void setUp() {}

// Nothing to clean up after each test
void tearDown() {}

/******************************** GET /api/config ********************************/

// Every display setting is sent to the web app with the field names it expects
void test_settings_are_written_with_the_web_app_field_names() {
	JsonDocument doc;
	DeviceConfig config = buildDefaultConfig();
	config.volumeVisible = false;
	config.formatType = FORMAT_EU;
	config.matrixIntensity = 12;

	// Write the settings
	writeSettingsToJson(doc.to<JsonObject>(), config);

	// Expect every field with its value
	TEST_ASSERT_TRUE(doc["currentPrice"].as<bool>());
	TEST_ASSERT_FALSE(doc["volume"].as<bool>());
	TEST_ASSERT_EQUAL_STRING("bitcoin", doc["cryptoCoin"]);
	TEST_ASSERT_EQUAL_STRING("EU", doc["formatType"]);
	TEST_ASSERT_EQUAL_UINT8(12, doc["matrixIntensity"].as<uint8_t>());
	TEST_ASSERT_EQUAL_UINT8(1, doc["scrollSpeed"].as<uint8_t>());
	TEST_ASSERT_TRUE(doc["yearHighLow"].is<bool>());
}

// Secrets never go out with the display settings (the API key is masked separately)
void test_api_key_is_not_written() {
	JsonDocument doc;
	DeviceConfig config = buildDefaultConfig();
	strcpy(config.apiKey, "CG-secret");

	// Write the settings
	writeSettingsToJson(doc.to<JsonObject>(), config);

	// Expect no API key
	TEST_ASSERT_FALSE(doc["apiKey"].is<const char*>());
}

/******************************** POST /api/settings ********************************/

// Every field the web app sends is applied
void test_all_fields_are_applied() {
	const DeviceConfig config = applyJsonText("{\"currentPrice\":false,\"priceChange\":false,\"marketCap\":false,\"dailyHighLow\":false,\"yearHighLow\":false,\"openPrice\":false,\"volume\":false,\"cryptoCoin\":\"kaspa\",\"formatType\":\"EU\",\"matrixIntensity\":15,\"scrollSpeed\":9}");

	// Expect every value
	TEST_ASSERT_FALSE(config.currentPriceVisible);
	TEST_ASSERT_FALSE(config.priceChangeVisible);
	TEST_ASSERT_FALSE(config.marketCapVisible);
	TEST_ASSERT_FALSE(config.dailyHighLowVisible);
	TEST_ASSERT_FALSE(config.yearHighLowVisible);
	TEST_ASSERT_FALSE(config.openPriceVisible);
	TEST_ASSERT_FALSE(config.volumeVisible);
	TEST_ASSERT_EQUAL_STRING("kaspa", config.cryptoCoin);
	TEST_ASSERT_EQUAL(FORMAT_EU, config.formatType);
	TEST_ASSERT_EQUAL_UINT8(15, config.matrixIntensity);
	TEST_ASSERT_EQUAL_UINT8(9, config.scrollSpeed);
}

// Missing fields keep their current values (the web app can send a partial update)
void test_missing_fields_keep_current_values() {
	const DeviceConfig config = applyJsonText("{\"matrixIntensity\":9}");

	// Expect only the sent field to change
	TEST_ASSERT_EQUAL_UINT8(9, config.matrixIntensity);
	TEST_ASSERT_EQUAL_STRING("bitcoin", config.cryptoCoin);
	TEST_ASSERT_TRUE(config.volumeVisible);
	TEST_ASSERT_EQUAL(FORMAT_US, config.formatType);
}

// The number format is sent as "US" or "EU"
void test_format_type_values() {
	TEST_ASSERT_EQUAL(FORMAT_EU, applyJsonText("{\"formatType\":\"EU\"}").formatType);
	TEST_ASSERT_EQUAL(FORMAT_US, applyJsonText("{\"formatType\":\"US\"}").formatType);
}

// A coin name longer than its field is cut off safely instead of overflowing
void test_overlong_coin_is_cut_off_safely() {
	const DeviceConfig config = applyJsonText("{\"cryptoCoin\":\"averyveryverylongcoinname\"}");

	// Expect the field to be full and terminated
	TEST_ASSERT_EQUAL_size_t(CRYPTO_COIN_SIZE - 1, strlen(config.cryptoCoin));
}

// Writing and applying the settings gives the same config back
void test_settings_round_trip() {
	JsonDocument doc;
	DeviceConfig config = buildDefaultConfig();
	strcpy(config.cryptoCoin, "kaspa");
	config.marketCapVisible = false;
	config.scrollSpeed = 7;

	// Write, then apply on top of the defaults
	writeSettingsToJson(doc.to<JsonObject>(), config);
	DeviceConfig reloadedConfig = buildDefaultConfig();
	applySettingsFromJson(doc.as<JsonObjectConst>(), reloadedConfig);

	// Expect the same values
	TEST_ASSERT_EQUAL_STRING("kaspa", reloadedConfig.cryptoCoin);
	TEST_ASSERT_FALSE(reloadedConfig.marketCapVisible);
	TEST_ASSERT_EQUAL_UINT8(7, reloadedConfig.scrollSpeed);
}

// Run all the settings JSON tests
int main() {
	UNITY_BEGIN();

	// GET /api/config
	RUN_TEST(test_settings_are_written_with_the_web_app_field_names);
	RUN_TEST(test_api_key_is_not_written);

	// POST /api/settings
	RUN_TEST(test_all_fields_are_applied);
	RUN_TEST(test_missing_fields_keep_current_values);
	RUN_TEST(test_format_type_values);
	RUN_TEST(test_overlong_coin_is_cut_off_safely);
	RUN_TEST(test_settings_round_trip);

	return UNITY_END();
}