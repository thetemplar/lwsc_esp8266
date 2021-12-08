#pragma once

#include <cstdint>

namespace led {
    void setup();
    void update();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
}
