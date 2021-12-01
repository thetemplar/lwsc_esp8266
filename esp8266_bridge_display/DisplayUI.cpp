/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <map>

#include "DisplayUI.h"

// ===== adjustable ===== //
void DisplayUI::configInit() {
    // initialize display
    display.init();

    /*
       In case of a compiler (conversion char/uint8_t) error,
       make sure to have version 4 of the display library installed
       https://github.com/ThingPulse/esp8266-oled-ssd1306/releases/tag/4.0.0
     */
    display.setFont(DejaVu_Sans_Mono_8);

    display.setContrast(255);

    if (FLIP_DIPLAY) display.flipScreenVertically();

    display.clear();
    display.display();
}

void DisplayUI::configOn() {
    display.displayOn();
}

void DisplayUI::configOff() {
    display.displayOff();
}

void DisplayUI::updatePrefix() {
    display.clear();
}

void DisplayUI::updateSuffix() {
    display.display();
}

void DisplayUI::drawString(int x, int y, String str) {
    display.drawString(x, y, str);
}

void DisplayUI::drawString(int row, String str) {
    drawString(0, row * lineHeight, str);
}

void DisplayUI::drawLine(int x1, int y1, int x2, int y2) {
    display.drawLine(x1, y1, x2, y2);
}

// ====================== //


DisplayUI::DisplayUI() {}

DisplayUI::~DisplayUI() {}

extern void setupFreedom();
extern void setupAP();
extern void reqRssi(uint32_t dest);
extern std::vector<MachineData> machines;
extern std::map<uint32_t, uint8_t> machinesIndexCache;
extern bool showNames;

void DisplayUI::setup() {
    configInit();
#ifndef ETH_ENABLE
    setupButtons();
#endif
    buttonTime = millis();

    // ===== MENUS ===== //

    // MAIN MENU
    createMenu(&mainMenu, NULL, [this]() {
        addMenuNode(&mainMenu, "ScreenSaver", [this]() {
          mode = DISPLAY_MODE::SCREEN_SAVER;
        });
        addMenuNode(&mainMenu, "FROM APP", [this]() {
          mode = DISPLAY_MODE::LIVE_APP;
        });
        addMenuNode(&mainMenu, "FROM MACHINE", [this]() {
          mode = DISPLAY_MODE::LIVE_MACHINE;
          setupFreedom();
        });
        addMenuNode(&mainMenu, "SWAP ID<>NAME", [this]() {
          showNames = !showNames;
        });
        addMenuNode(&mainMenu, "SEND RSSI REQ", [this]() {
          listSelIndex = 0;
          listSelId = 0;
          setupFreedom();
          reqRssi(0xFFFFFFFF);
          mode = DISPLAY_MODE::REQ_RSSI;
        });
        addMenuNode(&mainMenu, "LIST RSSI", [this]() {
          listSelIndex = 0;
          listSelId = 0;
          mode = DISPLAY_MODE::LIST_RSSI;
        });
        addMenuNode(&mainMenu, "LIST CONNECTED APPS", [this]() {
          mode = DISPLAY_MODE::LIST_CONNECTED;
        });

#ifdef HIGHLIGHT_LED
        addMenuNode(&mainMenu, "LED", [this]() {     // LED
            highlightLED = !highlightLED;
            digitalWrite(2, highlightLED);
            digitalWrite(HIGHLIGHT_LED, highlightLED);
        });
#endif // ifdef HIGHLIGHT_LED
    });
    
    // ===================== //

    // set current menu to main menu
    changeMenu(&mainMenu);
    mode = DISPLAY_MODE::SCREEN_SAVER;
    startTime = millis();
}

void DisplayUI::setupLED() {
    pinMode(HIGHLIGHT_LED, OUTPUT);
    digitalWrite(HIGHLIGHT_LED, HIGH);
    highlightLED = true;
}

void DisplayUI::update(bool force) {
    //if (!enabled) return;

#ifndef ETH_ENABLE
    up->update();
    down->update();
    a->update();
#endif

    draw(force);
}

void DisplayUI::bt_up() {
  if (mode == DISPLAY_MODE::MENU) {                 // when in menu, go up or down with cursor
      if (currentMenu->selected > 0) currentMenu->selected--;
      else currentMenu->selected = currentMenu->list->size() - 1;
  } else if (mode == DISPLAY_MODE::LIST_RSSI) {
      if (listSelIndex > 0) listSelIndex--;
      else listSelIndex = machines.size() - 1;
  }
}
void DisplayUI::bt_down() {
  if (mode == DISPLAY_MODE::MENU) {                 // when in menu, go up or down with cursor
      if (currentMenu->selected < currentMenu->list->size() - 1) currentMenu->selected++;
      else currentMenu->selected = 0;
  } else if (mode == DISPLAY_MODE::LIST_RSSI) {           // when in menu, go up or down with cursor
      if (listSelIndex < machines.size() - 1) listSelIndex++;
      else listSelIndex = 0;
  }
}
void DisplayUI::bt_click() {
  switch (mode) {
      case DISPLAY_MODE::MENU:
          if (currentMenu->list->get(currentMenu->selected).click) {
              currentMenu->list->get(currentMenu->selected).click();
          }
          break;
          
      case DISPLAY_MODE::LIST_RSSI:
        listSelLvl = listSelLvl == 0 ? 1 : 0;
        listSelIndex = 0;
        break;
        
      case DISPLAY_MODE::REQ_RSSI:
      case DISPLAY_MODE::LIVE_MACHINE:
        setupAP();
        mode = DISPLAY_MODE::MENU;
        break;
        
      case DISPLAY_MODE::SCREEN_SAVER:
      case DISPLAY_MODE::LIST_CONNECTED:
      case DISPLAY_MODE::LIVE_APP:
        mode = DISPLAY_MODE::MENU;
        break;
  }
}
void DisplayUI::bt_home() {
  mode = DISPLAY_MODE::MENU;
}


