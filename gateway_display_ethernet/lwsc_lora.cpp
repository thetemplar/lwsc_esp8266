#include "lwsc_lora.h"
#include "Arduino.h"
#include <LoRa.h>
extern void udpMsg(String msg);

void lora_ping(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] blink to %08x = %d\n", dest, res);
}

void lora_reboot(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] reboot to %08x = %d\n", dest, res);
}

void lora_blink(uint32_t dest)
{
  LoRa.beginPacket();
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  LoRa.print(loraDest);
  LoRa.print(0x03);
  LoRa.endPacket();
  int res = 0;
  Serial.printf("[LoRa] blink to %08x = %d\n", dest, res);
  udpMsg("[LoRa] blink to " + String(loraDest));
}

uint16_t lora_fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask)
{
  LoRa.beginPacket();
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  LoRa.print(loraDest);
  LoRa.print(0x01);
  LoRa.print(relaisBitmask);
  LoRa.print((uint8_t) ((duration) & 0xFF));
  LoRa.print((uint8_t) ((duration <<  8) & 0xFF));
  LoRa.print((uint8_t) ((duration << 16) & 0xFF));
  LoRa.print((uint8_t) ((duration << 24) & 0xFF));
  LoRa.endPacket();
  int res = 0;
  Serial.printf("[LoRa] fired to %08x duration %d relaisBitmask %02X = %d\n", dest, duration, relaisBitmask, res);
  udpMsg("[LoRa] fired to " + String(dest));
  return 0;
}

void lora_reqRssi(uint32_t dest)
{
  int res = 0;
  Serial.printf("[LoRa] reqRssi to %08x = %d\n\n", dest, res);
}

void lora_processData()
{
  
}

void lora_setup()
{
  LoRa.setPins(2, -1, 15);

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  LoRa.setSyncWord(0xF3);


  LoRa.setSpreadingFactor(10);
}
