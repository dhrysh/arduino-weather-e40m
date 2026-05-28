#include "sun_mode.h"

#include "../shared_data.h"
#include "shared_data.h"
#include "sun_mode.h"

SunMode::SunMode() {
    sr_hour = 6;
    sr_min = 30;
    ss_hour = 19;
    ss_min = 45;
}

void SunMode::setTimes(int sunrise_h, int sunrise_m, int sunset_h,
                       int sunset_m) {
    sr_hour = sunrise_h;
    sr_min = sunrise_m;
    ss_hour = sunset_h;
    ss_min = sunset_m;
}

void SunMode::buildScrollBuffer() {
    int h = playingSunrise ? sr_hour : ss_hour;
    int m = playingSunrise ? sr_min : ss_min;

    for (int r = 0; r < 8; r++) {
        uint32_t rowData = 0;

        // 0x07 is a mask for last 3 vbits
        uint8_t h1 = (pgm_read_byte(&(numbers[0][h / 10][r])) >> 5) & 0x07;
        uint8_t h2 = (pgm_read_byte(&(numbers[0][h % 10][r])) >> 5) & 0x07;
        uint8_t m1 = (pgm_read_byte(&(numbers[0][m / 10][r])) >> 5) & 0x07;
        uint8_t m2 = (pgm_read_byte(&(numbers[0][m % 10][r])) >> 5) & 0x07;

        uint8_t colon = (r == 2 || r == 6) ? 0x01 : 0x00;

        rowData |= ((uint32_t)h1 << 21);     // Tens Hour
        rowData |= ((uint32_t)h2 << 17);     // Ones Hour
        rowData |= ((uint32_t)colon << 15);  // Colon
        rowData |= ((uint32_t)m1 << 11);     // Tens Min
        rowData |= ((uint32_t)m2 << 7);      // Ones Min

        scrollBuffer[r] = rowData;
    }
}

void SunMode::onEnter() {
    Serial.println("REQ|SUN_DATA");
    currentState = ANIMATING;
    currentAnimationFrame = 0;
    playingSunrise = true;
    lastFrameTime = millis();
}

void SunMode::onButtonPress() {
    // if (currentState == WAITING) {
        playingSunrise = !playingSunrise;
        currentState = ANIMATING;
        currentAnimationFrame = 0;
        lastFrameTime = millis();
    // }
}

void SunMode::update() {
    unsigned long now = millis();

    switch (currentState) {
        case ANIMATING:
            if (now - lastFrameTime >= 400) {
                lastFrameTime = now;
                currentAnimationFrame++;
                if (currentAnimationFrame >= 6) {
                    currentAnimationFrame = 5;
                    currentState = HOLD_ICON;
                }
            }
            break;

        case HOLD_ICON:
            if (now - lastFrameTime >= 1500) {
                lastFrameTime = now;
                buildScrollBuffer();
                currentState = SCROLLING;
                scrollOffset = 0;
            }
            break;

        case SCROLLING:
            if (now - lastFrameTime >= 250) {
                lastFrameTime = now;
                scrollOffset++;
                if (scrollOffset > 28) {
                    currentState = HOLD_BLANK;
                }
            }
            break;

        case HOLD_BLANK:
            if (now - lastFrameTime >= 500) {
                currentState = ANIMATING;
            }
            break;

        case WAITING:
            break;
    }
}

void SunMode::render(uint8_t pattern[8]) {
    if (currentState == ANIMATING || currentState == HOLD_ICON) {
        for (int row = 0; row < 8; row++) {
            if (playingSunrise) {
                pattern[row] = pgm_read_byte(
                    &(sunrise_animation[currentAnimationFrame][row]));
            } else {
                pattern[row] = pgm_read_byte(
                    &(sunset_animation[currentAnimationFrame][row]));
            }
        }
    } else if (currentState == SCROLLING) {
        for (int row = 0; row < 8; row++) {
            pattern[row] = (scrollBuffer[row] << scrollOffset) >> 24;
        }
    } else {
        for (int row = 0; row < 8; row++) {
            pattern[row] = 0;
        }
    }
}

void SunMode::processData(char* textPtr) {
    if (strcmp(textPtr, "SUN_DATA") == 0) {
        int sr_h = atoi(strtok(NULL, "|"));
        int sr_m = atoi(strtok(NULL, "|"));
        int ss_h = atoi(strtok(NULL, "|"));
        int ss_m = atoi(strtok(NULL, "|"));

        this->setTimes(sr_h, sr_m, ss_h, ss_m);
        Serial.println("ACK_SD");
    }
}