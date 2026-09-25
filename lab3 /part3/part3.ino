#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* ssid     = "iot";
const char* password = "password";
const char* apiKey   = "NZ08JABOBCKQL7LT";
const char* server   = "http://api.thingspeak.com/update";

#define DHT_PIN  4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

const unsigned long INTERVAL_MS = 20000;  // free ThingSpeak accounts accept at most one update per 15 s
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWi-Fi connected");
}

void loop() {
  if (millis() - lastSend < INTERVAL_MS) return;
  lastSend = millis();

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) { Serial.println("Sensor read failed"); return; }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(t, 1) + "&field2=" + String(h, 1);
    http.begin(url);
    int code = http.GET();z
    Serial.printf("T=%.1f C  H=%.1f %%  -> HTTP %d, entry id %s\n",
                  t, h, code, http.getString().c_str());
    http.end();
  } else {
    WiFi.reconnect();
   }
}