//#define DEBUG_ESP_HTTP_SERVER

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
//#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "DisplayUI.h"

#include "lwsc_lora.h"
#include "REST.h"
#include "src/SimpleTimer/SimpleTimer.h"

#include "LittleFS.h" // LittleFS is declared

#include <ENC28J60lwIP.h>
#define CSPIN 16
ENC28J60lwIP eth(CSPIN);
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

char lastMsg[400];
long lastMsgTimestamp = 0;

ESP8266WebServer server(80);
DisplayUI displayUI;
String network_ip;

SimpleTimer timer;

WiFiUDP Udp;

WiFiClient client;
HTTPClient http;
//NTPClient timeClient(Udp, "pool.ntp.org", 3600, 6000000);
int timerIdUDP;

MachineData machines[32];
User users[16];
int8_t machineCount;
FSInfo fs_info;
char packetBuffer[21];

uint64_t warning_remaining = 0;
uint64_t warning_last_sent = 0;
String warning_msg;

void udpBroadcast() {
  IPAddress broadcastIP(255, 255, 255, 255);
  Udp.beginPacket(broadcastIP, 5556);
  Udp.printf("WIFIBRIDGE %d.%d.%d.%d ETH", eth.localIP()[0], eth.localIP()[1], eth.localIP()[2], eth.localIP()[3]);
  Udp.endPacket();
}

void udpMsg(String msg) {
  String s = String("[+") + String(millis() - lastMsgTimestamp) + String("ms] ") + msg;
  
  Serial.println(s);
  IPAddress broadcastIP(255, 255, 255, 255);
  if(s.length() > 100) s = s.substring(0, 100);

  //http.begin(client, "http://lwsc-db.000webhostapp.com/dbwrite2.php");
  //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  //int httpCode = http.POST("sendval=" + String(s));
  //http.end();   
  
  Udp.beginPacket(broadcastIP, 5557);
  Udp.printf(s.c_str());
  Udp.endPacket();
  
  memmove(&lastMsg[s.length()+1], &lastMsg[0], 400 - (s.length() + 1));
  memcpy(&lastMsg[0], s.c_str(), s.length());
  //s.toCharArray(lastMsg, s.length());
  lastMsg[s.length()] = '\n';
  lastMsg[398] = 0x00;
  lastMsg[399] = 0x00;
  lastMsgTimestamp = millis();
}

bool InitalizeFileSystem() {
  bool initok = false;
  initok = LittleFS.begin();
  if (!(initok)) // Format SPIFS, of not formatted. - Try 1
  {
    udpMsg("LittleFS file system formatted.");
    LittleFS.format();
    initok = LittleFS.begin();
  }
  if (!(initok)) // format SPIFS. - Try 2
  {
    LittleFS.format();
    initok = LittleFS.begin();
  }
  if (initok)
  {
    udpMsg("LittleFS is OK");
  } else {
    udpMsg("LittleFS is not OK");
  }
  udpMsg("LittleFS Information:");
#ifdef ARDUINO_ARCH_ESP32
  // different methods of getting information
  Serial.print("Total bytes:    "); Serial.println(LittleFS.totalBytes());
  Serial.print("Used bytes:     "); Serial.println(LittleFS.usedBytes());
#else
  LittleFS.info(fs_info);
  udpMsg("Total bytes:    "); udpMsg(String(fs_info.totalBytes));
  udpMsg("Used bytes:     "); udpMsg(String(fs_info.usedBytes));
  udpMsg("Block size:     "); udpMsg(String(fs_info.blockSize));
  udpMsg("Page size:      "); udpMsg(String(fs_info.pageSize));
  udpMsg("Max open files: "); udpMsg(String(fs_info.maxOpenFiles));
  udpMsg("Max path length:"); udpMsg(String(fs_info.maxPathLength));
  udpMsg("---");
#endif

  return initok;
}

