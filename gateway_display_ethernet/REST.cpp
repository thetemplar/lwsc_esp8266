#include <ESP8266WebServer.h>

#include "REST.h"
#include "defines.h"
#include "lwsc_wifi.h"
#include "lwsc_lora.h"


#include "LittleFS.h" // LittleFS is declared

#include <ArduinoJson.h> //3rd Party Lib! Version 6.x.x

File fsUploadFile;

extern ESP8266WebServer server;
extern std::vector<MachineData> machines;
extern std::map<uint32_t, uint8_t> machinesIndexCache;
extern struct WifiLog AppBuffer[256];
extern uint8_t AppBufferIndex;
extern struct WifiLog MachineBuffer[256];
extern uint8_t MachineBufferIndex;
extern void ReadConfig();
extern void WriteConfig();

extern void reqRssi(uint32_t dest);

uint64_t ackStart;
uint32_t ackTimeout;
uint16_t ackSeq;
uint32_t ackId;

void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server @LWSC Display 0x00") + String(ESP.getChipId(), HEX) + " - " + millis() + "ms - Heap: " + ESP.getFreeHeap() + F("<br><a href='\web'>Web Fernbedienung</a>"));
    });
    server.on(F("/web"), HTTP_GET, web_interface);
    server.on(F("/host"), HTTP_GET, rest_get_host);
    server.on(F("/save_config"), HTTP_POST, rest_post_save_config);
    server.on(F("/machine_count"), HTTP_GET, rest_get_machine_count);
    server.on(F("/machine"), HTTP_GET, rest_get_machine);
    server.on(F("/machine"), HTTP_POST, rest_post_machine);
    server.on(F("/machine"), HTTP_DELETE, rest_delete_machine);
    server.on(F("/machine_rssi"), HTTP_GET, rest_get_machine_rssi);
    server.on(F("/function"), HTTP_POST, rest_post_function);
    server.on(F("/fire"), HTTP_POST, rest_post_fire);
    server.on(F("/fire"), HTTP_GET, rest_post_fire);
    server.on(F("/force_fire"), HTTP_POST, rest_force_fire);
    server.on(F("/force_fire"), HTTP_GET, rest_force_fire);
    server.on(F("/stm32_set_id"), HTTP_GET, rest_post_stm32_set_id);
    server.on(F("/blink"), HTTP_POST, rest_post_blink);
    server.on(F("/change_id"), HTTP_POST, rest_post_change_id);
    server.on(F("/file"), HTTP_GET, rest_get_file);  
    server.on(F("/upload"), HTTP_POST, [](){ server.send(200); }, rest_upload_handler );
    server.on(F("/query_rssi"), HTTP_POST, rest_post_query_rssi);
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
  String head = F("<!DOCTYPE html> <html> <head> <title>LWSC</title>  <script> function clearAll() { document.getElementById('lastFire').innerHTML = ''; } function fire(j, mid, fid) { if (j) { var last = document.getElementById(j); var copyOfLast = last.cloneNode(true); copyOfLast.setAttribute( 'onClick', 'fire(null, ' + mid + ', ' + fid + ')' ); document.getElementById('lastFire').appendChild(copyOfLast); } const request = new XMLHttpRequest(); const url = '/fire?id='+mid+'&f_id='+fid; request.open('GET', url); request.send();}</script> <style type='text/css'> html, body { overflow-x: hidden; } body { position: relative; } .myButton { box-shadow:inset 0px 10px 40px 10px #91b8b3; background-color:#768d87; border-radius:20px; border:6px solid #566963; display:inline-block; cursor:pointer; color:#ffffff; font-family:Arial; font-size:5em; font-weight:bold; padding:7px 50px; text-decoration:none; width: 80%; text-align:center; margin: 15px; } .myButton:hover { background-color:#6c7c7c; } .myButton:active { position:relative; top:1px; } </style> </head> <body><div id=\"lastFire\" style='width:100%;'></div><div style='width:100%'><a class=\"myButton\" style='background-color: #211cb9; border: 6px solid #1d1b66;' onclick=\"clearAll()\">Clear</a><br>");
  String message = "";
  std::vector<MachineData>::iterator it;
  int j = 0;
  for (it = machines.begin(); it != machines.end(); it++)
  {
    if (it->Disabled == 1)
      continue;
      
    for (int i = 0; i < 5; i++)
    {
      if(it->Functions[i].RelaisBitmask > 0x00)
      { 
        message += String("<a id=\"machine_") + String(j) + "\" class=\"myButton\" onclick=\"fire('machine_" + String(j) + "', " + String(it->Id) + ", " + String(i) + String(")\">") + String(it->Functions[i].Name) + String("</a><br>");
        j++;
      }
    }
  }  
  message += "</div></body></html>";
  server.send(200, "text/html", head + message);
}

