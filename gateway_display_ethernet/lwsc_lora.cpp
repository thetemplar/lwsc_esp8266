#include "lwsc_lora.h"
#include "Arduino.h"

#include <Wire.h>
#include <LoRa.h>
#include <ESP8266WebServer.h>

extern void udpMsg(String msg);
extern uint64_t ackStart;
extern uint32_t ackTimeout;
extern ESP8266WebServer server;
extern MachineData machines[256];

extern uint64_t warning_remaining;
extern String warning_msg;

uint8_t buf[128];

uint32_t fireCounter = 0;

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
  fireCounter++;
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

    if(ackStart > 0)
    {
      server.send(200, "text/json", "{\"result\": \"success\", \"mask\": \"" + String(mask) + " / " + String(relaisBitmask) + "\", \"roundtriptime\": \"0\", \"type\": \"direct\", \"rssi\": \"0\", \"snr\": \"0\", \"reply_rssi\": \"0\", \"reply_snr\": \"0\"}");
    }
    udpMsg("[LoRa] fired via gpio to " + String(dest));
            
    delay(duration);
    Wire.beginTransmission(0x20);
    if (Wire.write(0xFF) != 1) { Serial.println("err1"); }
    if (Wire.endTransmission(true) != (uint8_t) 0) { Serial.println("err2"); }
    ackStart = 0;
    return 0;
  }
  
  LoRa.beginPacket();
  LoRa.write(loraDest);
  LoRa.write(MSG_Lora_Fire_Base + relaisBitmask);
  LoRa.write(durationShort);
  LoRa.endPacket();
  int res = 0;
  udpMsg("[LoRa] fired via lora to " + String(dest) + " / " + String(relaisBitmask));
  return 0;
}

void lora_reqRssi(uint32_t dest)
{
  uint8_t loraDest = (uint8_t)dest & 0xFF;
  int res = 0;
  //udpMsg("[LoRa] reqRssi to %08x = %d\n\n", dest, res);
}

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

    if (buf[1] == MSG_Ack)
    {
      uint8_t machineId = buf[2];
      
      int16_t machineRssi = buf[3] + (buf[4] << 8);
      int8_t machineSnr = buf[5];
      
      int16_t ownRssi = LoRa.packetRssi();
      int8_t ownSnr = LoRa.packetSnr();
      
      machines[machineId].MachineRssi = machineRssi;
      machines[machineId].MachineSnr = machineSnr;
      machines[machineId].Rssi = ownRssi;
      machines[machineId].Snr = ownSnr;
      machines[machineId].LastSeen = millis();
      
      udpMsg("[LoRa] process: Msg_ACK from " + String(machineId) + " at Rssi: " + String(machineRssi) + " / " + String(ownRssi) + " at SNR: " + String(machineSnr) + " / " + String(ownSnr));
      
      if (ackStart > 0 && millis() < ackStart + ackTimeout)
      {
        uint32_t diff = millis() - ackStart;
        server.send(200, "text/json", "{\"result\": \"success\", \"roundtriptime\": \"" + String (diff) + "\", \"type\": \"lora\", \"rssi\": \"" + String(machineRssi) + "\", \"snr\": \"" + String(machineSnr) + "\", \"reply_rssi\": \"" + String(LoRa.packetRssi()) + "\", \"reply_snr\": \"" + String(LoRa.packetSnr()) + "\"}");
        ackStart = 0;
      }
    } else if (buf[1] == MSG_KeepAlive) {
      uint8_t machineId = buf[2];
      
      int16_t machineRssi = 0;
      int8_t machineSnr = 0;
      
      int16_t ownRssi = LoRa.packetRssi();
      int8_t ownSnr = LoRa.packetSnr();
      
      machines[machineId].MachineRssi = machineRssi;
      machines[machineId].MachineSnr = machineSnr;
      machines[machineId].Rssi = ownRssi;
      machines[machineId].Snr = ownSnr;
      machines[machineId].LastSeen = millis();
      
      udpMsg("[LoRa] process: keep-alive by " + String(machineId) + " - Rssi: " + String(LoRa.packetRssi()) + " at SNR: " + String(LoRa.packetSnr()));
    } else if (buf[1] == MSG_RSSI_Ping) {    
      delay(250);  
      int16_t ownRssi = LoRa.packetRssi();
      uint8_t aa1 = ownRssi;
      uint8_t aa2 = (ownRssi >> 8);
      LoRa.beginPacket();
      LoRa.write(0xFF);
      LoRa.write(MSG_RSSI_Ping_Reply);
      LoRa.write(aa1);
      LoRa.write(aa2);
      LoRa.endPacket();
      udpMsg("[LoRa] process: MSG_RSSI_Ping (" + String(ownRssi) + ")");
    } else if (buf[1] == MSG_Version_Reply) {    
      udpMsg("[LoRa] process: MSG_Version_Reply: '" + String((char*)&buf[2]) + "'");
    } else if (buf[1] == MSG_Warning_Safety) {    
      udpMsg("[LoRa] process: MSG_Warning_Safety!");
      warning_remaining = millis() + 30 * 1000;
      warning_msg = "Sicherheit!";
    } else {
      udpMsg("[LoRa] process: unknown payload: [0]='" + String((char*)&buf[1]) + "' [1]='" + String((char*)&buf[1]) + "'");
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
