#include <map>
#include <vector>
#include "BME280I2C.h"


static auto min_to_ms = [](int t) { return (t < 0) ? 0 : t*60000; };

struct Reading {
	String topic = "";
	uint32_t interval = min_to_ms(3);
	uint32_t last_reading = 0;
	float value = 0.0f;
};

class Sensor {
public:
	virtual void begin() {}
	virtual float update(const String&) {}

	void set_available_readings(std::vector<String> reading_names) {
		for (auto& name : reading_names)
			readings[name] = Reading{};
	}

	std::vector<String> get_reading_names() {
		std::vector<String> names;
		for (auto& pair : readings) {
			names.push_back(pair.first);
			Serial.println(pair.first);
		}
		return names;
	}

	void update_readings() {
		for (auto& pair : readings) {
			if (millis() - pair.second.last_reading > pair.second.interval)
				pair.second.value = update(pair.first);
		}
	}

private:
	std::map<String, Reading> readings;
};