void ReadConfig()
{
  File configFile = LittleFS.open("/machines.conf", "r");
  if (!configFile)
  {
    udpMsg(F("Failed to open machines.conf"));
  } else {
    udpMsg(F("Opened machines.conf"));
    configFile.readBytes((char*)&machineCount, 1);
    udpMsg(F("readBytes len: ") + String(machineCount));
    for (int i = 0; i < machineCount; i++)
    {
      udpMsg(F("reading # ") + String(i));
      configFile.readBytes((char*)&machines[i].Name, 38);
      configFile.readBytes((char*)&machines[i].ShortName, 9);
      configFile.readBytes((char*)&machines[i].Disabled, 1);
      configFile.readBytes((char*)&machines[i].SymbolX, 4);
      configFile.readBytes((char*)&machines[i].SymbolY, 4);
      udpMsg(F("read: 0x00") + String(i, HEX));

      configFile.readBytes((char*)&machines[i].Functions[0].Name, 38);
      configFile.readBytes((char*)&machines[i].Functions[0].Duration, 4);
      configFile.readBytes((char*)&machines[i].Functions[0].RelaisBitmask, 1);
      configFile.readBytes((char*)&machines[i].Functions[0].SymbolX, 4);
      configFile.readBytes((char*)&machines[i].Functions[0].SymbolY, 4);
      configFile.readBytes((char*)&machines[i].Functions[0].Rotation, 1);

      configFile.readBytes((char*)&machines[i].Functions[1].Name, 38);
      configFile.readBytes((char*)&machines[i].Functions[1].Duration, 4);
      configFile.readBytes((char*)&machines[i].Functions[1].RelaisBitmask, 1);
      configFile.readBytes((char*)&machines[i].Functions[1].SymbolX, 4);
      configFile.readBytes((char*)&machines[i].Functions[1].SymbolY, 4);
      configFile.readBytes((char*)&machines[i].Functions[1].Rotation, 1);

      configFile.readBytes((char*)&machines[i].Functions[2].Name, 38);
      configFile.readBytes((char*)&machines[i].Functions[2].Duration, 4);
      configFile.readBytes((char*)&machines[i].Functions[2].RelaisBitmask, 1);
      configFile.readBytes((char*)&machines[i].Functions[2].SymbolX, 4);
      configFile.readBytes((char*)&machines[i].Functions[2].SymbolY, 4);
      configFile.readBytes((char*)&machines[i].Functions[2].Rotation, 1);

      configFile.readBytes((char*)&machines[i].Functions[3].Name, 38);
      configFile.readBytes((char*)&machines[i].Functions[3].Duration, 4);
      configFile.readBytes((char*)&machines[i].Functions[3].RelaisBitmask, 1);
      configFile.readBytes((char*)&machines[i].Functions[3].SymbolX, 4);
      configFile.readBytes((char*)&machines[i].Functions[3].SymbolY, 4);
      configFile.readBytes((char*)&machines[i].Functions[3].Rotation, 1);

      configFile.readBytes((char*)&machines[i].Functions[4].Name, 38);
      configFile.readBytes((char*)&machines[i].Functions[4].Duration, 4);
      configFile.readBytes((char*)&machines[i].Functions[4].RelaisBitmask, 1);
      configFile.readBytes((char*)&machines[i].Functions[4].SymbolX, 4);
      configFile.readBytes((char*)&machines[i].Functions[4].SymbolY, 4);
      configFile.readBytes((char*)&machines[i].Functions[4].Rotation, 1);

      udpMsg("added: " + String(machines[i].ShortName));
    }
  }
  configFile.close();
}

void WriteConfig()
{
  File configFile = LittleFS.open("/machines.conf", "w");
  if (!configFile)
  {
    udpMsg(F("Failed to open machines.conf"));
  } else {
    udpMsg(F("Opened machines.conf"));
    configFile.write((char*)&machineCount, 1);
    udpMsg(F("write len: ") + String(machineCount));
    for (int i = 0; i < machineCount; i++)
    {
      Serial.println(F("writing # ") + String(i));
      configFile.write((char*)&machines[i].Name, 38);
      configFile.write((char*)&machines[i].ShortName, 9);
      configFile.write((char*)&machines[i].Disabled, 1);
      configFile.write((char*)&machines[i].SymbolX, 4);
      configFile.write((char*)&machines[i].SymbolY, 4);
      udpMsg(F("write: 0x00") + String(i, HEX));

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

      udpMsg("written: " + String(machines[i].ShortName));
    }
  }
  configFile.close();
}

