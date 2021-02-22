#include "BME280I2C.h"

BME280I2C bme;

class Sensor {
public:
	void begin() {

		/*while (!bme.begin()) {
			Serial.println("BME280 not found!");
			delay(1000);
		}*/

	}

	void update() {
		bme.read(pressure, temperature, humidity, temp_unit, pressure_unit);
	}

	void send() {
		Serial.printf("BME:\n\tTemperature: %.2f\tHumidity: %.2f%% RH\tPressure: %.2fHPa\n", temperature, humidity, pressure/100);
	}

private:
	float temperature, humidity, pressure;
	BME280::TempUnit temp_unit{BME280::TempUnit_Celsius};
	BME280::PresUnit pressure_unit{BME280::PresUnit_Pa};
};

