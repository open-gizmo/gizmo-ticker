#pragma once

#include <Arduino.h>
#include <pgmspace.h>

struct EmbeddedAsset {
    const char* routePath;
    const char* sourcePath;
    const char* contentType;
    const uint8_t* data;
    size_t size;
    bool gzip;
};

extern const EmbeddedAsset EMBEDDED_ASSETS[];
extern const size_t EMBEDDED_ASSET_COUNT;