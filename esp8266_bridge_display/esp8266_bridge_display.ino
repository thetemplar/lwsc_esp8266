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

char packetBuffer[255];       //buffer to hold incoming packet

simplebutton::Button* resetButton;
ESP8266WebServer server(80);
DisplayUI displayUI;

const char* ssid = "lwsc_wifibridge_display";
const char* password = "lauterbach";
unsigned int udpPort = 5555;

WiFiUDP wifiUdp;


void setup() {
  // start serial
  Serial.begin(115200);
  Serial.println();

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
