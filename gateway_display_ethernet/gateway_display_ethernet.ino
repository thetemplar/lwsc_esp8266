//#define DEBUG_ESP_HTTP_SERVER

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "A_config.h"
#include "DisplayUI.h"

#include "led.h"
#include "lwsc_wifi.h"
#include "REST.h"
#include "src/SimpleTimer/SimpleTimer.h"


#ifdef ETH_ENABLE
 #include <ENC28J60lwIP.h>
 #define CSPIN 16
  ENC28J60lwIP eth(CSPIN);
  byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
#endif

char packetBuffer[256];       //buffer to hold incoming packet 
byte ibuffer[100];

ESP8266WebServer server(80);
DisplayUI displayUI;


const char* ssid = "lwsc_wifibridge_display";
const char* password = "lauterbach";
String network_ip;

SimpleTimer timer;
int timerIdRssi;
int rssiOngoing;

WiFiUDP Udp;
int timerIdUDP;

#ifdef ETH_ENABLE

void wrap_bt_up()
{
  displayUI.bt_up();
}
void wrap_bt_down()
{
  displayUI.bt_down();
}
void wrap_bt_click()
{
  displayUI.bt_click();
}
void wrap_bt_home()
{
  displayUI.bt_home();
}
#endif


#ifndef ETH_ENABLE
void timer_query_rssi()
{
  Serial.println("timer_query_rssi");
  rssiOngoing--;
  Serial.println("rssiOngoing" + String(rssiOngoing));
  if (rssiOngoing == 0)
  {
    Serial.println("timer.disable(timerIdRssi)");
    timer.disable(timerIdRssi);
    setupAP();
  } else {
    reqRssi(0xFFFFFFFF);
  }
}

void start_query_rssi()
{
  if(rssiOngoing > 0)
    return;
  Serial.println("start_query_rssi");
  setupFreedom();
  reqRssi(0xFFFFFFFF);
  rssiOngoing = 2;
  timerIdRssi = timer.setInterval(7000, timer_query_rssi);
}
#endif

boolean InitalizeFileSystem() {   
  bool initok = false;   
  initok = SPIFFS.begin();   
  if (!(initok)) // Format SPIFS, of not formatted. - Try 1   
  {     
    Serial.println("SPIFFS file system formatted.");     
    SPIFFS.format();     
    initok = SPIFFS.begin();   
    }   
    if (!(initok)) // format SPIFS. - Try 2   
    {     
      SPIFFS.format();     
      initok = SPIFFS.begin();   
    }   
    if (initok) 
    {
      Serial.println("SPIFFS is OK"); 
    } else { 
      Serial.println("SPIFFS is not OK"); 
    }   
      Serial.println("SPIFFS Information:");
#ifdef ARDUINO_ARCH_ESP32
    // different methods of getting information
    Serial.print("Total bytes:    "); Serial.println(SPIFFS.totalBytes());
    Serial.print("Used bytes:     "); Serial.println(SPIFFS.usedBytes());
#else
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.print("Total bytes:    "); Serial.println(fs_info.totalBytes);
    Serial.print("Used bytes:     "); Serial.println(fs_info.usedBytes);
    Serial.print("Block size:     "); Serial.println(fs_info.blockSize);
    Serial.print("Page size:      "); Serial.println(fs_info.pageSize);
    Serial.print("Max open files: "); Serial.println(fs_info.maxOpenFiles);
    Serial.print("Max path length:"); Serial.println(fs_info.maxPathLength);
    Serial.println();
#endif

    return initok;
}

