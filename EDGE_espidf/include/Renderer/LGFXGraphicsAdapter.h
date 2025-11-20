#pragma once

#include "Renderer/Renderer.h"
#include "DisplayConfig.h"

#if defined(__has_include)
# if __has_include(<LovyanGFX.hpp>)
#  include <LovyanGFX.hpp>
#  define HAVE_LOVYANGFX 1
# endif
#endif

class LGFXGraphicsAdapter : public IGraphics {
public:
    explicit LGFXGraphicsAdapter(const DisplayConfig& conf);
    // Explicit device injection: pass a pointer to a LovyanGFX device instance
    explicit LGFXGraphicsAdapter(const DisplayConfig& conf, void* device);
    void setDevice(void* device);
    ~LGFXGraphicsAdapter() override;

    void init() override;
    void clear() override;
    void present() override;

    void drawText(int x, int y, const char* text) override;
    void drawCircle(int x, int y, int radius) override;
    void fillCircle(int x, int y, int radius) override;
    void drawRect(int x, int y, int width, int height) override;
    void fillRect(int x, int y, int width, int height) override;
    void drawLine(int x1, int y1, int x2, int y2) override;

    void drawBitmap(int x, int y, int w, int h, const uint16_t* data) override;
    void drawBitmap(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor) override;

    void setFont(const void* font) override;
    void setContrast(uint8_t level) override;
    void setColor(uint8_t r, uint8_t g, uint8_t b) override;

private:
    DisplayConfig config;
    void* _devicePtr = nullptr;
#ifdef HAVE_LOVYANGFX
    lgfx::LGFX_Device* _lgfx = nullptr;
    lgfx::LGFX_Sprite* _sprite = nullptr;
#endif
};
