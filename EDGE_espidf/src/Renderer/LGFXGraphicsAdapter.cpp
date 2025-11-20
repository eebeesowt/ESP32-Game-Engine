#include "Renderer/LGFXGraphicsAdapter.h"
#include <cstdio>

#ifdef HAVE_LOVYANGFX
// Use LovyanGFX panel/bus configuration if required later
#if defined(__GNUC__)
// Provide a weak default implementation so linking succeeds if the project
// doesn't supply `renderer_get_lcd_device`.
extern "C" void* renderer_get_lcd_device(void) __attribute__((weak));
extern "C" void* renderer_get_lcd_device(void) { return nullptr; }
#endif
#endif

LGFXGraphicsAdapter::LGFXGraphicsAdapter(const DisplayConfig& conf)
    : config(conf), _devicePtr(nullptr)
{
}

LGFXGraphicsAdapter::LGFXGraphicsAdapter(const DisplayConfig& conf, void* device)
    : config(conf), _devicePtr(device)
{
}

LGFXGraphicsAdapter::~LGFXGraphicsAdapter() {
#ifdef HAVE_LOVYANGFX
    if (_sprite) {
        delete _sprite;
        _sprite = nullptr;
    }
#endif
}

void LGFXGraphicsAdapter::setDevice(void* device) {
    _devicePtr = device;
    #ifdef HAVE_LOVYANGFX
    _lgfx = static_cast<lgfx::LGFX_Device*>(_devicePtr);
    #endif
}

void LGFXGraphicsAdapter::init() {
#ifdef HAVE_LOVYANGFX
    // If a device was injected, cast and init it
    if (_devicePtr) {
        _lgfx = static_cast<lgfx::LGFX_Device*>(_devicePtr);
        if (_lgfx) {
            _lgfx->init();
            _lgfx->setRotation(config.rotation); // Apply rotation from config
            if (!_sprite) {
                _sprite = new lgfx::LGFX_Sprite(_lgfx);
                _sprite->setColorDepth(16);
                _sprite->createSprite(config.width, config.height);
            }
        }
    }
#else
    // No-op fallback when LovyanGFX isn't available
#endif
}

void LGFXGraphicsAdapter::clear() {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->fillScreen(0);
    else if (_lgfx) _lgfx->fillScreen(0);
#else
    // no-op
#endif
}

void LGFXGraphicsAdapter::present() {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->pushSprite(0, 0);
#endif
}

void LGFXGraphicsAdapter::drawText(int x, int y, const char* text) {
#ifdef HAVE_LOVYANGFX
    if (!text) return;
    if (_sprite) {
        _sprite->setTextDatum(0);
        _sprite->drawString(text, x, y);
    } else if (_lgfx) {
        _lgfx->setTextDatum(0);
        _lgfx->drawString(text, x, y);
    }
#endif
}

void LGFXGraphicsAdapter::drawCircle(int x, int y, int radius) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->drawCircle(x, y, radius);
    else if (_lgfx) _lgfx->drawCircle(x, y, radius);
#endif
}

void LGFXGraphicsAdapter::fillCircle(int x, int y, int radius) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->fillCircle(x, y, radius);
    else if (_lgfx) _lgfx->fillCircle(x, y, radius);
#endif
}

void LGFXGraphicsAdapter::drawBitmap(int x, int y, int w, int h, const uint16_t* data) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->pushImage(x, y, w, h, data);
    else if (_lgfx) _lgfx->pushImage(x, y, w, h, data);
#endif
}

void LGFXGraphicsAdapter::drawBitmap(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->pushImage(x, y, w, h, data, transparentColor);
    else if (_lgfx) _lgfx->pushImage(x, y, w, h, data, transparentColor);
#endif
}

void LGFXGraphicsAdapter::drawRect(int x, int y, int width, int height) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->drawRect(x, y, width, height);
    else if (_lgfx) _lgfx->drawRect(x, y, width, height);
#endif
}

void LGFXGraphicsAdapter::fillRect(int x, int y, int width, int height) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->fillRect(x, y, width, height);
    else if (_lgfx) _lgfx->fillRect(x, y, width, height);
#endif
}

void LGFXGraphicsAdapter::drawLine(int x1, int y1, int x2, int y2) {
#ifdef HAVE_LOVYANGFX
    if (_sprite) _sprite->drawLine(x1, y1, x2, y2);
    else if (_lgfx) _lgfx->drawLine(x1, y1, x2, y2);
#endif
}

void LGFXGraphicsAdapter::setFont(const void* font) {
#ifdef HAVE_LOVYANGFX
    // LovyanGFX expects a font object; user can pass pointer compatible with LGFX
    if (font) {
        // Attempt to set font if type matches
        // _lgfx.setFont((const lgfx::IFont*)font);
    }
#endif
}

void LGFXGraphicsAdapter::setContrast(uint8_t level) {
#ifdef HAVE_LOVYANGFX
    if (_lgfx) _lgfx->setBrightness(level);
#endif
}

void LGFXGraphicsAdapter::setColor(uint8_t r, uint8_t g, uint8_t b) {
#ifdef HAVE_LOVYANGFX
    // Create 16-bit color (565)
    uint16_t color = lgfx::color565(r, g, b);
    if (_sprite) {
        _sprite->setColor(color);
        _sprite->setTextColor(color);
    } else if (_lgfx) {
        _lgfx->setColor(color);
        _lgfx->setTextColor(color);
    }
#endif
}
