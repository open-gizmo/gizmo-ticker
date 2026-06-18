#include "serial/serial.h"
#include "eeprom/eeprom.h"
#include "api/api.h"
#include "wifi/wifi.h"
#include "matrix/matrix.h"
#include "server/server.h"

// Setup
void setup() {
	initSerial(); // Init serial
	setupEEPROM(); // Setup EEPROM
	setAccessPointMode(); // Enter access point mode (Do not start the access point)
	setupServer(); // Setup server
	manageWiFiConnection(); // Manage the WiFi connection
	setupWebClient(); // Setup web client
	setupLedMatrix(); // Setup LED matrix
}

// Loop
void loop() {
	// Manage the WiFi connection
	manageWiFiConnection();

	// Wait until the current matrix animation cycle finishes before preparing the next message
	if (!isLedMatrixReadyForNextMessage()) {
		return;
	}

	// Only render ticker data when the device is connected
	if (!checkWifiConnection()) {
		return;
	}

	// Load or refresh the market data before rendering the next matrix message
	MarketTickerData marketData;
	if (!callAPI(marketData)) {
		return;
	}

	// Render the next ticker message on the LED matrix
	renderNextLedMatrixMessage(marketData);
}