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

extern MachineData machines[32];
extern User users[16];
extern int8_t machineCount;
extern FSInfo fs_info;

uint64_t ackStart;
uint32_t ackTimeout;
uint32_t ackMachine;

void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server @LWSC Display 0x00") + String(ESP.getChipId(), HEX) + " - " + millis() + "ms - Heap: " + ESP.getFreeHeap() + "<br>" + String(__DATE__) + " " + String(__TIME__) + F("<br><a href='/web'>Web Fernbedienung</a>"));
    });
    server.on(F("/web"), HTTP_GET, web_interface);
    server.on(F("/host"), HTTP_GET, rest_get_host);
    server.on(F("/save_config"), HTTP_POST, rest_post_save_config);
    server.on(F("/machine_count"), HTTP_GET, rest_get_machine_count);
    server.on(F("/machine"), HTTP_GET, rest_get_machine);
    server.on(F("/search_function"), HTTP_GET, rest_get_search_function);
    server.on(F("/machine"), HTTP_POST, rest_post_machine);
    server.on(F("/last_msg"), HTTP_GET, rest_get_last_msg);
    server.on(F("/function"), HTTP_POST, rest_post_function);
    server.on(F("/fire"), HTTP_POST, rest_post_fire);
    server.on(F("/fire"), HTTP_GET, rest_post_fire);
    server.on(F("/force_fire"), HTTP_POST, rest_force_fire);
    server.on(F("/force_fire"), HTTP_GET, rest_force_fire);
    server.on(F("/blink"), HTTP_POST, rest_post_blink);
    server.on(F("/quality"), HTTP_GET, rest_get_quality);  
    server.on(F("/warning"), HTTP_POST, rest_post_warning);
    server.on(F("/warning"), HTTP_GET, rest_get_warning);  
    server.on(F("/file"), HTTP_GET, rest_get_file);  
    server.on(F("/file_list"), HTTP_GET, rest_get_file_list);  
    server.on(F("/file"), HTTP_DELETE, rest_delete_file);   
    server.on(F("/file_delete"), HTTP_POST, rest_delete_file);  
    server.on(F("/upload"), HTTP_POST, [](){ server.send(200); }, rest_upload_handler );
    server.on(F("/all_functions"), HTTP_GET, rest_get_all_functions);
    server.on(F("/set_relaiscounter"), HTTP_POST, rest_post_set_relaiscounter);
    server.on(F("/reboot"), HTTP_POST, rest_post_reboot);
    server.on(F("/version"), HTTP_GET, rest_get_version);
    server.on(F("/check_user"), HTTP_GET, rest_get_check_user);
}
bool checkUserRights(String user, String password, UserRights neededRights){
    for(int i = 0; i < 64; i++)
    {
      if(strcmp(users[i].Name, user.c_str()) == 0)
      {
        if(strcmp(users[i].Password, password.c_str()) != 0)
        {
          server.send(401, "text/json", "{\"result\": \"no auth\"}");
          udpMsg("[REST] auth: no auth (wrong password)");
          return false;
        }
        if(users[i].Rights < neededRights)
        {
          server.send(401, "text/json", "{\"result\": \"no rights\"}");
          udpMsg("[REST] auth: no rights" + String(server.uri()));
          return false;
        }
        return true;
      }
    }
    
    server.send(401, "text/json", "{\"result\": \"no auth\"}");
    udpMsg("[REST] auth: no auth (no user)");
    return false;
}
  
