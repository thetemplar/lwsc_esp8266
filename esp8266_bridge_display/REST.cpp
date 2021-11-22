#include <ESP8266WebServer.h>

#include "REST.h"
#include "defines.h"

extern ESP8266WebServer server;

void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server @LWSC Display 0x00") + String(ESP.getChipId(), HEX));
    });
    server.on(F("/details"), HTTP_GET, rest_get_details);
    server.on(F("/fire"), HTTP_POST, rest_post_fire);
    server.on(F("/rssi"), HTTP_GET, rest_get_rssi);
}

void rest_get_details() {
    server.send(200, "text/json", "{\"id\": \"0x00" + String(ESP.getChipId(), HEX) + "\"}");
}

extern void fire(uint8_t type, uint32_t dest, uint8_t cmd);
extern struct WifiLog AppBuffer[255];
extern uint8_t AppBufferIndex;
void rest_post_fire() {
  if (server.arg("dest") == "" || server.arg("type") == "" || server.arg("cmd") == "" ){
    server.send(200, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t dest = server.arg("dest").toInt();
  uint8_t cmd = server.arg("cmd").toInt();
  uint8_t type = server.arg("type").toInt();
  fire((uint8_t)type, dest, (uint8_t)cmd);
  /*
  AppBuffer[AppBufferIndex].Id = dest;
  AppBuffer[AppBufferIndex].Cmd = cmd;
  AppBuffer[AppBufferIndex].Type = type;
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = 0x00;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;
  */
  server.send(200, "text/json", "{\"result\": \"success\", \"dest\": \"0x00" + String(dest, HEX) + "\", \"type\": \"0x00" + String(type, HEX) + "\", \"cmd\": \"0x00" + String(cmd, HEX) + "\"}");
}

extern std::vector<MachineData> machines;
extern std::map<uint32_t, uint8_t> machinesIndexCache;
void rest_get_rssi() {
  String message = "{\"result\": [";
  std::vector<MachineData>::iterator it;
  int i = 0;
  for (it = machines.begin(); it != machines.end(); it++)
  {
    message += "{\"id\": \" 0x00" + String(it->Id, HEX) + "\", \"name\": \"" + it->Name + "\", \"shortName\": \"" + it->ShortName + "\", \"rssi\": \"" + String(it->Rssi) + "\", \"SymbolX\": \"" + String(it->SymbolX) + "\", \"SymbolY\": \"" + String(it->SymbolY) + "\", \"rssiMap\": [";
    std::map<uint32_t, int8_t>::iterator it2;
    for (it2 = machines[i].RssiMap.begin(); it2 != machines[i].RssiMap.end(); it2++)
    {
      message += "{\"id\": \"" + String(it2->first) + "\", \"rssi\": \"" + String((int8_t)it2->second) + "\"},";
    }
    message += "]},";
    i++;
  }
  message += "]}";
  server.send(200, "text/json", message);
}
