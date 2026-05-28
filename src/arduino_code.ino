#include <TimerOne.h>
#include "modes/sun_mode.h"
#include "modes/weather_mode.h"
#include "modes/rain_mode.h"
#include "modes/listening_mode.h"
#include "screen_managing.h"
#include "shared_data.h"

const byte ANODE_PINS[8] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte CATHODE_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};

enum MainState {
    WEATHER,
    SUN,
    POP,
    LISTENING,
    numValues
};

WeatherMode weatherScreen;
SunMode sunScreen;
RainMode rainScreen;
ListeningMode listeningScreen;

DisplayMode* modes[] = {&weatherScreen, &sunScreen, &rainScreen, &listeningScreen};
MainState currentMode = MainState::WEATHER;

uint8_t currentDisplayPattern[8];


void createNumberPattern(int num, uint8_t pattern[8]) {
    if (num < 0) {
        num = 0;
    } else if (num > 99) {
        num = 99;
    }

    int firstDigit = num / 10;
    int secondDigit = num % 10;

    for (int row = 0; row < 8; row++) {
        uint8_t firstDigitRow = pgm_read_byte(&(numbers[0][firstDigit][row]));
        uint8_t secondDigitRow = pgm_read_byte(&(numbers[1][secondDigit][row]));

        pattern[row] = firstDigitRow | secondDigitRow;
    }
}

unsigned long lastMovement;

void moveScreen() {
    static unsigned long lastMovement = 0;
    if ((millis() - lastMovement) < 1000) {
        return;
    }

    int val = analogRead(A5);
    int newIndex = currentMode;

    if (val > 530) {
        newIndex = (currentMode + 1) % MainState::numValues;
    } else if (val < 490) {
        newIndex = (currentMode - 1 + MainState::numValues) % MainState::numValues;
    } else {
        return;
    }

    if (newIndex != currentMode) {
        currentMode = static_cast<MainState>(newIndex);
        modes[currentMode]->onEnter();
        Serial.print("Switched to screen index: ");
        Serial.println(currentMode);
    }

    lastMovement = millis();
}

void displayInterrupt() {
    static byte anode_n = 0;

    for (byte i = 0; i < 8; i++) {
        digitalWrite(ANODE_PINS[i], HIGH);
    }

    if (currentDisplayPattern[anode_n] == 0) {
        for (byte cathode_n = 0; cathode_n < 8; cathode_n++) {
            digitalWrite(CATHODE_PINS[cathode_n], HIGH); 
        }
    } else {
        for (byte cathode_n = 0; cathode_n < 8; cathode_n++) {
            if (bitRead(currentDisplayPattern[anode_n], 7 - cathode_n)) {
                digitalWrite(CATHODE_PINS[cathode_n], LOW);
            } else {
                digitalWrite(CATHODE_PINS[cathode_n], HIGH);
            }
        }
    }

    if (currentDisplayPattern[anode_n] != 0) {
        digitalWrite(ANODE_PINS[anode_n], LOW);
    }

    anode_n++;
    if (anode_n >= 8) anode_n = 0;
}

unsigned long lastButtonCheck = 0;
bool lastButtonState = false;

void checkA3Button() {
    if (millis() - lastButtonCheck < 50) return;
    lastButtonCheck = millis();

    noInterrupts(); 

    int activeAnode = -1;
    for (byte i = 0; i < 8; i++) {
        if (digitalRead(ANODE_PINS[i]) == LOW) {
            activeAnode = i; 
            digitalWrite(ANODE_PINS[i], HIGH);
        }
    }

    bool wasA3Low = (digitalRead(A3) == LOW); 

    pinMode(A3, INPUT_PULLUP);
    delayMicroseconds(10);
    bool isPressed = (digitalRead(A3) == LOW);

    pinMode(A3, OUTPUT);
    digitalWrite(A3, wasA3Low ? LOW : HIGH);

    if (activeAnode != -1) {
        digitalWrite(ANODE_PINS[activeAnode], LOW);
    }

    interrupts(); 

    if (isPressed == true && lastButtonState == false) {
        Serial.println("A3 Button Pressed!");
        modes[currentMode]->onButtonPress();
    }
    
    lastButtonState = isPressed;
}

unsigned long lastMicActivation = 0;

void checkMic() {
    if (currentMode != LISTENING) return;

    int val = analogRead(A4); 
    
    if (val > 700) { 
        if (millis() - lastMicActivation > 200) {
            Serial.print(val);
            Serial.println("Clap detected in LISTENING mode!");
            Serial.println("REQ|MIC_INPUT");
            lastMicActivation = millis();
        }
    }
}

const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];  // Temporary array for parsing

boolean newData = false;

void setup() {
    for (byte i = 0; i < 8; i++) {
        pinMode(ANODE_PINS[i], OUTPUT);
        pinMode(CATHODE_PINS[i], OUTPUT);
    }

    pinMode(A5, INPUT);

    for (byte i = 0; i < 8; i++) {
        digitalWrite(ANODE_PINS[i], HIGH);
        digitalWrite(CATHODE_PINS[i], HIGH);
    }

    Serial.begin(115200);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(displayInterrupt);

    modes[currentMode]->onEnter();
}

void loop() {
    checkMic();
    recvWithEndMarker();

    if (newData == true) {
        strcpy(tempChars, receivedChars);
        parseData();
        newData = false;
    }

    modes[currentMode]->update();
    uint8_t scratchBuffer[8];
    modes[currentMode]->render(scratchBuffer);

    noInterrupts();
    for (int i = 0; i < 8; i++) {
        currentDisplayPattern[i] = scratchBuffer[i];
    }
    interrupts();
    
    moveScreen();
    checkA3Button();
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        } else {
            receivedChars[ndx] = '\0';
            ndx = 0;
            newData = true;
        }
    }
}

void parseData() {
    char* strtokIndx;

    strtokIndx = strtok(tempChars, "|");
    if (strtokIndx == NULL) return;

    modes[currentMode]->processData(strtokIndx);
}