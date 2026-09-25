#include <WiFi.h>

const char* ssid     = "iot";
const char* password = "password";

void scanNetworks() {
  Serial.println("Scanning...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    Serial.printf("%2d: %-24s RSSI %4d dBm  CH %2d  %s\n",
      i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i),
      WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "secured");
  }
  WiFi.scanDelete();
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP address : "); Serial.println(WiFi.localIP());
    Serial.print("Gateway    : "); Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet     : "); Serial.println(WiFi.subnetMask());
    Serial.print("DNS        : "); Serial.println(WiFi.dnsIP());
    Serial.print("MAC        : "); Serial.println(WiFi.macAddress());
    Serial.print("RSSI (dBm) : "); Serial.println(WiFi.RSSI());
  } else {
    Serial.printf("\nFailed, status code %d\n", WiFi.status());
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
    delay(100);
  scanNetworks();
  connectWiFi();
}

void loop() {
  Serial.printf("RSSI now: %d dBm\n", WiFi.RSSI());   // used in step 5
  delay(2000);
  // Auto-reconnect if the link drops
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost, reconnecting...");
    WiFi.reconnect();
    delay(5000);
  }
}