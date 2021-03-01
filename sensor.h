#include <map>
#include <vector>
#include "BME280I2C.h"


static auto min_to_ms = [](int t) { return (t < 0) ? 0 : t*60000; };
static auto ms_to_min = [](int t) { return t/60000; };

struct Reading {
	String topic = "";
	uint32_t interval = min_to_ms(3);
	uint32_t last_reading = 0;
};

struct Sensor {
	virtual void begin() {}
	virtual float update(const String&) {}

	void set_available_readings(std::vector<String> reading_names) {
		for (auto& name : reading_names)
			readings[name] = Reading{};
	}

	std::vector<String> get_available_readings() {
		std::vector<String> names;
		for (auto& pair : readings) {
			names.push_back(pair.first);
		}
		return names;
	}

	std::vector<std::pair<const String, Reading>*> get_readings_to_send() {
		std::vector<std::pair<const String, Reading>*> readings_to_send;
		for (auto& pair : readings) {
			auto& reading = pair.second;
			if (reading.topic != "" && millis() - reading.last_reading > reading.interval) {
				readings_to_send.push_back(&pair);
				reading.last_reading = millis();
			}
		}
		return readings_to_send;
	}

	std::map<String, Reading> readings;
};
