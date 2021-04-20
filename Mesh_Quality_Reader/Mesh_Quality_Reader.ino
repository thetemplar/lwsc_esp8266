#include <Wire.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
 

#define VERSION 0x02
//uint8_t VERSION;
#define START_TTL 0x05
#define MSG_TYPE 0x00

#include <UIPEthernet.h>
#include <map>
#include "defines.h"


char packetBuffer[255]; //buffer to hold incoming packet
uint8_t rssi;
bool inProcess = false;
uint16_t ledState;
struct sniffer_buf2 *sniffer;
uint16_t seqnum = 0x000;

std::map<uint32_t,uint32_t> lastRssi;

static inline uint32_t intDisable()
{
    return xt_rsil(15);
    
}
static inline void intEnable(uint32_t state)
{
    xt_wsr_ps(state);
}

void processData(struct sniffer_buf2 *sniffer)
{
  if(sniffer->buf[4] != 0xef || sniffer->buf[5] != 0x50) return;
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
  
  Serial.printf("Data (dst: %02x:%02x:%02x:%02x:%02x:%02x (%08x), src: %02x:%02x:%02x:%02x:%02x:%02x (%08x), rssi: %d, ttl: %d, type: %02x, seq: %d: ", sniffer->buf[4], sniffer->buf[5], sniffer->buf[6], sniffer->buf[7], sniffer->buf[8], sniffer->buf[9], msg.dst, sniffer->buf[10], sniffer->buf[11], sniffer->buf[12], sniffer->buf[13], sniffer->buf[14], sniffer->buf[15], msg.src, rssi, msg.ttl, msg.type, msg.seq);
  for(uint16_t i = 0; i < msg.dataLength; i++)
    Serial.printf("%02x ", msg.data[i]);
  Serial.printf("\n"); 

  uint32_t newRssi = millis() >> 8;
  if(lastRssi.count(msg.src) > 0)
    rssi = ((lastRssi[msg.src] >> 24) * 3 + sniffer->rx_ctrl.rssi) / 4;

  newRssi += rssi << 24;
  lastRssi[msg.src] = newRssi; //1byte rssi, 3byte timestamp
  for(std::map<uint32_t,uint32_t>::const_iterator it = lastRssi.begin(); it != lastRssi.end(); ++it)
  {
    printf("\t%08x %i %i\n", it->first, ((millis()-((it->second & 0x00FFFFFF)<<8))/1000), (it->second >> 24));
  }
  
  rssi = 0;
}

void ICACHE_RAM_ATTR promisc_cb(uint8_t *buf, uint16_t len)
{
  uint32_t old_ints = intDisable();
  if (len == 128 && buf[12+4] == 0xef && buf[12] == 0x80){
    if (!inProcess ){
      inProcess = true;  
      sniffer = (struct sniffer_buf2*) buf;
      rssi = buf[0];
      if (sniffer->buf[0] == 0x80 /*beacon*/&& sniffer->buf[37] == 0x00 /*hidden ssid*/&& sniffer->buf[38] == 0xDD /*vendor info*/&& sniffer->buf[4] == 0xef /*magic word1*/&& sniffer->buf[5] == 0x50/*magic word2*/)
      {
        //dont process data here in interrupt  
      }
      else
      {
        inProcess = false; 
      }
    }
  }
  intEnable(old_ints);
}

void setup() { 
  Serial.begin(115200);
  Serial.setTimeout(100);
  pinMode(2, OUTPUT);

  WiFi.mode(WIFI_STA); 
  wifi_set_channel(1);
  wifi_set_phy_mode(PHY_MODE_11B);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
}

unsigned long next;
void loop() {
  if(inProcess)
  {
    processData(sniffer);
    inProcess = false;
  }
}


