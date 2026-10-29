#include "serial.h"

namespace {
    // Serial logging switch
    const bool serialDebug = true;
}

// Init serial
void initSerial() {
    gizmo::initLog(serialDebug);
}