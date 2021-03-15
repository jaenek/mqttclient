FQBN ?= esp8266:esp8266:d1_mini
BPRP ?=
PORT ?= /dev/ttyUSB0
BAUD ?= 9600

DIR = build/esp8266.esp8266.d1_mini
MKFS = ../tools/mklittlefs/mklittlefs

.PHONY: clean upload uploadfs terminal

all: clean upload uploadfs terminal

clean:
	@echo cleaning
	rm -rf $(DIR)

$(DIR):
	@mkdir -pv $@
	@arduino-cli compile -b $(FQBN)$(BPRP)

$(DIR)/data.bin: data
	@$(MKFS) -p 256 -b 8192 -s 1044464 -c $^ $@

upload: $(DIR) $(DIR)/data.bin
	@arduino-cli upload -b $(FQBN)$(BRPR) -p $(PORT)

uploadfs: $(DIR)/data.bin
	@esptool.py --chip esp8266 --port $(PORT) write_flash -z 0x200000 $^

terminal:
	@stty -F $(PORT) $(BAUD)
	@tail -f $(PORT)
