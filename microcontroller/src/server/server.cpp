#include "server.h"
#include "../config/config.h"

namespace {
	constexpr unsigned long RESTART_DELAY = 1000; // Give the web server time to send the last response

	// Deferred restart state (written by the web server task, read by the main loop)
	bool restartScheduled = false;
	unsigned long restartScheduledAt = 0;
	portMUX_TYPE restartMux = portMUX_INITIALIZER_UNLOCKED;
}

// Setup routes
void setupRoutes() {
	// Setup the routes
	setupApiKeyPostRoute();
	setupConfigGetRoute();
	setupSettingsPostRoute();
	setupResetSettingsPostRoute();
	setupCaptivePortalRoute();
}

// Setup server
void setupServer() {
	setupRoutes(); // Setup routes
	server.begin(); // Start the server
}

// Schedule a restart once the current response has been sent
void scheduleRestart() {
	portENTER_CRITICAL(&restartMux);
	restartScheduled = true;
	restartScheduledAt = millis();
	portEXIT_CRITICAL(&restartMux);
}

// Restart the device from the main loop once the scheduled delay has passed
void manageScheduledRestart() {
	// Check whether a restart is due
	portENTER_CRITICAL(&restartMux);
	const bool isRestartDue = restartScheduled && millis() - restartScheduledAt >= RESTART_DELAY;
	portEXIT_CRITICAL(&restartMux);

	// Restart the device
	if (isRestartDue) {
		ESP.restart();
	}
}