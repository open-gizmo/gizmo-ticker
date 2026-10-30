#include "settings_json.h"
#include <stdio.h>
#include <string.h>

namespace {
	// Copy a string into a fixed-size field, cutting it off safely if it is too long
	void copyText(char* destination, const char* text, size_t length) {
		if (length == 0) return;
		snprintf(destination, length, "%s", text == nullptr ? "" : text);
	}
}

// Write the display settings to a JSON object
void writeSettingsToJson(JsonObject json, const DeviceConfig& config) {
	json["currentPrice"] = config.currentPriceVisible;
	json["priceChange"] = config.priceChangeVisible;
	json["marketCap"] = config.marketCapVisible;
	json["dailyHighLow"] = config.dailyHighLowVisible;
	json["yearHighLow"] = config.yearHighLowVisible;
	json["openPrice"] = config.openPriceVisible;
	json["volume"] = config.volumeVisible;
	json["cryptoCoin"] = config.cryptoCoin;
	json["formatType"] = config.formatType == FORMAT_US ? "US" : "EU";
	json["matrixIntensity"] = config.matrixIntensity;
	json["scrollSpeed"] = config.scrollSpeed;
}

// Apply the display settings present in a JSON object (missing fields keep their current value)
void applySettingsFromJson(JsonObjectConst json, DeviceConfig& config) {
	// Section visibility
	if (!json["currentPrice"].isNull()) config.currentPriceVisible = json["currentPrice"].as<bool>();
	if (!json["priceChange"].isNull()) config.priceChangeVisible = json["priceChange"].as<bool>();
	if (!json["marketCap"].isNull()) config.marketCapVisible = json["marketCap"].as<bool>();
	if (!json["dailyHighLow"].isNull()) config.dailyHighLowVisible = json["dailyHighLow"].as<bool>();
	if (!json["yearHighLow"].isNull()) config.yearHighLowVisible = json["yearHighLow"].as<bool>();
	if (!json["openPrice"].isNull()) config.openPriceVisible = json["openPrice"].as<bool>();
	if (!json["volume"].isNull()) config.volumeVisible = json["volume"].as<bool>();

	// Coin, number format and matrix
	if (!json["cryptoCoin"].isNull()) copyText(config.cryptoCoin, json["cryptoCoin"].as<const char*>(), sizeof(config.cryptoCoin));
	if (!json["formatType"].isNull()) config.formatType = strcmp(json["formatType"].as<const char*>(), "US") == 0 ? FORMAT_US : FORMAT_EU;
	if (!json["matrixIntensity"].isNull()) config.matrixIntensity = json["matrixIntensity"].as<uint8_t>();
	if (!json["scrollSpeed"].isNull()) config.scrollSpeed = json["scrollSpeed"].as<uint8_t>();
}