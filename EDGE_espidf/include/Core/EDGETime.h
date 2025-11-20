#pragma once

#include <stdint.h>

#if defined(__has_include)
    #if __has_include(<esp_timer.h>)
        #include <esp_timer.h>
    #endif
#endif

// If esp_timer is not available, fallback to FreeRTOS tick count
#if !defined(esp_timer_get_time)
    #if defined(__has_include)
        #if __has_include(<freertos/FreeRTOS.h>)
            #include <freertos/FreeRTOS.h>
        #endif
        #if __has_include(<freertos/task.h>)
            #include <freertos/task.h>
        #endif
    #endif
#endif

static inline unsigned long edge_millis() {
#if defined(esp_timer_get_time)
        return (unsigned long)(esp_timer_get_time() / 1000ULL);
#else
        return (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS);
#endif
}
