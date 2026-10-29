#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

extern const bool serialDebug;

// Init serial
void initSerial();

// Print log
template<typename T>
inline void printLog(T message) {
    if (serialDebug) Serial.print(message);
}

// Print log with newline
template<typename T>
inline void printLogln(T message) {
    if (serialDebug) Serial.println(message);
}

// Printf-style debug print with format string (the compiler checks the arguments against the format)
void printLogf(const char* format, ...) __attribute__((format(printf, 1, 2)));

// Printf-style debug print with format string and newline (the compiler checks the arguments against the format)
void printLogfln(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif