#include "config_json.h"
#include <stdio.h>
#include <string.h>

namespace {
    // Copy a string into a fixed-size field, cutting it off safely if it is too long
    void copyText(char* destination, const char* text, size_t length) {
        if (length == 0) return;
        snprintf(destination, length, "%s", text == nullptr ? "" : text);
    }
}

// Apply config values to a JSON document
void applyConfigToJson(JsonDocument& doc, const DeviceConfig& config) {
    // Add the config fields to the JSON object
    doc["currentPrice"] = config.currentPriceVisible;
    doc["priceChange"] = config.priceChangeVisible;
    doc["marketCap"] = config.marketCapVisible;
    doc["dailyHighLow"] = config.dailyHighLowVisible;
    doc["yearHighLow"] = config.yearHighLowVisible;
    doc["openPrice"] = config.openPriceVisible;
    doc["volume"] = config.volumeVisible;
    doc["cryptoCoin"] = config.cryptoCoin;
    doc["formatType"] = config.formatType == FORMAT_US ? "US" : "EU";
    doc["matrixIntensity"] = config.matrixIntensity;
    doc["scrollSpeed"] = config.scrollSpeed;

    // Add WiFi credentials if present (the password can be empty for open networks)
    if (strlen(config.ssid) != 0) {
        doc["ssid"] = config.ssid;
        doc["password"] = config.password;
    }

    // Add API key if present
    if (strlen(config.apiKey) != 0) {
        doc["apiKey"] = config.apiKey;
    }
}

// Apply config values from a JSON document to a config snapshot
void applyConfigFromJson(JsonDocument& doc, DeviceConfig& config) {
    if (!doc["currentPrice"].isNull())
        config.currentPriceVisible = doc["currentPrice"].as<bool>();
    if (!doc["priceChange"].isNull())
        config.priceChangeVisible = doc["priceChange"].as<bool>();
    if (!doc["marketCap"].isNull())
        config.marketCapVisible = doc["marketCap"].as<bool>();
    if (!doc["dailyHighLow"].isNull())
        config.dailyHighLowVisible = doc["dailyHighLow"].as<bool>();
    if (!doc["yearHighLow"].isNull())
        config.yearHighLowVisible = doc["yearHighLow"].as<bool>();
    if (!doc["openPrice"].isNull())
        config.openPriceVisible = doc["openPrice"].as<bool>();
    if (!doc["volume"].isNull())
        config.volumeVisible = doc["volume"].as<bool>();
    if (!doc["cryptoCoin"].isNull())
        copyText(config.cryptoCoin, doc["cryptoCoin"].as<const char*>(), sizeof(config.cryptoCoin));
    if (!doc["formatType"].isNull())
        config.formatType = strcmp(doc["formatType"].as<const char*>(), "US") == 0 ? FORMAT_US : FORMAT_EU;
    if (!doc["matrixIntensity"].isNull())
        config.matrixIntensity = doc["matrixIntensity"].as<uint8_t>();
    if (!doc["scrollSpeed"].isNull())
        config.scrollSpeed = doc["scrollSpeed"].as<uint8_t>();
    if (!doc["apiKey"].isNull())
        copyText(config.apiKey, doc["apiKey"], sizeof(config.apiKey));
    if (!doc["ssid"].isNull())
        copyText(config.ssid, doc["ssid"], sizeof(config.ssid));
    if (!doc["password"].isNull())
        copyText(config.password, doc["password"], sizeof(config.password));
}