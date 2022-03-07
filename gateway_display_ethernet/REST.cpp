#include <ESP8266WebServer.h>

#include "REST.h"
#include "defines.h"
#include "lwsc_lora.h"


#include "LittleFS.h" // LittleFS is declared

#include <ArduinoJson.h> //3rd Party Lib! Version 6.x.x

File fsUploadFile;

extern ESP8266WebServer server;
extern void ReadConfig();
extern void WriteConfig();
extern void udpMsg(String msg);

extern MachineData machines[256];
extern int8_t machineCount;

uint64_t ackStart;
uint32_t ackTimeout;
uint32_t ackId;

void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server @LWSC Display 0x00") + String(ESP.getChipId(), HEX) + " - " + millis() + "ms - Heap: " + ESP.getFreeHeap() + F("<br><a href='/web'>Web Fernbedienung</a>"));
    });
    server.on(F("/web"), HTTP_GET, web_interface);
    server.on(F("/save_config"), HTTP_POST, rest_post_save_config);
    server.on(F("/machine_count"), HTTP_GET, rest_get_machine_count);
    server.on(F("/machine"), HTTP_GET, rest_get_machine);
    server.on(F("/machine"), HTTP_POST, rest_post_machine);
    server.on(F("/last_msg"), HTTP_GET, rest_get_last_msg);
    server.on(F("/function"), HTTP_POST, rest_post_function);
    server.on(F("/fire"), HTTP_POST, rest_post_fire);
    server.on(F("/fire"), HTTP_GET, rest_post_fire);
    server.on(F("/force_fire"), HTTP_POST, rest_force_fire);
    server.on(F("/force_fire"), HTTP_GET, rest_force_fire);
    server.on(F("/blink"), HTTP_POST, rest_post_blink);
    server.on(F("/file"), HTTP_GET, rest_get_file);  
    server.on(F("/file_list"), HTTP_GET, rest_get_file_list);  
    server.on(F("/upload"), HTTP_POST, [](){ server.send(200); }, rest_upload_handler );
    server.on(F("/all_functions"), HTTP_GET, rest_get_all_functions);
    server.on(F("/set_relaiscounter"), HTTP_POST, rest_post_set_relaiscounter);
    server.on(F("/reboot"), HTTP_POST, rest_post_reboot);
    server.on(F("/version"), HTTP_GET, rest_get_version);
}
void setCrossOrigin(){
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("POST,GET"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

void web_interface() {
  setCrossOrigin();
  String head = F("<!DOCTYPE html> "\
                  "<html> "\
                  "   <head> "\
                  "      <title>LWSC</title> "\
                  "      <script> "\
                  "        function clearAll() { "\
                  "            document.getElementById('lastFire').innerHTML = ''; "\
                  "        } "\
                  "         "\
                  "        function fire(j, mid, fid) { "\
                  "            if (j) { "\
                  "                var last = document.getElementById(j); "\
                  "                var copyOfLast = last.cloneNode(true); "\
                  "                copyOfLast.setAttribute('onClick', 'fire(null, ' + mid + ', ' + fid + ')'); "\
                  "                document.getElementById('lastFire').appendChild(copyOfLast); "\
                  "            } "\
                  "            const request = new XMLHttpRequest(); "\
                  "            const url = '/fire?id=' + mid + '&f_id=' + fid; "\
                  "            request.onload  = function() { "\
                  "                var jsonResponse = JSON.parse(request.responseText); "\
                  "                if(jsonResponse.result == 'success') "\
                  "                { "\
                  "                    const canVibrate = window.navigator.vibrate; "\
                  "              if (canVibrate) {window.navigator.vibrate(100);} "\
                  "                    if (j) {document.getElementById(j).style.backgroundColor = \"#768d87\";} "\
                  "                } else  "\
                  "                { "\
                  "                  if (j) {document.getElementById(j).style.backgroundColor = \"#661e1e\";} "\
                  "                } "\
                  "            }; "\
                  "            request.open('GET', url); "\
                  "            request.send();   "\
                  "        } "\
                  "      </script>  "\
                  "      <style type='text/css'>"\	
                  "        html,"\
                  "        body {"\
                  "          overflow-x: hidden;"\
                  "        }"\
                  "        "\
                  "        body {"\
                  "          position: relative;"\
                  "        }"\
                  "        "\
                  "        .myButton {"\
                  "          box-shadow: inset 0px 10px 40px 10px #91b8b3;"\
                  "          background-color: #768d87;"\
                  "          border-radius: 20px;"\
                  "          border: 6px solid #566963;"\
                  "          display: inline-block;"\
                  "          cursor: pointer;"\
                  "          color: #ffffff;"\
                  "          font-family: Arial;"\
                  "          font-size: 5em;"\
                  "          font-weight: bold;"\
                  "          padding: 7px 50px;"\
                  "          text-decoration: none;"\
                  "          width: 80%;"\
                  "          text-align: center;"\
                  "          margin: 15px;"\
                  "        }"\
                  "        "\
                  "        .myButton:hover {"\
                  "          background-color: #6c7c7c;"\
                  "        }"\
                  "        "\
                  "        .myButton:active {"\
                  "          position: relative;"\
                  "          top: 1px;"\
                  "        }"\
                  "      </style> "\
                  "   </head> "\
                  "   <body> "\
                  "      <div id=\"lastFire\" style='width:100%;'></div> "\
                  "      <div style='width:100%'> "\
                  "      <a class=\"myButton\" style='background-color: #211cb9; border: 6px solid #1d1b66;' onclick=\"clearAll()\">Clear</a><br> "\
                  "");
  String message = "";
  //int j = 0;
  for (int i = 0; i < machineCount; i++)
  {
    MachineData* it = &machines[i];
    if (it->Disabled == 1)
      continue;
      
    for (int j = 0; j < 5; j++)
    {
      if(it->Functions[j].RelaisBitmask > 0x00)
      { 
        message += String("<a id=\"machine_") + String(i) + "\" class=\"myButton\" onclick=\"fire('machine_" + String(i) + "', " + String(i) + ", " + String(j) + String(")\">") + String(it->Functions[j].Name) + String("</a><br>");
        //j++;
      }
    }
  }  
  message += "</div></body></html>";
  server.send(200, "text/html", head + message);
}

void rest_get_version() {
  setCrossOrigin();
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  lora_get_version(id);
  server.send(200, "text/json", "{}");
}

void rest_post_save_config() {     
  setCrossOrigin();
  WriteConfig();
  server.send(200, "text/json", "{\"result\": \"success\"}");
}

extern char lastMsg[400];
extern uint32_t fireCounter;
void rest_get_last_msg() {     
  setCrossOrigin();
  server.send(200, "text/json", "{\"result\": \"" + String(lastMsg) + "\", \"firecounter\": \"" + String(fireCounter) + "\"}");
}

void rest_get_machine_count()
{
  setCrossOrigin();
  server.send(200, "text/json", "{\"count\": \"" + String(machineCount) + "\"}");
}

void rest_get_machine()
{  
  setCrossOrigin();
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }

  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  
  if(machineCount == 0 || id >= machineCount)
  {
    server.send(404, "text/json", "{\"result\": \"no data\"}");
    return;
  }
  
  MachineData* md = &machines[id];  
  
  String message = "";
  DynamicJsonDocument doc(2048);
  doc["id"] = id;
  doc["name"] = md->Name;
  doc["shortName"] = md->ShortName;
  doc["disabled"] = md->Disabled;
  doc["symbolX"] = md->SymbolX;
  doc["symbolY"] = md->SymbolY;
  doc["relais1Counter"] = md->Relais1Counter;
  doc["relais2Counter"] = md->Relais2Counter;
  
  for (int f = 0; f < 5; f++)
  {
    if(md->Functions[f].RelaisBitmask > 0x00 || server.arg("force") != "")
    {    
      doc["functions"][f]["functionId"] = f;
      doc["functions"][f]["machineId"] = id;
      doc["functions"][f]["name"] = md->Functions[f].Name;
      doc["functions"][f]["duration"] = md->Functions[f].Duration;
      doc["functions"][f]["RelaisBitmask"] = md->Functions[f].RelaisBitmask;
      doc["functions"][f]["symbolX"] = md->Functions[f].SymbolX;
      doc["functions"][f]["symbolY"] = md->Functions[f].SymbolY;
      doc["functions"][f]["rotation"] = md->Functions[f].Rotation;
    }
  }
  serializeJson(doc, message);
  server.send(200, "text/json", message);
}

void rest_post_machine() {
  setCrossOrigin();
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  if(id >= 64)
  {
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  Serial.println("rest_post_machine: " + String(id));
  
  String s = "changed";
  MachineData* md;
  if(id >= machineCount)
  {
    s = "added";
    memset(&machines[id].Name, 0x00, 38);
    memset(&machines[id].ShortName, 0x00, 9);
    for(int i = 0; i < 5; i++)
    {
      memset(&machines[id].Functions[i].Name, 0x00, 38);
      machines[id].Functions[i].Duration = 0;
      machines[id].Functions[i].RelaisBitmask = 0;
      machines[id].Functions[i].SymbolX = 0;
      machines[id].Functions[i].SymbolY = 0;
      machines[id].Functions[i].Rotation = 0;
    }
    machineCount = id + 1;
    Serial.println("New Machine: 0x00" + String(id, HEX));
  }

  if(server.arg("name") != "")
  {
    for(int i = 0; i < 37; i++)
    {
      if(i < server.arg("name").length())
        machines[id].Name[i] = server.arg("name")[i];
      else
        machines[id].Name[i] = 0x00;
    }
  }
    
  if(server.arg("shortName") != "")
  {
    for(int i = 0; i < 8; i++)
    {
      if(i < server.arg("shortName").length())
        machines[id].ShortName[i] = server.arg("shortName")[i];
      else
        machines[id].ShortName[i] = 0x00;
    }
  }

  if(server.arg("disabled") != "")
    machines[id].Disabled = server.arg("disabled").toInt();
  if(server.arg("symbolX") != "")
    machines[id].SymbolX = server.arg("symbolX").toInt();
  if(server.arg("symbolY") != "")
    machines[id].SymbolY = server.arg("symbolY").toInt();
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"" + s + "\"}");
}

void rest_post_function() {
  setCrossOrigin();
  if (server.arg("id") == "" || server.arg("f_id") == "" || server.arg("name") == "" || server.arg("duration") == "" || server.arg("relaisBitmask") == "" || server.arg("symbolX") == "" || server.arg("symbolY") == "" || server.arg("rotation") == ""){
    server.send(200, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t f_id = strtoul(server.arg("f_id").c_str(), NULL, 10);

  String s = "changed";
  if(machines[id].Functions[f_id].RelaisBitmask == 0x00)
  {
    s = "added";
  } else if (server.arg("relaisBitmask").toInt() == 0x00) {
    s = "removed";
  }
    
  for(int i = 0; i < 37; i++)
  {
    if(i < server.arg("name").length())
      machines[id].Functions[f_id].Name[i] = server.arg("name")[i];
    else
      machines[id].Functions[f_id].Name[i] = 0x00;
  }
  
  machines[id].Functions[f_id].Duration = server.arg("duration").toInt();
  machines[id].Functions[f_id].RelaisBitmask = server.arg("relaisBitmask").toInt();
  machines[id].Functions[f_id].SymbolX = server.arg("symbolX").toInt();
  machines[id].Functions[f_id].SymbolY = server.arg("symbolY").toInt();
  machines[id].Functions[f_id].Rotation = server.arg("rotation").toInt();
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"" + s + "\", \"id\": \"" + String(id) + "\", \"f_id\": \"" + String(f_id) + "\"}");
}

void rest_force_fire()
{
  if (server.arg("id") == "" || server.arg("duration") == ""  || server.arg("bitmask") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t duration = strtoul(server.arg("duration").c_str(), NULL, 10);
  uint32_t bitMask = strtoul(server.arg("bitmask").c_str(), NULL, 10);
  lora_fire(id, duration, bitMask); 
  server.send(200, "text/json", "{\"result\": \"ok\"}");
}


void IRAM_ATTR rest_post_fire() {
  setCrossOrigin();
  if (server.arg("id") == "" || server.arg("f_id") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t f_id = strtoul(server.arg("f_id").c_str(), NULL, 10);

  if(machines[id].Functions[f_id].RelaisBitmask & 0x01 == 0x01) machines[id].Relais1Counter++;
  if(machines[id].Functions[f_id].RelaisBitmask & 0x02 == 0x02) machines[id].Relais2Counter++;
  lora_fire(id, machines[id].Functions[f_id].Duration, machines[id].Functions[f_id].RelaisBitmask);
  
  ackStart = millis();
  ackTimeout = 350;
  ackId = id;

  //no server.send -> in processWiFiData()->ack!
}

void rest_post_blink() {
  setCrossOrigin();
   if (server.arg("id") == "lora"){
    lora_blink(0xff);
    server.send(200, "text/json", "{\"result\": \"lora\"}");
    return;
  }
   if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  lora_blink(id);
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
}

void rest_get_file() {
  setCrossOrigin();
  if (server.arg("filename") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  File file = LittleFS.open("/" + server.arg("filename"), "r");
  size_t sent = server.streamFile(file, "application/octet-stream");
  file.close();
}

void rest_get_file_list() {
  String message = "[";
  File root = LittleFS.open("/", "r");
 
  File file = root.openNextFile();
 
  while(file){
 
      Serial.print("FILE: ");
      message += String(file.name()) + ",";
 
      file = root.openNextFile();
  }
  message += "]";
  
  server.send(200, "text/plain", message);
}

void rest_upload_handler(){ // upload a new file to the LittleFS  
  setCrossOrigin();
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START)
  {
    Serial.println("UPLOAD_FILE_START");
    String filename = upload.filename;
    if(!filename.startsWith("/")) 
    {
    filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: "); 
    Serial.println(filename);
    fsUploadFile = LittleFS.open(filename, "w");            // Open the file for writing in LittleFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    Serial.println("UPLOAD_FILE_WRITE");
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    Serial.println("UPLOAD_FILE_END");
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); 
      Serial.println(upload.totalSize);
      server.send(200);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void rest_get_all_functions() {
  setCrossOrigin();
  String message = "";
  DynamicJsonDocument doc(8192);
  int ff = 0;
  for (int i = 0; i < machineCount; i++)
  {
    if (machines[i].Disabled == 1)
      continue;
      
    for (int f = 0; f < 5; f++)
    {
      if(machines[i].Functions[f].RelaisBitmask > 0x00)
      {
        //Serial.print("rest_get_all_functions: " + String(machines[i].Functions[f].Name) + " - " + String(i) + " - " + String(f) + "\r\n"); 
        doc[ff]["functionId"] = f;
        doc[ff]["machineId"] = i;
        doc[ff]["name"] = machines[i].Functions[f].Name;
        ff++;
      }
    }
  }
  serializeJson(doc, message);
  server.send(200, "text/json", message);
}

void rest_post_set_relaiscounter() {
  setCrossOrigin();
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }

  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  
  MachineData* md = &machines[id];  

  if(server.arg("relais1Counter") != "")
  {
    md->Relais1Counter = server.arg("relais1Counter").toInt();
  }
  if(server.arg("relais2Counter") != "")
  {
    md->Relais2Counter = server.arg("relais2Counter").toInt();
  }
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
}

void rest_post_reboot() {
  setCrossOrigin();
   if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  if(id == 0)
  {
    server.send(200, "text/json", "{\"result\": \"success\"}");
    udpMsg("REBOOT");
    delay(10);
    ESP.restart();
    return;
  }
  
  lora_reboot(id);
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
}
