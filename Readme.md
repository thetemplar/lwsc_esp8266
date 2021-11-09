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
### Die Relais-Platinen
Die Platinen sind in ```PBA-URCL-001-01-PT.PDF``` und ```PBA-URCL-001-01-SC.PDF``` dokumentiert. Sie verfügen über ein 230V und ein 12V Anschluss zur Stromversorgung. Gesteuert werden 2 Relais deren Terminal ausgeführt ist. Zur visuellen Kontrolle haben sie jeweils eine LED auf der Platine.
> Achtung, die obere LED ist für das untere Relais und andersrum!

Zur Kommunikation wird eine Aufsteckplatine genutzt, die 3.3V, GND, die Steuerleitungen der Relais, ein "Flash"-Pin sowie RX/TX nutzt, wobei die letzten drei nur an ein Terminal weitergeleitet werden und somit "normale" GPIOs sind.
Als Standart-Aufsteckplatine ist ein ESP8266 Board verbaut (weiße Platine), wobei es auch eine Version mit ESP8266/ESP32 und einem Lora Modul (grüne Platine). Lora ist als Feature/Fallback angedacht, aber nicht umgesetzt.

### Gateway-Platinen
NodeMCU Platinen (auch nichts anderes als ESP8266 mit UART-FTDI) werden genutzt um WLAN Hotspots aufzumachen, mit dem sich Smartphones verbinden können.

### Propertäre Fernbedienungen
ESP8266 mit I2C-GPIO Port-Externdern für eine Matrix-Tastatur. Funkt direkt mit den Platinen ohne Gateway.

### Android App
App zur Steuerung der Relaisplatinen, sind im WLAN mit einem Gateway (entweder über Accesspoints oder direkt).

### Rasperry Pi
Es ist ein Rasp-Pi im WLAN Netz der Accesspoints vorgesehen gewesen, der sowohl als Gateway dient als auch als Datenbank für die App (Nutzer, Historie, ID<>Namen, etc...). Aktuell nur testweise und nicht im Betrieb.

## Funktionsweise
Die eigentlichen Relais-Platinen werden nicht über WLAN angesprochen, sondern über ein modifiziertes Beacon Frame des IEEE 802.11 Standarts. Dabei wird von Gateway ein Paket "an alle" gesendet, die in Reichweite sind. Die jeweiligen Platinen/Clients, sind dabei in keinem Netzwerk verbunden, sondern aus Sicht des IEEE 802.11 Standarts "nicht verbunden". Sie empfangen jedoch alle Beacon Pakete aller sendenten WLAN Geräte in Reichweite.
Sendet nun ein Gateway ein Paket, enthält es Chip-ID (MAC) des Empfängers (oder 0xFFFFFF für Broadcast), die eigene Chip-ID, den Message-Type, Payload, SqeuenceNumver und TTL. In der Payload steht zB "löse Relais 1 für 200ms aus", siehe [releaseRelais() in Machines.ino](https://github.com/thetemplar/lwsc_esp8266/blob/4c43346d362184e6cd283a0189910880ec8b1b6a/Machines/Machines.ino#L275   "Machines.ino"). 

Damit die Reichweite nicht auf die die des Gateways beschräkt ist, sendet jede Relais-Platine (sofern sie nicht die Destination des Pakets ist oder das Paket schon ein Mal gesehen hat) das Paket weiter. Somit sollte das Paket jede Platine erreichen, auch wenn sie nur "verkettet" erreicht werden kann.

## Code
### ReleaisPlatinen
Sämtlicher Code der Platinen ist in [Machines/Machines.ino](https://github.com/thetemplar/lwsc_esp8266/blob/4c43346d362184e6cd283a0189910880ec8b1b6a/Machines/Machines.ino#L275   "Machines/Machines.ino")  zu finden. Leider ist nicht 100% sicher, ob dies auch der Code ist, der auf den aktuell verbauten Platinen läuft, da es ein Repo-Problem gab. Daher wird aktuell einfach die vorhandene Firmware extrahiert und auf neue ESP8266 geflasht. Die ID wird durch die eindeutige MAC Adresse nicht doppelt vergeben.
### Gateway
[mini_bridge_no_internet/mini_bridge_no_internet.ino](https://github.com/thetemplar/lwsc_esp8266/blob/main/mini_bridge_no_internet/mini_bridge_no_internet.ino   "mini_bridge_no_internet/mini_bridge_no_internet.ino")  enthält den Code der Gateways (mit LoRa).
### Android
Der Android Code ist in einem getrennten Repo: https://github.com/thetemplar/lwsc_android_app (privates Repo) und ist in native-Java geschrieben (_würg_).

## ToDo
* Fernbedienungen! -> Bessere/Stabilere Fernbedienungen sind einfach herzustellen und zu nutzen. Das Problem wäre eher das "Akku" Problem und wie man das professionell löst. Erste Prototypen auf Basis von TP4056-Breakoutboards existieren.
* Quality-Display -> Jede Relais-Platine sollte auf Anfrage die Verbindungsqualität der Nachbarn dem Anfragenden mitteilen. Daraus kann man eine Übersicht bauen, mit den jeweiligen Verbindungen und ihrer RSSI. (Anzeige in der Android App?)
* LoRa? Ist als Alternative gedacht gewesen, als simples "send-only" System mit Stern-Topologie und (vielleicht?) besserer Reichweite. Erste Tests haben das bestätigt, wurde aber nicht weiter verfolgt da WLAN funktioniert.
* App -> App verbessern. Ich bin kein App Entwickler und Java stinkt.
* iOS -> ggf iOS Port (kosten des AppStores sind aber höher). Wäre nur sinnvoll wenn man gleichzeitig eine neue App entwickelt die Plattformunabhängig ist. 