void rest_get_host() {
  setCrossOrigin();
  server.send(200, "text/json", "{\"id\": \"0x00" + String(ESP.getChipId(), HEX) + "\"}");
}

void rest_get_version() {
  setCrossOrigin();
  if (server.arg("id") == "" && server.arg("it") == ""){
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

void rest_get_machine_count()
{
  setCrossOrigin();
  server.send(200, "text/json", "{\"count\": \"" + String(machines.size()) + "\"}");
}

void rest_get_machine()
{  
  setCrossOrigin();
  if (server.arg("id") == "" && server.arg("it") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }

  int i = 0;
  if(server.arg("it") == "")
  {
    uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
    if(machinesIndexCache.count(id) == 0)
    {
      server.send(404, "text/json", "{\"result\": \"not found\"}");
      return;
    }
    i = machinesIndexCache[id];
  } else {
    i = server.arg("it").toInt();
    if (i < 0 || i > machines.size() - 1) {
      server.send(404, "text/json", "{\"result\": \"not found\"}");
      return;
    }
  }
  
  if(machines.size() == 0 || machinesIndexCache.size() == 0)
  {
    server.send(404, "text/json", "{\"result\": \"no data\"}");
    return;
  }
  
  MachineData* md = &machines[i];  
  
  String message = "";
  DynamicJsonDocument doc(2048);
  doc["id"] = md->Id;
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
      doc["functions"][f]["machineId"] = md->Id;
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
  
  String s = "changed";
  MachineData* md;
  if(machinesIndexCache.count(id) == 0)
  {
    s = "added";
    md = new MachineData;
    memset(&md->Name, 0x00, 38);
    memset(&md->ShortName, 0x00, 9);
    for(int i = 0; i < 5; i++)
    {
      memset(&md->Functions[i].Name, 0x00, 38);
      md->Functions[i].Duration = 0;
      md->Functions[i].RelaisBitmask = 0;
      md->Functions[i].SymbolX = 0;
      md->Functions[i].SymbolY = 0;
      md->Functions[i].Rotation = 0;
    }
    md->Id = id;
    machines.push_back(*md);  
    machinesIndexCache[id] = machines.size() - 1;
    Serial.println("New Machine: 0x00" + String(id, HEX));
  }
  
  md = &machines[machinesIndexCache[id]];

  if(server.arg("name") != "")
  {
    for(int i = 0; i < 37; i++)
    {
      if(i < server.arg("name").length())
        md->Name[i] = server.arg("name")[i];
      else
        md->Name[i] = 0x00;
    }
  }
    
  if(server.arg("shortName") != "")
  {
    for(int i = 0; i < 8; i++)
    {
      if(i < server.arg("shortName").length())
        md->ShortName[i] = server.arg("shortName")[i];
      else
        md->ShortName[i] = 0x00;
    }
  }

  if(server.arg("disabled") != "")
    md->Disabled = server.arg("disabled").toInt();
  if(server.arg("symbolX") != "")
    md->SymbolX = server.arg("symbolX").toInt();
  if(server.arg("symbolY") != "")
    md->SymbolY = server.arg("symbolY").toInt();
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"" + s + "\"}");
}

void rest_delete_machine() {
  setCrossOrigin();
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  
  std::vector<MachineData>::iterator it;
  int i = 0;
  for (it = machines.begin(); it != machines.end(); it++)
  {
    if(it->Id == id)
    {
      machines.erase(it);
      machinesIndexCache.erase(id);
      server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"delete\"}");
      return;
    }
    i++;
  }
  
  server.send(200, "text/json", "{\"result\": \"fail\"}");
}

