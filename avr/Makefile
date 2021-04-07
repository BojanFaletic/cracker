.PHONY: all clean
.SILENT:

CXXPATH=/usr/bin/
CC=$(CXXPATH)avr-g++
OBJDUMP=$(CXXPATH)avr-objcopy
SUMARY=$(CXXPATH)avr-size
READDEF=$(CXXPATH)avr-readelf

CXXFLAGS=-g -Os -w -std=gnu++11 -Wall -Wextra -Iheader -Werror
CXXETRA=-fpermissive -fno-exceptions -ffunction-sections -fdata-sections -DF_CPU=16000000L

all: main.elf
	$(SUMARY) -A main.elf
	#$(READDEF) -a main.elf
	@make clean


main.elf: main.cpp
	$(CC) -mmcu=atmega328p $(CXXFLAGS) $(CXXETRA) $< -o $@


main.hex: main.elf
	$(OBJDUMP) -O ihex $< $@

clean:
	rm -f main.elf main.hex