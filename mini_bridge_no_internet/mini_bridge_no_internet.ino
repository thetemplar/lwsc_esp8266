#include <Wire.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}


byte msgCount = 0;            // count of outgoing messages
int localAddress = 0x00;      // address of this device
long lastSendTime = 0;        // last send time

bool LoRaEnabled = false;

const char* ssid = "lwsc_wifibridge_simple";
const char* password = "lauterbach";
WiFiUDP wifiUdp;
unsigned int udpPort = 5555;

char packetBuffer[255]; //buffer to hold incoming packet


uint8_t beacon_raw[] = {
  0x80, 0x00,             // 0-1: Frame Control
  0x00, 0x00,             // 2-3: Duration
  0xef, 0x50, 0xff, 0xff, 0xff, 0xff,       // 4-9: Destination address (broadcast)
  0xef, 0x50, 0x00, 0x00, 0x00, 0x00,       // 10-15: Source address
  0xef, 0x50, 0x00, 0x00, 0x00, 0x00,       // 16-21: BSSID (transmitter)
  0x00, 0x00,             // 22-23: Sequence / fragment number
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,     // 24-31: Timestamp (GETS OVERWRITTEN TO 0 BY HARDWARE)
  0x64, 0x00,             // 32-33: Beacon interval
  0x31, 0x04,             // 34-35: Capability info
  0x00, 0x00,             // 36-37: SSID (hidden)
  0xDD, 0x05, 0x07, 0xFA, 0x00, 0x05, 0x00 //38-44++
};

int seqnum = 123;

uint16_t createPacket(uint8_t* result, uint8_t *buf, uint16_t len, uint32_t dst, uint8_t type)
{
  memcpy(&result[0], &beacon_raw[0], sizeof(beacon_raw));
  memcpy(&result[sizeof(beacon_raw)], &buf[0], len);

  //dst
  result[4 + 2] = (dst >> 24) & 0xFF;
  result[4 + 3] = (dst >> 16) & 0xFF;
  result[4 + 4] = (dst >> 8) & 0xFF;
  result[4 + 5] = (dst) & 0xFF;

  //src
  result[10 + 2] = (ESP.getChipId() >> 24) & 0xFF;
  result[10 + 3] = (ESP.getChipId() >> 16) & 0xFF;
  result[10 + 4] = (ESP.getChipId() >> 8) & 0xFF;
  result[10 + 5] = (ESP.getChipId()) & 0xFF;

  //transmitc
  result[16 + 2] = (ESP.getChipId() >> 24) & 0xFF;
  result[16 + 3] = (ESP.getChipId() >> 16) & 0xFF;
  result[16 + 4] = (ESP.getChipId() >> 8) & 0xFF;
  result[16 + 5] = (ESP.getChipId()) & 0xFF;

  result[22] = (seqnum >> 8) & 0xFF;
  result[23] = (seqnum) & 0xFF;

  uint16_t seqTmp = seqnum;
  seqnum++;
  if (seqnum > 0xfff)
    seqnum = 1;

  result[39] += len;
  result[44] = type;

  return seqTmp;
}

void fire(uint8_t type, uint32_t dest, uint8_t cmd)
{   
  uint8_t result[sizeof(beacon_raw) + 2];   
  uint8_t data[2] = {type, cmd};
  createPacket(result, data, 2, dest, 0x01);
  int res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("fired to %08x cmd %d type %d = %d\n\n", dest, cmd, type, res);  
}


void setup() { 
  Serial.begin(115200);
  Serial.println("");
  pinMode(2, OUTPUT);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);
  
  wifiUdp.begin(udpPort);  

  localAddress = ESP.getChipId();
}

unsigned long next;
void loop() {
  int cb = wifiUdp.parsePacket();
  if (cb == 6) {    
    Serial.println("[wifi] package: " + String(packetBuffer[0], HEX) + " " + String(packetBuffer[1], HEX) + " " + String(packetBuffer[2], HEX) + " " + String(packetBuffer[3], HEX) + " " + String(packetBuffer[4], HEX) + " " + String(packetBuffer[5], HEX));
    wifiUdp.read(packetBuffer, cb);
    uint32_t dest = 0;
    dest += packetBuffer[4];
    dest += packetBuffer[3]<<8;
    dest += packetBuffer[2]<<16;
    dest += packetBuffer[1]<<24;
    fire(packetBuffer[0], dest, packetBuffer[5]);
    delay(50);
    fire(packetBuffer[0], dest, packetBuffer[5]);
  } else if (cb)
  {    
    Serial.printf("[wifi] length = %i\n", cb);
  }
  delay(1);
}
