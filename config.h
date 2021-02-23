
class Config {
public:
	void save_mqtt_config(String host, String port, String username, String password) {
		File file = LittleFS.open(mqtt_config, "w");
		write_to_file(file, host, port, username, password);
		file.close();
	}

	bool load_mqtt_config(String& host, String& port, String& username, String& password) {
		if (!LittleFS.exists(mqtt_config))
			return false;

		File file = LittleFS.open(mqtt_config, "r");
		read_from_file(file, host, port, username, password);
		file.close();
		return true;
	}

	void save_wifi_config(String ssid, String password) {
		File file = LittleFS.open(wifi_config, "w");
		write_to_file(file, ssid, password);
		file.close();
	}

	bool load_wifi_config(String& ssid, String& password) {
		if (!LittleFS.exists(wifi_config))
			return false;

		File file = LittleFS.open(wifi_config, "r");
		read_from_file(file, ssid, password);
		file.close();
		return true;
	}

	void save_sensor_config(String sensor_config, String topic, String interval) {
		File file = LittleFS.open(sensor_config, "w");
		write_to_file(file, topic, interval);
		file.close();
	}

	bool load_sensor_config(String sensor_config, String& topic, int& interval) {
		if (!LittleFS.exists(sensor_config))
			return false;

		File file = LittleFS.open(sensor_config, "r");
		String tmp;
		read_from_file(file, topic, tmp);
		interval = tmp.toInt();
		file.close();
		return true;
	}

private:
	const String wifi_config = "/wifi_config";
	const String mqtt_config = "/mqtt_config";

	void write_to_file(File file) {}

	template <typename ... Args>
	void write_to_file(File file, String arg, Args ... args) {
		for (int i = 0; i < arg.length(); i++)
			file.write(arg[i]);
		file.write('\n');
		write_to_file(file, args...);
	}

	void read_from_file(File file) {}

	template <typename ... Args>
	void read_from_file(File file, String& arg, Args ... args) {
		char c = file.read();
		while (c != '\n') {
			arg += c;
			c = file.read();
		}
		read_from_file(file, args...);
	}
};
