#include "lwsc_lora.h"
#include "Arduino.h"
#include <LoRa.h>
#include <ESP8266WebServer.h>

extern void udpMsg(String msg);

void lora_ping(uint32_t dest)
{
  udpMsg("[LoRa] blink (as 0/0-fire) to " + String(dest) + ":");
  lora_fire(dest, 0, 0);
}

void lora_get_version(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  LoRa.beginPacket();
  LoRa.write(loraDest);
  LoRa.write(MSG_Version);
  LoRa.endPacket();
  udpMsg("[LoRa] lora_getVersion to " + String(loraDest));
}

void lora_reboot(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  LoRa.beginPacket();
  LoRa.write(loraDest);
  LoRa.write(MSG_Reboot);
  LoRa.endPacket();
  udpMsg("[LoRa] reboot to " + String(loraDest));
}

void lora_blink(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  LoRa.beginPacket();
  LoRa.write(loraDest);
  LoRa.write(MSG_Blink);
  LoRa.endPacket();
  int res = 0;
  udpMsg("[LoRa] blink to " + String(loraDest));
}

uint16_t lora_fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  if(duration > 255*20-2)
  { 
    udpMsg("[LoRa] NOT fired to " + String(dest) + " duration: " + String(duration) + ": DURATION TOO LONG.");
    return 0;
  }
  uint8_t durationShort = 0;
  if(duration >= 0)
  {
    durationShort = duration / 20 + 1;
  }
  LoRa.beginPacket();
  LoRa.write(loraDest);
  LoRa.write(MSG_Lora_Fire_Base + relaisBitmask);
  LoRa.write(durationShort);
  LoRa.endPacket();
  int res = 0;
  udpMsg("[LoRa] fired to " + String(dest) + " duration: " + String(duration) + " (" + String(durationShort) + ") bitmask: " + String(relaisBitmask));
  return 0;
}

void lora_reqRssi(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  int res = 0;
  //udpMsg("[LoRa] reqRssi to %08x = %d\n\n", dest, res);
}

uint8_t buf[128];
extern uint64_t ackStart;
extern uint32_t ackTimeout;
extern ESP8266WebServer server;
void lora_processData()
{

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //read packet
    memset(buf, 0, sizeof(buf));
    int i = 0;
    while (LoRa.available()) {
      buf[i] = LoRa.read();
      i++;
    }

    if (buf[0] == MSG_Ack)
    {
      int16_t machineRssi = buf[2] + (buf[3] << 8);
      int8_t machineSnr = buf[4];
      udpMsg("[LoRa] processLoRaData: Got Msg_ACK from " + String(buf[1]) + " at Rssi(Gateway->Machine): " + String(machineRssi) + " at LocalRssi(Machine->Gateway): " + String(LoRa.packetRssi()) + " at SNR(Gateway->Machine): " + String(machineSnr) + " at SNR(Machine->Gateway): " + String(LoRa.packetSnr()));
         
      if (ackStart > 0 && millis() < ackStart + ackTimeout)
      {
        uint32_t diff = millis() - ackStart;
        server.send(200, "text/json", "{\"result\": \"success\", \"roundtriptime\": \"" + String (diff) + "\", \"type\": \"lora\", \"rssi\": \"" + String(machineRssi) + "\", \"snr\": \"" + String(machineSnr) + "\", \"reply_rssi\": \"" + String(LoRa.packetRssi()) + "\", \"reply_snr\": \"" + String(LoRa.packetSnr()) + "\"}");
        ackStart = 0;
      }
    } else if (buf[0] == MSG_KeepAlive) {
      udpMsg("[LoRa] processLoRaData: keep-alive by " + String(buf[1]) + " - Rssi: " + String(LoRa.packetRssi()) + " at SNR: " + String(LoRa.packetSnr()));
    } else {
      udpMsg("[LoRa] processLoRaData: unknown payload: '" + String((char*)&buf[0]) + "'");
    }
  }
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
  LoRa.enableCrc();
}
