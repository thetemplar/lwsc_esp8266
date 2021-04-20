Ort zum Tool:

```	cd "C:\Users\Thinkpad T500\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.5.2\tools\esptool" ```


Gerätemanager für COM-Port.


Flash auslesen (schon erledigt)

```	esptool.py  --port COM5 --baud 115200 read_flash 0x00000 0x100000 flash_1M.bin ```


Flash schreiben:

```	esptool.py  --port COM5 --baud 115200  write_flash 0x00000 flash_1M.bin ```


ID auslesen:

```	esptool.py chip_id ```