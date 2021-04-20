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

//weiße platine: 13 oder 4; neue lora-platine: 5 oder 4
#define RELAIS1 13
#define RELAIS2 4


#include <SPI.h>              // include libraries
#include <LoRa.h>
const int csPin = 2;          // LoRa radio chip select
const int resetPin = 16;       // LoRa radio reset
const int irqPin = 15;         // change for your board; must be a hardware interrupt pin

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SimpleTimer.h>
#include <map>
#include "defines.h"

WiFiClient client;
WiFiServer server(9900);

time_t timestamp = 0;

uint16_t waitForAck = 0;
uint8_t retrySendCounter = 5;

uint8_t* sendBuffer;
size_t sendBufferLength;
char packetBuffer[255]; //buffer to hold incoming packet
int localAddress = 0x00;      // address of this device

bool LoRaEnabled = false;

SimpleTimer timer;
uint16_t updateModeTimer;
bool inUpdateMode = false;
uint16_t requestUpdateTimer;
bool requestedUpdate = false;
uint16_t inUpdateTimeoutTimer;
uint16_t ntpUpdateTimer;
uint16_t ackTimer;

bool inProcess = false;
uint16_t ledState;

struct sniffer_buf2 *sniffer;

uint16_t seqnum = 0x000;
std::map<uint32_t,uint16_t> lastSeqNum;
std::map<uint32_t,uint32_t> lastRssi;

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

  //set transmitter
  result[16 + 2] = (ESP.getChipId() >> 24) & 0xFF;
  result[16 + 3] = (ESP.getChipId() >> 16) & 0xFF;
  result[16 + 4] = (ESP.getChipId() >> 8) & 0xFF;
  result[16 + 5] = (ESP.getChipId()) & 0xFF;

  //decrease ttl
  result[43]--;
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
  
  Serial.printf("Data (dst: %02x:%02x:%02x:%02x:%02x:%02x (%d), src: %02x:%02x:%02x:%02x:%02x:%02x (%d), rssi: %d, ttl: %d, type: %02x, seq: %d: ", sniffer->buf[4], sniffer->buf[5], sniffer->buf[6], sniffer->buf[7], sniffer->buf[8], sniffer->buf[9], msg.dst, sniffer->buf[10], sniffer->buf[11], sniffer->buf[12], sniffer->buf[13], sniffer->buf[14], sniffer->buf[15], msg.src, sniffer->rx_ctrl.rssi, msg.ttl, msg.type, msg.seq);
  for(uint16_t i = 0; i < msg.dataLength; i++)
    Serial.printf("%02x ", msg.data[i]);
  Serial.printf("\n"); 

  uint32_t newRssi = millis() >> 8;
  int8_t rssi = sniffer->rx_ctrl.rssi;
  if(lastRssi.count(msg.src) > 0)
    rssi = ((lastRssi[msg.src] >> 24) * 3 + sniffer->rx_ctrl.rssi) / 4;

  newRssi += rssi << 24;
  lastRssi[msg.src] = newRssi; //1byte rssi, 3byte timestamp

  if(lastSeqNum[msg.src] == msg.seq)
  {
    Serial.printf("No new seq num :(\n");    
    return;
  }
  lastSeqNum[msg.src] = msg.seq;

  if(msg.dst != ESP.getChipId() && msg.ttl > 0 && msg.ttl < START_TTL+1)
  {
    //delayMicroseconds(2000+random(6000)); //2-8ms delay to avoid parallel-fwd of multiple nodes
    //forward!
    Serial.printf("Forward to dst(%d) from me(%d) with new ttl:%d!\n", msg.dst, ESP.getChipId(), (msg.ttl-1));
    forwardPacket(sniffer->buf);
    uint16_t res = wifi_send_pkt_freedom(sniffer->buf, sizeof(beacon_raw)+ msg.dataLength, 0);      
  }
  
  if(msg.type == MSG_Data && (msg.dst == ESP.getChipId() || msg.dst == 0xffffffff))
  {
    releaseRelais(msg.data[0], msg.data[1]);
    
    //i am the reciever! yaaaaaay
    Serial.printf("I am the dst (dst(%d) == chipid(%d))! Sending ack...\n", msg.dst, ESP.getChipId());  
    Serial.printf("My data is:\n");    
    for(uint16_t i = 0; i < msg.dataLength; i++)
      Serial.printf("%02x ", msg.data[i]); 
    Serial.printf("\n\n");  
    
    if(msg.dst != 0xffffffff)
    {
      //send reply
      uint8_t result[sizeof(beacon_raw) + 2];    
      uint8_t data[2] = {(msg.seq >> 8) & 0xFF, msg.seq & 0xFF}; //ack with msg.src & msg.seq
      createPacket(result, data, 2, msg.src, MSG_Data_Ack);
      uint16_t res = wifi_send_pkt_freedom(result, sizeof(result), 0);
      /*for(uint16_t i = 0; i < sizeof(result); i++)
        Serial.printf("%02x ", result[i]); 
      Serial.printf("\n"); */
    }
  }
  else if(msg.type == MSG_Data_Ack && (msg.dst == ESP.getChipId() || msg.dst == 0xffffffff))
  {      
    uint16_t ackSeq = (msg.data[0] << 8) | msg.data[1];
    Serial.printf("Got ack for %d (waiting for %d)\n", ackSeq, waitForAck);
    if(ackSeq == waitForAck)
    {
      waitForAck = 0;
      retrySendCounter = 5;
      timer.disable(ackTimer);
    }
  } 
 
}

