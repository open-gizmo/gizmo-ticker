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

	// Guard the config (shared by the main loop, the web server task, and the market data task)
	portMUX_TYPE configMux = portMUX_INITIALIZER_UNLOCKED;
}

// Get a copy of the current device config
DeviceConfig getDeviceConfig() {
	portENTER_CRITICAL(&configMux);
	const DeviceConfig config = currentConfig;
	portEXIT_CRITICAL(&configMux);
	return config;
}

// Update the device config
void setDeviceConfig(const DeviceConfig& config) {
	portENTER_CRITICAL(&configMux);
	currentConfig = config;
	portEXIT_CRITICAL(&configMux);
}

// Global message buffers shared by serial and scrolling functions
char currentMessage[BUF_SIZE]; // Current message

// WiFi and client configuration
AsyncWebServer server(80);
WiFiClient client;
const char accessPointSSID[] = "Gizmo-Ticker";
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