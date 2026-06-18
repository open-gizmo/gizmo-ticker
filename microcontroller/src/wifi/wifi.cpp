#include "wifi.h"
#include <Arduino.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "../config/config.h"
#include "../eeprom/eeprom.h"
#include "../matrix/matrix.h"
#include "../serial/serial.h"

/*
The main loop never waits inside a Wi-Fi retry loop.
Instead it starts a connection attempt, polls its progress across later loop iterations,
keeps the captive-portal access point available, and only surfaces the current status message.
*/

namespace {
	DNSServer dnsServer; // DNS server for captive portal
	constexpr byte DNS_PORT = 53; // DNS port for captive portal
	constexpr unsigned long WIFI_CONNECT_TIMEOUT = 12500;
	constexpr unsigned long WIFI_RETRY_INTERVAL = 10000;

	// Shared Wi-Fi struct
	struct WiFiRuntimeState {
		bool mdnsActive = false;
		bool connectInProgress = false;
		bool persistCredentialsOnSuccess = false;
		unsigned long connectStartedAt = 0;
		unsigned long nextReconnectAttemptAt = 0;
	};

	// Create the shared Wi-Fi state instance
	WiFiRuntimeState wifiState;

	// Forward declarations for helper functions
	void stopMdns();
	void startMdns();

	// Clear the current connection attempt state
	void resetConnectionAttempt() {
		wifiState.connectInProgress = false;
		wifiState.connectStartedAt = 0;
	}

	// Start a connection attempt if credentials are available
	bool beginWiFiConnectionAttempt() {
		const DeviceConfig& config = getDeviceConfig();

		// If the SSID is empty, we cannot start a connection attempt
		if (config.ssid[0] == '\0') {
			return false;
		}

		// Start the connection attempt and mark it as in progress
		WiFi.disconnect();
		WiFi.begin(config.ssid, config.password);
		wifiState.connectInProgress = true;
		wifiState.connectStartedAt = millis();
		printLogfln("Connecting to WiFi SSID: %s", config.ssid);
		return true;
	}

	// Mark the current Wi-Fi attempt as successful and schedule no further retries
	void completeWiFiConnectionSuccess() {
		const bool wasAttemptInProgress = wifiState.connectInProgress;
		resetConnectionAttempt();
		wifiState.nextReconnectAttemptAt = 0;
		wiFiConnectionStatus = WIFI_OK;

		// If we weren't already connected, print the success message and persist the credentials if needed
		if (wasAttemptInProgress) {
			printLogln(" connected!");
			if (wifiState.persistCredentialsOnSuccess) {
				writeEEPROM();
				wifiState.persistCredentialsOnSuccess = false;
			}
		}

		// Start mDNS service
		startMdns();
	}

	// Mark the current Wi-Fi attempt as failed and schedule a future retry
	void completeWiFiConnectionFailure() {
		WiFi.disconnect();
		resetConnectionAttempt();
		wiFiConnectionStatus = WIFI_KO;
		wifiState.nextReconnectAttemptAt = millis() + WIFI_RETRY_INTERVAL;
		printLogfln("Failed to connect to WiFi, retrying in %lu ms", WIFI_RETRY_INTERVAL);
	}

	// Check whether a fresh connection attempt should start on this loop iteration
	bool shouldStartWiFiConnectionAttempt(unsigned long currentMillis) {
		return wiFiConnectionStatus == WIFI_TRY || currentMillis >= wifiState.nextReconnectAttemptAt;
	}

	// Choose the matrix message for the current Wi-Fi state
	const char* getWiFiStatusMessage() {
		const char connectingMessage[] = "Connecting to Wi-Fi...";
		const char disconnectedMessage[] = "Not connected to Wi-Fi. Use the 'Bitcoin-Ticker' access point to enter the Wi-Fi credentials.";
		return wiFiConnectionStatus == WIFI_TRY ? connectingMessage : disconnectedMessage;
	}

	// Stop mDNS service if active
	void stopMdns() {
		if (!wifiState.mdnsActive) return;
		MDNS.end();
		wifiState.mdnsActive = false;
		printLogfln("mDNS stopped");
	}

