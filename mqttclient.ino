#include <ESP8266WiFi.h>
#include <FS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "config.h"
#include "sensor.h"

class MQTTClient : PubSubClient {
public:
	std::vector<Sensor> sensors;

	void begin() {
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP(ap_ssid, ap_pass);

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
		server.on("/gen_204",      HTTP_GET,  [this]{ redirect("/setup"); });
		server.on("/generate_204", HTTP_GET,  [this]{ redirect("/setup"); });
		server.on("/success.txt",  HTTP_GET,  [this]{ redirect("/setup"); });
		server.onNotFound([this]{ serve_file(server.uri()); });

		String ssid, pass;
		if (config.load_wifi_config(ssid, pass)) {
			Serial.println("Connecting to WiFi");
			WiFi.begin(ssid, pass);
		} else {
			Serial.println("Warning: Waiting for wifi credentials under 172.0.0.1/setup");
		}

		for (auto &s : sensors)
			s.begin();
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
		String ssid = server.arg("ssid"), pass = server.arg("pass");

		Serial.println("ssid: " + ssid + "\tpass: " + pass);

		if (ssid != "" && pass != "")
			server.send(200, "text/plain", "");
		else
			server.send(400, "text/plain", "Wszystkie pola muszą być wypełnione!");

		config.save_wifi_config(ssid, pass);

		WiFi.begin(ssid, pass);
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
			for (auto &s : sensors)
				s.update();
		}

	}
private:
	IPAddress APIP{172, 0, 0, 1};

	const String ap_ssid = "CzujnikTemp";
	const String ap_pass = "password123";
	const String status_ok = "Ok!";
	const String status_bad = "Błąd!";

	uint32_t update_interval;

	DNSServer dnsServer;
	ESP8266WebServer server{80};

	Config config;
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
