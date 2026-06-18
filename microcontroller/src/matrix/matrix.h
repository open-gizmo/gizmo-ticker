#ifndef MATRIX_H
#define MATRIX_H

#include "../config/config.h"

struct MarketTickerData;

void printOnLedMatrix(const char* message, const byte stringLength, uint16_t messageStill = scrollPause);
void setMatrixIntensity(uint8_t intensity);
void setMatrixSpeed(uint8_t speed);
void setupLedMatrix();
bool isLedMatrixReadyForNextMessage();
void renderNextLedMatrixMessage(const MarketTickerData& marketData);

#endif