void setup() {
  pinMode(15, INPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  
  
  // start serial
  Serial.begin(115200);
  Serial.println();

  // start display
  displayUI.setup();
  
  WiFi.mode(WIFI_OFF);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(4000000);

  //eth.config(IPAddress(192, 168, 178, 250), IPAddress(192, 168, 178, 1), IPAddress(255, 255, 255, 0));
  eth.setDefault(); // use ethernet for default route
  int present = eth.begin(mac);
  if (!present) {
    Serial.println("no ethernet hardware present");
    while (1) yield();
  }

  Serial.print("connecting ethernet");
  displayUI.waitForETH();
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


  InitalizeFileSystem();
  ReadConfig();

  Wire.begin(4, 5);
  
  FSInfo fs_info;
  LittleFS.info(fs_info);    
  Serial.println(fs_info.usedBytes + "/" + fs_info.totalBytes);
  
  for(int i = 0; i < 16; i++)
  {
    for(int j = 0; j < 41; j++)
    {
      users[i].Name[j] = 0x00;
      users[i].Password[j] = 0x00;
    }
    users[i].Rights = (UserRights)0;
  }

  strncpy(users[0].Name, "Admin", 5);
  strncpy(users[0].Password, "lwsc-remote", 11);
  users[0].Rights = Admin;

  strncpy(users[1].Name, "Manager", 7);
  strncpy(users[1].Password, "lauterbach", 10);
  users[1].Rights = Saves;

  strncpy(users[2].Name, "User", 4);
  strncpy(users[2].Password, "lwsc", 4);
  users[2].Rights = Fire;
  
  //Serial.println("Loaded " + String(machines.size()) + " machines");


  pinMode(2, OUTPUT);

  lora_setup();
  
  timerIdUDP = timer.setInterval(1000, udpBroadcast);
  timer.enable(timerIdUDP);

  restServerRouting();
  server.begin();
  
  Udp.begin(5550);
  udpMsg("Listening on UDP port 5550");

  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  
  //timeClient.begin();
  //timeClient.forceUpdate();
  
  udpMsg("Hello everyone! I am " + String(eth.localIP()[0]) + "." + String(eth.localIP()[1]) + "." + String(eth.localIP()[2]) + "." + String(eth.localIP()[3]) + "!");
  udpMsg("Build Date: " + String(__DATE__) + " " + String(__TIME__));
  udpMsg("Loaded " + String(machineCount) + " machines");
}

extern uint64_t ackStart;
extern uint32_t ackTimeout;
extern uint32_t ackMachine;
void loop() {
  //uint32_t loopTime = millis();
  ArduinoOTA.handle();
  timer.run();
  displayUI.update();
  server.handleClient();
  //timeClient.update();
  lora_processData();

  if(millis() < warning_remaining && millis() > warning_last_sent + 2500)
  {
    IPAddress broadcastIP(255, 255, 255, 255);
    Udp.beginPacket(broadcastIP, 5558);
    Udp.printf(String("Warning active: '" + warning_msg + "'").c_str());
    Udp.endPacket();
    warning_last_sent = millis();
  }

  if (ackStart > 0 && millis() >= ackStart + ackTimeout)
  {
    server.send(200, "text/json", "{\"result\": \"no reply\", \"timeout\": \"" + String(ackTimeout) + "\"}");
    udpMsg("[SYSTEM] fire reply timout (" + String(ackMachine) + ")");
    ackStart = 0;
  }
  
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Udp.read(packetBuffer, sizeof(packetBuffer));
    packetBuffer[packetSize] = 0;
    udpMsg("[UDP] read: " + String(packetBuffer));
    if(packetSize == 4)
    {
      char id_c[2] = {packetBuffer[0], packetBuffer[1]};
      char f_id_c[2] = {packetBuffer[3]};
      uint32_t id = strtoul(id_c, NULL, 10);
      uint32_t f_id = strtoul(f_id_c, NULL, 10);
      //udpMsg("[UDP] fire at id: " + String(id) + " & f_id: " + String(f_id));
      
      if((machines[id].Functions[f_id].RelaisBitmask & 0x01) == 0x01) machines[id].Relais1Counter++;
      if((machines[id].Functions[f_id].RelaisBitmask & 0x02) == 0x02) machines[id].Relais2Counter++;
      
      //ackStart = millis();
      //ackTimeout = 350;
      //udpMsg("[UDP] wait for /ack");
      
      lora_fire(id, machines[id].Functions[f_id].Duration, machines[id].Functions[f_id].RelaisBitmask); 
    }
  }
  
  //Serial.println("looptime: " + String(millis() - loopTime));
  //delay(10);
}