void setCrossOrigin(){
    //udpMsg("[REST] function called: " + String(server.uri()));
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("POST,GET,DELETE"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

void rest_get_check_user()
{  
  setCrossOrigin();
  for(int i = 0; i < 64; i++)
  {
    if(strcmp(users[i].Name, server.arg("username").c_str()) == 0)
    {
      if(strcmp(users[i].Password, server.arg("password").c_str()) != 0)
      {
        server.send(401, "text/json", "{\"result\": \"wrong password\"}");
        return;
      }
      if(users[i].Rights == 0)
        server.send(200, "text/json", "{\"result\": \"success\", \"rights\": \"None\"}");
      if(users[i].Rights == Fire)
        server.send(200, "text/json", "{\"result\": \"success\", \"rights\": \"Fire\"}");
      if(users[i].Rights == Saves)
        server.send(200, "text/json", "{\"result\": \"success\", \"rights\": \"Write File\"}");
      if(users[i].Rights == Admin)
        server.send(200, "text/json", "{\"result\": \"success\", \"rights\": \"Admin\"}");

      udpMsg("[REST] rest_get_check_user: success - '" + String(server.arg("username")) + "' is " + String(users[i].Rights));
      return;
    }
  }
  
  server.send(401, "text/json", "{\"result\": \"no auth\"}");
}

void web_interface() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  
    
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
                  "            const url = '/fire?password=lwsc&username=User&id=' + mid + '&f_id=' + fid; "\
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

void rest_get_host() {
  setCrossOrigin();
  server.send(200, "text/json", "{\"chip_id\": \"" + String(String(ESP.getChipId(), HEX)) + "\", \"build\": \"" + String(__DATE__) + " " + String(__TIME__) + "\", \"flash\": \"" + String(fs_info.usedBytes/(fs_info.totalBytes*100)) +"%\"}");
}

void rest_get_version() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
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
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return; 
  WriteConfig();
  server.send(200, "text/json", "{\"result\": \"success\"}");
  udpMsg("[REST] rest_post_save_config: saved");
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
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  server.send(200, "text/json", "{\"count\": \"" + String(machineCount) + "\"}");
  //udpMsg("[REST] rest_get_machine_count: count: " + String(machineCount));
}

void rest_get_machine()
{  
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_get_machine: fail: no id");
    return;
  }

  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  
  if(machineCount == 0 || id >= (uint32_t)machineCount)
  {
    server.send(404, "text/json", "{\"result\": \"no data\"}");
    udpMsg("[REST] rest_get_machine: fail: no data");
    return;
  }
  
  MachineData* md = &machines[id];  
  
  String message = "";
  DynamicJsonDocument doc(2048);
  doc["id"] = id;
  doc["name"] = machines[id].Name;
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
  udpMsg("[REST] rest_get_machine: ok");
}

void rest_get_search_function()
{  
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  if (server.arg("name") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    return;
  }

  for(int id = 0; id < machineCount; id++)
  {
    MachineData* md = &machines[id];  
    
    String message = "";
  
    for (int f = 0; f < 5; f++)
    {
      if(md->Functions[f].RelaisBitmask > 0x00 || strcmp(server.arg("name").c_str(), md->Functions[f].Name) == 0)
      {    
        DynamicJsonDocument doc(2048);
        doc["success"] = "found";
        doc["functionId"] = f;
        doc["machineId"] = id;
        doc["name"] = md->Functions[f].Name;
        serializeJson(doc, message);
        server.send(200, "text/json", message); 
        udpMsg("[REST] rest_get_search_function: ok");
      }
    }
  }
  server.send(404, "text/json", "{\"result\": \"no data\", \"functionId\": \"-1\", \"machineId\": \"-1\", \"name\": \"-\"}");
  udpMsg("[REST] rest_get_search_function: fail: no data");
}

