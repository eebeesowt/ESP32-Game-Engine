#pragma once

#include <cstdint>

/**
 * @brief Supported display types for the EDGE Renderer.
 */
enum DisplayType {
    SSD1306,  ///< SSD1306 OLED Display (128x64)
    SH1106,   ///< SH1106 OLED Display (128x64)
    ST7789,   ///< ST7789 TFT Display (170x320)
    ILI9341   ///< ILI9341 TFT Display (320x240)
};

/**
 * @brief Configuration settings for display initialization.
 * 
 * Структура адаптирована для ESP-IDF, убраны зависимости от U8g2.
 * Будет использоваться с LovyanGFX.
 */
struct DisplayConfig {
    DisplayType type;               ///< Type of display
    uint8_t clockPin;               ///< I2C/SPI clock pin (SCL/SCK)
    uint8_t dataPin;                ///< I2C data pin (SDA) or SPI MOSI pin
    uint8_t resetPin;               ///< Reset pin (optional), set to 255 if unused
    uint8_t csPin;                  ///< Chip Select pin for SPI (255 if unused)
    uint8_t dcPin;                  ///< Data/Command pin for SPI (255 if unused)
    uint16_t width;                 ///< Display width in pixels
    uint16_t height;                ///< Display height in pixels
    uint8_t rotation;               ///< Display rotation (0, 1, 2, 3)
    bool useHardwareInterface;      ///< True for hardware I2C/SPI, false for software
    int xOffset = 0;                ///< X offset for display rendering
    int yOffset = 0;                ///< Y offset for display rendering

    /**
     * @brief Constructor для I2C дисплеев (SSD1306, SH1106)
     */
    DisplayConfig(DisplayType displayType, uint8_t clk, uint8_t data, uint8_t rst,
                  uint16_t w, uint16_t h, uint8_t rot = 0, bool hwInterface = true)
        : type(displayType), clockPin(clk), dataPin(data), resetPin(rst),
          csPin(255), dcPin(255), width(w), height(h), rotation(rot),
          useHardwareInterface(hwInterface), xOffset(0), yOffset(0) {}

    /**
     * @brief Constructor для SPI дисплеев (ST7789, ILI9341)
     */
    DisplayConfig(DisplayType displayType, uint8_t sck, uint8_t mosi, uint8_t rst,
                  uint8_t cs, uint8_t dc, uint16_t w, uint16_t h, uint8_t rot = 0)
        : type(displayType), clockPin(sck), dataPin(mosi), resetPin(rst),
          csPin(cs), dcPin(dc), width(w), height(h), rotation(rot),
          useHardwareInterface(true), xOffset(0), yOffset(0) {}
};