#ifndef ETH_ENABLE
void DisplayUI::setupButtons() {
    up   = new ButtonPullup(BUTTON_UP);
    down = new ButtonPullup(BUTTON_DOWN);
    a    = new ButtonPullup(BUTTON_A);

    // === BUTTON UP === //
    up->setOnClicked([this]() {
        scrollCounter = 0;
        buttonTime    = millis();

        bt_up();
    });

    up->setOnHolding([this]() {
        scrollCounter = 0;
        buttonTime    = millis();

        bt_up();
    }, buttonDelay);

    // === BUTTON DOWN === //
    down->setOnClicked([this]() {
        scrollCounter = 0;
        buttonTime    = millis();

        bt_down();
    });

    down->setOnHolding([this]() {
        scrollCounter = 0;
        buttonTime    = millis();

        bt_down();
    }, buttonDelay);

    // === BUTTON A === //
    a->setOnClicked([this]() {
        scrollCounter = 0;
        buttonTime    = millis();

        bt_click();
    });

    a->setOnHolding([this]() {
        scrollCounter = 0;
        buttonTime    = millis();
        if (mode == DISPLAY_MODE::MENU) {
            if (currentMenu->list->get(currentMenu->selected).hold) {
                currentMenu->list->get(currentMenu->selected).hold();
            }
        } else if (mode == DISPLAY_MODE::LIST_RSSI) {
            mode = DISPLAY_MODE::MENU;
        }
    }, 800);
}
#endif


void DisplayUI::draw(bool force) {
    if (force || ((millis() - drawTime > drawInterval) && currentMenu)) {
        drawTime = millis();

        updatePrefix();

        switch (mode) {

            case DISPLAY_MODE::MENU:
                drawMenu();
                break;
                
            case DISPLAY_MODE::SCREEN_SAVER:
                drawScreenSaver();
                break;

            case DISPLAY_MODE::LIVE_APP:
                drawLiveApp();
                break;

            case DISPLAY_MODE::LIVE_MACHINE:
                drawLiveMachine();
                break;

            case DISPLAY_MODE::LIST_CONNECTED:
                drawConnectedApp();
                break;

            case DISPLAY_MODE::REQ_RSSI:            
                drawString(1, "Wait 5 secs");   
                drawString(2, "  ... then click");
                break;

                
            case DISPLAY_MODE::LIST_RSSI:
                drawRSSIList();
                break;
        }

        updateSuffix();
    }
}

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void DisplayUI::drawRSSIList() {
  if(listSelLvl == 0)
  {
    std::vector<MachineData>::iterator it;
    int i = 0;
    for (it = machines.begin(); it != machines.end(); it++)
    {        
      char ctmp[30] = { 0 };
      char cInd = ' ';
      if (listSelIndex == (i)) 
      {
        cInd = '>';
        listSelId = it->Id;
      }
      if(showNames && it->ShortName[0] != '?')
        sprintf(ctmp, "%c %8.8s Rssi: %-4d", cInd, it->ShortName, (int8_t)it->Rssi);
      else
        sprintf(ctmp, "%c %08X Rssi: %-4d", cInd, it->Id, (int8_t)it->Rssi);
        
      drawString(i-listSelIndex, String(ctmp));
      
      i++;
    }
  } 
  else {
    std::map<uint32_t, int8_t>::iterator it;
    int i = 0;
    for (it = machines[machinesIndexCache[listSelId]].RssiMap.begin(); it != machines[machinesIndexCache[listSelId]].RssiMap.end(); it++)
    {
      char ctmp[30] = { 0 };
      char cInd = ' ';
      if (listSelIndex == (i)) cInd = '>';
      if(showNames && machines[machinesIndexCache[it->first]].ShortName[0] != '?')
        sprintf(ctmp, "%c %8.8s Rssi: %-4d db", cInd, machines[machinesIndexCache[it->first]].ShortName, (int8_t)it->second);
      else
        sprintf(ctmp, "%c %08X Rssi: %-4d db", cInd, it->first, (int8_t)it->second);
      drawString(i-listSelIndex, String(ctmp));
      i++;
    }
  }
}

