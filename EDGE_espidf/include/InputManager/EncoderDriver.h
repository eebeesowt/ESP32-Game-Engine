#pragma once

/*
 * Simple encoder/button hardware adapter template for ESP-IDF.
 *
 * Purpose:
 * - Demonstrate mapping of GPIO interrupts to engine InputManager calls.
 * - ISR pushes lightweight events to a FreeRTOS queue; a background task
 *   debounces/handles events and calls InputManager methods.
 *
 * Notes:
 * - This is a template: adapt pin numbers, debounce logic and encoder state
 *   machine to your hardware.
 * - The adapter does not implement a full-featured rotary-encoder decoder,
 *   it shows structure and safe ISR → task handoff.
 */

#include "InputManager/InputManager.h"
#include <cstdint>

extern volatile int g_isr_count;
extern volatile int g_task_count;

class EncoderDriver {
public:
    // Create adapter bound to an InputManager instance (not owned).
    explicit EncoderDriver(InputManager* im);
    ~EncoderDriver();

    // Initialize a single encoder with pins and an id used when calling InputManager.
    // encoderId is an application-defined id (0..N).
    bool initEncoder(int encoderId, int pinA, int pinB, int buttonPin = -1);

    // Start the driver task (creates FreeRTOS task & ISR handlers).
    bool start();

    // Stop and cleanup (removes ISRs and deletes task/queue).
    void stop();

private:
    InputManager* _inputManager;

    // non-copyable
    EncoderDriver(const EncoderDriver&) = delete;
    EncoderDriver& operator=(const EncoderDriver&) = delete;
};
