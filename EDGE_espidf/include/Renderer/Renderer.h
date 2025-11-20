#pragma once

#include <functional>
#include <cstdint>
#include <cstdarg>
#include "DisplayConfig.h"

// Define the logger type
using EDGELogger = std::function<void(const char* message)>;

/**
 * @brief Абстрактный интерфейс для графической библиотеки
 * 
 * Позволяет абстрагироваться от конкретной реализации (U8g2, LovyanGFX и т.д.)
 * На данном этапе миграции - временная заглушка, будет реализован позже
 */
class IGraphics {
public:
    virtual ~IGraphics() = default;
    
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void present() = 0;
    
    virtual void drawText(int x, int y, const char* text) = 0;
    virtual void drawCircle(int x, int y, int radius) = 0;
    virtual void fillCircle(int x, int y, int radius) = 0;
    virtual void drawRect(int x, int y, int width, int height) = 0;
    virtual void fillRect(int x, int y, int width, int height) = 0;
    virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
    
    // Bitmap drawing
    virtual void drawBitmap(int x, int y, int w, int h, const uint16_t* data) = 0;
    virtual void drawBitmap(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor) = 0;

    virtual void setFont(const void* font) = 0;
    virtual void setContrast(uint8_t level) = 0;
    
    // New methods for color control
    virtual void setColor(uint8_t r, uint8_t g, uint8_t b) = 0;
};

/**
 * @brief Renderer - обёртка над графической библиотекой
 * 
 * Адаптирован для ESP-IDF. На данном этапе использует абстрактный IGraphics.
 * Позже будет реализован адаптер для LovyanGFX.
 */
class Renderer {
public:
    Renderer(IGraphics* gfx_ptr, const DisplayConfig& displayConf);
    
    void init();
    void beginFrame();
    void endFrame();
    
    void setLogger(EDGELogger logger) { _logger = logger; }
    
    // Методы рисования
    void setColor(uint8_t r, uint8_t g, uint8_t b); // Added
    void drawText(int x, int y, const char* str);
    void drawTextSafe(int x, int y, const char* format, ...);
    void drawCircle(int x, int y, int radius);
    void drawFilledCircle(int x, int y, int radius);
    void drawRectangle(int x, int y, int width, int height);
    void drawFilledRectangle(int x, int y, int width, int height);
    void drawLine(int x1, int y1, int x2, int y2);
    
    void drawBitmap(int x, int y, int w, int h, const uint16_t* data);
    void drawBitmap(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor);

    void setContrast(uint8_t level);
    void setFont(const void* font);
    
    // Геттеры
    int getXOffset() const { return xOffset; }
    int getYOffset() const { return yOffset; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    IGraphics* getGraphics() { return gfx; }
    
private:
    IGraphics* gfx;
    const DisplayConfig& config;
    EDGELogger _logger;
    
    int width;
    int height;
    int xOffset;
    int yOffset;
};
