#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "config.h"
#include "sensor.h"

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

		server.on("/setup",        HTTP_GET,  [this]{ serve_file("/setup.html"); });
		server.on("/wifi_setup",   HTTP_POST, [this]{ setup_wifi(); });
		server.on("/mqtt_setup",   HTTP_POST, [this]{ setup_mqtt(); });
		server.on("/topic_setup",  HTTP_POST, [this]{ setup_topic(); });
		server.on("/wifi_status",  HTTP_GET,  [this]{ wifi_status(); });
		server.on("/mqtt_status",  HTTP_GET,  [this]{ mqtt_status(); });
		server.on("/readings",     HTTP_GET,  [this]{ get_all_reading_names(); });
		server.on("/gen_204",      HTTP_GET,  [this]{ redirect("/setup"); });
		server.on("/generate_204", HTTP_GET,  [this]{ redirect("/setup"); });
		server.on("/success.txt",  HTTP_GET,  [this]{ redirect("/setup"); });
		server.onNotFound([this]{ serve_file(server.uri()); });

		String ssid, password;
		if (config.load_wifi_config(ssid, password)) {
			Serial.println("Connecting to WiFi");
			WiFi.begin(ssid, password);
		} else {
			Serial.println("Warning: Waiting for wifi credentials under 172.0.0.1/setup");
		}

		for (auto &sensor : sensors)
			sensor->begin();
	}

	void serve_file(String filepath) {
		String mime = "text/plain";
		if (filepath.endsWith(".html")) mime = "text/html";
		else if (filepath.endsWith(".css")) mime = "text/css";
		else if (filepath.endsWith(".js")) mime = "text/javascript";
		else if (filepath.endsWith(".ttf")) mime = "font/ttf";

		if (LittleFS.exists(filepath)) {
			File file = LittleFS.open(filepath, "r");
			server.streamFile(file, mime);
			file.close();
		} else {
			String err = "Error: " + filepath + " not found";
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
		server.send(400, "text/plain", "TODO: Narazie nieobsługiwane!");
	}

	void setup_topic() {
		server.send(400, "text/plain", "TODO: Narazie nieobsługiwane!");
	}

	void wifi_status() {
		server.send(200, "text/plain", WiFi.status() == WL_CONNECTED ? status_ok : status_bad);
	}

	void mqtt_status() {
		//server.send(200, "text/plain", WiFi.status() != WL_CONNECTED ? status_ok : status_bad);
		server.send(200, "text/plain", status_bad);
	}

	void get_all_reading_names() {
		String reading_names;
		for (auto& sensor : sensors) {
			for (auto& name : sensor->get_reading_names()) {
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

	bool connected() {
		return WiFi.status() == WL_CONNECTED;
	}

	void reconnect() {

	}

	void loop() {
		dnsServer.processNextRequest();
		server.handleClient();

		if (!connected()) {
			reconnect();
		} else {
			// publish sensor values
		}

	}
private:
	IPAddress APIP{172, 0, 0, 1};

	const String ap_ssid;
	const String ap_password;
	const String status_ok = "Ok!";
	const String status_bad = "Błąd!";

	uint32_t update_interval;

	DNSServer dnsServer;
	ESP8266WebServer server{80};

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
		else
			Serial.printf("Error: Unknown %s reading name.\n", reading_name.c_str());
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
