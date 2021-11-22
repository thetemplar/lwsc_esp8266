#pragma once

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

#include <map>
#include "defines.h"
#include "led.h"
  
static inline uint32_t intDisable()
{
    return xt_rsil(15);
    
}
static inline void intEnable(uint32_t state)
{
    xt_wsr_ps(state);
}

void ICACHE_RAM_ATTR promisc_cb(uint8_t *buf, uint16_t len);

uint16_t createPacket(uint8_t* result, uint8_t *buf, uint16_t len, uint32_t dst, uint8_t type);

void fire(uint8_t type, uint32_t dest, uint8_t cmd);
void reqRssi(uint32_t dest);

void setupFreedom();
void setupAP();

void readWifi(char* buf, uint8_t len);

void wifi_setup();
