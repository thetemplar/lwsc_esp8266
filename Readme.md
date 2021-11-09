### Schnell-Hilfe vor Ort:

Ort zum Tool:
```	cd "C:\Users\Thinkpad T500\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.5.2\tools\esptool" ```


Gerätemanager für COM-Port.


Flash auslesen (schon erledigt)
```	esptool.py  --port COM5 --baud 115200 read_flash 0x00000 0x100000 flash_1M.bin ```

Flash schreiben:
```	esptool.py  --port COM5 --baud 115200  write_flash 0x00000 flash_1M.bin ```

ID auslesen:
```	esptool.py chip_id ```

# Kurzdokumentation
## Bauteile
Relais auf Platinen an den Wurfmaschinen werden über Fernbedienungen oder Smartphone Apps gesteuert, damit sie den jeweiligen Wurfarm betätigen (oder andere Aktionen steuern).
### Die Platinen
Die Platinen sind in ```PBA-URCL-001-01-PT.PDF``` und ```PBA-URCL-001-01-SC.PDF``` dokumentiert. Sie verfügen über ein 230V und ein 12V Anschluss zur Stromversorgung. Gesteuert werden 2 Relais deren Terminal ausgeführt ist. Zur visuellen Kontrolle haben sie jeweils eine LED auf der Platine.
> Achtung, die obere LED ist für das untere Relais und andersrum!

Zur Kommunikation wird eine Aufsteckplatine genutzt, die 3.3V, GND, die Steuerleitungen der Relais, ein "Flash"-Pin sowie RX/TX nutzt, wobei die letzten drei nur an ein Terminal weitergeleitet werden und somit "normale" GPIOs sind.
Als Standart-Aufsteckplatine ist ein ESP8266 Board verbaut (weiße Platine), wobei es auch eine Version mit ESP8266/ESP32 und einem Lora Modul (grüne Platine). Lora ist als Feature/Fallback angedacht, aber nicht umgesetzt.

### Gateway-Platinen
NodeMCU Platinen (auch nichts anderes als ESP8266 mit UART-FTDI) werden genutzt um Wifi Hotspots aufzumachen, mit dem sich Smartphones verbinden können.

### Propertäre Fernbedienungen
ESP8266 mit I2C-GPIO Port-Externdern für eine Matrix-Tastatur. Funkt direkt mit den Platinen ohne Gateway.

### Android App
App zur Steuerung der Relaisplatinen, sind im WLAN mit einem Gateway (entweder über Accesspoints oder direkt).

## Funktionsweise

