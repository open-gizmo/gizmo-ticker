#include "../server.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "../../config/config.h"

namespace {
	constexpr int WIFI_SCAN_FAILED_STATUS = -2;

	// Struct to hold scanned network information
	struct ScannedNetwork {
		String ssid;
		int32_t rssi;
		uint8_t channel;
		bool secured;
		uint8_t quality;
	};

	// Cache for scanned networks and state tracking (cache and scan flag are guarded by networksMutex)
	std::vector<ScannedNetwork> cachedNetworks;
	SemaphoreHandle_t networksMutex = nullptr;
	bool scanCallbackRegistered = false;
	bool wifiScanInProgress = false;

	// Calculate signal quality percentage from RSSI
	uint8_t calculateSignalQuality(int32_t rssi) {
		if (rssi <= -100) return 0;
		if (rssi >= -50) return 100;
		return static_cast<uint8_t>(2 * (rssi + 100));
	}

	// Read the current scan results from the Wi-Fi stack
	std::vector<ScannedNetwork> collectScannedNetworks(int numNetworks) {
		std::vector<ScannedNetwork> scannedNetworks;
		scannedNetworks.reserve(numNetworks);

		// Collect information for each scanned network
		for (int i = 0; i < numNetworks; i++) {
			const String ssid = WiFi.SSID(i);
			if (ssid.length() == 0) {
				continue;
			}

			ScannedNetwork network;
			network.ssid = ssid;
			network.rssi = WiFi.RSSI(i);
			network.channel = WiFi.channel(i);
			network.secured = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
			network.quality = calculateSignalQuality(network.rssi);
			scannedNetworks.push_back(network);
		}

		// Clear the scan results from the Wi-Fi stack to free memory
		WiFi.scanDelete();

		// Return the results
		return scannedNetworks;
	}

	// Register the ESP32 Wi-Fi scan done callback and create the cache mutex once
	bool ensureScanCallbackRegistered() {
		// If the callback is already registered, do nothing
		if (scanCallbackRegistered) {
			return true;
		}

		// Create the mutex that guards the cache (written by the Wi-Fi event task, read by the web server task)
		networksMutex = xSemaphoreCreateMutex();
		if (networksMutex == nullptr) {
			return false;
		}

		// Register a callback for when the Wi-Fi scan is complete
		WiFi.onEvent([](arduino_event_id_t event, arduino_event_info_t info) {
			// We only care about the scan done event, ignore others
			if (event != ARDUINO_EVENT_WIFI_SCAN_DONE) {
				return;
			}

			// Read the scan results outside the lock (an empty list if the scan failed)
			const int numNetworks = WiFi.scanComplete();
			std::vector<ScannedNetwork> scannedNetworks;
			if (numNetworks < 0) {
				WiFi.scanDelete();
			} else {
				scannedNetworks = collectScannedNetworks(numNetworks);
			}

			// Swap the results into the cache and mark the scan as complete
			xSemaphoreTake(networksMutex, portMAX_DELAY);
			cachedNetworks.swap(scannedNetworks);
			wifiScanInProgress = false;
			xSemaphoreGive(networksMutex);
		}, ARDUINO_EVENT_WIFI_SCAN_DONE);

		// Mark the callback as registered to avoid registering it multiple times
		scanCallbackRegistered = true;
		return true;
	}

	// Start an asynchronous Wi-Fi scan to avoid blocking the async_tcp task
	bool startAsyncScan() {
		// Ensure the scan done callback is registered before starting a new scan
		if (!ensureScanCallbackRegistered()) {
			return false;
		}

		// Mark the scan as in progress before starting it (so a fast scan done event can't be overwritten)
		xSemaphoreTake(networksMutex, portMAX_DELAY);
		const bool isScanAlreadyRunning = wifiScanInProgress;
		wifiScanInProgress = true;
		xSemaphoreGive(networksMutex);

		// If a scan is already running, keep serving the cached results
		if (isScanAlreadyRunning) {
			return true;
		}

		// Start a new asynchronous Wi-Fi scan (non-blocking)
		WiFi.scanDelete();
		const int scanStatus = WiFi.scanNetworks(true, true);

		// If the scan failed to start, clear the in-progress flag
		if (scanStatus == WIFI_SCAN_FAILED_STATUS) {
			xSemaphoreTake(networksMutex, portMAX_DELAY);
			wifiScanInProgress = false;
			xSemaphoreGive(networksMutex);
			return false;
		}

		// Scan started
		return true;
	}

	// Build the JSON payload from the list of scanned networks
	JsonDocument buildNetworksData(const std::vector<ScannedNetwork>& networks) {
		JsonDocument doc;
		JsonObject data = doc.to<JsonObject>();
		JsonArray array = data["networks"].to<JsonArray>();

		// Add each network to the JSON array
		for (const ScannedNetwork& network : networks) {
			JsonObject item = array.add<JsonObject>();
			item["ssid"] = network.ssid;
			item["signal"] = network.rssi;
			item["quality"] = network.quality;
			item["channel"] = network.channel;
			item["secured"] = network.secured;
		}

		// Return the JSON document
		return doc;
	}

	// Build the JSON payload from the cached networks while holding the cache lock
	JsonDocument buildCachedNetworksData() {
		xSemaphoreTake(networksMutex, portMAX_DELAY);
		JsonDocument doc = buildNetworksData(cachedNetworks);
		xSemaphoreGive(networksMutex);
		return doc;
	}
}

// Get the list of available Wi-Fi networks
void setupNetworksGetRoute() {
	server.on("/api/networks", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Start a new scan if not already in progress, otherwise return the cached results
		if (!startAsyncScan()) {
			sendErrorResponse(request, 500, "wifi_scan_failed", "Unable to scan Wi-Fi networks.");
			return;
		}

		// Build the JSON response with the current scan results
		JsonDocument doc = buildCachedNetworksData();
		sendSuccessResponse(request, 200, &doc);
	});
}