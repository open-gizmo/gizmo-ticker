#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <stddef.h>
#include <stdint.h>

// Device settings, kept free of hardware headers so the native tests can use them

// Numeric formatting type
enum formatNum { FORMAT_US = 1,
	FORMAT_EU = 2 };

constexpr size_t API_KEY_SIZE = 35;
constexpr size_t CRYPTO_COIN_SIZE = 16;

// Matrix modules (8x8 blocks) the display chains: sold as 4-in-1 boards, so in steps of 4
constexpr uint8_t MATRIX_MODULES_STEP = 4;
constexpr uint8_t MATRIX_MODULES_MIN = 4;
constexpr uint8_t MATRIX_MODULES_MAX = 32;
constexpr uint8_t MATRIX_MODULES_DEFAULT = 4; // One 4-in-1 board, as in the OpenGizmo enclosure

// Whether a number of modules is one the display can chain (a whole number of 4-in-1 boards, within the limits)
constexpr bool isValidMatrixModules(long modules) {
	return modules >= MATRIX_MODULES_MIN && modules <= MATRIX_MODULES_MAX && modules % MATRIX_MODULES_STEP == 0;
}

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
	uint8_t matrixModules;
	char apiKey[API_KEY_SIZE];
};

#endif