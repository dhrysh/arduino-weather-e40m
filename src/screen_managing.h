#ifndef DISPLAY_MODE_H
#define DISPLAY_MODE_H
#include <Arduino.h>

class DisplayMode {
   public:
    virtual ~DisplayMode() {}
    virtual void onEnter() {}
    virtual void update() {}
    virtual void render(uint8_t pattern[8]) = 0;
    virtual void processData(char* textPtr) {}
    virtual void onButtonPress() {}
};
#endif