void rest_post_machine() {  
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_machine: fail: no id");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  if(id >= 64)
  {
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_machine: fail: id >= 64");
    return;
  }
  Serial.println("rest_post_machine: " + String(id));
  
  String s = "changed";
  if(id >= (uint32_t)machineCount)
  {
    s = "added";
    memset(&machines[id].Name, 0x00, 38);
    memset(&machines[id].ShortName, 0x00, 9);
    for(uint8_t i = 0; i < 5; i++)
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
    for(unsigned int i = 0; i < 37; i++)
    {
      if(i < server.arg("name").length())
        machines[id].Name[i] = server.arg("name")[i];
      else
        machines[id].Name[i] = 0x00;
    }
  }
    
  if(server.arg("shortName") != "")
  {
    for(unsigned int i = 0; i < 8; i++)
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
  udpMsg("[REST] rest_post_machine: ok");
}

void rest_post_function() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
  if (server.arg("id") == "" || server.arg("f_id") == "" || server.arg("name") == "" || server.arg("duration") == "" || server.arg("relaisBitmask") == "" || server.arg("symbolX") == "" || server.arg("symbolY") == "" || server.arg("rotation") == ""){
    server.send(200, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_function: fail: no id");
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
    
  for(unsigned int i = 0; i < 37; i++)
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
  udpMsg("[REST] rest_post_function: ok");
}

void rest_force_fire()
{
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
  if (server.arg("id") == "" || server.arg("duration") == ""  || server.arg("bitmask") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_force_fire: fail: arguments");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t duration = strtoul(server.arg("duration").c_str(), NULL, 10);
  uint32_t bitMask = strtoul(server.arg("bitmask").c_str(), NULL, 10);
  lora_fire(id, duration, bitMask); 
  server.send(200, "text/json", "{\"result\": \"ok\"}");
  udpMsg("[REST] rest_force_fire: ok");
}

int sessionCounter = 0;
unsigned long long lastFire = 0;
void IRAM_ATTR rest_post_fire() {
  setCrossOrigin();
  //if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  if (server.arg("id") == "" || server.arg("f_id") == "" ){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_fire: fail: arguments");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  uint32_t f_id = strtoul(server.arg("f_id").c_str(), NULL, 10);

  if((machines[id].Functions[f_id].RelaisBitmask & 0x01) == 0x01) machines[id].Relais1Counter++;
  if((machines[id].Functions[f_id].RelaisBitmask & 0x02) == 0x02) machines[id].Relais2Counter++;
  
  
  udpMsg("[REST] fire: " + String(machines[id].Functions[f_id].Name));
  lora_fire(id, machines[id].Functions[f_id].Duration, machines[id].Functions[f_id].RelaisBitmask); 
  ackStart = millis();
  ackTimeout = 400;
  ackMachine = id;
  
  //no server.send -> in processWiFiData()->ack!    

  if(millis() > lastFire + 43200000)
  {
    sessionCounter = 0;
  }
  lastFire = millis();
  sessionCounter++;
}

void rest_get_quality() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_get_quality: fail: no id");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  
  server.send(200, "text/json", "{\"result\": \"success\", \"machine\": \"" + String(id) + "\", \"name\": \"" + String(machines[id].Name) + "\", \"last_seen\": \"" + String((millis() - machines[id].LastSeen)/1000.0) + "\", \"machine_rssi\": \"" + String(machines[id].MachineRssi) + "\", \"machine_snr\": \"" + String(machines[id].MachineSnr) + "\", \"rssi\": \"" + String(machines[id].Rssi) + "\", \"snr\": \"" + String(machines[id].Snr) + "\"}");
  udpMsg("[REST] rest_get_quality: ok");
}

void rest_post_blink() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
   if (server.arg("id") == "lora"){
    lora_blink(0xff);
    server.send(200, "text/json", "{\"result\": \"lora\"}");
    udpMsg("[REST] rest_post_blink: ok (0xff)");
    return;
  }
   if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_blink: fail: no id");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  lora_blink(id);
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
  udpMsg("[REST] rest_post_blink: ok");
}

extern uint64_t warning_remaining;
extern String warning_msg;
void rest_get_warning() {
  setCrossOrigin();
  if(millis() < warning_remaining)
    server.send(200, "text/json", "{\"result\": \"success\", \"type\": \"warning\", \"message\": \"" + String(warning_msg) + "\", \"time_left\": \"" + String((warning_remaining - millis()) / 1000) + "\"}");
  else
    server.send(200, "text/json", "{\"result\": \"success\", \"type\": \"none\"}");
  udpMsg("[REST] rest_get_warning: ok");
}

void rest_post_warning() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
  
  if (server.arg("message") == "")
    warning_msg = "Achtung!";
  else
  {
    String s = server.arg("message");
    if(s.length() > 100) s = s.substring(0, 100);
    warning_msg = s;
  }
    
  if (server.arg("time_left") == "")
    warning_remaining = millis() + 30 * 1000;
  else
  {
    uint32_t ttl = strtoul(server.arg("time_left").c_str(), NULL, 10);
    if(ttl > 300)
      ttl = 300;
    warning_remaining = millis() + ttl * 1000;
  }
    
  server.send(200, "text/json", "{\"result\": \"success\", \"message\": \"" + String(warning_msg) + "\", \"time_left\": \"" + String((warning_remaining - millis()) / 1000) + "\"}");
  udpMsg("[REST] rest_post_warning: ok");
}

