/* Code Written by Rishi Tiwari
 *  Website:- https://tricksumo.com
 *  https://www.instructables.com/How-to-Connect-NodeMCU-ESP8266-to-MySQL-Database-1/
*/



#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#define WIFI_SSID "lwsc_wifibridge_haupt"                               
#define WIFI_PASSWORD "winchester"

WiFiUDP UDP;
WiFiClient client;
HTTPClient http;
char packet[255];

void setup() {     
  delay(4000);
  Serial.begin(115200); 
  Serial.println("Communication Started \n\n");  
    
  
  pinMode(LED_BUILTIN, OUTPUT);     // initialize built in led on the board
   
  
  
  WiFi.mode(WIFI_STA);           
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());    //print local IP address
  
  delay(30);
  
  // Begin listening to UDP port
  UDP.begin(5557);
  Serial.print("Listening on UDP port 5557");
}



void loop() { 
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    int len = UDP.read(packet, 255);
    if (len > 0)
    {
      packet[len] = '\0';
    }
    
    http.begin(client, "http://lwsc-db.000webhostapp.com/dbwrite.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST("sendval=" + String(packet));
    http.end();    
  }
}
