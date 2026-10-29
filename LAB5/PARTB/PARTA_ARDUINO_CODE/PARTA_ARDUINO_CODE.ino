
#include <SoftwareSerial.h>

#define LDR_PIN A0
#define BUZZER_PIN 8

SoftwareSerial link(10, 11);

String linkBuf = "", usbBuf = "";
unsigned long lastSend = 0;

void setBuzzer(bool on) {
  digitalWrite(BUZZER_PIN, on ? LOW : HIGH);
  link.print("S,");
  link.println(on ? 1 : 0);
  Serial.println(on ? "Buzzer ON" : "Buzzer OFF");
}

void beep3() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
  }
  setBuzzer(false);
}

void handleLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  Serial.print("CMD: ");
  Serial.println(line);

  if (line == "B,1") setBuzzer(true);
  else if (line == "B,0") setBuzzer(false);
  else if (line == "B,2") beep3();
  else Serial.println("Unknown command");
}

void readLines(Stream &s, String &buf) {
  while (s.available()) {
    char c = s.read();

    if (c == '\n') {
      handleLine(buf);
      buf = "";
    } else if (buf.length() < 20) {
      buf += c;
    }
  }
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);

  Serial.begin(9600);
  link.begin(9600);

  Serial.println("Uno I/O node ready");
}

void loop() {
  readLines(link, linkBuf);
  readLines(Serial, usbBuf);

  if (millis() - lastSend >= 500) {
    lastSend = millis();

    int raw = analogRead(LDR_PIN);

    link.print("L,");
    link.println(raw);

    Serial.print("L,");
    Serial.println(raw);
  }
}
