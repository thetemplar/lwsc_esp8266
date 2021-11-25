/// Expose Espressif SDK functionality - wrapped in ifdef so that it still
// compiles on other platforms
#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#define VERSION 0x03
//uint8_t VERSION;
#define START_TTL 0x05
#define MSG_TYPE 0x00

#define TIMEOUT_HOST 60
#define TIMEOUT_REQUEST 20
#define KEEPALIVE_INTERVAL 60
#define CHANNEL 1

//weiße platine: RELAIS1=4 RELAIS2=13; neue lora-platine: RELAIS1=5 RELAIS2=4
#define RELAIS1 4
#define RELAIS2 13

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "SimpleTimer.h"
#include <map>
#include "defines.h"
#include <stack>          // std::stack

WiFiClient client;
WiFiServer server(9900);

time_t timestamp = 0;

uint8_t* sendBuffer;
size_t sendBufferLength;
char packetBuffer[255]; //buffer to hold incoming packet
int localAddress = 0x00;      // address of this device

bool LoRaEnabled = false;

SimpleTimer timer;
int timerId;

uint16_t ledState;

std::stack<struct sniffer_buf2> dataStack;

uint16_t seqnum = 0x000;
int8_t rssi;
std::map<uint32_t,uint16_t> lastSeqNum;
std::map<uint32_t,uint32_t> lastTimestamp;
std::map<uint32_t,int8_t> lastRssi;

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


  uint32_t id = ESP.getChipId();

  //dst
  result[4 + 2] = (dst >> 24) & 0xFF;
  result[4 + 3] = (dst >> 16) & 0xFF;
  result[4 + 4] = (dst >> 8) & 0xFF;
  result[4 + 5] = (dst) & 0xFF;

  //src
  result[10 + 2] = (id >> 24) & 0xFF;
  result[10 + 3] = (id >> 16) & 0xFF;
  result[10 + 4] = (id >> 8) & 0xFF;
  result[10 + 5] = (id) & 0xFF;

  //transmitc
  result[16 + 2] = (id >> 24) & 0xFF;
  result[16 + 3] = (id >> 16) & 0xFF;
  result[16 + 4] = (id >> 8) & 0xFF;
  result[16 + 5] = (id) & 0xFF;

  result[22] = (seqnum >> 8) & 0xFF;
  result[23] = (seqnum) & 0xFF;

  uint16_t seqTmp = seqnum;
  seqnum++;
  if (seqnum > 0xfff)
    seqnum = 1;

  result[39] += len;
  result[42] = VERSION;
  result[43] = START_TTL;
  result[44] = type;

  return seqTmp;
}

void forwardPacket(uint8_t* result)
{
  if(result[43] == 0) //double safty. if ttl is == 0, then make packet invalid
  {
    result[0] = 0;
    result[1] = 0;
  }
  else
  {
    //decrease ttl
    result[43]--;
  }

  //set transmitter
  result[16 + 2] = (ESP.getChipId() >> 24) & 0xFF;
  result[16 + 3] = (ESP.getChipId() >> 16) & 0xFF;
  result[16 + 4] = (ESP.getChipId() >> 8) & 0xFF;
  result[16 + 5] = (ESP.getChipId()) & 0xFF;
}

