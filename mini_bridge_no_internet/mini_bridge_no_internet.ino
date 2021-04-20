#include <Wire.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}

#include <SPI.h>              // include libraries
#include <LoRa.h>
const int csPin = 2;          // LoRa radio chip select
const int resetPin = 16;       // LoRa radio reset
const int irqPin = 5;         // change for your board; must be a hardware interrupt pin


#include <UIPEthernet.h>

byte msgCount = 0;            // count of outgoing messages
int localAddress = 0x00;      // address of this device
long lastSendTime = 0;        // last send time

bool LoRaEnabled = false;

const char* ssid = "lwsc__wifibringe_black_3";
const char* password = "lauterbach";
WiFiUDP wifiUdp;
EthernetUDP ethUdp;
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
  
  if(LoRaEnabled)
  {
    String message = String(type, HEX) + String(cmd, HEX);   // send a message
    sendMessage(message, dest);
  }
}


void setup() { 
  Serial.begin(115200);
  Serial.println("");
  pinMode(2, OUTPUT);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);
  
  wifiUdp.begin(udpPort);  

  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  
  if (!LoRa.begin(868E6)) {             // initialize ratio at 868 MHz
    Serial.println("LoRa init failed. Check your connections."); 
  } else {
    LoRa.onReceive(onReceive);
    LoRa.receive();
    LoRaEnabled = true;
    Serial.println("LoRa init succeeded.");  
  }
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


void sendMessage(String outgoing, uint32_t destination) {
  byte len = (byte)outgoing.length();
  byte crc = destination ^ localAddress ^ msgCount ^ len;
  for(int i = 0; i < len; i++)
    crc ^= outgoing[i];

  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(destination >> 8);         // add destination address
  LoRa.write(destination >> 16);        // add destination address
  LoRa.write(destination >> 24);        // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(localAddress >> 8);        // add sender address
  LoRa.write(localAddress >> 16);       // add sender address
  LoRa.write(localAddress >> 24);       // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(crc);                      // add payload
  LoRa.write(len);                      // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it

  Serial.println(" > Sending '" + outgoing + "' to: 0x" + String(destination, HEX));
  Serial.println();
  
  LoRa.receive();
  
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return


  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  recipient += LoRa.read() << 8;        // recipient address
  recipient += LoRa.read() << 16;       // recipient address
  recipient += LoRa.read() << 24;       // recipient address
  int sender = LoRa.read();             // sender address
  sender += LoRa.read() << 8;           // sender address
  sender += LoRa.read() << 16;          // sender address
  sender += LoRa.read() << 24;          // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte crc = LoRa.read();               // incoming msg length
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";
  
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  byte crcCalc = recipient ^ sender ^ incomingMsgId ^ incomingLength;
  for(int i = 0; i < incomingLength; i++)
    crcCalc ^= incoming[i];


  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }
  
  if (crcCalc != crc) {   // check length for error
    Serial.println("error: crc does not match");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFFFFFFFF) {
    Serial.println("This message is not for me, but for " + String(recipient, HEX) + ".");
    return;                             // skip rest of function
  }
  
  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("CRC: " + String(crc));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}


