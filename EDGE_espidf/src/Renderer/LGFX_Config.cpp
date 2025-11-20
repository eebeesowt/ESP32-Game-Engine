// Concrete LovyanGFX device for LilyGO T-Embed (ST7789 SPI)
#include "Renderer/LGFXGraphicsAdapter.h"
#include "driver/gpio.h"
#if defined(__has_include)
# if __has_include(<LovyanGFX.hpp>)
#  include <LovyanGFX.hpp>
#  define HAVE_LOVYANGFX 1
# endif
#endif

#ifdef HAVE_LOVYANGFX

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    LGFX(void) {
        // Initialize power GPIO first
        gpio_set_direction(GPIO_NUM_46, GPIO_MODE_OUTPUT);  // PIN_POWER_ON
        gpio_set_level(GPIO_NUM_46, 1);  // Power ON
        
        // Configure SPI bus for T-Embed
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;         // HSPI
            cfg.spi_mode = 0;                 // SPI mode 0
            cfg.freq_write = 40000000;        // 40MHz write
            cfg.freq_read = 16000000;         // 16MHz for reads
            cfg.pin_sclk = 12;                // CLK
            cfg.pin_mosi = 11;                // MOSI
            cfg.pin_miso = -1;                // No MISO for display
            cfg.pin_dc = 13;                  // D/C (Command/Data)
            cfg.use_lock = true;              // Use mutex for thread safety
            
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // Configure ST7789 panel
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 10;                  // CS
            cfg.pin_rst = 9;                  // RST
            cfg.pin_busy = -1;
            cfg.panel_width = 170;            // Active glass is 170px wide inside 240px controller RAM
            cfg.panel_height = 320;           // Active glass height
            cfg.offset_x = 35;                // Shift column window so visible area is centered
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;          // Offsets specified for the default rotation
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;            // Panel expects inverted polarity; otherwise colors flip
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            cfg.memory_width = 240;           // ST7789 internal RAM dimensions
            cfg.memory_height = 320;
            _panel_instance.config(cfg);
        }

        // Backlight PWM on GPIO15
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = 15;                  // BL
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 0;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};

static LGFX lgfx_instance;

extern "C" void* renderer_get_lcd_device(void) {
    return &lgfx_instance;
}

#endif // HAVE_LOVYANGFX
