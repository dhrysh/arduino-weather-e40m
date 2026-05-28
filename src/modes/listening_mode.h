#ifndef LISTENING_MODE_H
#define LISTENING_MODE_H
#include "../screen_managing.h"

class ListeningMode : public DisplayMode {
   private:

   public:
       void render(uint8_t pattern[8]) override;
       void update() override;
};
#endif