#include <Wire.h>
#include <SDM.h>

#ifndef DEBUG
class DevNull: public Print
{
public:
    virtual size_t write(uint8_t) { return 1; }
};
DevNull devnull;
#define SERIAL devnull
#endif


#include "mqttclient.h"
#include "BME280I2C.h"

MQTTClient client;

class BME : public Sensor {
	void begin() override {
		while (!bme.begin()) {
			SERIAL.println("BME280 not found!");
			delay(1000);
		}

		set_available_readings({"bme_temp", "bme_hum", "bme_pres"});
	}

	float update(const String& reading_name) override {
		if (reading_name == "bme_temp")
			return bme.temp(temperature_unit);
		else if (reading_name == "bme_hum")
			return bme.hum();
		else if (reading_name == "bme_pres")
			return bme.pres(pressure_unit);
		return 0.0f;
	}

	BME280I2C bme;
	BME280::TempUnit temperature_unit{BME280::TempUnit_Celsius};
	BME280::PresUnit pressure_unit{BME280::PresUnit_Pa};
};

class SDM120M : public Sensor {
public:
	SDM120M() : sdm(Serial, 9600, D0, SERIAL_8N1, false) {}

private:
	void begin() override {
		sdm.begin();
		set_available_readings({"voltage", "current", "power", "apparent_power", "reactive_power", "frequency",
			"total_active_energy", "total_reactive_energy"});
	}

	float update(const String& reading_name) override {
		if (reading_name == "voltage") {
			return sdm.readVal(SDM_PHASE_1_VOLTAGE, 0x01);
		}
		else if (reading_name == "current") {
			return sdm.readVal(SDM_PHASE_1_CURRENT, 0x01);
		}
		else if (reading_name == "power") {
			return sdm.readVal(SDM_PHASE_1_POWER, 0x01);
		}
		else if (reading_name == "apparent_power") {
			return sdm.readVal(SDM_PHASE_1_APPARENT_POWER, 0x01);
		}
		else if (reading_name == "reactive_power") {
			return sdm.readVal(SDM_PHASE_1_REACTIVE_POWER, 0x01);
		}
		else if (reading_name == "frequency") {
			return sdm.readVal(SDM_FREQUENCY, 0x01);
		}
		else if (reading_name == "total_active_energy") {
			return sdm.readVal(SDM_TOTAL_ACTIVE_ENERGY, 0x01);
		}
		else if (reading_name == "total_reactive_energy") {
			return sdm.readVal(SDM_TOTAL_REACTIVE_ENERGY, 0x01);
		}
		return 0.0f;
	}

	SDM sdm;
};

void setup() {
#ifdef DEBUG
	SERIAL.begin(9600);
	while (!SERIAL);
#endif


	Wire.begin();

	client.sensors.insert({"BME", new BME()});
	client.sensors.insert({"SDM120M", new SDM120M()});

	client.begin();

	SERIAL.print("Setup done!");
}

void loop() {
	client.loop();
}