void processData()
{
  struct sniffer_buf2 snifferElement = dataStack.top();
  struct sniffer_buf2* sniffer = &snifferElement;
  rssi = ((uint8_t*)sniffer)[0];
  dataStack.pop();
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
  Serial.printf("Data (dst: 0x%08x, src: 0x%08x, rssi: %d, ttl: %d, type: %02x, seq: %d:, len: %d: ", msg.dst, msg.src, rssi, msg.ttl, msg.type, msg.seq, msg.dataLength);
  //for(uint16_t i = 0; i < msg.dataLength; i++)
  //  Serial.printf("%02x ", msg.data[i]);
  Serial.printf("\n"); 

  if(lastRssi.count(msg.src) > 0)
  {
    //int8_t lastRssiVal = lastRssi[msg.src] >> 24;
    //rssi = (lastRssiVal * 3 + sniffer->rx_ctrl.rssi) / 4;
  }

  lastRssi[msg.src] = rssi; 
  lastTimestamp[msg.src] = millis(); 

  if(lastSeqNum[msg.src] == msg.seq)
  {
    Serial.printf("No new seq num :(\n");    
    return;
  }
  lastSeqNum[msg.src] = msg.seq;

  if(msg.dst != ESP.getChipId() && msg.ttl > 0 && msg.ttl < START_TTL+1)
  {
    delayMicroseconds(1000+random(20000)); //1-30ms delay to avoid parallel-fwd of multiple nodes
    //forward!
    Serial.printf("Forward to dst(0x%08x) from me(0x%08x) with new ttl:%d!\n", msg.dst, ESP.getChipId(), (msg.ttl-1));
    forwardPacket(sniffer->buf);
    uint16_t res = wifi_send_pkt_freedom(sniffer->buf, sizeof(beacon_raw)+ msg.dataLength, 0);      
  }
  else
  {
    Serial.printf("No Forward: TTL Death or I'm the dst\n");  
  }
  
  if(msg.dst == ESP.getChipId() || msg.dst == 0xffffffff)
  {      
    //i am the reciever! yaaaaaay
    Serial.printf("I am the dst (dst(0x%08x) == chipid(0x%08x))! Sending ack...\n", msg.dst, ESP.getChipId());   
    
    if(msg.type == MSG_Blink)
    {
      Serial.println("MSG_Blink");  
      digitalWrite(2, LOW);
      delay(400);      
      digitalWrite(2, HIGH);
    }
    if(msg.type == MSG_Fire)
    {
      if (msg.dataLength == 5)
      {
        int32_t duration = (msg.data[3] << 24) | (msg.data[2]  << 16) | (msg.data[1] << 8) | msg.data[0];
        uint8_t bitmask = msg.data[4];
        Serial.println("releaseRelais " + String(duration) + " - 0x" + String(bitmask, HEX));  
        releaseRelais(duration, bitmask);
      }
      else if (msg.dataLength == 2)
      {
        Serial.println("releaseRelais_LegacyVersion");  
        releaseRelais_LegacyVersion(msg.data[0], msg.data[1]);
      }
      
      if(msg.dst != 0xffffffff)
      {
        //send reply
        uint8_t result[sizeof(beacon_raw) + 2];    
        uint8_t data[2] = {(msg.seq >> 8) & 0xFF, msg.seq & 0xFF}; //ack with msg.src & msg.seq
        createPacket(result, data, 2, msg.src, MSG_Fire_Ack);
        uint16_t res = wifi_send_pkt_freedom(result, sizeof(result), 0);
        Serial.printf("Send MSG_Fire_Ack");  
        /*for(uint16_t i = 0; i < sizeof(result); i++)
          Serial.printf("%02x ", result[i]); 
        Serial.printf("\n"); */
      }
    } else if(msg.type == MSG_RequestRssi) {
      Serial.printf("MSG_RequestRssi \n"); 
      digitalWrite(2, LOW);
      if(msg.dst == 0xffffffff)
      {
        timer.disable(timerId);
        uint32_t randNumber = random(5000);
        delay(randNumber);
      }
      sendRssiToDest(msg.dst);
      digitalWrite(2, HIGH);
      if(msg.dst == 0xffffffff)
      {
        delay(5000); //so the "air" stays free for others!
        timer.enable(timerId);
      }
      Serial.printf("MSG_RequestRssi Done \n"); 
    }
  }
 
}

void ICACHE_RAM_ATTR promisc_cb(uint8_t *buf, uint16_t len)
{
  //uint32_t old_ints = intDisable();
  if (len == 128 && buf[12+4] == 0xef && buf[12] == 0x80){
    if(dataStack.size() < 10)
    {
      struct sniffer_buf2* b = (struct sniffer_buf2*)buf;
      dataStack.push(*b);    
    } 
  }
  //intEnable(old_ints);
}

void sendRssi()
{
  sendRssiToDest(0xffffffff);
}

void sendRssiToDest(uint32_t dest)
{
  uint8_t s = 5 * lastRssi.size() + 1;
  uint8_t result[sizeof(beacon_raw) + s];    
  uint8_t data[s] = {0};
  
  std::map<uint32_t, int8_t>::iterator it;
  result[sizeof(beacon_raw)] = lastRssi.size() & 0xFF;
  uint8_t i = 1;
  for (it = lastRssi.begin(); it != lastRssi.end(); it++)
  {
    memcpy((uint8_t*)&data[i], &(it->first), 4);
    memcpy((uint8_t*)&data[i + 4], &(it->second), 1);
    
    Serial.printf("   id %08X - ", it->first);  
    Serial.printf("rssi %d\n", it->second);  
    
    i+=5;
  }
  
  uint16_t seq = createPacket(result, data, s, dest, MSG_SendRssi);
  uint16_t res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("sending MSG_SendRssi (seqnum: %d, res: %d)\n", seq, res);
}

