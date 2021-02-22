
class Config {
public:
	void save_mqtt_config(String url, String port) {
		File file = LittleFS.open(mqtt_config, "w");
		write_to_file(file, url, port);
		file.close();
	}

	void load_mqtt_config(String& url, String& port) {
		File file = LittleFS.open(mqtt_config, "r");
		read_from_file(file, url, port);
		file.close();
	}

	void save_wifi_config(String ssid, String pass) {
		File file = LittleFS.open(wifi_config, "w");
		write_to_file(file, ssid, pass);
		file.close();
	}

	void load_wifi_config(String& ssid, String& pass) {
		File file = LittleFS.open(wifi_config, "r");
		read_from_file(file, ssid, pass);
		file.close();
	}

	void save_sensor_config(String name, String topic, String pass = "") {
		File file = LittleFS.open(wifi_config, "w");
		write_to_file(file, name, topic, pass);
		file.close();
	}

	void load_sensor_config(String& name, String& topic, String& pass) {
		File file = LittleFS.open(wifi_config, "r");
		read_from_file(file, name, topic, pass);
		file.close();
	}

private:
	const String wifi_config = "/wifi_config";
	const String mqtt_config = "/mqtt_config";
	std::vector<String> sensor_configs;

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
