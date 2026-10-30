#ifndef SETTINGS_JSON_H
#define SETTINGS_JSON_H

#include <ArduinoJson.h>
#include "../config/device_config.h"

// Mapping between the display settings and the web API JSON, free of hardware headers so it can be tested on the host
void writeSettingsToJson(JsonObject json, const DeviceConfig& config);
void applySettingsFromJson(JsonObjectConst json, DeviceConfig& config);

#endif