void sendKeepAlive()
{
  uint8_t result[sizeof(beacon_raw)];
  uint16_t seq = createPacket(result, {}, 0, 0xffffffff, MSG_KeepAlive);
  uint16_t res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("sending MSG_KeepAlive (seqnum: %d, res: %d)\n", seq, res);
}


void setupFreedom()
{
  Serial.println("Setting up Freedom Mode");
  WiFi.mode(WIFI_STA); 
  wifi_set_channel(CHANNEL);
  wifi_set_phy_mode(PHY_MODE_11B);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
}

void setup() {
  
  Serial.begin(115200);
  delay(2000);
  Serial.printf("\n\nSDK version: %s - chipId: 0x%08x - fw-version: %d\n", system_get_sdk_version(), ESP.getChipId(), VERSION);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(RELAIS1, OUTPUT);
  pinMode(RELAIS2, OUTPUT);
 
  // Promiscuous works only with station mode
  seqnum = ESP.getChipId() & 0xfff; //semi-rnd init
  
  WiFi.mode(WIFI_STA); 
  setupFreedom();

  sendKeepAlive();
  timerId = timer.setInterval(KEEPALIVE_INTERVAL * 1000 + (ESP.getChipId() & 0xffff) * 10, sendKeepAlive);

  localAddress = ESP.getChipId();
}

void loop() {   
  //uint32_t currentMillis = millis();
  if (millis() > 2592000000) ESP.restart(); //(every 30days)
  
  timer.run();
  if(!dataStack.empty())
  {
    Serial.printf("<"); 
    processData();
    Serial.println(">"); 
  }
}


void releaseRelais(int32_t duration, uint8_t bitmask)
{
  uint32_t currentMillis = millis();
  ledState != ledState;

  Serial.println("releaseRelais on " + String(duration));
  digitalWrite(2, LOW);
  if(bitmask & 0b00000001)
    digitalWrite(RELAIS1, HIGH);
  if(bitmask & 0b00000010)
    digitalWrite(RELAIS2, HIGH);

  if(duration >= 0)
  {
    Serial.println("releaseRelais delay");
    while (millis() < duration + currentMillis) delay(1);
  
    Serial.println("releaseRelais off");
    digitalWrite(2, HIGH);
    if(bitmask & 0b00000001)
      digitalWrite(RELAIS1, LOW);
    if(bitmask & 0b00000010)
      digitalWrite(RELAIS2, LOW);
  }
}


void releaseRelais_LegacyVersion(byte lwscType, byte lwscPort)
{
  uint32_t currentMillis = millis();
  if(lwscType == 0xfa)
  {
    //1s
    if(lwscPort == 0x01)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      Serial.println("1st Release");
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS1, LOW); 
    }
    
    if(lwscPort == 0x11)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("2nd Release");
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS2, LOW); 
    }
    
    if(lwscPort == 0x21)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("Both Release");
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS1, LOW); 
      digitalWrite(RELAIS2, LOW); 
    }

    //3s
    if(lwscPort == 0x03)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      Serial.println("1st Release");
      while (millis() < currentMillis + 3000) delay(1);
      digitalWrite(RELAIS1, LOW); 
    }
    
    if(lwscPort == 0x13)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("2nd Release");
      while (millis() < currentMillis + 3000) delay(1);
      digitalWrite(RELAIS2, LOW); 
    }
    
    if(lwscPort == 0x23)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("Both Release");
      while (millis() < currentMillis + 3000) delay(1);
      digitalWrite(RELAIS1, LOW); 
      digitalWrite(RELAIS2, LOW); 
    }

    //0.4s
    if(lwscPort == 0x04)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      Serial.println("1st Release");
      while (millis() < currentMillis + 400) delay(1);
      digitalWrite(RELAIS1, LOW); 
    }
    
    if(lwscPort == 0x14)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("2nd Release");
      while (millis() < currentMillis + 400) delay(1);
      digitalWrite(RELAIS2, LOW); 
    }
    
    if(lwscPort == 0x24)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("Both Release");
      while (millis() < currentMillis + 400) delay(1);
      digitalWrite(RELAIS1, LOW); 
      digitalWrite(RELAIS2, LOW); 
    }
    
    //dauer an
    if(lwscPort == 0x0a)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
    }
    
    if(lwscPort == 0x1a)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, HIGH);
    }
    
    if(lwscPort == 0x2a)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      digitalWrite(RELAIS2, HIGH); 
    }
    
    //dauer aus
    if(lwscPort == 0x0b)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, LOW);
    }
    
    if(lwscPort == 0x1b)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, LOW);
    }
    
    if(lwscPort == 0x2b)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, LOW);
      digitalWrite(RELAIS2, LOW); 
    }
  }
}
