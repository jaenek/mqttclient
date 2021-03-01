#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "sensor.h"
#include "config.h"

class MQTTClient : PubSubClient {
public:
	std::vector<Sensor*> sensors;

	MQTTClient(String ap_ssid, String ap_password) : ap_ssid(ap_ssid), ap_password(ap_password) {}

	void begin() {
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP(ap_ssid, ap_password);

		Serial.print("AP IP address: ");
		Serial.println(APIP);

		dnsServer.start(53, "*", APIP);

		server.begin();

		server.on("/",             HTTP_GET,  [this]{ serve_file("/setup.html"); });
		server.on("/wifi_setup",   HTTP_POST, [this]{ setup_wifi(); });
		server.on("/mqtt_setup",   HTTP_POST, [this]{ setup_mqtt(); });
		server.on("/topic_setup",  HTTP_POST, [this]{ setup_topic(); });
		server.on("/wifi_status",  HTTP_GET,  [this]{ wifi_status(); });
		server.on("/mqtt_status",  HTTP_GET,  [this]{ mqtt_status(); });
		server.on("/readings",     HTTP_GET,  [this]{ get_all_reading_names(); });
		server.on("/gen_204",      HTTP_GET,  [this]{ redirect("/"); });
		server.on("/generate_204", HTTP_GET,  [this]{ redirect("/"); });
		server.on("/success.txt",  HTTP_GET,  [this]{ redirect("/"); });
		server.onNotFound([this]{ serve_file(server.uri()); });

		String ssid, password;
		if (config.load_wifi_config(ssid, password)) {
			Serial.println("Connecting to WiFi");
			WiFi.begin(ssid, password);
			config.load_mqtt_config(mqtt_host, mqtt_port, mqtt_username, mqtt_password);
			mqtt_connect();
		} else {
			Serial.println("Warning: Waiting for wifi credentials under 172.0.0.1/setup");
		}

		for (auto& sensor : sensors)
			sensor->begin();

		for (auto& sensor : sensors) {
			for (auto& name : sensor->get_available_readings()) {
				if (config.load_sensor_config(name, sensor->readings[name].topic, sensor->readings[name].interval))
					Serial.printf("%s read successfuly\n", name.c_str());
			}
		}
	}

	bool mqtt_connect() {
		if (connected())
			return true;

		IPAddress ip;

		if (ip.fromString(mqtt_host.c_str()))
			setServer(ip, mqtt_port.toInt());
		else
			setServer(mqtt_host.c_str(), mqtt_port.toInt());

		setClient(wifi_client);

		if (mqtt_username != "" && mqtt_password != "")
			return connect("Czujnik", mqtt_username.c_str(), mqtt_password.c_str());
		else
			return connect("Czujnik");
	}

	void serve_file(String filepath) {
		String mime = "text/plain";
		if (filepath.endsWith(".html")) mime = "text/html";
		else if (filepath.endsWith(".css")) mime = "text/css";
		else if (filepath.endsWith(".js")) mime = "text/javascript";
		else if (filepath.endsWith(".ttf")) mime = "font/ttf";

		filepath = srv_path + filepath;
		if (LittleFS.exists(filepath)) {
			File file = LittleFS.open(filepath, "r");
			server.streamFile(file, mime);
			file.close();
		} else {
			String err = "Error: not found";
			Serial.println(err);
			server.send(404, "text/html", err);
		}
	}

	void setup_wifi() {
		String ssid = server.arg("ssid"), password = server.arg("password");

		if (ssid != "" && password != "")
			server.send(200, "text/plain", "");
		else
			server.send(400, "text/plain", "Wszystkie pola muszą być wypełnione!");

		config.save_wifi_config(ssid, password);

		WiFi.begin(ssid, password);
	}

	void setup_mqtt() {
		mqtt_host = server.arg("host");
		mqtt_port = server.arg("port"),
		mqtt_username = server.arg("username");
		mqtt_password = server.arg("password");

		if (mqtt_connect()) {
			config.save_mqtt_config(mqtt_host, mqtt_port, mqtt_username, mqtt_password);
			server.send(200, "text/plain", "");
		} else {
			server.send(500, "text/plain", "Błąd konfiguracji serwera!");
		}
	}

	void setup_topic() {
		for (auto& sensor : sensors) {
			String reading = server.arg("reading");
			auto needle = sensor->readings.find(reading);
			if (needle != sensor->readings.end()) {
				String topic = server.arg("topic");
				uint32_t interval = min_to_ms(server.arg("interval").toInt());

				needle->second.topic = topic;
				needle->second.interval = interval;

				config.save_sensor_config(reading, topic, interval);

				server.send(200, "text/plain", "");
				return;
			}
		}
		server.send(500, "text/plain", "Błąd konfiguracji odczytu!");
	}

	void wifi_status() {
		server.send(200, "text/plain", WiFi.status() == WL_CONNECTED ? status_ok : status_bad);
	}

	void mqtt_status() {
		server.send(200, "text/plain", connected() ? status_ok : status_bad);
	}

	void get_all_reading_names() {
		String reading_names;
		for (auto& sensor : sensors) {
			for (auto& name : sensor->get_available_readings()) {
				reading_names += name + '\n';
			}
		}
		server.send(200, "text/plain", reading_names);
	}

	void redirect(String path) {
		server.sendHeader("Location", path, true);
		server.send(302, "text/plain", "");
		server.client().stop(); // Stop is needed because we sent no content length
	}

	void loop() {
		dnsServer.processNextRequest();
		server.handleClient();

		if (millis() - 5000 > last && mqtt_connect()) {
			for (auto& sensor : sensors) {
				auto readings = sensor->get_readings_to_send();
				for (auto& reading : readings)
					publish(reading->second.topic.c_str(), String(sensor->update(reading->first)).c_str());
			}
			last = millis();
		}
	}
private:
	IPAddress APIP{172, 0, 0, 1};

	const String ap_ssid;
	const String ap_password;

	const String status_ok = "Ok!";
	const String status_bad = "Błąd!";

	const String srv_path = "/public";

	String mqtt_host;
	String mqtt_port;
	String mqtt_username;
	String mqtt_password;

	uint32_t last = 0;

	DNSServer dnsServer;
	ESP8266WebServer server{80};
	WiFiClient wifi_client;

	Config config;
} client("Czujnik", "password123");

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

void setup() {
	Serial.begin(9600);
	while (!Serial);

	LittleFS.begin();

	Wire.begin();

	client.sensors.push_back(new BME());

	client.begin();
}

void loop() {
	client.loop();
}
