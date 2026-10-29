#ifndef CONFIG_H
#define CONFIG_H

#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "device_config.h"

// Hardware configuration
constexpr MD_MAX72XX::moduleType_t HARDWARE_TYPE = MD_MAX72XX::FC16_HW;
constexpr uint16_t BUF_SIZE = 250; // Buffer length
constexpr uint8_t MAX_DEVICES = 16; // Number of modules
constexpr uint8_t CLK_PIN = 18; // SCK
constexpr uint8_t DATA_PIN = 23; // MOSI
constexpr uint8_t CS_PIN = 5; // SS

// Device config getter and setter
DeviceConfig getDeviceConfig();
void setDeviceConfig(const DeviceConfig& config);

// Global message buffers shared by serial and scrolling functions
extern char currentMessage[BUF_SIZE]; // Current message

// WiFi and client configuration
extern AsyncWebServer server;
extern WiFiClient client;
extern const char accessPointSSID[];
extern const char mdnsHostname[];
extern bool accessPointEnabled;
extern bool disableAccessPoint;
enum connectionStatus { WIFI_TRY = 2, WIFI_OK = 1, WIFI_KO = 0 };
extern connectionStatus wiFiConnectionStatus;
extern unsigned long timestampStockData;
extern unsigned long timestampWiFiConnection;

// Scrolling text configuration
enum printType { PRINT_PRICE = 0, PRINT_CHANGE = 1, PRINT_MARKET_CAP = 2, PRINT_DAILY_HIGH_LOW = 3, PRINT_YEAR_HIGH_LOW = 4, PRINT_OPEN = 5, PRINT_VOLUME = 6 };
extern printType switchText;
extern MD_Parola P;
extern uint8_t scrollDelay;
extern textEffect_t scrollEffect;
extern textPosition_t scrollAlign;
extern uint16_t scrollPause;

#endif