# radio-station
Transmitting radio waves and listening with a walkie-talkie

To compile and run the project you need to use the make command in the "current" directory. Then you need to use the dtekv-run tool to run it. To upload a file you must use the dtekv-upload command where you specify the path of the file you are trying to upload as well as the address where you are trying to upload it to on the board. We use the address 0x00100000 as the base upload address so upload files there. 

Components:
You will need a DE10-Lite board as well as a "E07-M1101D-SMA Ebyte 10dBm 530m CC1101 Rf 433MHz Fsk Module" or other compatable CC1101 based component. 
Connect the board to the peripheral like this: 
GROUND                  PIN 30 (GND)
VCC (POWER)             PIN 29 (3.3 V)
SCL (CLOCK)             PIN 1 ([0])
CSN (CHIP SELECT)       PIN 2 ([1])
MOSI (MASTER OUTPUT)    PIN 5 ([4])
MISO (MASTER INPUT)     PIN 7 ([6])
GPO0                    PIN 6 ([5]) 


NOTE: 
We do not own the files:
boot.o
boot.S
dtekv-lib.c
dtekv-lib.h
dtekv-lib.o
dtekv-script.lds
Makefile
main.elf
main.elf.txt
softfloat.a
The copyright belongs to 
"Copyright (c) 2024, Artur Podobas
Copyright (c) 2024, Wiktor Szczerek
Copyright (c) 2024, Pedro Antunes"