struct station_info *stat_info;
struct ip4_addr *IPaddress;
IPAddress address;
void DisplayUI::drawConnectedApp() {  
  stat_info = wifi_softap_get_station_info();
  int i = 0;
  while (stat_info != NULL)
  {
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;
    drawString(i, ipToString(address));
    i++;
    stat_info = STAILQ_NEXT(stat_info, next);
  } 
}

String DisplayUI::BufferToString(WifiLog entry)
{
  char tmp[30] = {0};

  char tmp2[10]  = {0};
  if(entry.Type == MSG_Fire)
    sprintf(tmp2, "%02X %d", entry.RelaisBitmask, entry.Duration/100);   
  
  if(showNames && machines[machinesIndexCache[entry.Id]].ShortName[0] != '?')
    sprintf(tmp, "%02X %8.8s %-4d %02X %s", entry.Seq, machines[machinesIndexCache[entry.Id]].ShortName, entry.Rssi, entry.Type, tmp2);
  else
    sprintf(tmp, "%02X %08X %-4d %02X %s", entry.Seq, entry.Id, entry.Rssi, entry.Type, tmp2);
  return String(tmp);
}

extern struct WifiLog MachineBuffer[256];
extern uint8_t MachineBufferIndex;
void DisplayUI::drawLiveMachine()
{
    drawString(0, "Sq  ID/Name  db  ty rm du");
    
    drawString(1, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 1) % 256]));
    drawString(2, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 2) % 256]));
    drawString(3, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 3) % 256]));
    drawString(4, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 4) % 256]));
    drawString(5, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 5) % 256]));
}

extern struct WifiLog AppBuffer[256];
extern uint8_t AppBufferIndex;
void DisplayUI::drawLiveApp()
{
    drawString(0, "Sq  ID/Name  db  ty rm du");
    
    drawString(1, BufferToString(AppBuffer[(AppBufferIndex + 256 - 1) % 256]));
    drawString(2, BufferToString(AppBuffer[(AppBufferIndex + 256 - 2) % 256]));
    drawString(3, BufferToString(AppBuffer[(AppBufferIndex + 256 - 3) % 256]));
    drawString(4, BufferToString(AppBuffer[(AppBufferIndex + 256 - 4) % 256]));
    drawString(5, BufferToString(AppBuffer[(AppBufferIndex + 256 - 5) % 256]));
}

extern String network_ip;
void DisplayUI::drawScreenSaver() {
  drawString(0, " - LWSC ETH/WIFI BRIDGE - ");
  drawString(1, "");
  drawString(2, "Ethernet IP Address:");
  drawString(3, network_ip);
  drawString(4, "Last Command: ");
  drawString(5, BufferToString(MachineBuffer[(MachineBufferIndex + 256 - 1) % 256]));
}

void DisplayUI::drawMenu() {
    String tmp;
    int    tmpLen;
    int    row = (currentMenu->selected / 5) * 5;

    // correct selected if it's off
    if (currentMenu->selected < 0) currentMenu->selected = 0;
    else if (currentMenu->selected >= currentMenu->list->size()) currentMenu->selected = currentMenu->list->size() - 1;

    // draw menu entries
    for (int i = row; i < currentMenu->list->size() && i < row + 5; i++) {
        
        tmp    = String(currentMenu->list->get(i).str);
        tmpLen = tmp.length();

        tmp = (currentMenu->selected == i ? ">" : " ") + tmp;
        drawString(0, (i - row) * 12, tmp);
    }
}

void DisplayUI::clearMenu(Menu* menu) {
    while (menu->list->size() > 0) {
        menu->list->remove(0);
    }
}

void DisplayUI::changeMenu(Menu* menu) {
    if (menu) {
        if (currentMenu) clearMenu(currentMenu);
        currentMenu           = menu;
        currentMenu->selected = 0;
        buttonTime            = millis();

        if (selectedID < 0) selectedID = 0;

        if (currentMenu->parentMenu) {
            addMenuNode(currentMenu, "<<", currentMenu->parentMenu); // add [BACK]
            currentMenu->selected = 1;
        }

        if (currentMenu->build) currentMenu->build();
    }
}

void DisplayUI::goBack() {
    if (currentMenu->parentMenu) changeMenu(currentMenu->parentMenu);
}

void DisplayUI::createMenu(Menu* menu, Menu* parent, std::function<void()>build) {
    menu->list       = new SimpleList<MenuNode>;
    menu->parentMenu = parent;
    menu->selected   = 0;
    menu->build      = build;
}

void DisplayUI::addMenuNode(Menu* menu, String str, std::function<void()>click, std::function<void()>hold) {
    menu->list->add(MenuNode{ str, click, hold });
}

void DisplayUI::addMenuNode(Menu* menu, String str, std::function<void()>click) {
    addMenuNode(menu, str, click, NULL);
}

void DisplayUI::addMenuNode(Menu* menu, String str, Menu* next) {
    addMenuNode(menu, str, [this, next]() {
        changeMenu(next);
    });
}

void DisplayUI::addMenuNode(Menu* menu, const char* ptr, std::function<void()>click) {
    addMenuNode(menu, String(ptr), click);
}

void DisplayUI::addMenuNode(Menu* menu, const char* ptr, Menu* next) {
    addMenuNode(menu, String(ptr), next);
}
