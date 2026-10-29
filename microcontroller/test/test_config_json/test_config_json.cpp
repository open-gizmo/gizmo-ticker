#include <unity.h>
#include <string.h>

#include "eeprom/config_json.h"

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

	// Save a config to JSON and read it back into a default config, like a reboot does
	DeviceConfig saveAndReload(const DeviceConfig& config) {
		JsonDocument doc;
		applyConfigToJson(doc, config);

		// Go through text, like the EEPROM does
		char json[512];
		serializeJson(doc, json, sizeof(json));
		JsonDocument storedDoc;
		deserializeJson(storedDoc, json);

		// Read it back on top of the defaults
		DeviceConfig reloadedConfig = buildDefaultConfig();
		applyConfigFromJson(storedDoc, reloadedConfig);
		return reloadedConfig;
	}
}

// Nothing to prepare before each test
void setUp() {}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Saving and reloading ********************************/

// Every setting survives a save and reload
void test_all_settings_survive_a_reboot() {
	// Change every setting away from its default
	DeviceConfig config = buildDefaultConfig();
	config.currentPriceVisible = false;
	config.volumeVisible = false;
	strcpy(config.cryptoCoin, "kaspa");
	config.formatType = FORMAT_EU;
	config.matrixIntensity = 12;
	config.scrollSpeed = 7;
	strcpy(config.ssid, "HomeNetwork");
	strcpy(config.password, "secret-password");
	strcpy(config.apiKey, "CG-abc123");

	// Save and reload
	const DeviceConfig reloadedConfig = saveAndReload(config);

	// Expect every value back
	TEST_ASSERT_FALSE(reloadedConfig.currentPriceVisible);
	TEST_ASSERT_TRUE(reloadedConfig.priceChangeVisible);
	TEST_ASSERT_FALSE(reloadedConfig.volumeVisible);
	TEST_ASSERT_EQUAL_STRING("kaspa", reloadedConfig.cryptoCoin);
	TEST_ASSERT_EQUAL(FORMAT_EU, reloadedConfig.formatType);
	TEST_ASSERT_EQUAL_UINT8(12, reloadedConfig.matrixIntensity);
	TEST_ASSERT_EQUAL_UINT8(7, reloadedConfig.scrollSpeed);
	TEST_ASSERT_EQUAL_STRING("HomeNetwork", reloadedConfig.ssid);
	TEST_ASSERT_EQUAL_STRING("secret-password", reloadedConfig.password);
	TEST_ASSERT_EQUAL_STRING("CG-abc123", reloadedConfig.apiKey);
}

// Open networks (empty password) used to be dropped, so the ticker forgot its Wi-Fi on every reboot
void test_open_network_survives_a_reboot() {
	// Configure an open network
	DeviceConfig config = buildDefaultConfig();
	strcpy(config.ssid, "CafeWiFi");

	// Save and reload
	const DeviceConfig reloadedConfig = saveAndReload(config);

	// Expect the network to be remembered, with an empty password
	TEST_ASSERT_EQUAL_STRING("CafeWiFi", reloadedConfig.ssid);
	TEST_ASSERT_EQUAL_STRING("", reloadedConfig.password);
}

// Without a network, no Wi-Fi fields are stored
void test_no_network_stores_no_wifi_fields() {
	JsonDocument doc;

	// Save a config without a network
	applyConfigToJson(doc, buildDefaultConfig());

	// Expect no Wi-Fi keys
	TEST_ASSERT_FALSE(doc["ssid"].is<const char*>());
	TEST_ASSERT_FALSE(doc["password"].is<const char*>());
}

// An empty API key is not stored
void test_empty_api_key_is_not_stored() {
	JsonDocument doc;

	// Save a config without an API key
	applyConfigToJson(doc, buildDefaultConfig());

	// Expect no API key
	TEST_ASSERT_FALSE(doc["apiKey"].is<const char*>());
}

/******************************** Reading stored JSON ********************************/

// Missing fields keep their current values (for example after a firmware update adds a setting)
void test_missing_fields_keep_current_values() {
	JsonDocument doc;
	deserializeJson(doc, "{\"matrixIntensity\":9}");

	// Read the partial JSON on top of the defaults
	DeviceConfig config = buildDefaultConfig();
	applyConfigFromJson(doc, config);

	// Expect only the stored field to change
	TEST_ASSERT_EQUAL_UINT8(9, config.matrixIntensity);
	TEST_ASSERT_EQUAL_STRING("bitcoin", config.cryptoCoin);
	TEST_ASSERT_TRUE(config.volumeVisible);
	TEST_ASSERT_EQUAL(FORMAT_US, config.formatType);
}

// The number format is stored as "US" or "EU"
void test_format_type_values() {
	JsonDocument usDoc;
	JsonDocument euDoc;
	deserializeJson(usDoc, "{\"formatType\":\"US\"}");
	deserializeJson(euDoc, "{\"formatType\":\"EU\"}");

	// Read each value
	DeviceConfig usConfig = buildDefaultConfig();
	DeviceConfig euConfig = buildDefaultConfig();
	usConfig.formatType = FORMAT_EU;
	applyConfigFromJson(usDoc, usConfig);
	applyConfigFromJson(euDoc, euConfig);

	// Expect the matching format
	TEST_ASSERT_EQUAL(FORMAT_US, usConfig.formatType);
	TEST_ASSERT_EQUAL(FORMAT_EU, euConfig.formatType);
}

// Strings longer than their field are cut off safely instead of overflowing
void test_overlong_strings_are_cut_off_safely() {
	JsonDocument doc;
	deserializeJson(doc, "{\"ssid\":\"ThisNetworkNameIsWayTooLongForTheField1234567890\"}");

	// Read the overlong value
	DeviceConfig config = buildDefaultConfig();
	applyConfigFromJson(doc, config);

	// Expect the field to be full and terminated
	TEST_ASSERT_EQUAL_size_t(WIFI_SSID_SIZE - 1, strlen(config.ssid));
	TEST_ASSERT_EQUAL_STRING_LEN("ThisNetworkNameIsWayTooLong", config.ssid, 27);
}

// The stored JSON fits the EEPROM even with every string field full
void test_full_config_fits_the_eeprom() {
	DeviceConfig config = buildDefaultConfig();
	memset(config.ssid, 'S', sizeof(config.ssid) - 1);
	memset(config.password, 'P', sizeof(config.password) - 1);
	memset(config.apiKey, 'K', sizeof(config.apiKey) - 1);
	memset(config.cryptoCoin, 'C', sizeof(config.cryptoCoin) - 1);

	// Serialize the largest possible config
	JsonDocument doc;
	applyConfigToJson(doc, config);

	// Expect it to fit the 512-byte EEPROM area with its terminator
	TEST_ASSERT_LESS_THAN_size_t(512, measureJson(doc));
}

// Run all the config JSON tests
int main() {
	UNITY_BEGIN();

	// Saving and reloading
	RUN_TEST(test_all_settings_survive_a_reboot);
	RUN_TEST(test_open_network_survives_a_reboot);
	RUN_TEST(test_no_network_stores_no_wifi_fields);
	RUN_TEST(test_empty_api_key_is_not_stored);

	// Reading stored JSON
	RUN_TEST(test_missing_fields_keep_current_values);
	RUN_TEST(test_format_type_values);
	RUN_TEST(test_overlong_strings_are_cut_off_safely);
	RUN_TEST(test_full_config_fits_the_eeprom);

	return UNITY_END();
}