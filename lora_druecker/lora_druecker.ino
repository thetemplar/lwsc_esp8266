/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/ttgo-lora32-sx1276-arduino-ide/
*********/

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoOTA.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include "Keypad_I2C.h"
#include "Keypad.h"


//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 868E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

const char* ssid = "lwsc_wifibridge_haupt";
const char* password = "winchester";

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'0', '1', '2'},
  {'3', '4', '5'},
  {'6', '7', '8'},
  {'9', 'A', 'B'}
};
String machines[ROWS][COLS] = {
  {"", "", ""},
  {"", "", ""},
  {"", "", ""},
  {"", "", ""}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad_I2C customKeypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, 0x20);

String selectedPad = "";

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don"t proceed, loop forever
  }

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  customKeypad.begin();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA LWSC");
  display.display();

  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0, 10);
  display.println("LoRa Initializing OK!");
  display.display();
  LoRa.setSyncWord(0xF3);

  LoRa.setSpreadingFactor(10);
  delay(2000);
}

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
}

void loop() {

  if (selectedPad == "")
  {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      selectedPad = "pad_druecker";
      String serverPath = "http://lwsc.ddns.net:8280/file?filename=" + selectedPad + "&username=User&password=lwsc";

      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        int l = strlen(payload.c_str());

        int i = 0;
        const char s[2] = ";";
        char *token;
        char str[l + 1];
        str[l] = 0x00;
        memcpy(&str, &((payload.c_str()))[0], l);
        Serial.println(str);
        token = strtok(str, s);
        while ( token != NULL ) {
          Serial.printf(" %s\n", token);
          if (i == 2) machines[0][0] = String(token);
          else if (i == 3) machines[0][1] = String(token);
          else if (i == 4) machines[0][2] = String(token);
          else if (i == 5) machines[1][0] = String(token);
          else if (i == 6) machines[1][1] = String(token);
          else if (i == 7) machines[1][2] = String(token);
          else if (i == 8) machines[2][0] = String(token);
          else if (i == 9) machines[2][1] = String(token);
          else if (i == 10) machines[2][2] = String(token);
          else if (i == 11) machines[3][0] = String(token);
          else if (i == 13) machines[3][1] = String(token);
          else if (i == 14) machines[3][2] = String(token);
          i++;

          token = strtok(NULL, s);
        }

        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("loaded: ");
        display.setCursor(2, 0);
        display.print(selectedPad);
        display.display();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LORA RECEIVER ");
    display.setCursor(0, 0);
    display.print("  ..loading");
    display.display();

  }


  char customKey = customKeypad.getKey();

  if (customKey != NO_KEY) {
    Serial.println(customKey);

    char t[2] = {0x00, 0x00};
    t[0] = customKey;
    int ic = StrToHex(t);
    Serial.println(ic);

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(selectedPad);
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(machines[ic / 3][ic % 3]);
    display.display();
  }

}
