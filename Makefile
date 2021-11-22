FQBN ?= esp8266:esp8266:d1_mini
BPRP ?=
PORT ?= /dev/ttyUSB0
BAUD ?= 9600

DIR = build/esp8266.esp8266.d1_mini
MKFS = /root/.arduino15/packages/esp8266/tools/mklittlefs/3.0.4-gcc10.3-1757bed/mklittlefs
ESPTOOL = /root/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/tools/esptool/esptool.py
.PHONY: $(DIR)/data.bin clean uploadfs terminal

all: clean $(DIR) uploadfs terminal

clean:
	@echo cleaning
	rm -rf $(DIR)

$(DIR):
	@mkdir -pv $@
	@arduino-cli compile -b $(FQBN)$(BPRP)
	@arduino-cli upload -b $(FQBN)$(BRPR) -p $(PORT)

uploadfs: $(DIR)/data.bin
	@$(MKFS) -p 256 -b 8192 -s 1044464 -c data $^
	@$(ESPTOOL) --chip esp8266 --port $(PORT) write_flash -z 0x200000 $^

terminal:
	@stty -F $(PORT) $(BAUD)
	@tail -f $(PORT)
