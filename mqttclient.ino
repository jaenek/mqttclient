#include <Wire.h>

#include "mqttclient.h"
#include "BME280I2C.h"

MQTTClient client("Czujnik", "");

class BME : public Sensor {
	void begin() override {
		while (!bme.begin()) {
			Serial.println("BME280 not found!");
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

class Current : public Sensor {
public:
	Current(uint8_t pin) : analog_pin(pin) {}

private:
	void begin() override {
		pinMode(analog_pin, INPUT);
		set_available_readings({"current"});
	}

	float update(const String& reading_name) override {
		if (reading_name == "current") {
			float current = 0.0f;
			return analogRead(analog_pin);
		}
		return 0.0f;
	}

	uint8_t analog_pin;
};

void setup() {
	Serial.begin(9600);
	while (!Serial);

	Wire.begin();

	client.sensors.insert({"BME", new BME()});

	client.begin();
}

void loop() {
	client.loop();
}
