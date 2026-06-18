#include "eeprom.h"
#include <EEPROM.h>
#include <StreamUtils.h>
#include <cstring>
#include "../utils/utils.h"
#include "../serial/serial.h"

namespace {
    constexpr size_t EEPROM_SIZE = 512;

    // Apply config values to a JSON document
    void applyConfigToJson(JsonDocument& doc, const DeviceConfig& config) {
        // Add the config fields to the JSON object
        doc["currentPrice"] = config.currentPriceVisible;
        doc["priceChange"] = config.priceChangeVisible;
        doc["marketCap"] = config.marketCapVisible;
        doc["dailyHighLow"] = config.dailyHighLowVisible;
        doc["yearHighLow"] = config.yearHighLowVisible;
        doc["openPrice"] = config.openPriceVisible;
        doc["volume"] = config.volumeVisible;
        doc["cryptoCoin"] = config.cryptoCoin;
        doc["formatType"] = config.formatType == FORMAT_US ? "US" : "EU";
        doc["matrixIntensity"] = config.matrixIntensity;
        doc["scrollSpeed"] = config.scrollSpeed;

        // Add WiFi credentials if present
        if (strlen(config.ssid) != 0 && strlen(config.password) != 0) {
            doc["ssid"] = config.ssid;
            doc["password"] = config.password;
        }

        // Add API key if present
        if (strlen(config.apiKey) != 0) {
            doc["apiKey"] = config.apiKey;
        }
    }

    // Apply config values from a JSON document to a config snapshot
    void applyConfigFromJson(JsonDocument& doc, DeviceConfig& config) {
        if (!doc["currentPrice"].isNull())
            config.currentPriceVisible = doc["currentPrice"].as<bool>();
        if (!doc["priceChange"].isNull())
            config.priceChangeVisible = doc["priceChange"].as<bool>();
        if (!doc["marketCap"].isNull())
            config.marketCapVisible = doc["marketCap"].as<bool>();
        if (!doc["dailyHighLow"].isNull())
            config.dailyHighLowVisible = doc["dailyHighLow"].as<bool>();
        if (!doc["yearHighLow"].isNull())
            config.yearHighLowVisible = doc["yearHighLow"].as<bool>();
        if (!doc["openPrice"].isNull())
            config.openPriceVisible = doc["openPrice"].as<bool>();
        if (!doc["volume"].isNull())
            config.volumeVisible = doc["volume"].as<bool>();
        if (!doc["cryptoCoin"].isNull())
            stringCopy(config.cryptoCoin, doc["cryptoCoin"].as<const char*>(), sizeof(config.cryptoCoin));
        if (!doc["formatType"].isNull())
            config.formatType = strcmp(doc["formatType"].as<const char*>(), "US") == 0 ? FORMAT_US : FORMAT_EU;
        if (!doc["matrixIntensity"].isNull())
            config.matrixIntensity = doc["matrixIntensity"].as<uint8_t>();
        if (!doc["scrollSpeed"].isNull())
            config.scrollSpeed = doc["scrollSpeed"].as<uint8_t>();
        if (!doc["apiKey"].isNull())
            stringCopy(config.apiKey, doc["apiKey"], sizeof(config.apiKey));
        if (!doc["ssid"].isNull())
            stringCopy(config.ssid, doc["ssid"], sizeof(config.ssid));
        if (!doc["password"].isNull())
            stringCopy(config.password, doc["password"], sizeof(config.password));
    }

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