void rest_get_file() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  if (server.arg("filename") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_get_file: fail: no filename");
    return;
  }
  File file = LittleFS.open("/" + server.arg("filename"), "r");
  server.streamFile(file, "application/octet-stream");
  file.close();
  udpMsg("[REST] rest_get_file: ok");
}

void rest_get_file_list() {
  setCrossOrigin(); 
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
  String message = ""; 
  DynamicJsonDocument doc(2048); 
  File root = LittleFS.open("/"+server.arg("path"), "r"); 

  File file = root.openNextFile(); 
  int f = 0; 
  while(file){ 
    Serial.print("FILE: "); 
    doc["files"][f] = String(file.name()); 

    file = root.openNextFile(); 
    f++; 
  } 
  serializeJson(doc, message); 
  server.send(200, "text/plain", message); 
  //udpMsg("[REST] rest_get_file_list: ok");
}

void rest_delete_file() { 
  setCrossOrigin(); 
  if(!checkUserRights(server.arg("username"), server.arg("password"), Saves)) return;
  if (server.arg("filename") == ""){ 
    server.send(400, "text/json", "{\"result\": \"fail\"}"); 
    udpMsg("[REST] rest_delete_file: fail: no filename");
    return; 
  } 
  if(LittleFS.exists("/" + server.arg("path") + server.arg("filename"))){ 
    LittleFS.remove("/" + server.arg("path") + server.arg("filename")); 
    server.send(200, "text/json", "{\"result\": \"success\"}"); 
    udpMsg("[REST] rest_delete_file: ok");
  }else{ 
    server.send(404, "text/json", "{\"result\": \"not found\"}"); 
    udpMsg("[REST] rest_delete_file: fail: not found");
  } 
} 

void rest_upload_handler(){ // upload a new file to the LittleFS  
  if(!checkUserRights(server.arg("username"), server.arg("password"), Saves)) return;
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
      setCrossOrigin();
      server.send(200);   
      udpMsg("[REST] rest_upload_handler: ok"); 
      LittleFS.info(fs_info);
    } else {
      setCrossOrigin();
      server.send(500, "text/plain", "500: couldn't create file");
      udpMsg("[REST] rest_upload_handler: fail: couldn't create file"); 
    }
  }
}

void rest_get_all_functions() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Fire)) return;
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
  //udpMsg("[REST] rest_get_all_functions: ok");
}

void rest_post_set_relaiscounter() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
  if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_set_relaiscounter: fail: no id");
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
  udpMsg("[REST] rest_post_set_relaiscounter: ok");
}

void rest_post_reboot() {
  setCrossOrigin();
  if(!checkUserRights(server.arg("username"), server.arg("password"), Admin)) return;
   if (server.arg("id") == ""){
    server.send(400, "text/json", "{\"result\": \"fail\"}");
    udpMsg("[REST] rest_post_reboot: fail: no id");
    return;
  }
  uint32_t id = strtoul(server.arg("id").c_str(), NULL, 10);
  if(id == 0)
  {
    server.send(200, "text/json", "{\"result\": \"success\"}");
    udpMsg("[REST] rest_post_reboot: ok - local reboot");
    udpMsg("REBOOT");
    delay(10);
    ESP.restart();
    return;
  }
  
  lora_reboot(id);
  
  server.send(200, "text/json", "{\"result\": \"success\"}");
  udpMsg("[REST] rest_post_reboot: ok");
}
