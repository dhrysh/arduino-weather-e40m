#ifndef WEATHER_MODE_H
#define WEATHER_MODE_H
#include "../screen_managing.h"

class WeatherMode : public DisplayMode {
   private:
    enum WeatherState { TEMP,
                        HUM };
    WeatherState currentState;

    int currentTemp;
    int currentHumidity;
    int screenValue;

   public:
    WeatherMode();
    void onEnter() override;
    void setValue(int temp, int hum);
    void render(uint8_t pattern[8]) override;
    void processData(char* textPtr) override;
    void onButtonPress() override;
};
#endif