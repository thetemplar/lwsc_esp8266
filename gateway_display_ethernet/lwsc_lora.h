#ifndef LWSC_LORA_
#define LWSC_LORA_

#pragma once


#include "defines.h"
#include "Arduino.h"

void lora_get_version(uint32_t dest);
void lora_ping(uint32_t dest);
void lora_reboot(uint32_t dest);
void lora_blink(uint32_t dest);
uint16_t lora_fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask);
void lora_processData();
void lora_setup();

#endif
