#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);                          // USB
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);   // link to Uno
  Serial.println("Bridge test: type B,1 / B,0 / B,2");
}

void loop() {
  while (Serial2.available()) Serial.write(Serial2.read());   // Uno -> PC
  while (Serial.available())  Serial2.write(Serial.read());   // PC -> Uno
}
