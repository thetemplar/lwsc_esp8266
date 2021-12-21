#ifndef LWSC_WIFI_
#define LWSC_WIFI_

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

typedef void (*freedom_outside_cb_t)(uint8_t status);
int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
void wifi_unregister_send_pkt_freedom_cb(void);
int wifi_send_pkt_freedom(uint8_t *buf, int len, bool sys_seq); 

void ICACHE_RAM_ATTR promisc_cb(uint8_t *buf, uint16_t len);

uint16_t createPacket(uint8_t* result, uint8_t *buf, uint16_t len, uint32_t dst, uint8_t type);

uint16_t fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask);
void ping(uint32_t dest);
void blink(uint32_t dest);
void reboot(uint32_t dest);
void reqRssi(uint32_t dest);

void setupFreedom();
void setupAP();
void processData();

void readWifi(char* buf, uint8_t len);

void wifi_setup();

#endif
