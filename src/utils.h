#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H
#include <Arduino.h>

void createNumberPattern(int num, uint8_t pattern[8]);
void checkMic();

extern const uint8_t numbers[2][10][8];
extern const uint8_t degree_symbol[8];

#endif