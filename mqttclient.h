#include <LittleFS.h>
#include <Ethernet2.h>
#define USE_ETHERNET2 true
#include <EthernetWebServer.h>
#include <PubSubClient.h>

#define LITTLEFS LittleFS

#include "sensor.h"
#include "config.h"

class MQTTClient : PubSubClient {
public:
	std::map<String, Sensor*> sensors;

	void begin() {
		Ethernet.init(D3);
		while (Ethernet.begin(ethernet_mac) == 0) {
			SERIAL.print("Error: Hardware not detected or link is down!");
			delay(3000);
		}
		//Ethernet.begin(ethernet_mac, IPAddress(172, 17, 0, 108));
		SERIAL.print("Assigned IP: ");
		SERIAL.println(Ethernet.localIP());

		LITTLEFS.begin();

		server.begin();

		server.on("/",             HTTP_GET,  [this]{ serve_file("/setup.html"); });
		server.on("/mqtt_setup",   HTTP_POST, [this]{ setup_mqtt(); });
		server.on("/topic_setup",  HTTP_POST, [this]{ setup_topic(); });
		server.on("/mqtt_status",  HTTP_GET,  [this]{ mqtt_status(); });
		server.on("/readings",     HTTP_GET,  [this]{ server.send(200, "text/plain", get_all_reading_names()); });
		server.on("/configs",      HTTP_GET,  [this]{ server.send(200, "text/plain", get_reading_configs()); });
		server.on("/delete",       HTTP_POST, [this]{ delete_config(); });
		server.on("/gen_204",      HTTP_GET,  [this]{ redirect("/"); });
		server.on("/generate_204", HTTP_GET,  [this]{ redirect("/"); });
		server.on("/success.txt",  HTTP_GET,  [this]{ redirect("/"); });
		server.onNotFound([this]{ serve_file(server.uri()); });

		if (config.load_mqtt_config(mqtt_host, mqtt_port, mqtt_username, mqtt_password)) {
			mqtt_reconnect();
		} else {
			SERIAL.print("Warning: Waiting for mqtt configuration.");
		}

		delay(1000);

		for (auto& pair: sensors)
			pair.second->begin();

		SERIAL.print(get_reading_configs());
	}

	bool mqtt_reconnect() {
		if (mqtt_host == "")
			return false;

		IPAddress ip;

		if (ip.fromString(mqtt_host.c_str()))
			setServer(ip, mqtt_port.toInt());
		else
			setServer(mqtt_host.c_str(), mqtt_port.toInt());

		setClient(ethernet_client);
		setKeepAlive(60);
		setSocketTimeout(5);

		if (mqtt_username != "" && mqtt_password != "")
			return connect("MQTTClient", mqtt_username.c_str(), mqtt_password.c_str());
		else
			return connect("MQTTClient");
	}

	void serve_file(String filepath) {
		String mime = "text/plain";
		if (filepath.endsWith(".html")) mime = "text/html";
		else if (filepath.endsWith(".css")) mime = "text/css";
		else if (filepath.endsWith(".js")) mime = "text/javascript";
		else if (filepath.endsWith(".ttf")) mime = "font/ttf";

		filepath = srv_path + filepath;
		if (LITTLEFS.exists(filepath)) {
			File file = LITTLEFS.open(filepath, "r");
			server.streamFile(file, mime);
			file.close();
		} else {
			const char* err = "Error: not found";
			SERIAL.println(err);
			server.send(404, "text/html", err);
		}
	}

	void setup_mqtt() {
		mqtt_host = server.arg("host");
		mqtt_port = server.arg("port"),
		mqtt_username = server.arg("username");
		mqtt_password = server.arg("password");

		if (mqtt_reconnect()) {
			config.save_mqtt_config(mqtt_host, mqtt_port, mqtt_username, mqtt_password);
			server.send(200, "text/plain", "");
		} else {
			server.send(500, "text/plain", "Błąd konfiguracji serwera!");
		}
	}

	void setup_topic() {
		// incoming reading is in format <sensor name>/<reading name>
		String reading = server.arg("reading");
		if (get_all_reading_names().indexOf(reading) == -1) {
			server.send(500, "text/plain", "Błąd konfiguracji odczytu!");
			return;
		}

		int slash_pos = server.arg("reading").indexOf('/');
		auto sensor_name = reading.substring(0,slash_pos);
		auto reading_name = reading.substring(slash_pos+1);
		auto& sensor_reading = sensors[sensor_name]->readings[reading_name];

		String topic = server.arg("topic");
		uint32_t interval = min_to_ms(server.arg("interval").toInt());

		sensor_reading.topic = topic;
		sensor_reading.interval = interval;

		config.save_sensor_config(reading, topic, interval);

		server.send(200, "text/plain", "");
	}

	void mqtt_status() {
		server.send(200, "text/plain", connected() ? status_ok : status_bad);
	}

	String get_all_reading_names() {
		String reading_names;
		for (auto& pair : sensors) {
			for (auto& name : pair.second->get_available_readings()) {
				reading_names += pair.first + '/' + name + '\n';
			}
		}
		return reading_names;
	}

	void redirect(const char* path) {
		server.sendHeader("Location", path, true);
		server.send(302, "text/plain", "");
	}

	String get_reading_configs() {
		String reading_configs;
		for (auto& pair : sensors) {
			auto& sensor = pair.second;
			for (auto& name : sensor->get_available_readings()) {
				String reading_name = pair.first + '/' + name;
				sensor->readings[name].topic = "";
				if (config.load_sensor_config(reading_name, sensor->readings[name].topic, sensor->readings[name].interval))
					reading_configs += reading_name + ',' + sensor->readings[name].topic + ',' + ms_to_min(sensor->readings[name].interval) + '\n';
			}
		}
		return reading_configs;
	}

	void delete_config() {
		for (auto& pair : sensors) {
			for (auto& name : pair.second->get_available_readings()) {
				String reading_name = pair.first + '/' + name;
				if (server.hasArg(reading_name)) {
					config.remove_sensor_config(reading_name);
					server.send(200, "text/plain", "");
				}
			}
		}
		server.send(500, "text/plain", "Błąd usuwania!");
	}

	void loop() {
		server.handleClient();

		int now = millis();
		if (now - last_action > 5000) {
			if (connected()) {
				for (auto& pair : sensors) {
					auto& sensor = pair.second;
					auto readings = sensor->get_readings_to_send();
					for (auto& reading : readings)
						publish(reading->second.topic.c_str(), String(sensor->update(reading->first)).c_str());
				}
				last_action = now;
			} else if (now - last_action > 60000) {
				last_action = now;
				if (mqtt_reconnect()) last_action = 0;
			}
		}
	}
private:
	uint8_t ethernet_mac[6] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };

	const String status_ok = "Ok!";
	const String status_bad = "Błąd!";

	const String srv_path = "/public";

	String mqtt_host;
	String mqtt_port;
	String mqtt_username;
	String mqtt_password;

	int last_action;

	EthernetClient ethernet_client;
	EthernetWebServer server{80};

	Config config;
};
