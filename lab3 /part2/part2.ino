#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid     = "iot";
const char* password = "password";

#define LED_PIN  2
#define DHT_PIN  4
#define DHT_TYPE DHT11   // change to DHT22 if needed

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);
bool ledOn = false;

String page() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  String s = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width'>";
  s += "<meta http-equiv='refresh' content='5'><title>ESP32 Lab</title></head><body style='font-family:sans-serif;text-align:center'>";
  s += "<h2>ESP32 Web Server</h2>";
  s += "<p>Temperature: " + (isnan(t) ? String("--") : String(t, 1)) + " &deg;C</p>";
  s += "<p>Humidity: "    + (isnan(h) ? String("--") : String(h, 1)) + " %</p>";
  s += "<p>LED is <b>" + String(ledOn ? "ON" : "OFF") + "</b></p>";
  s += "<a href='/led/on'><button>ON</button></a> <a href='/led/off'><button>OFF</button></a>";
  s += "</body></html>";
  return s;
}

void handleRoot()   { server.send(200, "text/html", page()); }
void handleLedOn()  { ledOn = true;  digitalWrite(LED_PIN, HIGH); server.sendHeader("Location", "/"); server.send(303); }
void handleLedOff() { ledOn = false; digitalWrite(LED_PIN, LOW);  server.sendHeader("Location", "/"); server.send(303); }
void handleData() {
  String json = "{\"temperature\":" + String(dht.readTemperature(), 1) +
                ",\"humidity\":"    + String(dht.readHumidity(), 1) +
                ",\"led\":"         + String(ledOn ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}
void handleNotFound() { server.send(404, "text/plain", "Not found"); }

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print("\nOpen http://"); Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}