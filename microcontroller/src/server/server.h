#ifndef SERVER_H
#define SERVER_H

#include <ArduinoJson.h>
#include "../config/config.h"

// Standard API responses
void sendSuccessResponse(AsyncWebServerRequest* request, uint16_t statusCode = 200, const JsonDocument* dataDoc = nullptr);
void sendErrorResponse(AsyncWebServerRequest* request, uint16_t statusCode, const char* errorId, const char* errorMessage);
bool ensureCorsAllowed(AsyncWebServerRequest* request);
void sendCorsPreflightResponse(AsyncWebServerRequest* request, const char* allowedMethods);

// Web server routes
void setupConnectPostRoute();
void setupCheckConnectionGetRoute();
void setupNetworksGetRoute();
void setupApiKeyPostRoute();
void setupConfigGetRoute();
void setupSettingsPostRoute();
void setupResetSettingsGetRoute();
void setupCaptivePortalRoute();

// Server setup
void setupRoutes();
void setupServer();

#endif