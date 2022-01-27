#include "lwsc_lora.h"

void ping(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] blink to %08x = %d\n", dest, res);
}

void reboot(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] reboot to %08x = %d\n", dest, res);
}

void blink(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] blink to %08x = %d\n", dest, res);
}

uint16_t fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask)
{
  int res = 0;
  Serial.printf("[LoRa] fired to %08x duration %d relaisBitmask %02X = %d\n", dest, duration, relaisBitmask, res);
  return 0;
}

void reqRssi(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] reqRssi to %08x = %d\n\n", dest, res);
}

void processLoRaData()
{
  
}

void lora_setup()
{
  
}
