#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <Arduino.h>
#include <avr/pgmspace.h>

void createNumberPattern(int num, uint8_t pattern[8]);

extern const uint8_t numbers[2][10][8] PROGMEM;
extern const uint8_t degree_symbol[8] PROGMEM;
extern const uint8_t sunrise_animation[6][8] PROGMEM;
extern const uint8_t sunset_animation[6][8] PROGMEM;
extern const uint8_t rain_animation[4][8] PROGMEM;

#endif