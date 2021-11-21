/* =====================
   This software is licensed under the MIT License:
   https://github.com/spacehuhntech/esp8266_deauther
   ===================== */


#include "src/ArduinoJson-v5.13.5/ArduinoJson.h"
#if ARDUINOJSON_VERSION_MAJOR != 5
// The software was build using ArduinoJson v5.x
// version 6 is still in beta at the time of writing
// go to tools -> manage libraries, search for ArduinoJSON and install version 5
#error Please upgrade/downgrade ArduinoJSON library to version 5!
#endif // if ARDUINOJSON_VERSION_MAJOR != 5

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

#include "DisplayUI.h"
#include "A_config.h"

#include "led.h"
#include "defines.h"

#include <map>

#define CHANNEL 1

simplebutton::Button* resetButton;
DisplayUI displayUI;


uint32_t autosaveTime = 0;

bool booted = false;

String AppBuffer[5];
String MachineBuffer[5];

std::map<uint32_t, std::map<uint32_t, uint8_t> > mapOfRssi;

const char* ssid = "lwsc_wifibridge_display";
const char* password = "lauterbach";
WiFiUDP wifiUdp;
unsigned int udpPort = 5555;

byte msgCount = 0;            // count of outgoing messages
int localAddress = 0x00;      // address of this device
long lastSendTime = 0;        // last send time

char packetBuffer[255]; //buffer to hold incoming packet


struct sniffer_buf2 *sniffer;

uint16_t seqnum = 0x000;
uint8_t rssi;

static inline uint32_t intDisable()
{
    return xt_rsil(15);
    
}
static inline void intEnable(uint32_t state)
{
    xt_wsr_ps(state);
}

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
  createPacket(result, data, 2, dest, MSG_Data);
  int res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  //delay(20);
  //res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("fired to %08x cmd %d type %d = %d\n\n", dest, cmd, type, res);  
  led::setColor(0,0,100);
  delay(70);
  led::setColor(0,100,0);
}

void reqRssi(uint32_t dest)
{   
  uint8_t result[sizeof(beacon_raw) + 1];  
  uint8_t data[1] = {0}; 
  createPacket(result, data, 1, 0xFFFFFFFF, MSG_RequestRssi);
  int res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  led::setColor(100,100,100);
  delay(70);
  led::setColor(0,100,0);
}

byte cbCounter;
uint16_t lastSeq;
uint32_t lastSrc;
void ICACHE_RAM_ATTR promisc_cb(uint8_t *buf, uint16_t len)
{
  uint32_t old_ints = intDisable();
  if (len == 128 && buf[12+4] == 0xef && buf[12] == 0x80){
    sniffer = (struct sniffer_buf2*) buf;
    rssi = buf[0];
    if (sniffer->buf[0] == 0x80 /*beacon*/&& sniffer->buf[37] == 0x00 /*hidden ssid*/&& sniffer->buf[38] == 0xDD /*vendor info*/&& sniffer->buf[4] == 0xef /*magic word1*/&& sniffer->buf[5] == 0x50/*magic word2*/)
    {
      msgData msg;
      msg.dst = (sniffer->buf[6]  << 24) | (sniffer->buf[7]  << 16) | (sniffer->buf[8]  << 8) | sniffer->buf[9];
      msg.src = (sniffer->buf[12] << 24) | (sniffer->buf[13] << 16) | (sniffer->buf[14] << 8) | sniffer->buf[15];
      msg.trs = (sniffer->buf[18] << 24) | (sniffer->buf[19] << 16) | (sniffer->buf[20] << 8) | sniffer->buf[21];
      msg.seq = (sniffer->buf[22] << 8) | sniffer->buf[23];
      msg.ver = sniffer->buf[42];
      msg.ttl = sniffer->buf[43];
      msg.type = sniffer->buf[44];
      msg.dataLength = (sniffer->buf[39])-5;
      memcpy(msg.data, &(sniffer->buf[45]), sniffer->buf[39]-5);

      if(msg.seq == lastSeq && msg.src == lastSrc)
      {
      }
      else
      {
        lastSeq = msg.seq;
        lastSrc = msg.src;
        char msg_dst[50] = {0};
        sprintf(msg_dst,"%02X %04X %02X%02X%02X%02X %02X %d",cbCounter,msg.seq,sniffer->buf[12],sniffer->buf[13],sniffer->buf[14],sniffer->buf[15], msg.type, sniffer->rx_ctrl.rssi);
  
        
        MachineBuffer[4] = MachineBuffer[3];
        MachineBuffer[3] = MachineBuffer[2];
        MachineBuffer[2] = MachineBuffer[1];
        MachineBuffer[1] = MachineBuffer[0];
        MachineBuffer[0] = String(msg_dst);
        cbCounter++;

        if(cbCounter%2==0)
          led::setColor(100,30,0);
        else
          led::setColor(100,0,30);

        if(msg.type == MSG_SendRssi)
        {  
          //Serial.println(String(msg.src, HEX));
          for(int i = 1; i < msg.dataLength; i+=5)
          {
            uint32_t id_tmp;
            uint8_t rssi_tmp;
            memcpy((uint8_t*)&id_tmp, (uint8_t*)&msg.data + i, 4);
            memcpy((uint8_t*)&rssi_tmp, (uint8_t*)&msg.data + i + 4, 1);
            Serial.println("_" + String(id_tmp, HEX) + ":" + String(rssi_tmp));
            mapOfRssi[msg.src][id_tmp] = rssi_tmp;
          }
        }
      }
    }
  }
  intEnable(old_ints);
}

bool cbIsSet = false;
void setupFreedom()
{
  led::setColor(100,0,0);
  Serial.println("Setting up Freedom Mode");
  WiFi.mode(WIFI_STA); 
  wifi_set_channel(CHANNEL);
  wifi_set_phy_mode(PHY_MODE_11B);
  if(!cbIsSet)
  {
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(promisc_cb);
    cbIsSet = true;
  }
  wifi_promiscuous_enable(1);
}

void setupAP()
{
  led::setColor(0,100,0);
  Serial.println("Setting up AP Mode");
  
  wifi_promiscuous_enable(0);
  wifi_set_phy_mode(PHY_MODE_11B);
  wifi_set_channel(CHANNEL);
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password, CHANNEL);
}

void setup() {
  // start serial
  Serial.begin(115200);
  Serial.println();

  // start display
  displayUI.setup();
  displayUI.mode = DISPLAY_MODE::MENU;

  // setup LED
  led::setup();
  led::setColor(0,100,0);

  // setup reset button
  resetButton = new ButtonPullup(RESET_BUTTON);
  
  pinMode(2, OUTPUT);

  
  wifi_set_phy_mode(PHY_MODE_11B);
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);
  wifiUdp.begin(udpPort);  

  localAddress = ESP.getChipId();

  seqnum = random(3000);
}

void loop() {

  led::update();   // update LED color
  displayUI.update();


  if (!booted) {
    booted = true;
  }

  
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

    AppBuffer[4] = AppBuffer[3];
    AppBuffer[3] = AppBuffer[2];
    AppBuffer[2] = AppBuffer[1];
    AppBuffer[1] = AppBuffer[0];
    char msg_dst[50] = {0};
    sprintf(msg_dst,"%04X %02X%02X%02X%02X %02X:%02X",seqnum,packetBuffer[1],packetBuffer[2],packetBuffer[3],packetBuffer[4],packetBuffer[0],packetBuffer[5]);
    AppBuffer[0] = String(msg_dst);
  } else if (cb) {    
    Serial.printf("[wifi] length = %i\n", cb);
  }
  delay(1);
}
