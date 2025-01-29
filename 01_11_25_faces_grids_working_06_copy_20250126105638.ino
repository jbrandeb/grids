// License: MIT
// Copyright (c) 2025 Andy Batt
// Copyright (c) 2025 Jesse Brandeburg

#include "FastLED.h"

// LED configuration
#define NUM_LEDS1 52       // 100 WS2812B per meter - GRID 01 - User 01
#define NUM_LEDS2 52       // 100 WS2812B per meter - GRID 02 - User 02
#define NUM_LEDS3 52       // 8x32 panel - GRID 03 - RANDOMIZE
#define NUM_LEDS4 50        // FCOB Strip 90 WS2812B - User 1 Control for User 2 Face
#define NUM_LEDS5 50        // FCOB Strip 90 WS2812B - User 1 Control for User 1 Face
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Data pins for the LED strips
#define DATA_PIN1 3         // GRID 01 - User 01
#define DATA_PIN2 5         // GRID 02 - User 02
#define DATA_PIN3 7         // GRID 03 - Randomized
#define DATA_PIN4 9         // USER 2 Face
#define DATA_PIN5 11        // USER 1 Face

// Potentiometer pins
#define POT_PIN1 A0
#define POT_PIN2 A1

// Button pins
#define BUTTON_PIN1 2        // RESTART Grid 01 Timer - User 01
#define BUTTON_PIN2 4        // RESTART Grid 02 Timer - User 02
#define BUTTON_PIN3 6        // UNUSED
#define BUTTON_PIN4 8        // U1 TURN ON U2 FACE LIGHT
#define BUTTON_PIN5 10       // U2 TURN ON U1 FACE LIGHT

// Define the arrays of LEDs
CRGB leds1[NUM_LEDS1];
CRGB leds2[NUM_LEDS2];
CRGB leds3[NUM_LEDS3];
CRGBArray<NUM_LEDS4> leds4;
CRGBArray<NUM_LEDS5> leds5;

// Timer variables
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;
unsigned long ledOnTime4 = 0;
unsigned long ledOnTime5 = 0;

const unsigned long initialRunTime = 5000; // 5 seconds for initial run
const unsigned long buttonRunTime = 10000; // 10 seconds for Strip 1 and 2
const unsigned long strip3RunTime = 3000; // 3 seconds for Strip 3
const unsigned long ledStripRunTime = 10000; // 30 seconds for Strip 4 and 5

const unsigned long debounceDelay = 50; // Debounce delay

// Debounce variables
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime4 = 0;
unsigned long lastDebounceTime5 = 0;
bool currentButtonState1;
bool currentButtonState2;
bool currentButtonState4;
bool currentButtonState5;
bool lastButtonState1 = HIGH;
bool lastButtonState2 = HIGH;
bool lastButtonState4 = HIGH;
bool lastButtonState5 = HIGH;

// LED state variables
bool ledState4 = false;
bool ledState5 = false;

// Fade values
uint8_t fadeVal1 = 200;
uint8_t fadeVal2 = 200;
uint8_t fadeVal3 = 200;
uint8_t fadeVal4 = 225; // FADEVALUE for LED strip 4
uint8_t fadeVal5 = 225; // FADEVALUE for LED strip 5

// Function declarations
void turnOffLEDs4();
void turnOffLEDs5();
void animateStrip4();
void animateStrip5();

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<WS2812B, DATA_PIN1, GRB>(leds1, NUM_LEDS1);
    FastLED.addLeds<WS2812B, DATA_PIN2, GRB>(leds2, NUM_LEDS2);
    FastLED.addLeds<WS2812B, DATA_PIN3, GRB>(leds3, NUM_LEDS3);
    FastLED.addLeds<LED_TYPE, DATA_PIN4, COLOR_ORDER>(leds4, NUM_LEDS4);
    FastLED.addLeds<LED_TYPE, DATA_PIN5, COLOR_ORDER>(leds5, NUM_LEDS5);
    FastLED.setBrightness(255);

     // Initialize parallel output
    FastLED.addLeds<WS2812B, DATA_PIN1, GRB>(leds1, NUM_LEDS1).setDither(0);
    FastLED.addLeds<WS2812B, DATA_PIN2, GRB>(leds2, NUM_LEDS2).setDither(0);
    FastLED.addLeds<WS2812B, DATA_PIN3, GRB>(leds3, NUM_LEDS3).setDither(0);
    FastLED.addLeds<WS2812B, DATA_PIN4, GRB>(leds4, NUM_LEDS4).setDither(0);
    FastLED.addLeds<WS2812B, DATA_PIN5, GRB>(leds5, NUM_LEDS5).setDither(0);

    pinMode(BUTTON_PIN1, INPUT_PULLUP);
    pinMode(BUTTON_PIN2, INPUT_PULLUP);
    pinMode(BUTTON_PIN4, INPUT_PULLUP);
    pinMode(BUTTON_PIN5, INPUT_PULLUP);
    pinMode(POT_PIN1, INPUT);
    pinMode(POT_PIN2, INPUT);

    // Start the initial animation
    timer1 = millis();
    timer2 = millis();
    timer3 = millis();

    // Blink both LED strips on and off once at startup
    blinkLEDs();
    delay(2000);
}

