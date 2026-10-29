#include "eeprom.h"
#include <EEPROM.h>
#include <StreamUtils.h>
#include <cstring>
#include "../serial/serial.h"
#include "config_json.h"

namespace {
    constexpr size_t EEPROM_SIZE = 512;

    // Commit the changes to EEPROM and print a log message
    bool commitEEPROMChanges(const char* successMessage) {
        if (!EEPROM.commit()) return false;
        printLogfln(successMessage);
        return true;
    }
}

// Read a JSON document from EEPROM by extracting the stored JSON payload
bool readEEPROM(JsonDocument& doc) {
    char eepromData[EEPROM_SIZE + 1];
    size_t jsonStart = EEPROM_SIZE;
    size_t jsonEnd = 0;

    // Read the EEPROM byte by byte and look for the JSON start and end characters
    for (size_t i = 0; i < EEPROM_SIZE; i++) {
        const char currentByte = static_cast<char>(EEPROM.read(i));
        eepromData[i] = currentByte;
        if (jsonStart == EEPROM_SIZE && currentByte == '{') jsonStart = i;
        if (jsonStart != EEPROM_SIZE && currentByte == '}') jsonEnd = i;
        if (currentByte == '\0') break;
    }

    // If the JSON start or end characters are not found, return false
    if (jsonStart == EEPROM_SIZE || jsonEnd < jsonStart) {
        return false;
    }

    // Extract the JSON substring from the EEPROM data
    const size_t jsonLength = jsonEnd - jsonStart + 1;
    char jsonData[EEPROM_SIZE + 1];
    memcpy(jsonData, eepromData + jsonStart, jsonLength);
    jsonData[jsonLength] = '\0';

    // Parse the JSON data
	DeserializationError error = deserializeJson(doc, jsonData);
    if (error) {
        return false;
    }

    // Parsing successful
    return true;
}

// Write data to the EEPROM
bool writeEEPROM() {
    const DeviceConfig& config = getDeviceConfig();
    return saveDeviceConfigToEEPROM(config);
}

// Load the config snapshot from EEPROM without mutating runtime state
bool loadDeviceConfigFromEEPROM(DeviceConfig& config) {
    // Get current config
    config = getDeviceConfig();

    // Read the EEPROM
    JsonDocument doc;
    if (!readEEPROM(doc)) {
        return false;
    }

    // Apply the config values from EEPROM data
    applyConfigFromJson(doc, config);
    return true;
}

// Save the config snapshot to EEPROM
bool saveDeviceConfigToEEPROM(const DeviceConfig& config) {
    // Build a JSON document from the config values
    JsonDocument doc;
    applyConfigToJson(doc, config);

    // Serialize the JSON document directly to EEPROM
    EepromStream eepromStream(0, EEPROM_SIZE);
    const size_t jsonSize = serializeJson(doc, eepromStream);
    if (jsonSize == 0) {
        return false;
    }

    // Add a null terminator
    if (jsonSize < EEPROM_SIZE) {
        EEPROM.write(jsonSize, '\0');
    }

    // Commit the changes to EEPROM
    return commitEEPROMChanges("Data saved on EEPROM");
}

// Clear all saved data from the EEPROM
bool clearEEPROM() {
    // Write 0xFF to all EEPROM bytes
    for (size_t i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0xFF);
    }

    // Commit the changes to EEPROM
    return commitEEPROMChanges("EEPROM cleared");
}

// Setup EEPROM
void setupEEPROM() {
    // Initialize EEPROM
	EEPROM.begin(EEPROM_SIZE);

    // Load the config from EEPROM
    DeviceConfig config;
    if (!loadDeviceConfigFromEEPROM(config)) {
        printLogfln("EEPROM is empty or invalid, writing default settings...");
        writeEEPROM();
        return;
    }

    // Apply the loaded config to runtime state
    setDeviceConfig(config);
    printLogfln("EEPROM data loaded successfully");
}