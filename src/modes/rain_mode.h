#ifndef RAIN_MODE_H
#define RAIN_MODE_H
#include <Arduino.h>
#include "../screen_managing.h"

class RainMode : public DisplayMode {
  private:
    enum RainState { ANIMATING, HOLD_ICON, SCROLLING, HOLD_BLANK };
    RainState currentState;

    unsigned long lastFrameTime;
    int currentAnimationFrame;
    int pop_chance; // Probability of precipitation

    uint32_t scrollBuffer[8];
    int scrollOffset;

    void buildScrollBuffer();

  public:
    RainMode();
    void onEnter() override;
    void update() override;
    void render(uint8_t pattern[8]) override;
    void processData(char* textPtr) override;
    
    void setChance(int percent);
};
#endif