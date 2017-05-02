
#define echoPin 52 // Echo Pin for ultrasonic
#define trigPin 53 // Trigger Pin for ultrasonic
#define buttonPin 41
#define CAN_FULL_CAPACITY  40 // in CM
#define SSID "ssid"
#define PASS "pass"
#define DST_IP "46.137.108.231" //aast.voidbits.co
#define DST_PORT 80
#define BAUD_RATE 115200
#define esp Serial3

String resp; //temp response buffer

bool isConnected = false;
int isBtnPressed = 0;
int lastCapacity = 0;

void setup() {
  //Setup ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //
  //Button
  pinMode(buttonPin, INPUT);
  //


  Serial.begin(BAUD_RATE);
  esp.begin(BAUD_RATE);

  //  while (!Serial);
  while (!esp);

  Serial.println("Recycle bin monitor");


  disconnectIfConnected();
  delay(100);
  resp = send_cmd("AT");
  //  Serial.println(resp);
  if (resp.indexOf("OK") >= 0) {
    Serial.println("ESP is ready");

    resp = send_cmd("AT+CWJAP?");
    //    Serial.println(resp);
    if (resp.indexOf("No AP") >= 0) {
      Serial.println("Connecting to WiFi");
      connectToWifi();
    }
    else {
      Serial.println("Already connected to WiFi");
    }

    connectToServer();
  }
  else {
    Serial.println("Error: ESP8266 is not OK. Restart Arduino.");
    while (1);
  }


}



int ticks = 0;
int currentCapacity;
int btnState = 0;
void loop() {
  //  while (Serial.available()) {
  //    esp.write(Serial.read());
  //  }
  //
  //  while (esp.available())
  //    Serial.write(esp.read());

  currentCapacity = calculatePercentage();
  isBtnPressed = digitalRead(buttonPin);
  Serial.println(ticks);
  Serial.print("Button: ");
  Serial.println(isBtnPressed == 0 ? "pressed" : "not pressed");
  Serial.print("Capacity: ");
  Serial.print(currentCapacity);
  Serial.println("%");
  Serial.println();
  if (needUpdate()) {
    trySend();
    ticks = 0;
    lastCapacity = currentCapacity;
    btnState = isBtnPressed;
    return;
  }
  lastCapacity = currentCapacity;
  btnState = isBtnPressed;
  delay(1000);
  if (ticks >= 8) {
    ticks = 0;
    trySend();
  }
  else {
    ticks += 1;
  }

}

boolean needUpdate() {
  int diff = lastCapacity - currentCapacity;
  if(diff < 0) diff = diff * -1;
  
  return (diff > 5) || (btnState != isBtnPressed);
}

void trySend() {
  if (!isConnected) {
    disconnectIfConnected();
    connectToServer();
  }
  else
    sendHttp();
}

void connectToWifi() {
  String s = "AT+CWJAP=\"";
  s += SSID;
  s += "\",\"";
  s += PASS;
  s += "\"";
  resp = send_cmd_delay(s, 4000); // give it 4 sec
  if (resp.indexOf("GOT IP") >= 0) {
    Serial.print("Connected to WiFi successfully");
  }
  else {
    Serial.println("Error: Couldn't connect to WiFi. Restart Arduino.");
    while (1);
  }

}

void disconnectIfConnected() {
  resp = send_cmd("AT+CIPCLOSE");
  //  Serial.println(resp);

}

int retries = 0;
void connectToServer() {
  String s = "AT+CIPSTART=\"TCP\",\"";
  s += DST_IP;
  s += "\",";
  s += DST_PORT;
  retries += 1;
  resp = send_cmd_delay(s, 2000);
  //  Serial.println(resp);
  if (resp.indexOf("OK") >= 0) {
    retries = 0;
    Serial.println("Connected successfully");
    isConnected = true;
    //    sendHttp();
  }
  else {
    Serial.println(resp);
    Serial.println("Error: Couldn't connect to server");
    if (retries >= 5) {
      Serial.println("Error: Couldn't connect after 5 retries. Check the address/port and restart Arduino");
      while (1);
    }
    else {
      connectToServer();
    }
  }
}

void sendHttp() {
  if (!isConnected) return;
  isBtnPressed = digitalRead(buttonPin);
  String r = "GET /api/bins/1?capacity=";
  r += currentCapacity;
  r += "&open=";
  r += (isBtnPressed == 0 ? "false" : "true");
  r += " HTTP/1.1\r\nHost: aast.voidbits.co\r\nConnection: keep-alive\r\n\r\n";
  String s = "AT+CIPSEND=";
  s += r.length();

  resp = send_cmd(s);
  //  Serial.println(resp);
  Serial.println("Sending update..");
  esp.print(r);

  Serial.println("Done sending");
  delay(1000);
  boolean ok = false;
  while (esp.available()) {
    if (esp.find("OK")) {
      ok = true;
      Serial.println("Got OK response");
      return;
    }
  }
  if (!ok) {
    Serial.println("No OK response...");
    isConnected = false;
  }
}



char c;
String send_cmd(String cmd) {
  esp.print(cmd);
  esp.print("\r\n");
  delay(500);
  String output = "";
  while (esp.available()) {
    c = esp.read();
    output += c;
  }
  return output;
}

String send_cmd_delay(String cmd, int d) {
  esp.print(cmd);
  esp.print("\r\n");
  delay(d);
  String output = "";
  while (esp.available()) {
    c = esp.read();
    output += c;
  }
  return output;
}


int calculatePercentage() {
  int cm = calculateDInCM();
  if (cm > CAN_FULL_CAPACITY)
    cm = CAN_FULL_CAPACITY;
  return (cm * 100.0 / CAN_FULL_CAPACITY);
}

int calculateDInCM()
{
  double duration = 0.0, distance = 0.0;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = (duration / 2) / 29.1;
  int output = distance;
  return output;
}
