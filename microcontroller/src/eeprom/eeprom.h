#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <ArduinoJson.h>
#include "../config/config.h"

void setupEEPROM();
bool readEEPROM(JsonDocument& doc);
bool clearEEPROM();
bool loadDeviceConfigFromEEPROM(DeviceConfig& config);
bool saveDeviceConfigToEEPROM(const DeviceConfig& config);
bool writeEEPROM();

#endif