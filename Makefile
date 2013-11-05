PORT=/dev/ttyUSB1
MCU=32MX320F128H

LDFLAGS=-mprocessor=$(MCU) -T uno.ld
CFLAGS=-mprocessor=$(MCU) -mno-smart-io  -w  -fno-exceptions  -ffunction-sections  -fdata-sections  -g  -mdebugger  -Wcast-align  -fno-short-double
DEPFLAGS=-MD -MP -MF $(@).o.d 

OBJECTS:=main.o

-include *.d

all: flash

clean: 
	rm *.o 
	rm *.d
	rm *.elf
	rm *.hex

%.o: %.c
	xc32-gcc $(CFLAGS) $(DEPFLAGS) -c -o $(@) $(<)

blinky.elf: $(OBJECTS)
	xc32-gcc $(LDFLAGS) -o $(@) $(<)

blinky.lss: blinky.elf
	xc32-objdump -h -S $(<) > $(@)

blinky.hex: blinky.elf
	xc32-objcopy -O ihex $(<) $(@)

flash: blinky.hex
	./serial_reset.sh $(PORT)
	avrdude -C avrdude.pic32.conf -c stk500v2 -p $(MCU) -P $(PORT) -b 115200 -U flash:w:$(<)
	touch $(@)
