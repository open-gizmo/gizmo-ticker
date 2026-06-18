#include "matrix.h"
#include "../config/config.h"
#include "../utils/utils.h"
#include "../api/api.h"
#include "../serial/serial.h"

namespace {
	constexpr uint16_t PRICE_MESSAGE_PAUSE = 30000;
	constexpr byte MAX_NUMBER_SIZE = 30;

	// Format the current price message
	void formatPriceMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		char tempString2[MAX_NUMBER_SIZE];
		formatCurrency(marketData.currentPrice, tempString, MAX_NUMBER_SIZE);
		formatCurrency(marketData.priceChangePercentage24h, tempString2, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, " %s $ %s  (%s%%)", marketData.tickerLabel, tempString, tempString2);
	}

	// Format the price change message
	void formatChangeMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		formatCurrency(marketData.priceChange24h, tempString, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, "Daily Change: $ %s", tempString);
	}

	// Format the market cap message
	void formatMarketCapMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		formatCurrency(marketData.marketCap, tempString, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, "Market Cap: $ %s", tempString);
	}

	// Format the daily high/low message
	void formatDailyHighLowMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		char tempString2[MAX_NUMBER_SIZE];
		formatCurrency(marketData.dailyHigh, tempString, MAX_NUMBER_SIZE);
		formatCurrency(marketData.dailyLow, tempString2, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, "Daily High: $ %s  -  Daily Low: $ %s", tempString, tempString2);
	}

	// Format the year high/low message
	void formatYearHighLowMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		char tempString2[MAX_NUMBER_SIZE];
		formatCurrency(marketData.yearHigh, tempString, MAX_NUMBER_SIZE);
		formatCurrency(marketData.yearLow, tempString2, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, "Year High: $ %s  -  Year Low: $ %s", tempString, tempString2);
	}

	// Format the open price message
	void formatOpenMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		formatCurrency(marketData.openPrice, tempString, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, "Open: $ %s", tempString);
	}

	// Format the volume message
	void formatVolumeMessage(char* message, size_t messageLength, const MarketTickerData& marketData) {
		char tempString[MAX_NUMBER_SIZE];
		formatCurrency(marketData.volume, tempString, MAX_NUMBER_SIZE);
		snprintf(message, messageLength, "Volume: $ %s", tempString);
	}
}

// Print the message on the matrix
void printOnLedMatrix(const char* message, const byte stringLength, uint16_t messageStill) {
	stringCopy(currentMessage, message, stringLength); // Copy the message
	P.displayText(currentMessage, scrollAlign, scrollDelay, messageStill, scrollEffect, scrollEffect); // Print the message on the matrix
}

// Set the intensity of the matrix
void setMatrixIntensity(uint8_t intensity) {
	P.setIntensity(intensity); // Set LED intensity (0-15, 15 is brightest)
}

// Set the speed of the matrix
void setMatrixSpeed(uint8_t speed) {
    scrollDelay = map(speed, 0, 15, 100, 20); // Map 0-15 to 100-20ms (slower to faster)
}

// Setup the LED matrix
void setupLedMatrix() {
	P.begin(); // Start the LED matrix
	const DeviceConfig& config = getDeviceConfig();
	setMatrixIntensity(config.matrixIntensity); // Set the intensity of the matrix
	printOnLedMatrix("Initializing...", BUF_SIZE); // Print the message on the matrix
}

// Check whether the matrix finished the current animation cycle
bool isLedMatrixReadyForNextMessage() {
	return P.displayAnimate();
}

// Manage the LED matrix using already-available market data
void renderNextLedMatrixMessage(const MarketTickerData& marketData) {
	// Get the current config
	const DeviceConfig& config = getDeviceConfig();
	
	// Single buffer for all the messages
	char message[BUF_SIZE];

	// Print messages
	switch (switchText) {
		case PRINT_PRICE:
			// Show the data only if enabled
			if (config.currentPriceVisible) {
				printLogfln("Showing price section...");
				formatPriceMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message), PRICE_MESSAGE_PAUSE); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_CHANGE;
			break;

		case PRINT_CHANGE:
			// Show the data only if enabled
			if (config.priceChangeVisible) {
				printLogfln("Showing change section...");
				formatChangeMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message)); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_MARKET_CAP;
			break;
		
		case PRINT_MARKET_CAP:
			// Show the data only if enabled
			if (config.marketCapVisible) {
				printLogfln("Showing market cap section...");
				formatMarketCapMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message)); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_DAILY_HIGH_LOW;
			break;

		case PRINT_DAILY_HIGH_LOW:
			// Show the data only if enabled
			if (config.dailyHighLowVisible) {
				printLogfln("Showing daily high/low section...");
				formatDailyHighLowMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message)); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_YEAR_HIGH_LOW;
			break;

		case PRINT_YEAR_HIGH_LOW:
			// Show the data only if enabled
			if (config.yearHighLowVisible) {
				printLogfln("Showing year high/low section...");
				formatYearHighLowMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message)); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_OPEN;
			break;

		case PRINT_OPEN:
			// Show the data only if enabled
			if (config.openPriceVisible) {
				printLogfln("Showing open price section...");
				formatOpenMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message)); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_VOLUME;
			break;
			
		case PRINT_VOLUME:
			// Show the data only if enabled
			if (config.volumeVisible) {
				printLogfln("Showing volume section...");
				formatVolumeMessage(message, sizeof(message), marketData);
				printOnLedMatrix(message, sizeof(message)); // Print the message on the matrix
			}

			// Move to the next section
			switchText = PRINT_PRICE;
			break;
	}
}