#include "storage.h"
#include <Preferences.h>
#include "../config/config.h"
#include "../serial/serial.h"

namespace {
	// NVS namespace and keys (keys are limited to 15 characters)
	constexpr char STORAGE_NAMESPACE[] = "ticker";
	constexpr char CURRENT_PRICE_KEY[] = "showPrice";
	constexpr char PRICE_CHANGE_KEY[] = "showChange";
	constexpr char MARKET_CAP_KEY[] = "showMarketCap";
	constexpr char DAILY_HIGH_LOW_KEY[] = "showDayRange";
	constexpr char YEAR_HIGH_LOW_KEY[] = "showAllTime";
	constexpr char OPEN_PRICE_KEY[] = "showOpen";
	constexpr char VOLUME_KEY[] = "showVolume";
	constexpr char CRYPTO_COIN_KEY[] = "coin";
	constexpr char FORMAT_TYPE_KEY[] = "format";
	constexpr char MATRIX_INTENSITY_KEY[] = "intensity";
	constexpr char SCROLL_SPEED_KEY[] = "scrollSpeed";
	constexpr char API_KEY_KEY[] = "apiKey";

	// Read the stored settings on top of the defaults (missing keys keep their default)
	DeviceConfig loadDeviceConfig(Preferences& preferences, const DeviceConfig& defaults) {
		DeviceConfig config = defaults;

		// Section visibility
		config.currentPriceVisible = preferences.getBool(CURRENT_PRICE_KEY, defaults.currentPriceVisible);
		config.priceChangeVisible = preferences.getBool(PRICE_CHANGE_KEY, defaults.priceChangeVisible);
		config.marketCapVisible = preferences.getBool(MARKET_CAP_KEY, defaults.marketCapVisible);
		config.dailyHighLowVisible = preferences.getBool(DAILY_HIGH_LOW_KEY, defaults.dailyHighLowVisible);
		config.yearHighLowVisible = preferences.getBool(YEAR_HIGH_LOW_KEY, defaults.yearHighLowVisible);
		config.openPriceVisible = preferences.getBool(OPEN_PRICE_KEY, defaults.openPriceVisible);
		config.volumeVisible = preferences.getBool(VOLUME_KEY, defaults.volumeVisible);

		// Coin, number format and matrix
		if (preferences.isKey(CRYPTO_COIN_KEY)) preferences.getString(CRYPTO_COIN_KEY, config.cryptoCoin, sizeof(config.cryptoCoin));
		config.formatType = preferences.getUChar(FORMAT_TYPE_KEY, defaults.formatType) == FORMAT_EU ? FORMAT_EU : FORMAT_US;
		config.matrixIntensity = preferences.getUChar(MATRIX_INTENSITY_KEY, defaults.matrixIntensity);
		config.scrollSpeed = preferences.getUChar(SCROLL_SPEED_KEY, defaults.scrollSpeed);

		// API key
		if (preferences.isKey(API_KEY_KEY)) preferences.getString(API_KEY_KEY, config.apiKey, sizeof(config.apiKey));

		// Return the loaded settings
		return config;
	}
}

// Load the stored settings into the runtime config
void setupStorage() {
	Preferences preferences;

	// Without stored settings (first boot), keep the defaults
	if (!preferences.begin(STORAGE_NAMESPACE, true)) {
		printLogln("No stored settings, using the defaults");
		return;
	}

	// Apply the stored settings
	setDeviceConfig(loadDeviceConfig(preferences, getDeviceConfig()));
	preferences.end();
	printLogln("Settings loaded");
}

// Save the settings (NVS skips values that didn't change)
bool saveDeviceConfigToStorage(const DeviceConfig& config) {
	Preferences preferences;
	if (!preferences.begin(STORAGE_NAMESPACE, false)) return false;

	// Write every setting, checking each write
	bool isSaved = true;
	isSaved &= preferences.putBool(CURRENT_PRICE_KEY, config.currentPriceVisible) > 0;
	isSaved &= preferences.putBool(PRICE_CHANGE_KEY, config.priceChangeVisible) > 0;
	isSaved &= preferences.putBool(MARKET_CAP_KEY, config.marketCapVisible) > 0;
	isSaved &= preferences.putBool(DAILY_HIGH_LOW_KEY, config.dailyHighLowVisible) > 0;
	isSaved &= preferences.putBool(YEAR_HIGH_LOW_KEY, config.yearHighLowVisible) > 0;
	isSaved &= preferences.putBool(OPEN_PRICE_KEY, config.openPriceVisible) > 0;
	isSaved &= preferences.putBool(VOLUME_KEY, config.volumeVisible) > 0;
	isSaved &= preferences.putString(CRYPTO_COIN_KEY, config.cryptoCoin) > 0;
	isSaved &= preferences.putUChar(FORMAT_TYPE_KEY, config.formatType) > 0;
	isSaved &= preferences.putUChar(MATRIX_INTENSITY_KEY, config.matrixIntensity) > 0;
	isSaved &= preferences.putUChar(SCROLL_SPEED_KEY, config.scrollSpeed) > 0;
	isSaved &= preferences.putString(API_KEY_KEY, config.apiKey) == strlen(config.apiKey);
	preferences.end();

	// Log the result
	printLogln(isSaved ? "Settings saved" : "Failed to save the settings");
	return isSaved;
}

// Remove all stored settings (the defaults apply after the next boot)
bool clearStoredDeviceConfig() {
	Preferences preferences;
	if (!preferences.begin(STORAGE_NAMESPACE, false)) return false;
	const bool isCleared = preferences.clear();
	preferences.end();
	return isCleared;
}