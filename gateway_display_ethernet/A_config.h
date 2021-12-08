#pragma once

#define ETH_ENABLE

// ===== LED ===== //
#define LED_NEOPIXEL_GRB
#define LED_NUM 1
#define LED_NEOPIXEL_PIN 15

#define HIGHLIGHT_LED 16

// ===== DISPLAY ===== //
#define SH1106_I2C
#define FLIP_DIPLAY true
#define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
#define BUTTON_UP 12
#define BUTTON_DOWN 13
#define BUTTON_A 14

#define LED_MODE_BRIGHTNESS 10

#define I2C_ADDR 0x3C
#define I2C_SDA 5
#define I2C_SCL 4
