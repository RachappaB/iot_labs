#include <WiFi.h>
#include <PubSubClient.h>

const char *WIFI_SSID = "TP-Link_DF6C_Cave_2.4";
const char *WIFI_PASS = "Caveiot@123";
const char *BROKER = "192.168.1.121";
#define BENCH "b07"

#define RXD2 16
#define TXD2 17
#define T(x) "iotlab/" BENCH "/" x

WiFiClient net;
PubSubClient mqtt(net);

int ldrRaw = -1;
int threshold = 40;
bool autoMode = false;
bool alarmDark = false;
bool alarmKnown = false;
String unoBuf = "";
unsigned long lastPub = 0;

void onMessage(char *topic, byte *payload, unsigned int len) {
  String msg;

  for (unsigned int i = 0; i < len; i++) {
    msg += (char)payload[i];
  }

  msg.trim();
  String t = topic;

  Serial.printf("RX %s = %s\n", topic, msg.c_str());

  if (t == T("buzzer/set")) {
    if (msg.equalsIgnoreCase("ON")) {
      Serial2.println("B,1");
    } else if (msg.equalsIgnoreCase("OFF")) {
      Serial2.println("B,0");
    } else if (msg.equalsIgnoreCase("BEEP")) {
      Serial2.println("B,2");
    } else {
      Serial.println("Ignored: unknown buzzer command");
    }
  } else if (t == T("config/threshold")) {
    int v = msg.toInt();

    if (msg.length() > 0 && v >= 0 && v <= 100) {
      threshold = v;
      alarmKnown = false;
      Serial.printf("Threshold set to %d\n", threshold);
    } else {
      Serial.println("Ignored: invalid threshold");
    }
  } else if (t == T("config/auto")) {
    if (msg.equalsIgnoreCase("ON")) {
      autoMode = true;
    } else if (msg.equalsIgnoreCase("OFF")) {
      autoMode = false;
      Serial2.println("B,0");
    } else {
      Serial.println("Ignored: invalid auto setting");
      return;
    }

    alarmKnown = false;
    Serial.printf("Auto mode = %s\n", autoMode ? "ON" : "OFF");
  }
}

void handleUno(String line) {
  line.trim();

  if (line.startsWith("L,")) {
    ldrRaw = line.substring(2).toInt();
  } else if (line == "S,1" || line == "S,0") {
    bool on = (line == "S,1");

    mqtt.publish(T("buzzer/state"), on ? "ON" : "OFF", true);
    Serial.printf("PUB buzzer/state = %s\n", on ? "ON" : "OFF");
  }
}

void readUno() {
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '\n') {
      handleUno(unoBuf);
      unoBuf = "";
    } else if (unoBuf.length() < 20) {
      unoBuf += c;
    }
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\nIP %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    String id = String("esp32-" BENCH "-") +
                String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.printf("MQTT connect as %s ... ", id.c_str());

    if (mqtt.connect(id.c_str(), T("status"), 1, true, "offline")) {
      Serial.println("OK");

      mqtt.publish(T("status"), "online", true);
      mqtt.subscribe(T("buzzer/set"), 1);
      mqtt.subscribe(T("config/#"), 1);
    } else {
      Serial.printf("failed, rc=%d, retry in 2 s\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  connectWiFi();

  mqtt.setServer(BROKER, 1883);
  mqtt.setCallback(onMessage);
  mqtt.setBufferSize(512);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();
  readUno();

  if (ldrRaw >= 0 && millis() - lastPub >= 2000) {
    lastPub = millis();

    int pct = map(ldrRaw, 0, 1023, 100, 0);
    pct = constrain(pct, 0, 100);

    char buf[12];

    snprintf(buf, sizeof(buf), "%d", ldrRaw);
    mqtt.publish(T("ldr/raw"), buf);

    snprintf(buf, sizeof(buf), "%d", pct);
    mqtt.publish(T("ldr/percent"), buf);

    Serial.printf("PUB ldr raw=%d percent=%d\n", ldrRaw, pct);

    bool dark = pct < threshold;

    if (!alarmKnown || dark != alarmDark) {
      alarmKnown = true;
      alarmDark = dark;

      mqtt.publish(T("alarm"), dark ? "DARK" : "LIGHT", true);

      Serial.printf("PUB alarm = %s\n", dark ? "DARK" : "LIGHT");

      if (autoMode) {
        Serial2.println(dark ? "B,1" : "B,0");
      }
    }
  }
}
