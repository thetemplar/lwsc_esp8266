#include "lwsc_wifi.h"

#define CHANNEL 1

extern const char* ssid;
extern const char* password;

int localAddress = 0x00;      // address of this device
struct sniffer_buf2 *sniffer;

uint16_t seqnum = 0x000;
uint8_t rssi;

uint16_t lastSeq;
uint32_t lastSrc;

struct WifiLog AppBuffer[255];
uint8_t AppBufferIndex;
struct WifiLog MachineBuffer[255];
uint8_t MachineBufferIndex;

std::vector<MachineData> machines;
std::map<uint32_t, uint8_t> machinesIndexCache;


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
  0xDD, 0x05, 0x07, 0xFA, VERSION, START_TTL, MSG_Unknown //38-44++
};

void /*ICACHE_RAM_ATTR*/ promisc_cb(uint8_t *buf, uint16_t len)
{
  uint32_t old_ints = intDisable();
  if (len == 128 && buf[12+4] == 0xef && buf[12+5] == 0x50 && buf[12] == 0x80){
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
  
        MachineBuffer[MachineBufferIndex].Id = msg.src;
        MachineBuffer[MachineBufferIndex].Duration = (msg.data[0] << 24) | (msg.data[1]  << 16) | (msg.data[2] << 8) | msg.data[3];
        MachineBuffer[MachineBufferIndex].RelaisBitmask = msg.data[5];
        MachineBuffer[MachineBufferIndex].Type = msg.type;
        MachineBuffer[MachineBufferIndex].Rssi = sniffer->rx_ctrl.rssi;
        MachineBuffer[MachineBufferIndex].Seq = msg.seq;
        MachineBuffer[MachineBufferIndex].Timestamp = millis();
        MachineBufferIndex++;
        Serial.println(" --> " + String(msg.src, HEX));

        if(MachineBufferIndex%2==0)
          led::setColor(100,30,0);
        else
          led::setColor(100,0,30);

        if(machinesIndexCache.count(msg.src) == 0)
        {
          Serial.println("new MachineData():" + String(msg.src, HEX));
          MachineData* md = new MachineData();
          md->Id = msg.src;
          md->ShortName[0] = '?';
          machines.push_back(*md);

          machinesIndexCache[msg.src] = machines.size() - 1;
        }
        
        uint8_t mId = machinesIndexCache[msg.src];
        machines[mId].Rssi = (int8_t)sniffer->rx_ctrl.rssi;
          
        if(msg.type == MSG_SendRssi)
        {  
          Serial.println("_mId:" + String(mId));
          if(machines[mId].Id == msg.src)
          {
            for(int i = 1; i < msg.dataLength; i+=5)
            {
              uint32_t id_tmp;
              int8_t rssi_tmp;
              memcpy((uint8_t*)&id_tmp, (uint8_t*)&msg.data + i, 4);
              memcpy((uint8_t*)&rssi_tmp, (uint8_t*)&msg.data + i + 4, 1);
              Serial.println("_" + String(id_tmp, HEX) + ":" + String(rssi_tmp));
              
              machines[mId].RssiMap[id_tmp] = rssi_tmp;

              if(machinesIndexCache.count(id_tmp) == 0)
              {
                Serial.println(" > new MachineData(): " + String(id_tmp, HEX));
                MachineData* md = new MachineData();
                md->Id = id_tmp;
                md->ShortName[0] = '?';
                machines.push_back(*md);
      
                machinesIndexCache[id_tmp] = machines.size() - 1;
              }
            }
          }
        }
      }
    }
  }
  intEnable(old_ints);
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
  result[10 + 2] = (localAddress >> 24) & 0xFF;
  result[10 + 3] = (localAddress >> 16) & 0xFF;
  result[10 + 4] = (localAddress >> 8) & 0xFF;
  result[10 + 5] = (localAddress) & 0xFF;

  //transmitc
  result[16 + 2] = (localAddress >> 24) & 0xFF;
  result[16 + 3] = (localAddress >> 16) & 0xFF;
  result[16 + 4] = (localAddress >> 8) & 0xFF;
  result[16 + 5] = (localAddress) & 0xFF;

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

