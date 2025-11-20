#pragma once
#include <cstdint>
#include "HardwareConfig.h"

struct CRGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class LEDDriver {
public:
    LEDDriver();
    ~LEDDriver();

    bool init();
    void setPixel(int index, uint8_t r, uint8_t g, uint8_t b);
    void setPixel(int index, CRGB color);
    void clear();
    void show();
    void setBrightness(uint8_t brightness); // 0-31 for APA102

private:
    CRGB _leds[LED_COUNT];
    uint8_t _brightness;
    
    void sendByte(uint8_t b);
    void startFrame();
    void endFrame();
};