void fadeall(CRGB* leds, uint8_t fadeVal, int numLeds) {
    for (int i = 0; i < numLeds; i++) {
        leds[i].nscale8(fadeVal);
    }
}

void animateStrip1() {
    static uint8_t hue1 = 96; // Start with green hue
    static int index1 = 0;
    static int direction1 = 1;

    leds1[index1] = CHSV(hue1, 255, 255);
    hue1 += 1;
    if (hue1 > 128) {
        hue1 = 64;
    }
    FastLED.show();
    fadeall(leds1, fadeVal1, NUM_LEDS1);

    if (direction1 == 1) {
        index1++;
        if (index1 >= NUM_LEDS1) {
            index1 = NUM_LEDS1 - 1;
            direction1 = -1;
        }
    } else {
        index1--;
        if (index1 < 0) {
            index1 = 0;
            direction1 = 1;
        }
    }
}

void animateStrip2() {
    static uint8_t hue2 = 96; // Start with green hue
    static int index2 = 0;
    static int direction2 = 1;

    leds2[index2] = CHSV(hue2, 255, 255);
    hue2 += 1;
    if (hue2 > 128) {
        hue2 = 64;
    }
    FastLED.show();
    fadeall(leds2, fadeVal2, NUM_LEDS2);

    if (direction2 == 1) {
        index2++;
        if (index2 >= NUM_LEDS2) {
            index2 = NUM_LEDS2 - 1;
            direction2 = -1;
        }
    } else {
        index2--;
        if (index2 < 0) {
            index2 = 0;
            direction2 = 1;
        }
    }
}

void animateStrip3() {
    static uint8_t hue3 = 32; // Start with orange hue
    static int index3 = 0;
    static int direction3 = 1;

    leds3[index3] = CHSV(hue3, 255, 255);
    hue3 += 1; // Increment hue
    if (hue3 > 64) { // Limit hue to orange to yellow range (32-64)
        hue3 = 32;
    }
    FastLED.show();
    fadeall(leds3, fadeVal3, NUM_LEDS3);

    if (direction3 == 1) {
        index3++;
        if (index3 >= NUM_LEDS3) {
            index3 = NUM_LEDS3 - 1;
            direction3 = -1;
        }
    } else {
        index3--;
        if (index3 < 0) {
            index3 = 0;
            direction3 = 1;
        }
    }
}

void animateStrip4() {
    static uint8_t hue4 = 0; // Start with red hue
    static int index4 = 0;
    static int direction4 = 1;

    leds4[index4] = CHSV(hue4, 255, 255);
    hue4 += 1;
    if (hue4 > 255) {
        hue4 = 0;
    }
    FastLED.show();
    fadeall(leds4, fadeVal4, NUM_LEDS4);

    if (direction4 == 1) {
        index4++;
        if (index4 >= NUM_LEDS4) {
            index4 = NUM_LEDS4 - 1;
            direction4 = -1;
        }
    } else {
        index4--;
        if (index4 < 0) {
            index4 = 0;
            direction4 = 1;
        }
    }
}

void animateStrip5() {
    static int index5 = 0;
    static int direction5 = 1;

    // Set the color to full red and brightness to 250
    leds5[index5] = CRGB::Red;
    leds5[index5].nscale8(250); // Set brightness to 250

    FastLED.show();
    fadeall(leds5, fadeVal5, NUM_LEDS5);

    if (direction5 == 1) {
        index5++;
        if (index5 >= NUM_LEDS5) {
            index5 = NUM_LEDS5 - 1;
            direction5 = -1;
        }
    } else {
        index5--;
        if (index5 < 0) {
            index5 = 0;
            direction5 = 1;
        }
    }
}