void ICACHE_RAM_ATTR promisc_cb(uint8_t *buf, uint16_t len)
{
  uint32_t old_ints = intDisable();
  if (len == 128 && buf[12+4] == 0xef && buf[12] == 0x80){
    Serial.printf("*");
    if (!inProcess ){
      inProcess = true;  
      sniffer = (struct sniffer_buf2*) buf;
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

void resendMsg()
{
  retrySendCounter--;
  if(retrySendCounter <= 0) 
  {    
    Serial.printf("failed to reach destination for data. (seqnum: %d, len %d)\n", waitForAck, sendBufferLength);
    retrySendCounter = 5;
    waitForAck = 0;
    return;
  }
  uint16_t res = wifi_send_pkt_freedom(sendBuffer, sendBufferLength, 0);
  Serial.printf("re-sending data (seqnum: %d, len %d)\n", waitForAck, sendBufferLength);
  
  ackTimer = timer.setTimer(200, resendMsg, 1);
}

void sendDataMsg(uint8_t* data, size_t length, uint32_t destination)
{
  if(inUpdateMode) return;
  uint8_t result[sizeof(beacon_raw)+length];
  uint16_t seq = createPacket(result, data, length, destination, MSG_Data);
  uint16_t res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  waitForAck = seq;
  Serial.printf("sending data (seqnum: %d, len %d) to %d\n", seq, length, destination);

  if(destination != 0xffffffff)
  {
    sendBuffer = result;
    sendBufferLength = sizeof(beacon_raw)+length;
    ackTimer = timer.setTimer(200, resendMsg, 1);
  }
}

void sendKeepAlive()
{
  if(inUpdateMode) return;
  uint8_t result[sizeof(beacon_raw)];
  uint16_t seq = createPacket(result, {}, 0, 0xffffffff, MSG_KeepAlive);
  uint16_t res = wifi_send_pkt_freedom(result, sizeof(result), 0);
  Serial.printf("sending KeepAlive (seqnum: %d, res: %d)\n", seq, res);
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
  Serial.printf("\n\nSDK version: %s - chipId: %08x - fw-version: %d\n", system_get_sdk_version(), ESP.getChipId(), VERSION);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(RELAIS1, OUTPUT);
  pinMode(RELAIS2, OUTPUT);
 
  // Promiscuous works only with station mode
  seqnum = ESP.getChipId() & 0xfff; //semi-rnd init
  
  WiFi.mode(WIFI_STA); 
  setupFreedom();

  sendKeepAlive();
  //timer.setInterval(KEEPALIVE_INTERVAL * 1000 + (ESP.getChipId() & 0xfff), sendKeepAlive);

  
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

void loop() {   
  //uint32_t currentMillis = millis();
  if (millis() > 2592000000) ESP.restart(); //(every 30days)
  
  //timer.run();
  if(inProcess)
  {
    processData(sniffer);
    inProcess = false;
  }
  delay(1);
}


void releaseRelais(byte lwscType, byte lwscPort)
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
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS1, LOW); 
    }
    
    if(lwscPort == 0x13)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("2nd Release");
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS2, LOW); 
    }
    
    if(lwscPort == 0x23)
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

    //0.4s
    if(lwscPort == 0x04)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS1, HIGH);
      Serial.println("1st Release");
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS1, LOW); 
    }
    
    if(lwscPort == 0x14)
    {
      ledState != ledState;
      digitalWrite(2, ledState);
      digitalWrite(RELAIS2, HIGH);
      Serial.println("2nd Release");
      while (millis() < currentMillis + 1000) delay(1);
      digitalWrite(RELAIS2, LOW); 
    }
    
    if(lwscPort == 0x24)
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
    Serial.println("This message is not for me (" + String(localAddress, HEX) + "), but for " + String(recipient, HEX) + ".");
    return;                             // skip rest of function
  }

  char* pEnd;
  uint32_t lwscType = strtoul(incoming.c_str(), &pEnd, 16);
  uint32_t lwscPort = strtoul(pEnd, NULL, 16);

  releaseRelais(lwscType, lwscPort);
  
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


