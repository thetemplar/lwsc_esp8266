/* =====================
   This software is licensed under the MIT License:
   https://github.com/spacehuhntech/esp8266_deauther
   ===================== */

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "DisplayUI.h"
#include "A_config.h"

#include "led.h"
#include "lwsc_wifi.h"
#include "REST.h"
#include "src/SimpleTimer/SimpleTimer.h"


char packetBuffer[255];       //buffer to hold incoming packet 
byte ibuffer[100];

simplebutton::Button* resetButton;
ESP8266WebServer server(80);
DisplayUI displayUI;

const char* ssid = "lwsc_wifibridge_display";
const char* password = "lauterbach";
unsigned int udpPort = 5555;

WiFiUDP wifiUdp;

SimpleTimer timer;
int timerId;
int rssiOngoing;


void timer_query_rssi()
{
  Serial.println("timer_query_rssi");
  rssiOngoing--;
  Serial.println("rssiOngoing" + String(rssiOngoing));
  if (rssiOngoing == 0)
  {
    Serial.println("timer.disable(timerId)");
    timer.disable(timerId);
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
  timerId = timer.setInterval(7000, timer_query_rssi);
}

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

void setup() {
  // start serial
  Serial.begin(115200);
  Serial.println();
  
  InitalizeFileSystem();  
  ReadConfig();
  Serial.println("Loaded " + String(machines.size()) + " machines");

  // start display
  displayUI.setup();
  displayUI.mode = DISPLAY_MODE::MENU;

  // setup LED
  led::setup();
  led::setColor(0,100,0);
  
  pinMode(2, OUTPUT);

  wifi_setup();
  
  wifi_set_phy_mode(PHY_MODE_11B);
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);
  wifiUdp.begin(udpPort); 

  if (MDNS.begin("gateway")) {
    Serial.println("MDNS responder started @ gateway.local");
  }
  restServerRouting();
  server.begin();
}

void loop() {
  timer.run();
  led::update();   // update LED color
  displayUI.update();
  server.handleClient();

  /*
  int cb = wifiUdp.parsePacket();
  if (cb == 6) {
    Serial.printf("[wifi] package: %02X %02X %02X %02X %02X %02X \n", packetBuffer[0], packetBuffer[1], packetBuffer[2], packetBuffer[3], packetBuffer[4], packetBuffer[5]);
    wifiUdp.read(packetBuffer, cb);   
    readWifi((char*)&packetBuffer[0], cb);
  } else if (cb) {    
    Serial.printf("[wifi] length = %i\n", cb);
  }
  delay(1);*/
}
