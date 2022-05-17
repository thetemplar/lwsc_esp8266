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

  if(dest == 1 || dest == 2 || dest == 3) { 
    uint8_t mask = 0x0F;
    if(dest == 1 && relaisBitmask == 1) mask = 0b11111110;
    if(dest == 1 && relaisBitmask == 2) mask = 0b11111101;
    if(dest == 1 && relaisBitmask == 3) mask = 0b11111100;
    
    if(dest == 2 && relaisBitmask == 1) mask = 0b11111011;
    if(dest == 2 && relaisBitmask == 2) mask = 0b11110111;
    if(dest == 2 && relaisBitmask == 3) mask = 0b11110011;
    
    if(dest == 3 && relaisBitmask == 1) mask = 0b11101111;
    if(dest == 3 && relaisBitmask == 2) mask = 0b11011111;
    if(dest == 3 && relaisBitmask == 3) mask = 0b11001111;
    
    Wire.beginTransmission(0x20);
    if (Wire.write(mask) != 1) { Serial.println("err1"); }
    if (Wire.endTransmission(true) != (uint8_t) 0) { Serial.println("err2"); }
    server.send(200, "text/json", "{\"result\": \"success\", \"mask\": \"" + String(mask) + " / " + String(relaisBitmask) + "\", \"roundtriptime\": \"0\", \"type\": \"direct\", \"rssi\": \"0\", \"snr\": \"0\", \"reply_rssi\": \"0\", \"reply_snr\": \"0\"}");
    udpMsg("[LoRa] fired locally to " + String(dest) + " duration: " + String(duration) + " (" + String(durationShort) + ") bitmask: " + String(relaisBitmask));
            
    delay(duration);
    Wire.beginTransmission(0x20);
    if (Wire.write(0xFF) != 1) { Serial.println("err1"); }
    if (Wire.endTransmission(true) != (uint8_t) 0) { Serial.println("err2"); }
    
    return 0;
  }
  
  LoRa.beginPacket();
  LoRa.write(loraDest);
  LoRa.write(MSG_Lora_Fire_Base + relaisBitmask);
  LoRa.write(durationShort);
  LoRa.endPacket();
  int res = 0;
  udpMsg("[LoRa] fired via lora to " + String(dest) + " duration: " + String(duration) + " (" + String(durationShort) + ") bitmask: " + String(relaisBitmask));
  return 0;
}

void lora_processData()
{
}

void lora_setup()
{
}
