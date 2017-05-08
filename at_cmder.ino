
#define SSID "ssid"
#define PASS "pass"
#define DST_IP "54.247.71.175"
#define DST_PORT 80
#define BAUD_RATE 115200
#define esp Serial3

String resp;
void setup() {
  Serial.begin(BAUD_RATE);
  esp.begin(BAUD_RATE);

  while (!Serial);
  while (!esp);
  Serial.println("ESP8266 Demo on Mega2560");

}

void loop() {
  while (Serial.available()) {
    esp.write(Serial.read());
  }

  while (esp.available())
    Serial.write(esp.read());

}
