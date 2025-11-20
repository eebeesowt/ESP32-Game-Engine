#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "HardwareConfig.h"
#include "Renderer/Renderer.h"
#include "Renderer/LGFXGraphicsAdapter.h"
#include "InputManager/InputManager.h"
#include "InputManager/EncoderDriver.h"
#include "SceneManager/Scene.h"
#include "SceneManager/SceneManager.h"
#include "Drivers/LEDDriver.h"
#include "Drivers/AudioDriver.h"
#include "Drivers/SDCardDriver.h"
#include "Scenes/IntroScene.h"
#include "Scenes/GameScene.h"

extern "C" void* renderer_get_lcd_device(void);

static const char* TAG = "APP_MAIN";

static void init_power_rails()
{
    gpio_set_direction(PIN_POWER_ON, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_POWER_ON, 1);

    gpio_set_direction(PIN_LCD_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_LCD_BL, 1);

    gpio_set_direction(PIN_LCD_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_LCD_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_LCD_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
}

extern "C" void app_main(void)
{
    printf("\n\n");
    printf("========================================\n");
    printf("ESP32-S3 T-Embed Initialization\n");
    printf("========================================\n");
    ESP_LOGI(TAG, "System started!");
    
    ESP_LOGI(TAG, "Enabling board power rails...");
    init_power_rails();

    // 1. Setup Drivers
    LEDDriver ledDriver;
    ledDriver.init();
    ledDriver.setBrightness(5);
    ledDriver.setPixel(0, 20, 0, 0); // Red on boot
    ledDriver.show();
    
    // AudioDriver audioDriver;
    // audioDriver.init(); // Commented out until pins verified
    
    SDCardDriver sdDriver;
    if (sdDriver.init()) {
        ESP_LOGI(TAG, "SD Card initialized successfully");
    } else {
        ESP_LOGE(TAG, "SD Card initialization failed");
    }

    // 2. Setup Display Config
    // Rotation 3 is usually correct for landscape on T-Embed
    DisplayConfig displayConfig(DisplayType::ST7789, 0, 0, 0, 0, 0, 320, 170, 3); 
    
    // 3. Setup Renderer
    void* device = renderer_get_lcd_device();
    LGFXGraphicsAdapter* gfxAdapter = new LGFXGraphicsAdapter(displayConfig, device);
    Renderer renderer(gfxAdapter, displayConfig);
    renderer.setLogger([](const char* msg){ ESP_LOGI("Renderer", "%s", msg); });
    renderer.init();

    // 4. Setup InputManager
    InputManager inputManager;
    inputManager.setLogger([](const char* msg){ ESP_LOGI("InputManager", "%s", msg); });
    inputManager.init();

    // 5. Setup SceneManager
    SceneManager sceneManager;
    sceneManager.setLogger([](const char* msg){ ESP_LOGI("SceneManager", "%s", msg); });
    sceneManager.setInputManager(&inputManager);
    inputManager.setSceneManager(&sceneManager);

    // 6. Setup EncoderDriver
    EncoderDriver encoderDriver(&inputManager);
    encoderDriver.initEncoder(0, PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN);
    encoderDriver.start();

    // 7. Setup Scenes
    // We capture sceneManager and inputManager by reference/pointer for the factories
    static SceneManager* s_sceneManager = &sceneManager;
    static InputManager* s_inputManager = &inputManager;

    sceneManager.registerScene("IntroScene", [](void* config) -> Scene* {
        return new IntroScene(s_sceneManager, s_inputManager);
    });

    sceneManager.registerScene("GameScene", [](void* config) -> Scene* {
        return new GameScene(s_sceneManager, s_inputManager);
    });
    
    // Start with IntroScene
    sceneManager.setCurrentScene("IntroScene");

    ESP_LOGI(TAG, "Entering main loop...");
    
    int64_t last_time = esp_timer_get_time();
    
    while (true) {
        int64_t current_time = esp_timer_get_time();
        // Calculate delta time in milliseconds
        unsigned long dt = (current_time - last_time) / 1000;
        last_time = current_time;

        // Cap dt to avoid huge jumps if debugging or lag spike (e.g. max 100ms)
        if (dt > 100) dt = 100;

        inputManager.update(dt);
        sceneManager.processSceneChanges();
        sceneManager.update(dt);
        sceneManager.draw(renderer);
        
        // Small yield to feed watchdog and allow other tasks to run
        // If we want max FPS, we can use taskYIELD() or vTaskDelay(1)
        vTaskDelay(1); 
    }
}
