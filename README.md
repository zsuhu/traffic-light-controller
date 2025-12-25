# Running The Bare Metal C Program
Microcontroller used: Atmega2560

To install requirements on Linux:
```bash
sudo apt install gcc-avr binutils-avr avr-libc avrdude
```
To run the program:
```bash
cd /mnt/c/Users/[your_username]/[project_location]

avr-gcc -mmcu=atmega2560 -DF_CPU=16000000UL -Os -Wall -c main.c

avr-gcc -mmcu=atmega2560 -o main.elf main.o

avr-objcopy -O ihex -R .eeprom main.elf main.hex

avrdude \
  -c wiring \
  -p atmega2560 \
  -P /dev/ttyACM0 \
  -b 115200 \
  -D \
  -U flash:w:main.hex:i
```
