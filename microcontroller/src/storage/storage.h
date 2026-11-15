#ifndef STORAGE_H
#define STORAGE_H

#include "../config/device_config.h"

// Load the saved settings into the current ones (the defaults on a new device)
void setupStorage();
// Save the settings (false when the flash couldn't be written)
bool saveSettings(const DeviceConfig &config);
// Forget the saved settings (the defaults apply after the next boot)
bool clearSettings();

#endif