#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <stddef.h>
#include <stdint.h>

// Device settings, kept free of hardware headers so the native tests can use them

// Numeric formatting type
enum formatNum { FORMAT_US = 1, FORMAT_EU = 2 };

constexpr size_t WIFI_SSID_SIZE = 35;
constexpr size_t WIFI_PASSWORD_SIZE = 70;
constexpr size_t API_KEY_SIZE = 35;
constexpr size_t CRYPTO_COIN_SIZE = 16;

struct DeviceConfig {
	bool currentPriceVisible;
	bool priceChangeVisible;
	bool marketCapVisible;
	bool dailyHighLowVisible;
	bool yearHighLowVisible;
	bool openPriceVisible;
	bool volumeVisible;
	char cryptoCoin[CRYPTO_COIN_SIZE];
	formatNum formatType;
	uint8_t matrixIntensity;
	uint8_t scrollSpeed;
	char ssid[WIFI_SSID_SIZE];
	char password[WIFI_PASSWORD_SIZE];
	char apiKey[API_KEY_SIZE];
};

#endif