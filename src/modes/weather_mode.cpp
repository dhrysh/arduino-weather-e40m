#include "weather_mode.h"

#include "../shared_data.h"

WeatherMode::WeatherMode() {
    currentState = WeatherState::TEMP;
}

void WeatherMode::onEnter() {
    Serial.println("REQ|CURRENT_TEMP");
}

void WeatherMode::setValue(int temp, int hum) {
    currentTemp = temp;
    currentHumidity = hum;

    if (currentState == WeatherState::TEMP) {
        screenValue = currentTemp;
    } else if (currentState == WeatherState::HUM) {
        screenValue = currentHumidity;
    }
}

void WeatherMode::render(uint8_t pattern[8]) {
    createNumberPattern(screenValue, pattern);

    if (currentState == WeatherState::TEMP) {
    for (int row = 0; row < 8; row++) {
        uint8_t ledRow = pgm_read_byte(&(degree_symbol[row]));
        pattern[row] = pattern[row] | ledRow;
    }
    }
}

void WeatherMode::processData(char* textPtr) {
    if (strcmp(textPtr, "CURRENT_TEMP") == 0) {
        int temp = atoi(strtok(NULL, "|"));
        int hum = atoi(strtok(NULL, "|"));

        Serial.println("ACK_CT");
        this->setValue(temp, hum);
    }
}

void WeatherMode::onButtonPress() {
    if (currentState == WeatherState::TEMP) {
        currentState = WeatherState::HUM;
        screenValue = this->currentHumidity;
    } else if (currentState == WeatherState::HUM) {
        currentState = WeatherState::TEMP;
        screenValue = this->currentTemp;
    }
}