	// Start mDNS service
	void startMdns() {
		// Don't start mDNS if not connected to Wi-Fi
		if (WiFi.status() != WL_CONNECTED) {
			stopMdns();
			return;
		}

		// Don't start mDNS if it's already active
		if (wifiState.mdnsActive) {
			return;
		}

		// Start mDNS with the configured hostname
		if (!MDNS.begin(mdnsHostname)) {
			printLogfln("Failed to start mDNS on http://%s.local", mdnsHostname);
			return;
		}

		// Advertise the HTTP service on mDNS
		MDNS.addService("http", "tcp", 80);
		wifiState.mdnsActive = true;
		printLogfln("mDNS ready on http://%s.local", mdnsHostname);
	}
}

// Enter access point mode
void setAccessPointMode() {
	WiFi.mode(WIFI_AP_STA);
}

// Setting up the access point
bool setupAccessPoint() {
	stopMdns();

	// If the access point is already enabled, return success
	if (accessPointEnabled) {
		return true; // If enabled exit the function
	}

	// Start the access point
	accessPointEnabled = WiFi.softAP(accessPointSSID);
	if (!accessPointEnabled) {
		printLogfln("Failed to start access point with SSID: %s", accessPointSSID);
		return false; // If failed to start access point exit the function
	}

	// Start the DNS server for captive portal
	dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
	printLogfln("Access point ready on http://%s", WiFi.softAPIP().toString().c_str());
	return true; // Access point enabled
}

// Queue a new Wi-Fi connection attempt for the next main-loop cycle
void queueWiFiConnectionAttempt() {
	resetConnectionAttempt();
	wifiState.nextReconnectAttemptAt = 0;
	wiFiConnectionStatus = WIFI_TRY;
	wifiState.persistCredentialsOnSuccess = true;
	WiFi.disconnect();
}

// Manage WiFi connection
bool manageWiFiConnection() {
	const unsigned long currentMillis = millis();

	// Process DNS requests if the access point is enabled
	if (accessPointEnabled) {
		dnsServer.processNextRequest();
	}

	// Check if I need to disable the access point
	if (disableAccessPoint && accessPointEnabled) {
		dnsServer.stop();
		accessPointEnabled = !WiFi.softAPdisconnect(); // Disable access point
		if (!accessPointEnabled) { // Check if disabled
			disableAccessPoint = false; // Mark as disabled
		}
	}

	// Check if connected to WiFi
	if (WiFi.status() == WL_CONNECTED) {
		completeWiFiConnectionSuccess();
		return true; // Connection success
	}

	// Stop mDNS if not connected to Wi-Fi
	stopMdns();

	// Poll the current connection attempt until it succeeds or times out
	if (wifiState.connectInProgress) {
		// If timeout, mark the attempt as failed and setup the access point for retries
		if (currentMillis - wifiState.connectStartedAt >= WIFI_CONNECT_TIMEOUT) {
			completeWiFiConnectionFailure();
			setupAccessPoint();
		}

		// Connection still in progress, return false for now
		return false;
	}

	// Check if credentials are already present
	const DeviceConfig& config = getDeviceConfig();
	if (config.ssid[0] != '\0') {
		// If credentials are present, start connection attempt immediately
		if (shouldStartWiFiConnectionAttempt(currentMillis)) {
			timestampWiFiConnection = currentMillis;
			if (beginWiFiConnectionAttempt()) {
				return false;
			}
		}

		// Keep the access point available while retries happen
		setupAccessPoint();
		return false;
	}

	// Setup access point
	setupAccessPoint();

	// Connection failed
	return false;
}

// Check if connected to WiFi
bool checkWifiConnection() {
	// If not connected, show a notice on the matrix
	if (WiFi.status() != WL_CONNECTED) {
		const char* errorMessage = getWiFiStatusMessage();
		printLogfln(errorMessage);
		printOnLedMatrix(errorMessage, strlen(errorMessage) + 1); // Print the message on the matrix

		// Not connected
		return false;
	}

	// Connected
	return true;
}