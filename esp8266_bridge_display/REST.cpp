#include <ESP8266WebServer.h>

#include "REST.h"
#include "defines.h"
#include <ArduinoJson.h> //3rd Party Lib! Version 6.x.x

File fsUploadFile;

extern ESP8266WebServer server;
extern std::vector<MachineData> machines;
extern std::map<uint32_t, uint8_t> machinesIndexCache;
extern uint16_t fire(uint32_t dest, int32_t duration, uint8_t relaisBitmask);
extern void blink(uint32_t dest);
extern struct WifiLog AppBuffer[256];
extern uint8_t AppBufferIndex;
extern struct WifiLog MachineBuffer[256];
extern uint8_t MachineBufferIndex;
extern void ReadConfig();
extern void WriteConfig();

#ifdef ETH_ENABLE
extern void reqRssi(uint32_t dest);
extern void wrap_bt_up();
extern void wrap_bt_down();
extern void wrap_bt_click();
extern void wrap_bt_home();
#else
extern void start_query_rssi();
#endif

void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server @LWSC Display 0x00") + String(ESP.getChipId(), HEX) + " - " + millis() + "ms - Heap: " + ESP.getFreeHeap());
    });
    server.on(F("/host"), HTTP_GET, rest_get_host);
    server.on(F("/save_config"), HTTP_POST, rest_post_save_config);
    server.on(F("/machine_count"), HTTP_GET, rest_get_machine_count);
    server.on(F("/machine"), HTTP_GET, rest_get_machine);
    server.on(F("/machine"), HTTP_POST, rest_post_machine);
    server.on(F("/machine_rssi"), HTTP_GET, rest_get_machine_rssi);
    server.on(F("/function"), HTTP_POST, rest_post_function);
    server.on(F("/fire"), HTTP_POST, rest_post_fire);
    server.on(F("/fire"), HTTP_GET, rest_post_fire);
    server.on(F("/blink"), HTTP_POST, rest_post_blink);
    server.on(F("/change_id"), HTTP_POST, rest_post_change_id);
    server.on(F("/config"), HTTP_GET, rest_get_config);  
    server.on("/upload", HTTP_POST, [](){ server.send(200); }, rest_upload_handler );
    server.on(F("/query_rssi"), HTTP_POST, rest_post_query_rssi);
    server.on(F("/all_functions"), HTTP_GET, rest_get_all_functions);
    server.on(F("/set_relaiscounter"), HTTP_POST, rest_post_set_relaiscounter);

#ifdef ETH_ENABLE
    //button via webinterface
    server.on(F("/bt_up"), HTTP_GET, rest_bt_up);  
    server.on(F("/bt_down"), HTTP_GET, rest_bt_down);  
    server.on(F("/bt_click"), HTTP_GET, rest_bt_click);  
    server.on(F("/bt_home"), HTTP_GET, rest_bt_home); 
#endif
}


void rest_get_host() {
  server.send(200, "text/json", "{\"id\": \"0x00" + String(ESP.getChipId(), HEX) + "\"}");
}

void rest_post_save_config() {
  WriteConfig();
  server.send(200, "text/json", "{\"result\": \"success\"}");
}

void rest_get_machine_count()
{
    server.send(200, "text/json", "{\"count\": \"" + String(machines.size()) + "\"}");
}

