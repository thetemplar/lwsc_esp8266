#include <Wire.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include "SimpleTimer.h"


const uint8_t chip_id = 0x13;


SimpleTimer timer;
int checkTimer;

uint8_t buf[128];

void lora_setup()
{
  LoRa.setPins(2, 16, 15);
    
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  LoRa.setSyncWord(0xF3);


  LoRa.setSpreadingFactor(10);
  LoRa.enableCrc();
}


void lora_processData()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //read packet
    memset(buf, 0, sizeof(buf));
    int i = 0;
    while (LoRa.available()) {
      buf[i] = LoRa.read();
      i++;
    }

    if(buf[0] == 0xff || buf[0] == chip_id)
    {
      Serial.println("lora_processData: got data!");
      int current_rssi = LoRa.packetRssi();
      int8_t rounded_snr = LoRa.packetSnr();
      
      switch (buf[1])
      {
        case 0x03: //blink
          {
            Serial.println("blink");
            LoRa.beginPacket();
            LoRa.write(0x00);
            LoRa.write(0x02);
            LoRa.write(chip_id);
            LoRa.write((current_rssi & 0xFF));
            LoRa.write(((current_rssi >> 8) & 0xFF));
            LoRa.write(rounded_snr);
            LoRa.endPacket();
            digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
            delay(700);                       // wait for a second
            digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
            delay(700);                       // wait for a second
            digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
            delay(700);                       // wait for a second
            digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
          }
          break;
        case 0x04: //Reboot
          {
            Serial.println("Reboot");
            delay(100);
            ESP.restart();
          }
          break;
        case 0x20: //RequestRssi
          {
            Serial.println("RequestRssi");
            LoRa.beginPacket();
            LoRa.write(0x00);
            LoRa.write(0x02);
            LoRa.write(chip_id);
            LoRa.write((current_rssi & 0xFF));
            LoRa.write(((current_rssi >> 8) & 0xFF));
            LoRa.write(rounded_snr);
            LoRa.endPacket();
          }
          break;
      }
    }
  }
}

void checkTimerCb()
{  
  Serial.println("checkTimerCb");
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(25);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
  
  if (digitalRead(4) == HIGH) {
    Serial.println(" > Sicherheit! <");
    LoRa.beginPacket();
    LoRa.write(0x00);
    LoRa.write(0xE0);
    LoRa.endPacket();
    Serial.println("Lora send complete.");
  }
}

void setup() {
  WiFi.mode(WIFI_OFF);
  delay(2000);
  Serial.begin(115200);
  Serial.println("Booting");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(4, INPUT);
  lora_setup();
  
  checkTimer = timer.setInterval(2500, checkTimerCb);
  timer.enable(checkTimer);
}

void loop() {  
  timer.run();
  lora_processData();
}
