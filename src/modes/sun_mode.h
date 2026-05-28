#ifndef SUN_MODE_H
#define SUN_MODE_H
#include "../screen_managing.h"

class SunMode : public DisplayMode {
   private:
    enum SunState { WAITING,
                    ANIMATING,
                    HOLD_ICON,
                    SCROLLING,
                    HOLD_BLANK };
    SunState currentState;

    unsigned long lastFrameTime;
    int currentAnimationFrame;
    bool playingSunrise;

    int sr_hour, sr_min;
    int ss_hour, ss_min;

    uint32_t scrollBuffer[8];
    int scrollOffset;

    void buildScrollBuffer();

   public:
    SunMode();
    void onEnter() override;
    void update() override;
    void render(uint8_t pattern[8]) override;
    void onButtonPress() override;
    void processData(char* textPtr) override;

    void setTimes(int sunrise_h, int sunrise_m, int sunset_h, int sunset_m);
};
#endif