void blink(uint32_t dest)
{   
  uint8_t result[sizeof(beacon_raw) + 2];   
  createPacket(result, {}, 0, dest, MSG_Blink);
  int res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  //delay(20);
  //res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("blink to %08x = %d\n\n", dest, res);  
  led::setColor(0,0,100);
  delay(70);
  led::setColor(0,100,0);
}

void fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask)
{ 
  uint8_t result[sizeof(beacon_raw) + 5];   
  uint8_t data[5] = {0}; 
  memcpy((uint8_t*)&data, (uint8_t*)&duration, 4);
  memcpy((uint8_t*)&data + 4, (uint8_t*)&relaisBitmask, 1);
  
  Serial.printf("data %02X %02X %02X %02X %02X\n", data[0], data[1], data[2], data[3], data[4]); 
  
  createPacket(result, data, 5, dest, MSG_Fire);
  int res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  //delay(20);
  //res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("fired to %08x duration %d relaisBitmask %02X = %d\n\n", dest, duration, relaisBitmask, res);  
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
  Serial.printf("reqRssi to %08x = %d\n\n", dest, res);  
  led::setColor(100,100,100);
  delay(70);
  led::setColor(0,100,0);
}

void setupFreedom()
{
  led::setColor(100,0,0);
  Serial.println("Setting up Freedom Mode");
  WiFi.mode(WIFI_STA); 
  wifi_set_channel(CHANNEL);
  wifi_set_phy_mode(PHY_MODE_11B);
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

void readWifi(char* buf, uint8_t len)
{
  if (len != 6) 
    return;
  uint32_t dest = 0;
  dest += buf[4];
  dest += buf[3]<<8;
  dest += buf[2]<<16;
  dest += buf[1]<<24;

  uint32_t duration;
  uint8_t relaisBitmask;
  //legacyMode
  switch(buf[5])
  {
    case 0x01:
      duration = 1000;
      relaisBitmask = 0x01;
      break;
    case 0x11:
      duration = 1000;
      relaisBitmask = 0x02;
      break;
    case 0x21:
      duration = 1000;
      relaisBitmask = 0x03;
      break;
    case 0x03:
      duration = 3000;
      relaisBitmask = 0x01;
      break;
    case 0x13:
      duration = 3000;
      relaisBitmask = 0x02;
      break;
    case 0x23:
      duration = 3000;
      relaisBitmask = 0x03;
      break;
    case 0x04:
      duration = 400;
      relaisBitmask = 0x01;
      break;
    case 0x14:
      duration = 400;
      relaisBitmask = 0x02;
      break;
    case 0x24:
      duration = 400;
      relaisBitmask = 0x03;
      break;
    case 0x0a:
      duration = -1;
      relaisBitmask = 0x01;
      break;
    case 0x1a:
      duration = -1;
      relaisBitmask = 0x02;
      break;
    case 0x2a:
      duration = -1;
      relaisBitmask = 0x03;
      break;
    case 0x0b:
      duration = 0;
      relaisBitmask = 0x01;
      break;
    case 0x1b:
      duration = 0;
      relaisBitmask = 0x02;
      break;
    case 0x2b:
      duration = 0;
      relaisBitmask = 0x03;
      break;
  }
  
  fire(dest, duration, relaisBitmask);
  
  AppBuffer[AppBufferIndex].Id = dest;
  AppBuffer[AppBufferIndex].Duration = duration;
  AppBuffer[AppBufferIndex].RelaisBitmask = relaisBitmask;
  AppBuffer[AppBufferIndex].Type = buf[0];
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = seqnum;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;
}

void wifi_setup()
{
  memset((uint8_t*)&AppBuffer, 0x00, sizeof(WifiLog) * 255);
  memset((uint8_t*)&MachineBuffer, 0x00, sizeof(WifiLog) * 255);
  
  localAddress = ESP.getChipId();

  seqnum = random(3000);

  wifi_set_promiscuous_rx_cb(promisc_cb);  
}