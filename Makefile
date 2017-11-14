DEVICE      = atmega2560
CLOCK       = 16000000UL
PROGRAMMER  = -c usbtiny 
OBJECTS     = rainbow_eq.o
FUSES       = -U lfuse:w:0xc2:m -U hfuse:w:0x99:m -U efuse:w:0xff:m 
AVRDUDE     = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE     = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -std=c99

all:	rainbow_eq.hex

.c.o:
	$(COMPILE) -c $< -o $@ 

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:rainbow_eq.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

make: flash fuse

load: all
	bootloadHID rainbow_eq.hex

clean:
	rm -f rainbow_eq.hex rainbow_eq.elf $(OBJECTS)

rainbow_eq.elf: $(OBJECTS)
	$(COMPILE) -o rainbow_eq.elf $(OBJECTS)

rainbow_eq.hex: rainbow_eq.elf
	rm -f rainbow_eq.hex
	avr-objcopy -j .text -j .data -O ihex rainbow_eq.elf rainbow_eq.hex
	avr-size --format=avr --mcu=$(DEVICE) rainbow_eq.elf


