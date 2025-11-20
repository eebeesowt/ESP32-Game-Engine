#include "InputManager/EncoderDriver.h"

#if defined(CONFIG_IDF_TARGET) || defined(__ESP32__)
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_log.h>
#endif

#include <vector>
#include <cstring>

volatile int g_isr_count = 0;
volatile int g_task_count = 0;

static const char* TAG = "EncoderDriver";

enum class _InternalEventType : uint8_t {
    ENCODER_ROTATE,
    BUTTON_PRESS,
    BUTTON_RELEASE
};

struct _InternalEvent {
    _InternalEventType type;
    int encoderId;
    int8_t direction;
};

static QueueHandle_t s_event_queue = NULL;
static InputManager* s_input_manager = nullptr;

struct EncoderInfo {
    int id;
    int pinA;
    int pinB;
    int btnPin;
    volatile int lastState;
    volatile int subStepCount; // Accumulator for micro-steps
    bool active;
};

static EncoderInfo s_encoders[4];
static int s_encoderCount = 0;

static void IRAM_ATTR encoder_isr_handler(void* arg) {
    g_isr_count++;
    EncoderInfo* info = (EncoderInfo*)arg;
    if (!info || !s_event_queue) return;

    int a = gpio_get_level((gpio_num_t)info->pinA);
    int b = gpio_get_level((gpio_num_t)info->pinB);
    int newState = (a << 1) | b;
    int oldState = info->lastState;
    
    if (newState != oldState) {
        int8_t direction = 0;
        int combined = (oldState << 2) | newState;
        // 00 -> 01 (+), 01 -> 11 (+), 11 -> 10 (+), 10 -> 00 (+)
        if (combined == 1 || combined == 7 || combined == 14 || combined == 8) direction = 1;
        // 00 -> 10 (-), 10 -> 11 (-), 11 -> 01 (-), 01 -> 00 (-)
        else if (combined == 2 || combined == 11 || combined == 13 || combined == 4) direction = -1;
        
        if (direction != 0) {
            info->subStepCount += direction;
            
            // Only fire event every 2 transitions (adjust this value if needed: 2 or 4)
            if (info->subStepCount >= 2 || info->subStepCount <= -2) {
                _InternalEvent ev;
                ev.type = _InternalEventType::ENCODER_ROTATE;
                ev.encoderId = info->id;
                ev.direction = (info->subStepCount > 0) ? 1 : -1;
                
                BaseType_t xHigher = pdFALSE;
                xQueueSendFromISR(s_event_queue, &ev, &xHigher);
                if (xHigher == pdTRUE) portYIELD_FROM_ISR();
                
                info->subStepCount = 0;
            }
        }
        info->lastState = newState;
    }
}

static void IRAM_ATTR button_isr_handler(void* arg) {
    EncoderInfo* info = (EncoderInfo*)arg;
    if (!info || !s_event_queue) return;
    
    int level = gpio_get_level((gpio_num_t)info->btnPin);
    _InternalEvent ev;
    ev.type = level == 0 ? _InternalEventType::BUTTON_PRESS : _InternalEventType::BUTTON_RELEASE;
    ev.encoderId = info->id;
    ev.direction = 0;
    
    BaseType_t xHigher = pdFALSE;
    xQueueSendFromISR(s_event_queue, &ev, &xHigher);
    if (xHigher == pdTRUE) portYIELD_FROM_ISR();
}

static void encoder_task(void* pv) {
    _InternalEvent ev;
    TickType_t pressStartTime = 0;
    const TickType_t longPressThreshold = pdMS_TO_TICKS(400);

    while (xQueueReceive(s_event_queue, &ev, portMAX_DELAY) == pdTRUE) {
        g_task_count++;
        if (!s_input_manager) continue;
        
        if (ev.type == _InternalEventType::ENCODER_ROTATE) {
            if (ev.direction > 0) s_input_manager->processEncoderEvent(ev.encoderId, EDGE_EncoderRotate::ROTATE_CW);
            else s_input_manager->processEncoderEvent(ev.encoderId, EDGE_EncoderRotate::ROTATE_CCW);
        } else if (ev.type == _InternalEventType::BUTTON_PRESS) {
             if (ev.encoderId == 0) {
                 s_input_manager->processButtonEvent(EDGE_Button::OK, EDGE_Event::PRESS);
                 pressStartTime = xTaskGetTickCount();
             }
        } else if (ev.type == _InternalEventType::BUTTON_RELEASE) {
             if (ev.encoderId == 0) {
                 s_input_manager->processButtonEvent(EDGE_Button::OK, EDGE_Event::RELEASE);
                 
                 TickType_t duration = xTaskGetTickCount() - pressStartTime;
                 if (duration > longPressThreshold) {
                     s_input_manager->processButtonEvent(EDGE_Button::OK, EDGE_Event::LONG_PRESS);
                 } else {
                     s_input_manager->processButtonEvent(EDGE_Button::OK, EDGE_Event::CLICK);
                 }
             }
        }
    }
    vTaskDelete(NULL);
}

EncoderDriver::EncoderDriver(InputManager* im) : _inputManager(im) {
    s_input_manager = im;
    s_encoderCount = 0;
    for(int i=0; i<4; i++) s_encoders[i].active = false;
}

EncoderDriver::~EncoderDriver() {
    stop();
}

bool EncoderDriver::initEncoder(int encoderId, int pinA, int pinB, int buttonPin) {
    if (s_encoderCount >= 4) return false;
    
    EncoderInfo& info = s_encoders[s_encoderCount++];
    info.id = encoderId;
    info.pinA = pinA;
    info.pinB = pinB;
    info.btnPin = buttonPin;
    info.active = true;
    info.subStepCount = 0;
    
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pin_bit_mask = (1ULL << pinA) | (1ULL << pinB);
    if (buttonPin >= 0) {
        io_conf.pin_bit_mask |= (1ULL << buttonPin);
    }
    gpio_config(&io_conf);
    
    int a = gpio_get_level((gpio_num_t)pinA);
    int b = gpio_get_level((gpio_num_t)pinB);
    info.lastState = (a << 1) | b;
    
    gpio_install_isr_service(0);
    
    gpio_isr_handler_add((gpio_num_t)pinA, encoder_isr_handler, &info);
    gpio_isr_handler_add((gpio_num_t)pinB, encoder_isr_handler, &info);
    
    if (buttonPin >= 0) {
        gpio_isr_handler_add((gpio_num_t)buttonPin, button_isr_handler, &info);
    }
    
    return true;
}

bool EncoderDriver::start() {
    if (s_event_queue == NULL) {
        s_event_queue = xQueueCreate(20, sizeof(_InternalEvent));
    }
    xTaskCreate(encoder_task, "encoder_task", 4096, NULL, 10, NULL);
    return true;
}

void EncoderDriver::stop() {
    // Cleanup not fully implemented
}