extern std::vector<MachineData> machines;
extern std::map<uint32_t, uint8_t> machinesIndexCache;
void ReadConfig()
{
  File configFile = SPIFFS.open("/machines.conf", "r");
  if (!configFile)
  {
    Serial.println(F("Failed to open machines.conf"));
  } else {
    Serial.println(F("Opened machines.conf"));
    uint8_t len = 0;
    configFile.readBytes((char*)&len, 1);
    Serial.println(F("readBytes len: ") + String(len));
    for(int i = 0; i < len; i++)
    {
      Serial.println(F("reading # ") + String(i));
      MachineData md;
      configFile.readBytes((char*)&md.Name, 38);
      configFile.readBytes((char*)&md.ShortName, 9);
      configFile.readBytes((char*)&md.Id, 4);
      configFile.readBytes((char*)&md.Disabled, 1);
      configFile.readBytes((char*)&md.SymbolX, 4);
      configFile.readBytes((char*)&md.SymbolY, 4);
      Serial.println(F("read: 0x00") + String(md.Id, HEX));
      
      configFile.readBytes((char*)&md.Functions[0].Name, 38);
      configFile.readBytes((char*)&md.Functions[0].Duration, 4);
      configFile.readBytes((char*)&md.Functions[0].RelaisBitmask, 1);
      configFile.readBytes((char*)&md.Functions[0].SymbolX, 4);
      configFile.readBytes((char*)&md.Functions[0].SymbolY, 4);
      configFile.readBytes((char*)&md.Functions[0].Rotation, 1);
      
      configFile.readBytes((char*)&md.Functions[1].Name, 38);
      configFile.readBytes((char*)&md.Functions[1].Duration, 4);
      configFile.readBytes((char*)&md.Functions[1].RelaisBitmask, 1);
      configFile.readBytes((char*)&md.Functions[1].SymbolX, 4);
      configFile.readBytes((char*)&md.Functions[1].SymbolY, 4);
      configFile.readBytes((char*)&md.Functions[1].Rotation, 1);
      
      configFile.readBytes((char*)&md.Functions[2].Name, 38);
      configFile.readBytes((char*)&md.Functions[2].Duration, 4);
      configFile.readBytes((char*)&md.Functions[2].RelaisBitmask, 1);
      configFile.readBytes((char*)&md.Functions[2].SymbolX, 4);
      configFile.readBytes((char*)&md.Functions[2].SymbolY, 4);
      configFile.readBytes((char*)&md.Functions[2].Rotation, 1);
      
      configFile.readBytes((char*)&md.Functions[3].Name, 38);
      configFile.readBytes((char*)&md.Functions[3].Duration, 4);
      configFile.readBytes((char*)&md.Functions[3].RelaisBitmask, 1);
      configFile.readBytes((char*)&md.Functions[3].SymbolX, 4);
      configFile.readBytes((char*)&md.Functions[3].SymbolY, 4);
      configFile.readBytes((char*)&md.Functions[3].Rotation, 1);
      
      configFile.readBytes((char*)&md.Functions[4].Name, 38);
      configFile.readBytes((char*)&md.Functions[4].Duration, 4);
      configFile.readBytes((char*)&md.Functions[4].RelaisBitmask, 1);
      configFile.readBytes((char*)&md.Functions[4].SymbolX, 4);
      configFile.readBytes((char*)&md.Functions[4].SymbolY, 4);
      configFile.readBytes((char*)&md.Functions[4].Rotation, 1);

      machines.push_back(md);
      machinesIndexCache[md.Id] = machines.size() - 1;
      Serial.println("added: " + String(md.ShortName));
    }
  }
  configFile.close();
}

void WriteConfig()
{
  File configFile = SPIFFS.open("/machines.conf", "w");
  if (!configFile)
  {
    Serial.println(F("Failed to open machines.conf"));
  } else {
    Serial.println(F("Opened machines.conf"));
    uint8_t len = machines.size();
    configFile.write((char*)&len, 1);
    Serial.println(F("write len: ") + String(len));
    for(int i = 0; i < len; i++)
    {
      Serial.println(F("writing # ") + String(i));
      configFile.write((char*)&machines[i].Name, 38);
      configFile.write((char*)&machines[i].ShortName, 9);
      configFile.write((char*)&machines[i].Id, 4);
      configFile.write((char*)&machines[i].Disabled, 1);
      configFile.write((char*)&machines[i].SymbolX, 4);
      configFile.write((char*)&machines[i].SymbolY, 4);
      Serial.println(F("write: 0x00") + String(machines[i].Id, HEX));
      
      configFile.write((char*)&machines[i].Functions[0].Name, 38);
      configFile.write((char*)&machines[i].Functions[0].Duration, 4);
      configFile.write((char*)&machines[i].Functions[0].RelaisBitmask, 1);
      configFile.write((char*)&machines[i].Functions[0].SymbolX, 4);
      configFile.write((char*)&machines[i].Functions[0].SymbolY, 4);
      configFile.write((char*)&machines[i].Functions[0].Rotation, 1);
      
      configFile.write((char*)&machines[i].Functions[1].Name, 38);
      configFile.write((char*)&machines[i].Functions[1].Duration, 4);
      configFile.write((char*)&machines[i].Functions[1].RelaisBitmask, 1);
      configFile.write((char*)&machines[i].Functions[1].SymbolX, 4);
      configFile.write((char*)&machines[i].Functions[1].SymbolY, 4);
      configFile.write((char*)&machines[i].Functions[1].Rotation, 1);
      
      configFile.write((char*)&machines[i].Functions[2].Name, 38);
      configFile.write((char*)&machines[i].Functions[2].Duration, 4);
      configFile.write((char*)&machines[i].Functions[2].RelaisBitmask, 1);
      configFile.write((char*)&machines[i].Functions[2].SymbolX, 4);
      configFile.write((char*)&machines[i].Functions[2].SymbolY, 4);
      configFile.write((char*)&machines[i].Functions[2].Rotation, 1);
      
      configFile.write((char*)&machines[i].Functions[3].Name, 38);
      configFile.write((char*)&machines[i].Functions[3].Duration, 4);
      configFile.write((char*)&machines[i].Functions[3].RelaisBitmask, 1);
      configFile.write((char*)&machines[i].Functions[3].SymbolX, 4);
      configFile.write((char*)&machines[i].Functions[3].SymbolY, 4);
      configFile.write((char*)&machines[i].Functions[3].Rotation, 1);
      
      configFile.write((char*)&machines[i].Functions[4].Name, 38);
      configFile.write((char*)&machines[i].Functions[4].Duration, 4);
      configFile.write((char*)&machines[i].Functions[4].RelaisBitmask, 1);
      configFile.write((char*)&machines[i].Functions[4].SymbolX, 4);
      configFile.write((char*)&machines[i].Functions[4].SymbolY, 4);
      configFile.write((char*)&machines[i].Functions[4].Rotation, 1);

      Serial.println("written: " + String(machines[i].ShortName));
    }
  }
  configFile.close();
}

