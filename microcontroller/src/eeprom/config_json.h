#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H

#include <ArduinoJson.h>
#include "../config/device_config.h"

// Mapping between the device config and its stored JSON, free of hardware headers so it can be tested on the host
void applyConfigToJson(JsonDocument& doc, const DeviceConfig& config);
void applyConfigFromJson(JsonDocument& doc, DeviceConfig& config);

#endif