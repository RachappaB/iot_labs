d#include <DHT.h>

#define DHTPIN     4
#define DHTTYPE    DHT11
#define LED_PIN    26
#define BUZZER_PIN 27

const float TEMP_HIGH = 30.0;   // alert ON above this (°C)
const float HYST      = 1.0;    // alert OFF below TEMP_HIGH - HYST

DHT dht(DHTPIN, DHTTYPE);
bool alertActive = false;
unsigned long lastRead = 0, lastBeep = 0;
bool buzzerOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  dht.begin();
}





void loop() {
  unsigned long now = millis();

  // 1) Read the sensor every 2 s (non-blocking)
  if (now - lastRead >= 2000) {
    lastRead = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Sensor read failed");
    } else {
      Serial.printf("Temp: %.1f °C | Humidity: %.1f %% | Alert: %s\n",
                    t, h, alertActive ? "ON" : "OFF");

      // 2) Threshold with hysteresis
      if (!alertActive && t > TEMP_HIGH) {
                alertActive = true;
        Serial.println(">>> ALERT ON: temperature above threshold");
      } else if (alertActive && t < TEMP_HIGH - HYST) {
        alertActive = false;
        Serial.println(">>> ALERT OFF: temperature back to normal");
      }
    }
  }

  // 3) Outputs: LED steady, buzzer beeps 200 ms on / 800 ms off
  digitalWrite(LED_PIN, alertActive ? HIGH : LOW);
  if (alertActive) {
    unsigned long period = buzzerOn ? 200 : 800;
    if (now - lastBeep >= period) {
      lastBeep = now;
      buzzerOn = !buzzerOn;
      digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
    }
  } else {
    buzzerOn = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
}