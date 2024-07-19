#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "DHT.h"
#include <BH1750.h>
#include <ArduinoJson.h>

#define DHTPIN D2
#define DHTTYPE DHT22
#define LED_PIN1 D6
#define LED_PIN2 D7
#define BH1750_ADDRESS A0
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter(BH1750_ADDRESS);

float humi; 
float temp;
float lux;

//WiFiSetting
const char* ssid = "Ahihi";
const char* password = "1236uitt";

WiFiClient client;

//ThingSpeakSetting
const int channelID = 2548945;
String writeAPIKey = "2AUH373HEDETRL9K";
String readAPIKey = "EFU9QQ9QK6LNJKLO";
const char* server = "api.thingspeak.com";

//FunctionDeclare
void wifiSetup();
void thingConnect();
void readSensor(void);
void printData(void);
void fetchData();

void setup() {
  Serial.begin(9600);
  Serial.println("Start Reading Sensor!\n");
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  wifiSetup();
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN1, HIGH);
  digitalWrite(LED_PIN2, HIGH);
  thingConnect();
  readSensor();
  printData();
  fetchData();
}

void wifiSetup() {
  Serial.print("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("\r\nWiFi connected");
}

void thingConnect() {
  if (client.connect(server, 80)) {
    String body = "field1=" + String(temp, 1) + "&field2=" + String(humi, 1) + "&field3=" + String(lux, 1);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");
  }
}

void printData(void) {
  Serial.printf("Temp: %s°C - Hum: %s% - Lux: %s%\r\n", String(temp, 1).c_str(), String(humi, 1).c_str(), String(lux, 1).c_str());
}

void readSensor(void) {
  temp 	= dht.readTemperature();
  humi 	= dht.readHumidity();
  lux = lightMeter.readLightLevel();
}
void fetchData() {
  if (client.connect(server, 80)) {
    String url = "/channels/" + String(channelID) + "/feeds.json?api_key=" + readAPIKey + "&results=1";
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    String line;
    while (client.connected() || client.available()) {
      if (client.available()) {
        line = client.readStringUntil('\n');
        if (line.startsWith("{\"feeds\"")) {
          parseAndControlLEDs(line);
          break;
        }
      }
    }
  }
  client.stop();
}

void parseAndControlLEDs(const String &data) {
  StaticJsonDocument<1024> jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, data);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject feeds = jsonBuffer["feeds"][0];
  String field4 = feeds["field4"];
  String field5 = feeds["field5"];

  if (field4 == "1") {
    digitalWrite(LED_PIN1, HIGH);
  } else {
    digitalWrite(LED_PIN1, LOW);
  }

  if (field5 == "1") {
    digitalWrite(LED_PIN2, HIGH);
  } else {
    digitalWrite(LED_PIN2, LOW);
  }
}
