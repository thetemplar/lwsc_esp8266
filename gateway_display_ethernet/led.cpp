/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#include "led.h"

#include "A_config.h" // Config for LEDs
#include <Arduino.h>  // digitalWrite, analogWrite, pinMode

#include "src/Adafruit_NeoPixel-1.7.0/Adafruit_NeoPixel.h"


namespace led {
    Adafruit_NeoPixel strip { LED_NUM, LED_NEOPIXEL_PIN, NEO_RGB + NEO_KHZ800 };

    void setColor(uint8_t r, uint8_t g, uint8_t b) {
        for (size_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, r, g, b);
        }
        strip.show();
    }


    void setup() {
        analogWriteRange(0xff);
        strip.begin();
        strip.setBrightness(LED_MODE_BRIGHTNESS);
        strip.show();
    }

    void update() {
    }
}
