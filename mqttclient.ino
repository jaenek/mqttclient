#include <ESP8266WiFi.h>
#include <FS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "BME280I2C.h"

BME280I2C bme;

class Sensor {
public:
	void begin() {
		while (!bme.begin()) {
			Serial.println("BME280 not found!");
			delay(1000);
		}

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

class MQTTClient : PubSubClient {
public:
	std::vector<Sensor> sensors;

	void begin() {
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP(ap_ssid, ap_pass);

		dnsServer.start(53, "*", APIP);

		server.begin();

		server.on("/setup",        HTTP_GET,  [this]{ serve_file("/setup.html"); });
		server.on("/setup",        HTTP_POST, [this]{ setup_wifi(); });
		server.on("/wifi_status",  HTTP_GET,  [this]{ wifi_status(); });
		server.on("/mqtt_status",  HTTP_GET,  [this]{ mqtt_status(); });
		server.on("/gen_204",      HTTP_GET,  [this]{ redirect("/setup"); });
		server.on("/generate_204", HTTP_GET,  [this]{ redirect("/setup"); });
		server.on("/success.txt",  HTTP_GET,  [this]{ redirect("/setup"); });
		server.onNotFound([this]{ serve_file(server.uri()); });

		Serial.print("AP IP address: ");
		Serial.println(APIP);

		if (LittleFS.exists(wifi_config)) {
			Serial.println("Connecting to WiFi");
			String ssid, pass, interval;

			int line = 0;
			File file = LittleFS.open(wifi_config, "r");
			while (file.available()) {
				char c = char(file.read());
				if (c == '\n') {
					line++;
					continue;
				}

				switch(line) {
				case 0: ssid += c; break;
				case 1: pass += c; break;
				default: break;
				}
			}
			file.close();

			WiFi.begin(ssid, pass);
			update_interval = interval.toInt();
		} else {
			Serial.println("Warning: Waiting for wifi credentials under 172.0.0.1/setup");
		}

		for (auto &s : sensors)
			s.begin();
	}

	void serve_file(String filepath) {
		String mime = "";
		if (filepath.endsWith(".html")) mime = "text/html";
		else if (filepath.endsWith(".css")) mime = "text/css";
		else if (filepath.endsWith(".js")) mime = "text/javascript";
		else if (filepath.endsWith(".ttf")) mime = "font/ttf";

		if (LittleFS.exists(filepath) && mime != "") {
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
		String ssid = server.arg("ssid"), pass = server.arg("pass");

		Serial.println("ssid: " + ssid + "\tpass: " + pass);

		if (ssid != "" && pass != "")
			server.send(200, "text/plain", "");
		else
			server.send(400, "text/plain", "Wszystkie pola muszą być wypełnione!");


		File file = LittleFS.open(wifi_config, "w");
		file.print(ssid + '\n' + pass + '\n' );
		file.close();

		WiFi.begin(ssid, pass);
	}

	void wifi_status() {
		server.send(200, "text/plain", WiFi.status() == WL_CONNECTED ? status_ok : status_bad);
	}

	void mqtt_status() {
		//server.send(200, "text/plain", WiFi.status() != WL_CONNECTED ? status_ok : status_bad);
		server.send(200, "text/plain", status_bad);
	}

	void redirect(String path) {
		server.sendHeader("Location", path, true);
		server.send(302, "text/plain", "");
		server.client().stop(); // Stop is needed because we sent no content length
	}

	bool connected() {
		return WiFi.status() != WL_CONNECTED;
	}

	void reconnect() {

	}

	void loop() {
		dnsServer.processNextRequest();
		server.handleClient();

		for (auto &s : sensors)
			s.update();

		if (!connected()) {
			reconnect();
		} else {
			for (auto &s : sensors)
				s.update();
		}

	}
private:
	IPAddress APIP{172, 0, 0, 1};

	const String wifi_config = "/wifi_config";
	const String mqtt_config = "/mqtt_config";
	const String ap_ssid = "CzujnikTemp";
	const String ap_pass = "password123";
	const String status_ok = "Ok!";
	const String status_bad = "Błąd!";

	uint32_t update_interval;

	DNSServer dnsServer;
	ESP8266WebServer server{80};

} client;

void setup() {
	Serial.begin(9600);
	while (!Serial);

	LittleFS.begin();

	Wire.begin();

	client.sensors.emplace_back();

	client.begin();
}

void loop() {
	client.loop();
}
