#include "serial.h"

#ifndef PIO_DEV_MODE
#define PIO_DEV_MODE 0
#endif

namespace {
	// Serial logging switch: off in the released firmware, on in the debug build (pio run -e xiao_esp32s3_debug)
	const bool serialDebug = PIO_DEV_MODE != 0;
}

// Init serial
void initSerial() {
	gizmo::initLog(serialDebug);
}