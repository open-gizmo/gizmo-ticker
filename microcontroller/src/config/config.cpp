#include "config.h"

namespace {
	DeviceConfig currentConfig = {
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		"bitcoin",
		FORMAT_US,
		1,
		1,
		"",
		"",
		""
	};
}

// Get the current device config
const DeviceConfig& getDeviceConfig() {
	return currentConfig;
}

// Update the device config
void setDeviceConfig(const DeviceConfig& config) {
	currentConfig = config;
}

// Global message buffers shared by serial and scrolling functions
char currentMessage[BUF_SIZE]; // Current message

// WiFi and client configuration
AsyncWebServer server(80);
WiFiClient client;
const char accessPointSSID[] = "Bitcoin-Ticker";
const char mdnsHostname[] = "ticker";
bool accessPointEnabled = false;
bool disableAccessPoint = false;
connectionStatus wiFiConnectionStatus = WIFI_KO;
unsigned long timestampStockData = 0;
unsigned long timestampWiFiConnection = 0;

// Scrolling text configuration
printType switchText = PRINT_PRICE;
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
uint8_t scrollDelay = 30;
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 0;