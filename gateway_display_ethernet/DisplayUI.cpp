#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <map>
#include <vector>

#include "DisplayUI.h"

extern String network_ip;

extern struct WifiLog MachineBuffer[256];
extern uint8_t MachineBufferIndex;
extern std::vector<MachineData> machines;
extern std::map<uint32_t, uint8_t> machinesIndexCache;

// ===== adjustable ===== //
void DisplayUI::drawString(int x, int y, String str) {
    display.drawString(x, y, str);
}

void DisplayUI::drawString(int row, String str) {
    drawString(0, row * lineHeight, str);
}

void DisplayUI::drawLine(int x1, int y1, int x2, int y2) {
    display.drawLine(x1, y1, x2, y2);
}

String DisplayUI::IpToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

String DisplayUI::BufferToString(WifiLog entry)
{
  if(machinesIndexCache.count(entry.Id) == 0)
    return "";
    
  char tmp[30] = {0};

  char tmp2[10]  = {0};
  if(entry.Type == MSG_Fire)
    sprintf(tmp2, "%02X %d", entry.RelaisBitmask, entry.Duration/100);   
  
  if(machines[machinesIndexCache[entry.Id]].ShortName[0] != '?')
    sprintf(tmp, "%02X %8.8s %-4d %02X %s", entry.Seq, machines[machinesIndexCache[entry.Id]].ShortName, entry.Rssi, entry.Type, tmp2);
  else
    sprintf(tmp, "%02X %08X %-4d %02X %s", entry.Seq, entry.Id, entry.Rssi, entry.Type, tmp2);
  return String(tmp);
}

// ====================== //


DisplayUI::DisplayUI() {}

DisplayUI::~DisplayUI() {}

void DisplayUI::setup() {
    // initialize display
    display.init();

    /*
       In case of a compiler (conversion char/uint8_t) error,
       make sure to have version 4 of the display library installed
       https://github.com/ThingPulse/esp8266-oled-ssd1306/releases/tag/4.0.0
     */
    display.setFont(DejaVu_Sans_Mono_8);

    display.setContrast(255);

    display.flipScreenVertically();

    display.clear();
    display.display();
}

void DisplayUI::update(bool force) {
  if (force || ((millis() - drawTime > drawInterval))) {
    drawTime = millis();
    display.clear();
    
    drawString(0, " -  LWSC ETH BRIDGE  - ");
    drawString(1, "Heap " + String(ESP.getFreeHeap()/1024) + "k, Uptime " + String(millis()/60000) + " min");
    drawString(2, "Ethernet IP Address:");
    drawString(3, network_ip);
    drawString(4, "Last Command: ");
    drawString(5, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 1) % 256]));
  
    display.display();
  }
}

void DisplayUI::waitForETH()
{  
  display.clear();
  drawString(1, "Wait for ETH... ");   
  drawString(2, "  Network connected?");
  display.display();
}
