#pragma once
#include "HardwareConfig.h"
#include "esp_err.h"

class SDCardDriver {
public:
    SDCardDriver();
    ~SDCardDriver();

    bool init();
    bool isMounted() const { return _isMounted; }
    void unmount();
    
    // Helper to print card info
    void printCardInfo();

private:
    bool _isMounted;
    const char* _mountPoint;
};
