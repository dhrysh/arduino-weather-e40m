#include "rain_mode.h"

#include "shared_data.h"

RainMode::RainMode() {
    pop_chance = 0;
}

void RainMode::setChance(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 99) percent = 99;
    pop_chance = percent;
}

void RainMode::buildScrollBuffer() {
    const uint8_t percent_icon[8] = {
        0b101,
        0b001,
        0b010,
        0b100,
        0b101,
        0b000,
        0b000,
        0b000};

    for (int r = 0; r < 8; r++) {
        uint32_t rowData = 0;

        uint8_t d1 = (pgm_read_byte(&(numbers[0][pop_chance / 10][r])) >> 5) & 0x07;
        uint8_t d2 = (pgm_read_byte(&(numbers[0][pop_chance % 10][r])) >> 5) & 0x07;
        uint8_t pct = percent_icon[r];

        // Digit 1 -> Space -> Digit 2 -> Space -> %
        rowData |= ((uint32_t)d1 << 17);
        rowData |= ((uint32_t)d2 << 13);
        rowData |= ((uint32_t)pct << 9);

        scrollBuffer[r] = rowData;
    }
}

void RainMode::onEnter() {
    Serial.println("REQ|RAIN_DATA");
    currentState = ANIMATING;
    currentAnimationFrame = 0;
    lastFrameTime = millis();
}

void RainMode::update() {
    unsigned long now = millis();

    switch (currentState) {
        case ANIMATING: {
            int speed = (pop_chance > 50) ? 150 : 300;

            if (now - lastFrameTime >= speed) {
                lastFrameTime = now;
                currentAnimationFrame++;

                if (currentAnimationFrame >= 12) {
                    currentAnimationFrame = 0;
                    currentState = HOLD_ICON;
                }
            }
            break;
        }

        case HOLD_ICON: {
            if (now - lastFrameTime >= 1000) {
                lastFrameTime = now;
                buildScrollBuffer();
                currentState = SCROLLING;
                scrollOffset = 0;
            }
            break;
        }
        case SCROLLING:
            if (now - lastFrameTime >= 250) {
                lastFrameTime = now;
                scrollOffset++;
                if (scrollOffset > 24) {
                    currentState = HOLD_BLANK;
                }
            }
            break;

        case HOLD_BLANK:
            if (now - lastFrameTime >= 1000) {
                currentState = ANIMATING;
                currentAnimationFrame = 0;
            }
            break;
        default:
            break;
    }
}

void RainMode::render(uint8_t pattern[8]) {
    if (currentState == ANIMATING || currentState == HOLD_ICON) {
        for (int row = 0; row < 8; row++) {
            uint8_t frameData = pgm_read_byte(&(rain_animation[currentAnimationFrame % 4][row]));

            if (pop_chance == 0) {
                // If 0%, only show top 2 rows
                if (row >= 2) {
                    pattern[row] = 0;
                } else {
                    pattern[row] = frameData;
                }
            } else {
                pattern[row] = frameData;
            }
        }
    } else if (currentState == SCROLLING) {
        for (int row = 0; row < 8; row++) {
            uint8_t scrollData = (scrollBuffer[row] << scrollOffset) >> 24;

            pattern[row] = scrollData;
        }
    } else {
        // HOLD_BLANK state
        for (int row = 0; row < 8; row++) pattern[row] = 0;
    }
}

void RainMode::processData(char* textPtr) {
    if (strcmp(textPtr, "RAIN_DATA") == 0) {
        float rainVal = atof(strtok(NULL, "|"));
        int percent = (int)(rainVal * 20.0);
        setChance(percent);
    }
}