void rest_post_query_rssi() {  
  setCrossOrigin();  
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
  }

  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  if(machinesIndexCache.count(id) == 0)
  {
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  reqRssi(id); 

  server.send(200, "text/json", "{\"result\": \"success\"}");
}

void rest_get_machine_rssi() {
  setCrossOrigin();
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  if(machinesIndexCache.count(id) == 0)
  {
    server.send(404, "text/json", "{\"result\": \"not existing\"}");
  }  
  
  MachineData* md;
  md = &machines[machinesIndexCache[id]];

  String message = "";
  DynamicJsonDocument doc(1024);
  int i = 0;
  doc["id"] = md->Id;
  doc["rssi"] = md->Rssi;
  std::map<uint32_t, int8_t>::iterator it;
  for (it = md-> RssiMap.begin(); it != md->RssiMap.end(); it++)
  {
    doc["rssiMap"][i]["id"] = it->first;
    doc["rssiMap"][i]["rssi"] = it->second;
    i++;
  }
  serializeJson(doc, message);
  server.send(200, "text/json", message);
}

void rest_post_function() {
  setCrossOrigin();
  if (server.arg("id") == "" || server.arg("f_id") == "" || server.arg("name") == "" || server.arg("duration") == "" || server.arg("relaisBitmask") == "" || server.arg("symbolX") == "" || server.arg("symbolY") == "" || server.arg("rotation") == ""){
    server.send(200, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t f_id = strtoul(server.arg("f_id").c_str(), NULL, 10);
  if(machinesIndexCache.count(id) == 0 || f_id >= 5){
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }

  MachineData* md;
  md = &machines[machinesIndexCache[id]];
  String s = "changed";
  if(md->Functions[f_id].RelaisBitmask == 0x00)
  {
    s = "added";
  } else if (server.arg("relaisBitmask").toInt() == 0x00) {
    s = "removed";
  }
    
  for(int i = 0; i < 37; i++)
  {
    if(i < server.arg("name").length())
      md->Functions[f_id].Name[i] = server.arg("name")[i];
    else
      md->Functions[f_id].Name[i] = 0x00;
  }
  
  md->Functions[f_id].Duration = server.arg("duration").toInt();
  md->Functions[f_id].RelaisBitmask = server.arg("relaisBitmask").toInt();
  md->Functions[f_id].SymbolX = server.arg("symbolX").toInt();
  md->Functions[f_id].SymbolY = server.arg("symbolY").toInt();
  md->Functions[f_id].Rotation = server.arg("rotation").toInt();
  
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
  server.send(400, "text/json", "{\"result\": \"ok\"}");
}


void IRAM_ATTR rest_post_fire() {
  setCrossOrigin();
  if (server.arg("id") == "" || server.arg("f_id") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t f_id = strtoul(server.arg("f_id").c_str(), NULL, 10);
  if(machinesIndexCache.count(id) == 0 || f_id >= 5 || machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask == 0x00){
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  
  AppBuffer[AppBufferIndex].Id = id;
  AppBuffer[AppBufferIndex].Duration = machines[machinesIndexCache[id]].Functions[f_id].Duration;
  AppBuffer[AppBufferIndex].RelaisBitmask = machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask;
  AppBuffer[AppBufferIndex].Type = MSG_Fire;
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = 0x00;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;

  if(machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask & 0x01 == 0x01) machines[machinesIndexCache[id]].Relais1Counter++;
  if(machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask & 0x02 == 0x02) machines[machinesIndexCache[id]].Relais2Counter++;
  uint16_t seq = fire(id, machines[machinesIndexCache[id]].Functions[f_id].Duration, machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask);
  lora_fire(id, machines[machinesIndexCache[id]].Functions[f_id].Duration, machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask);
  
  ackStart = millis();
  ackTimeout = 500;
  ackSeq = seq;
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
  if(machinesIndexCache.count(id) == 0 ){
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  blink(id);
  lora_blink(id);
  
  AppBuffer[AppBufferIndex].Id = id;
  AppBuffer[AppBufferIndex].Duration = 0;
  AppBuffer[AppBufferIndex].RelaisBitmask = 0;
  AppBuffer[AppBufferIndex].Type = MSG_Blink;
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = 0x00;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
}

void rest_post_change_id() {
  setCrossOrigin();
  if (server.arg("id") == "" || server.arg("new_id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t new_id = strtoul(server.arg("new_id").c_str(), NULL, 10);
  if(machinesIndexCache.count(id) == 0)
  {
    server.send(404, "text/json", "{\"result\": \"not existing\"}");
  }  
  if(machinesIndexCache.count(new_id) != 0)
  {
    server.send(404, "text/json", "{\"result\": \"already existing\"}");
  }  
  uint8_t index = machinesIndexCache[id];
  MachineData* md = &machines[index]; 
  md->Id = new_id;
  machinesIndexCache.erase(id);
  machinesIndexCache[new_id] = index;
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"change id\", \"id\": 0x00" + String(md->Id, HEX) + ", \"name\": \"" + md->Name + "\", \"shortName\": \"" + md->ShortName + "\"}");
}


void rest_post_stm32_set_id() {  
  setCrossOrigin();  
  if (server.arg("id") == "" || server.arg("new_id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
  }

  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t new_id = strtoul(server.arg("new_id").c_str(), NULL, 10);
  if(machinesIndexCache.count(id) == 0)
  {
    server.send(404, "text/json", "{\"result\": \"not existing\"}");
  }  
  if(machinesIndexCache.count(new_id) != 0)
  {
    server.send(404, "text/json", "{\"result\": \"already existing\"}");
  }  

  //TODO -> rfm command!
  
  uint8_t index = machinesIndexCache[id];
  MachineData* md = &machines[index]; 
  md->Id = new_id;
  machinesIndexCache.erase(id);
  machinesIndexCache[new_id] = index;
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"change id\", \"id\": 0x00" + String(md->Id, HEX) + ", \"name\": \"" + md->Name + "\", \"shortName\": \"" + md->ShortName + "\"}");

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
  std::vector<MachineData>::iterator it;
  int f = 0;
  for (it = machines.begin(); it != machines.end(); it++)
  {
    if (it->Disabled == 1)
      continue;
      
    for (int i = 0; i < 5; i++)
    {
      if(it->Functions[i].RelaisBitmask > 0x00)
      {
        doc[f]["functionId"] = i;
        doc[f]["machineId"] = it->Id;
        doc[f]["name"] = it->Functions[i].Name;
        f++;
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
  if(machinesIndexCache.count(id) == 0)
  {
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  int i = machinesIndexCache[id];
  
  MachineData* md = &machines[i];  

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
    delay(10);
    ESP.restart();
    return;
  }
  
  if(machinesIndexCache.count(id) == 0 ){
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  reboot(id);
  lora_reboot(id);
  
  AppBuffer[AppBufferIndex].Id = id;
  AppBuffer[AppBufferIndex].Duration = 0;
  AppBuffer[AppBufferIndex].RelaisBitmask = 0;
  AppBuffer[AppBufferIndex].Type = MSG_Reboot;
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = 0x00;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
}
