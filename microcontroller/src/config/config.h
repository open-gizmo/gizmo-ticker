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

// Matrix pins (hardware SPI; the display never reads, so it needs no MISO)
#if defined(ARDUINO_XIAO_ESP32S3)
// XIAO ESP32-S3: the pads next to 5V and GND, in the order of the module input (VCC, GND, DIN, CS, CLK), so a flat cable runs straight across (skip 3V3)
constexpr uint8_t DATA_PIN = 9; // DIN, pad D10 (MOSI)
constexpr uint8_t CS_PIN = 8; // CS, pad D9
constexpr uint8_t CLK_PIN = 7; // CLK, pad D8 (SCK)
#else
#error "The ticker runs on the Seeed Studio XIAO ESP32-S3 (pio run -e release)"
#endif

// Device config getter and setter
DeviceConfig getDeviceConfig();
void setDeviceConfig(const DeviceConfig &config);

// Global message buffers shared by serial and scrolling functions
extern char currentMessage[BUF_SIZE]; // Current message

// Web server and market data configuration
extern AsyncWebServer server;

// Scrolling text configuration
enum printType { PRINT_PRICE = 0,
	PRINT_CHANGE = 1,
	PRINT_MARKET_CAP = 2,
	PRINT_DAILY_HIGH_LOW = 3,
	PRINT_YEAR_HIGH_LOW = 4,
	PRINT_OPEN = 5,
	PRINT_VOLUME = 6 };
extern printType switchText;
extern uint8_t scrollDelay;
extern textEffect_t scrollEffect;
extern textPosition_t scrollAlign;
extern uint16_t scrollPause;

#endif