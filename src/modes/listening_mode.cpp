#include "listening_mode.h"
#include "../shared_data.h"
#include "../utils.h"

void ListeningMode::render(uint8_t pattern[8]) {
    for(int i=0; i<8; i++) pattern[i] = 0; // Clear
    pattern[0] = 0b10000000;
}

void ListeningMode::update() {
    checkMic(); 
}