void udpBroadcast() {
#ifdef ETH_ENABLE
  IPAddress broadcastIP(255, 255, 255, 255);
  Udp.beginPacket(broadcastIP, 5556);
  Udp.printf("WIFIBRIDGE %d.%d.%d.%d ETH", eth.localIP()[0], eth.localIP()[1], eth.localIP()[2], eth.localIP()[3]);
  Udp.endPacket();
#else
  struct station_info *stat_info;
  struct ip4_addr *IPaddress;
  stat_info = wifi_softap_get_station_info();
  while (stat_info != NULL)
  {
    IPaddress = &stat_info->ip;    
    Udp.beginPacket(IPaddress, 5556);
    Udp.printf("WIFIBRIDGE %d.%d.%d.%d WIFI", WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
    Udp.endPacket();
    stat_info = STAILQ_NEXT(stat_info, next);
  } 
#endif
}

void setup() {
  // start serial
  Serial.begin(115200);
  Serial.println();
  
  InitalizeFileSystem();  
  ReadConfig();
  Serial.println("Loaded " + String(machines.size()) + " machines");

  // start display
  displayUI.setup();

  // setup LED
  led::setup();
  led::setColor(0,100,0);
  
  pinMode(2, OUTPUT);

  wifi_setup();

#ifdef ETH_ENABLE
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(4000000);

  eth.config(IPAddress(192, 168, 178, 250), IPAddress(192, 168, 178, 1), IPAddress(255, 255, 255, 0));
  eth.setDefault(); // use ethernet for default route
  int present = eth.begin(mac);
  if (!present) {
    Serial.println("no ethernet hardware present");
    while(1) yield();
  }
  
  Serial.print("connecting ethernet");
  while (!eth.connected()) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("ethernet ip address: ");
  Serial.println(eth.localIP());
  network_ip = String(eth.localIP()[0]) + String(".") + String(eth.localIP()[1]) + String(".") + String(eth.localIP()[2]) + String(".") + String(eth.localIP()[3]);
  Serial.print("ethernet subnetMask: ");
  Serial.println(eth.subnetMask());
  Serial.print("ethernet gateway: ");
  Serial.println(eth.gatewayIP());
  setupFreedom();
#else
  setupAP();
  network_ip = String(WiFi.softAPIP()[0]) + String(".") + String(WiFi.softAPIP()[1]) + String(".") + String(WiFi.softAPIP()[2]) + String(".") + String(WiFi.softAPIP()[3]);
#endif
  
  timerIdUDP = timer.setInterval(5000, udpBroadcast);
  timer.enable(timerIdUDP);
  
  restServerRouting();
  server.begin();
}

void loop() {
  timer.run();
  led::update();
  displayUI.update();
  server.handleClient();
  processData();
}
