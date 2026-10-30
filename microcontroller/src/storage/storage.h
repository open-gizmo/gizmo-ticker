#ifndef STORAGE_H
#define STORAGE_H

#include "../config/device_config.h"

void setupStorage();
bool saveDeviceConfigToStorage(const DeviceConfig& config);
bool clearStoredDeviceConfig();

#endif