void rest_get_machine()
{  
  if (server.arg("id") == "" && server.arg("it") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }

  int i = 0;
  if(server.arg("it") == "")
  {
    uint32_t id = server.arg("id").toInt();
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
  if (server.arg("id") == "" || server.arg("name") == "" || server.arg("shortName") == "" || server.arg("disabled") == "" || server.arg("symbolX") == "" || server.arg("symbolY") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = server.arg("id").toInt();
  String s = "changed";
  MachineData* md;
  if(machinesIndexCache.count(id) == 0)
  {
    s = "added";
    md = new MachineData;    
    machines.push_back(*md);  
    machinesIndexCache[id] = machines.size() - 1;
    Serial.println("New Machine: 0x00" + String(id, HEX));
  }
  
  md = &machines[machinesIndexCache[id]];
  for(int i = 0; i < 38; i++)
  {
    if(i < server.arg("name").length())
      md->Name[i] = server.arg("name")[i];
    else
      md->Name[i] = 0x00;
  }
    
  for(int i = 0; i < 9; i++)
  {
    if(i < server.arg("shortName").length())
      md->ShortName[i] = server.arg("shortName")[i];
    else
      md->ShortName[i] = 0x00;
  }
    
  md->Disabled = server.arg("disabled").toInt();
  md->SymbolX = server.arg("symbolX").toInt();
  md->SymbolY = server.arg("symbolY").toInt();
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"" + s + "\"}");
}

void rest_get_machine_rssi() {
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = server.arg("id").toInt();
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
  if (server.arg("id") == "" || server.arg("f_id") == "" || server.arg("name") == "" || server.arg("duration") == "" || server.arg("relaisBitmask") == "" || server.arg("symbolX") == "" || server.arg("symbolY") == "" || server.arg("rotation") == ""){
    server.send(200, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = server.arg("id").toInt();
  uint32_t f_id = server.arg("f_id").toInt();
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
    
  for(int i = 0; i < 38; i++)
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
  
  server.send(200, "text/json", "{\"result\": \"success\", \"operation\": \"" + s + "\"");
}


void rest_post_fire() {
  if (server.arg("id") == "" || server.arg("f_id") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = server.arg("id").toInt();
  uint32_t f_id = server.arg("f_id").toInt();
  if(machinesIndexCache.count(id) == 0 || f_id >= 5 || machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask == 0x00){
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  
  AppBuffer[AppBufferIndex].Id = id;
  AppBuffer[AppBufferIndex].Duration = machines[machinesIndexCache[id]].Functions[f_id].Duration;
  AppBuffer[AppBufferIndex].RelaisBitmask = machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask;
  AppBuffer[AppBufferIndex].Type = 0x01;
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = 0x00;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;

  if(machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask & 0x01 == 0x01) machines[machinesIndexCache[id]].Relais1Counter++;
  if(machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask & 0x02 == 0x02) machines[machinesIndexCache[id]].Relais2Counter++;

#ifdef ETH_ENABLE
  uint32_t t1 = millis();
#endif

  uint16_t seq = fire(id, machines[machinesIndexCache[id]].Functions[f_id].Duration, machines[machinesIndexCache[id]].Functions[f_id].RelaisBitmask);
  
#ifdef ETH_ENABLE
  bool success = false;
  while(millis() < t1 + 2000 && !success)
  {
    int i = 1;
    while(true)
    {
      if(MachineBuffer[(MachineBufferIndex - i + 256) % 256].Timestamp < t1 || MachineBuffer[(MachineBufferIndex - i + 256) % 256].Timestamp == 0)
        break;
        
      uint16_t ackSeq = MachineBuffer[(MachineBufferIndex - i + 256) % 256].Duration >> 16;
      if (MachineBuffer[(MachineBufferIndex - i + 256) % 256].Type == 0x02 && seq == ackSeq && MachineBuffer[(MachineBufferIndex - i + 256) % 256].Id == id)
      {
        success = true;
        break;
      }
      i++;
    }
  }
  uint32_t t2 = millis();
  
  server.send(200, "text/json", "{\"result\": \"" + (success ? String("success") : String("fail")) + "\", \"roundtriptime\": \"" + String (t2 - t1) + "\"}");
#else
  server.send(200, "text/json", "{\"result\": \"true\"}");
#endif
}

void rest_post_blink() {
   if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = server.arg("id").toInt();
  if(machinesIndexCache.count(id) == 0 ){
    server.send(404, "text/json", "{\"result\": \"not found\"}");
    return;
  }
  blink(id);
  
  AppBuffer[AppBufferIndex].Id = id;
  AppBuffer[AppBufferIndex].Duration = 0;
  AppBuffer[AppBufferIndex].RelaisBitmask = 0;
  AppBuffer[AppBufferIndex].Type = 0xFB;
  AppBuffer[AppBufferIndex].Rssi = 0x00;
  AppBuffer[AppBufferIndex].Seq = 0x00;
  AppBuffer[AppBufferIndex].Timestamp = millis();
  AppBufferIndex++;
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
}

void rest_post_change_id() {
  if (server.arg("id") == "" || server.arg("new_id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }
  uint32_t id = server.arg("id").toInt();
  uint32_t new_id = server.arg("new_id").toInt();
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

void rest_get_config() {
  File file = SPIFFS.open("/machines.conf", "r");                    // Open the file
  size_t sent = server.streamFile(file, "application/octet-stream");    // Send it to the client
  file.close();
  //server.send(200, "application/octet-stream", message);
}

void rest_upload_handler(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){Serial.println("UPLOAD_FILE_START");
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){Serial.println("UPLOAD_FILE_WRITE");
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){Serial.println("UPLOAD_FILE_END");
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void rest_post_query_rssi()
{  
#ifdef ETH_ENABLE
  if (server.arg("id") == ""){
    reqRssi(0xFFFFFFFF);
  } else {
    uint32_t id = server.arg("id").toInt();
    if(machinesIndexCache.count(id) == 0)
    {
      server.send(404, "text/json", "{\"result\": \"not found\"}");
      return;
    }
    reqRssi(id); 
  }
  server.send(200, "text/json", "{\"result\": \"success\"}");
#else
  if (server.arg("id") == "")
  {
    server.send(200, "text/json", "{\"result\": \"success\"}");
    start_query_rssi();
  }
#endif
}

void rest_get_all_functions() {
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

void rest_post_set_relaiscounter()
{
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }

  uint32_t id = server.arg("id").toInt();
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

#ifdef ETH_ENABLE
void rest_bt_up() {
  wrap_bt_up();
  server.send(200, "");
}
void rest_bt_down() {
  wrap_bt_down();
  server.send(200, "");
}
void rest_bt_click() {
  wrap_bt_click();
  server.send(200, "");
}
void rest_bt_home() {
  wrap_bt_home();
  server.send(200, "");
}
#endif
