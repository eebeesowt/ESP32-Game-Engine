#include "Renderer/Renderer.h"
#include <cstring>
#include <cstdio>

Renderer::Renderer(IGraphics* gfx_ptr, const DisplayConfig& displayConf)
    : gfx(gfx_ptr), config(displayConf), _logger(nullptr)
{
    width = config.width;
    height = config.height;
    xOffset = config.xOffset;
    yOffset = config.yOffset;
}

void Renderer::init() {
    if (_logger) {
        _logger("Renderer: Initializing...");
    }
    
    if (gfx) {
        gfx->init();
    }
    
    if (_logger) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Renderer: Display %dx%d ready", width, height);
        _logger(buf);
    }
}

void Renderer::beginFrame() {
    if (gfx) {
        gfx->clear();
    }
}

void Renderer::endFrame() {
    if (gfx) {
        gfx->present();
    }
}

void Renderer::drawText(int x, int y, const char* str) {
    if (gfx && str) {
        gfx->drawText(x + xOffset, y + yOffset, str);
    }
}

void Renderer::drawTextSafe(int x, int y, const char* format, ...) {
    if (!gfx || !format) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    gfx->drawText(x + xOffset, y + yOffset, buffer);
}

void Renderer::drawCircle(int x, int y, int radius) {
    if (gfx) {
        gfx->drawCircle(x + xOffset, y + yOffset, radius);
    }
}

void Renderer::drawFilledCircle(int x, int y, int radius) {
    if (gfx) {
        gfx->fillCircle(x + xOffset, y + yOffset, radius);
    }
}

void Renderer::drawRectangle(int x, int y, int width, int height) {
    if (gfx) {
        gfx->drawRect(x + xOffset, y + yOffset, width, height);
    }
}

void Renderer::drawFilledRectangle(int x, int y, int width, int height) {
    if (gfx) {
        gfx->fillRect(x + xOffset, y + yOffset, width, height);
    }
}

void Renderer::drawLine(int x1, int y1, int x2, int y2) {
    if (gfx) {
        gfx->drawLine(x1 + xOffset, y1 + yOffset, x2 + xOffset, y2 + yOffset);
    }
}

void Renderer::drawBitmap(int x, int y, int w, int h, const uint16_t* data) {
    if (gfx) {
        gfx->drawBitmap(x + xOffset, y + yOffset, w, h, data);
    }
}

void Renderer::drawBitmap(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor) {
    if (gfx) {
        gfx->drawBitmap(x + xOffset, y + yOffset, w, h, data, transparentColor);
    }
}

void Renderer::setContrast(uint8_t level) {
    if (gfx) {
        gfx->setContrast(level);
    }
}

void Renderer::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if (gfx) {
        gfx->setColor(r, g, b);
    }
}

void Renderer::setFont(const void* font) {
    if (gfx) {
        gfx->setFont(font);
    }
}
