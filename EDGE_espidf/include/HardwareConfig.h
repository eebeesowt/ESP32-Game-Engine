#pragma once
#include "driver/gpio.h"

// --- Power Management ---
constexpr gpio_num_t PIN_POWER_ON = GPIO_NUM_46;

// --- Display (ST7789) ---
constexpr gpio_num_t PIN_LCD_BL   = GPIO_NUM_15;
constexpr gpio_num_t PIN_LCD_DC   = GPIO_NUM_13;
constexpr gpio_num_t PIN_LCD_RST  = GPIO_NUM_9;
constexpr gpio_num_t PIN_LCD_CS   = GPIO_NUM_10;
constexpr gpio_num_t PIN_LCD_CLK  = GPIO_NUM_12;
constexpr gpio_num_t PIN_LCD_MOSI = GPIO_NUM_11;
constexpr gpio_num_t PIN_LCD_MISO = GPIO_NUM_NC; // Not connected

// --- Rotary Encoder ---
constexpr gpio_num_t PIN_ENC_A    = GPIO_NUM_2;
constexpr gpio_num_t PIN_ENC_B    = GPIO_NUM_1;
constexpr gpio_num_t PIN_ENC_BTN  = GPIO_NUM_0;

// --- APA102 LEDs ---
// T-Embed has 7 addressable LEDs
constexpr gpio_num_t PIN_LED_DI   = GPIO_NUM_42;  // Data In
constexpr gpio_num_t PIN_LED_CI   = GPIO_NUM_45; // Clock In
constexpr int LED_COUNT           = 7;

// --- SD Card (SPI) ---
constexpr gpio_num_t PIN_SD_CS    = GPIO_NUM_39; 
constexpr gpio_num_t PIN_SD_MOSI  = GPIO_NUM_41;
constexpr gpio_num_t PIN_SD_MISO  = GPIO_NUM_38;
constexpr gpio_num_t PIN_SD_CLK   = GPIO_NUM_40;

// --- Audio (I2S) ---
// Placeholder values - NEED VERIFICATION
constexpr gpio_num_t PIN_I2S_BCLK = GPIO_NUM_6;
constexpr gpio_num_t PIN_I2S_LRCK = GPIO_NUM_7;
constexpr gpio_num_t PIN_I2S_DOUT = GPIO_NUM_5; // Speaker
constexpr gpio_num_t PIN_I2S_DIN  = GPIO_NUM_NC; // Mic?

// --- Battery ---
constexpr gpio_num_t PIN_BAT_VOLT = GPIO_NUM_4; // ADC
