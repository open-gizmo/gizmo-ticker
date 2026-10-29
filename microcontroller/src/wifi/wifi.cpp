#include "wifi.h"
#include <Arduino.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "../config/config.h"
#include "../eeprom/eeprom.h"
#include "../matrix/matrix.h"
#include "../serial/serial.h"
#include "../utils/utils.h"

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
	constexpr unsigned long ACCESS_POINT_SHUTDOWN_DELAY = 120000; // Keep the setup AP for 2 minutes after reconnecting (so a phone in the setup flow sees the result)

	// Shared Wi-Fi struct (used by the main loop only)
	struct WiFiRuntimeState {
		bool mdnsActive = false;
		bool connectInProgress = false;
		bool isAttemptWithNewCredentials = false; // The current attempt uses credentials that are not saved yet
		bool isWaitingForOldLinkToDrop = false; // The attempt started while connected, and the old link is still up
		bool isConnected = false;
		unsigned long connectedSince = 0;
		unsigned long connectStartedAt = 0;
		unsigned long reconnectDelayStartedAt = 0; // When the wait before the next attempt started
		unsigned long reconnectDelay = 0; // How long to wait before the next attempt (0 = try now)
	};

	// Create the shared Wi-Fi state instance
	WiFiRuntimeState wifiState;

	// New credentials from the web server, kept apart from the saved config until they work
	struct PendingCredentials {
		bool hasCredentials = false;
		bool isAttemptRequested = false;
		char ssid[WIFI_SSID_SIZE] = "";
		char password[WIFI_PASSWORD_SIZE] = "";
	};

	// Pending credentials, written by the web server task and read by the main loop
	PendingCredentials pendingCredentials;
	portMUX_TYPE pendingCredentialsMux = portMUX_INITIALIZER_UNLOCKED;

	// Forward declarations for helper functions
	void stopMdns();
	void startMdns();

	// Clear the current connection attempt state
	void resetConnectionAttempt() {
		wifiState.connectInProgress = false;
		wifiState.connectStartedAt = 0;
	}

	// Copy the pending credentials, if any
	bool readPendingCredentials(PendingCredentials& credentials) {
		portENTER_CRITICAL(&pendingCredentialsMux);
		credentials = pendingCredentials;
		portEXIT_CRITICAL(&pendingCredentialsMux);
		return credentials.hasCredentials;
	}

	// Forget the pending credentials
	void clearPendingCredentials() {
		portENTER_CRITICAL(&pendingCredentialsMux);
		pendingCredentials = PendingCredentials();
		portEXIT_CRITICAL(&pendingCredentialsMux);
	}

	// Check and clear a connection request from the web server
	bool takeConnectionRequest() {
		portENTER_CRITICAL(&pendingCredentialsMux);
		const bool isRequested = pendingCredentials.isAttemptRequested;
		pendingCredentials.isAttemptRequested = false;
		portEXIT_CRITICAL(&pendingCredentialsMux);
		return isRequested;
	}

	// Save the credentials that just worked and apply them to the config
	void persistWorkingCredentials(const PendingCredentials& credentials) {
		DeviceConfig config = getDeviceConfig();
		stringCopy(config.ssid, credentials.ssid, sizeof(config.ssid));
		stringCopy(config.password, credentials.password, sizeof(config.password));

		// Save before applying, so the runtime config matches what is stored
		if (!saveDeviceConfigToEEPROM(config)) {
			printLogln("Failed to save the new WiFi credentials");
		}
		setDeviceConfig(config);
	}

	// Start a connection attempt with the pending credentials, or the saved ones
	bool beginWiFiConnectionAttempt() {
		PendingCredentials credentials;
		const bool hasNewCredentials = readPendingCredentials(credentials);

		// Fall back to the saved credentials
		if (!hasNewCredentials) {
			const DeviceConfig config = getDeviceConfig();
			stringCopy(credentials.ssid, config.ssid, sizeof(credentials.ssid));
			stringCopy(credentials.password, config.password, sizeof(credentials.password));
		}

		// If the SSID is empty, we cannot start a connection attempt
		if (credentials.ssid[0] == '\0') {
			return false;
		}

		// Start the connection attempt and mark it as in progress (disconnecting is asynchronous, so the old link can linger)
		wifiState.isWaitingForOldLinkToDrop = WiFi.status() == WL_CONNECTED;
		WiFi.disconnect();
		WiFi.begin(credentials.ssid, credentials.password);
		wifiState.connectInProgress = true;
		wifiState.isAttemptWithNewCredentials = hasNewCredentials;
		wifiState.connectStartedAt = millis();
		printLogfln("Connecting to WiFi SSID: %s", credentials.ssid);
		return true;
	}

	// Mark the current Wi-Fi attempt as successful and schedule no further retries
	void completeWiFiConnectionSuccess() {
		const bool wasAttemptInProgress = wifiState.connectInProgress;
		resetConnectionAttempt();
		wifiState.reconnectDelay = 0;
		wiFiConnectionStatus = WIFI_OK;

		// If we weren't already connected, print the success message and save the new credentials if they were used
		if (wasAttemptInProgress) {
			printLogln(" connected!");
			PendingCredentials credentials;
			if (wifiState.isAttemptWithNewCredentials && readPendingCredentials(credentials)) {
				persistWorkingCredentials(credentials);
				clearPendingCredentials();
			}
			wifiState.isAttemptWithNewCredentials = false;
		}

		// Remember when the connection came back
		if (!wifiState.isConnected) {
			wifiState.isConnected = true;
			wifiState.connectedSince = millis();
		}

		// Turn the setup access point off once Wi-Fi has been back for a while
		if (accessPointEnabled && !disableAccessPoint && millis() - wifiState.connectedSince >= ACCESS_POINT_SHUTDOWN_DELAY) {
			printLogln("WiFi is back, turning the access point off");
			disableAccessPoint = true;
		}

		// Start mDNS service
		startMdns();
	}

	// Mark the current Wi-Fi attempt as failed and schedule a future retry
	void completeWiFiConnectionFailure() {
		// Drop new credentials that didn't work (the next attempt goes back to the saved network)
		if (wifiState.isAttemptWithNewCredentials) {
			printLogln("The new WiFi credentials didn't work, falling back to the saved ones");
			clearPendingCredentials();
			wifiState.isAttemptWithNewCredentials = false;
		}

		// Mark the attempt as failed
		WiFi.disconnect();
		resetConnectionAttempt();
		wiFiConnectionStatus = WIFI_KO;
		wifiState.reconnectDelayStartedAt = millis();
		wifiState.reconnectDelay = WIFI_RETRY_INTERVAL;
		printLogfln("Failed to connect to WiFi, retrying in %lu ms", WIFI_RETRY_INTERVAL);
	}

	// Check whether a fresh connection attempt should start on this loop iteration
	bool shouldStartWiFiConnectionAttempt(unsigned long currentMillis) {
		return wiFiConnectionStatus == WIFI_TRY || currentMillis - wifiState.reconnectDelayStartedAt >= wifiState.reconnectDelay; // Elapsed-time check stays correct across millis() rollover
	}

	// Choose the matrix message for the current Wi-Fi state
	const char* getWiFiStatusMessage() {
		const char connectingMessage[] = "Connecting to Wi-Fi...";
		const char disconnectedMessage[] = "Not connected to Wi-Fi. Use the 'Gizmo-Ticker' access point to enter the Wi-Fi credentials.";
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

// Queue a new Wi-Fi connection attempt for the next main-loop cycle (called from the web server task)
void queueWiFiConnectionAttempt(const char* ssid, const char* password) {
	// Hand the new credentials over to the main loop
	portENTER_CRITICAL(&pendingCredentialsMux);
	stringCopy(pendingCredentials.ssid, ssid, sizeof(pendingCredentials.ssid));
	stringCopy(pendingCredentials.password, password, sizeof(pendingCredentials.password));
	pendingCredentials.hasCredentials = true;
	pendingCredentials.isAttemptRequested = true;
	portEXIT_CRITICAL(&pendingCredentialsMux);

	// Report the attempt as in progress right away
	wiFiConnectionStatus = WIFI_TRY;
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

	// Start a connection attempt requested from the web server, even if already connected
	if (takeConnectionRequest()) {
		resetConnectionAttempt();
		wifiState.reconnectDelay = 0;
		wifiState.isConnected = false;
		stopMdns();
		beginWiFiConnectionAttempt();
		return false;
	}

	// While an attempt waits for the previous link to drop, don't take the old connection for the new one
	if (wifiState.connectInProgress && wifiState.isWaitingForOldLinkToDrop && WiFi.status() != WL_CONNECTED) {
		wifiState.isWaitingForOldLinkToDrop = false;
	}

	// Check if connected to WiFi
	if (WiFi.status() == WL_CONNECTED && !(wifiState.connectInProgress && wifiState.isWaitingForOldLinkToDrop)) {
		completeWiFiConnectionSuccess();
		return true; // Connection success
	}

	// Stop mDNS if not connected to Wi-Fi
	wifiState.isConnected = false;
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