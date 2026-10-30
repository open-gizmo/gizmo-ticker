#ifndef SERVER_H
#define SERVER_H

#include <ArduinoJson.h>
#include <gizmo/http.h>
#include "../config/config.h"

// Standard API responses and CORS checks come from gizmo-common
using gizmo::ensureCorsAllowed;
using gizmo::sendCorsPreflightResponse;
using gizmo::sendErrorResponse;
using gizmo::sendSuccessResponse;

// Web server routes (the Wi-Fi setup routes are registered by gizmo-common)
void setupApiKeyPostRoute();
void setupConfigGetRoute();
void setupSettingsPostRoute();
void setupResetSettingsPostRoute();
void setupCaptivePortalRoute();

// Server setup
void setupRoutes();
void setupServer();

// Deferred restart (requested by the web server task, performed by the main loop)
void scheduleRestart();
void manageScheduledRestart();

#endif