#include "serial.h"

const bool serialDebug = true;

// Init serial
void initSerial() {
    if (serialDebug) {
        Serial.begin(115200);
        while (!Serial) {
            delay(10);
        }
    }
}