#include "lwsc_lora.h"
#include "Arduino.h"
#include <ESP8266WebServer.h>

extern void udpMsg(String msg);
extern uint64_t ackStart;
extern uint32_t ackTimeout;
extern ESP8266WebServer server;
extern MachineData machines[256];

uint8_t buf[128];

uint32_t fireCounter = 0;

void lora_ping(uint32_t dest)
{
  udpMsg("[LoRa] blink (as 0/0-fire) to " + String(dest) + ":");
}

void lora_get_version(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  udpMsg("[LoRa] lora_getVersion to " + String(loraDest));
}

void lora_reboot(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  udpMsg("[LoRa] reboot to " + String(loraDest));
}

void lora_blink(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  udpMsg("[LoRa] blink to " + String(loraDest));}

uint16_t lora_fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask)
{
  fireCounter++;
  uint8_t durationShort = 0;
  if(duration >= 0)
  {
    durationShort = duration / 20 + 1;
  }
  

  int res = 0;
  udpMsg("[LoRa] fired via lora to " + String(dest));
  return 0;
}

void lora_processData()
{
}

void lora_setup()
{
}
