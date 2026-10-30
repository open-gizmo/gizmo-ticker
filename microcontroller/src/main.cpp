#include "serial/serial.h"
#include "storage/storage.h"
#include "api/api.h"
#include "wifi/wifi.h"
#include "matrix/matrix.h"
#include "server/server.h"

// Setup
void setup() {
	initSerial(); // Init serial
	setupStorage(); // Load the saved settings
	setupWiFi(); // Start the Wi-Fi setup and its web API
	setupServer(); // Setup server
	manageWiFiConnection(); // Manage the WiFi connection
	setupWebClient(); // Setup web client
	setupLedMatrix(); // Setup LED matrix
}

// Loop
void loop() {
	// Restart the device if the web server asked for it
	manageScheduledRestart();

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