FQBN ?= esp8266:esp8266:d1_mini
BPRP ?= :wipe=all
PORT ?= /dev/ttyUSB0
BAUD ?= 9600

all: compile upload terminal

clean:
	@echo cleaning
	-rm -r build/ binaires/data.bin

compile:
	@arduino-cli compile -b $(FQBN)$(BPRP)

binaires/data.bin: data
	@mkdir -pv binaires
	@../tools/mklittlefs/mklittlefs -p 256 -b 8192 -s 1044464 -c $^ $@

upload: binaires/data.bin
	@arduino-cli upload -b $(FQBN)$(BRPR) -p $(PORT)
	@esptool.py --chip esp8266 --port $(PORT) --baud 115200 write_flash 0x200000 $^

uploaddata: binaires/data.bin
	@esptool.py --chip esp8266 --port $(PORT) --baud 115200 write_flash 0x200000 $^

terminal:
	@stty -F $(PORT) $(BAUD)
	@tail -f $(PORT)
