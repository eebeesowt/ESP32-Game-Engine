#pragma once
#include <cstdint>
#include <cstddef>
#include "HardwareConfig.h"

class AudioDriver {
public:
    AudioDriver();
    ~AudioDriver();

    bool init();
    
    // Play raw PCM data
    void playSample(const int16_t* data, size_t length);
    
    // Record raw PCM data
    size_t recordSample(int16_t* buffer, size_t length);
    
    void setVolume(uint8_t volume); // 0-100

private:
    bool _initialized;
};