void loop() {
    unsigned long currentTime = millis();
    static unsigned long nextStartTime3 = 0;
    static unsigned long runStartTime3 = currentTime; // Start immediately
    static bool isRunning3 = true; // Start running immediately

    // Read potentiometer values
    fadeVal1 = map(analogRead(POT_PIN1), 0, 1023, 25, 255);
    fadeVal2 = map(analogRead(POT_PIN2), 0, 1023, 25, 255);

    // Debounce Button 1
    bool reading1 = digitalRead(BUTTON_PIN1);
    if (reading1 != lastButtonState1) {
        Serial.println("Button 1 pressed");
        lastDebounceTime1 = currentTime;
    }
    if ((currentTime - lastDebounceTime1) > debounceDelay) {
        if (reading1 != currentButtonState1) {
            currentButtonState1 = reading1;
            if (currentButtonState1 == HIGH) {
                timer1 = currentTime;
            }
        }
    }
    lastButtonState1 = reading1;

    // Debounce Button 2
    bool reading2 = digitalRead(BUTTON_PIN2);
    if (reading2 != lastButtonState2) {
        Serial.println("Button 2 pressed");
        lastDebounceTime2 = currentTime;
    }
    if ((currentTime - lastDebounceTime2) > debounceDelay) {
        if (reading2 != currentButtonState2) {
            currentButtonState2 = reading2;
            if (currentButtonState2 == HIGH) {
                timer2 = currentTime;
            }
        }
    }
    lastButtonState2 = reading2;

        // Debounce Button 4
    bool reading4 = digitalRead(BUTTON_PIN4);
    if (reading4 != lastButtonState4) {
        Serial.println("Button 4 pressed");
        lastDebounceTime4 = currentTime;
    }
    if ((currentTime - lastDebounceTime4) > debounceDelay) {
        if (reading4 != currentButtonState4) {
            currentButtonState4 = reading4;
            ledState5 = !ledState5;
            if (ledState5) {
                ledOnTime5 = currentTime;  // Record the time when LEDs are turned on
            } else {
                turnOffLEDs5();
            }
        }
    }
    lastButtonState4 = reading4;

    // Debounce Button 5
    bool reading5 = digitalRead(BUTTON_PIN5);
    if (reading5 != lastButtonState5) {
        Serial.println("Button 5 pressed");
        lastDebounceTime5 = currentTime;
    }
    if ((currentTime - lastDebounceTime5) > debounceDelay) {
        if (reading5 != currentButtonState5) {
            currentButtonState5 = reading5;
            ledState4 = !ledState4;
            if (ledState4) {
                ledOnTime4 = currentTime;  // Record the time when LEDs are turned on
            } else {
                turnOffLEDs4();
            }
        }
    }
    lastButtonState5 = reading5;

    // Run animations for Strip 1
    if (currentTime - timer1 < initialRunTime || (timer1 != 0 && currentTime - timer1 < buttonRunTime)) {
        animateStrip1();
    } else {
        fill_solid(leds1, NUM_LEDS1, CRGB::Black);
        FastLED.show();
        timer1 = 0; // Reset timer to prevent continuous running
    }

    // Run animations for Strip 2
    if (currentTime - timer2 < initialRunTime || (timer2 != 0 && currentTime - timer2 < buttonRunTime)) {
        animateStrip2();
    } else {
        fill_solid(leds2, NUM_LEDS2, CRGB::Black);
        FastLED.show();
        timer2 = 0; // Reset timer to prevent continuous running
    }

    // Run animations for Strip 3 at random intervals
    if (isRunning3) {
        if (currentTime - runStartTime3 < strip3RunTime) { // Run time
            animateStrip3();
        } else {
            fill_solid(leds3, NUM_LEDS3, CRGB::Black);
            FastLED.show();
            isRunning3 = false;
            nextStartTime3 = currentTime + random(15000, 30000); // Random interval between 15 and 30 seconds
        }
    } else if (currentTime >= nextStartTime3) {
        isRunning3 = true;
        runStartTime3 = currentTime;
    }

    // Turn off the LEDs after 5 seconds if they are on
    if (ledState4 && (currentTime - ledOnTime4 >= ledStripRunTime)) {
        turnOffLEDs4();
        ledState4 = false;
    } else if (ledState4) {
        animateStrip4();
    }

    if (ledState5 && (currentTime - ledOnTime5 >= ledStripRunTime)) {
        turnOffLEDs5();
        ledState5 = false;
    } else if (ledState5) {
        animateStrip5();
    }
}

// Function to blink all five LED strips once at startup
void blinkLEDs() {
    for (int i = 0; i < NUM_LEDS1; i++) {
        animateStrip1();
        delay(5);
    }
    for (int i = 0; i < NUM_LEDS2; i++) {
        animateStrip2();
        delay(5);
    }
    for (int i = 0; i < NUM_LEDS3; i++) {
        animateStrip3();
        delay(5);
    }for (int i = 0; i < NUM_LEDS4; i++) {
        animateStrip4();
        delay(5);
    }
    for (int i = 0; i < NUM_LEDS5; i++) {
        animateStrip5();
        delay(5);
    }
    delay(100);

    turnOffLEDs1();
    turnOffLEDs2();
    turnOffLEDs3();
    turnOffLEDs4();
    turnOffLEDs5();
}

// Function to turn off the LEDs for strip 1
void turnOffLEDs1() {
    fill_solid(leds1, NUM_LEDS1, CRGB::Black);
    FastLED.show();
}

// Function to turn off the LEDs for strip 2
void turnOffLEDs2() {
    fill_solid(leds2, NUM_LEDS2, CRGB::Black);
    FastLED.show();
}

// Function to turn off the LEDs for strip 3
void turnOffLEDs3() {
    fill_solid(leds3, NUM_LEDS3, CRGB::Black);
    FastLED.show();
}

// Function to turn off the LEDs for strip 4
void turnOffLEDs4() {
    fill_solid(leds4, NUM_LEDS4, CRGB::Black);
    FastLED.show();
}

// Function to turn off the LEDs for strip 5
void turnOffLEDs5() {
    fill_solid(leds5, NUM_LEDS5, CRGB::Black);
    FastLED.show();
}
