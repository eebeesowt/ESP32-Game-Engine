#include "Drivers/LEDDriver.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

LEDDriver::LEDDriver() : _brightness(5) { // Default low brightness
    for(int i=0; i<LED_COUNT; i++) {
        _leds[i] = {0, 0, 0};
    }
}

LEDDriver::~LEDDriver() {
}

bool LEDDriver::init() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_LED_DI) | (1ULL << PIN_LED_CI);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    gpio_set_level(PIN_LED_DI, 0);
    gpio_set_level(PIN_LED_CI, 0);
    
    return true;
}

void LEDDriver::setPixel(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= 0 && index < LED_COUNT) {
        _leds[index] = {r, g, b};
    }
}

void LEDDriver::setPixel(int index, CRGB color) {
    setPixel(index, color.r, color.g, color.b);
}

void LEDDriver::clear() {
    for(int i=0; i<LED_COUNT; i++) {
        _leds[i] = {0, 0, 0};
    }
}

void LEDDriver::setBrightness(uint8_t brightness) {
    if (brightness > 31) brightness = 31;
    _brightness = brightness;
}

void LEDDriver::sendByte(uint8_t b) {
    for (int i = 0; i < 8; i++) {
        gpio_set_level(PIN_LED_DI, (b & 0x80) ? 1 : 0);
        gpio_set_level(PIN_LED_CI, 1);
        gpio_set_level(PIN_LED_CI, 0);
        b <<= 1;
    }
}

void LEDDriver::startFrame() {
    // 32 bits of 0
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
}

void LEDDriver::endFrame() {
    // 32 bits of 1 (or just clock pulses)
    // APA102 needs at least LED_COUNT/2 clock cycles to push data through
    sendByte(0xFF);
    sendByte(0xFF);
    sendByte(0xFF);
    sendByte(0xFF);
}

void LEDDriver::show() {
    startFrame();
    
    for (int i = 0; i < LED_COUNT; i++) {
        // Header: 111 + 5bit brightness
        sendByte(0xE0 | _brightness);
        // B G R
        sendByte(_leds[i].b);
        sendByte(_leds[i].g);
        sendByte(_leds[i].r);
    }
    
    endFrame();
}
