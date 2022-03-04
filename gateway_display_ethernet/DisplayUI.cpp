#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <map>
#include <vector>

#include "DisplayUI.h"

extern String network_ip;

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

int loopDrawIndex = 0;
void DisplayUI::update(bool force) {
  if (force || ((millis() - drawTime > drawInterval))) {
    drawTime = millis();
    display.clear();
    
    drawString(0, " -  LWSC ETH BRIDGE  - ");
    drawString(1, "Heap " + String(ESP.getFreeHeap()/1024) + "k, Uptime " + String(millis()/60000) + " min");
    drawString(2, "Ethernet IP Address:");
    drawString(3, network_ip);
    if(loopDrawIndex == 0)
      drawString(5, "Machine Count: " + machineCount);
    else
      drawString(5, "[" + String(loopDrawIndex) + "] " + String(machines[loopDrawIndex].ShortName) + " " + String(machines[loopDrawIndex].Rssi));
  
    display.display();

    loopDrawIndex++;
    if (loopDrawIndex >= machineCount)
      loopDrawIndex = 0;
  }
}

void DisplayUI::waitForETH()
{  
  display.clear();
  drawString(1, "Wait for ETH... ");   
  drawString(2, "  Network connected?");
  display.display();
}
