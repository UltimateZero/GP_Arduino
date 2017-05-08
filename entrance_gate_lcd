
#define trig1Pin 52
#define echo1Pin 53
#define trig2Pin 50
#define echo2Pin 51
#define SSID "ssid"
#define PASS "password"
#define DST_IP "46.137.108.231" //aast.voidbits.co
#define DST_PORT 80
#define BAUD_RATE 115200
#define esp Serial3
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(42, 43, 41, 40, 39, 38);

String resp; //temp response buffer
bool isConnected = false;
String httpResp;
void setup() {
  //ultrasonic sensors
  pinMode(trig1Pin, OUTPUT);
  pinMode(echo1Pin, INPUT);
  pinMode(trig2Pin, OUTPUT);
  pinMode(echo2Pin, INPUT);
  //

  Serial.begin(BAUD_RATE);
  esp.begin(BAUD_RATE);

  Serial.println("Entrance gate monitor");
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Setting up..");

  disconnectIfConnected();
  delay(100);
  while (1) {
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
      break;
    }
    else {
      disconnectIfConnected();
      Serial.println("Error: ESP8266 is not OK. Restart Arduino.");
      delay(500);
      //    while (1);
    }
  }
  lcd.clear();
  lcd.print("Counter: 0");
}


bool passedOne = false;
bool passedTwo = false;
int dir; //1: entry, 2: exit
int currentDistance1;
int currentDistance2;
int counter = 0;
int type = 0;
long time_diff = 0;
bool wasAvailable = false;
void loop() {
  if (esp.available()) {
    Serial.println("START ESP RECEIVE");
    wasAvailable = true;
  }
  while (esp.available())
    Serial.write(esp.read());
  if (wasAvailable) {
    Serial.println("END ESP RECEIVE");
    wasAvailable = false;
  }
  if (passedOne && passedTwo) {
    if (calculateDistance(1) < 90 || calculateDistance(2) < 90) {
      Serial.println("Still blocking");
      lcd.clear();
      lcd.print("BLOCKED");
      delay(300);
      return;
    }
    passedOne = false;
    passedTwo = false;
    //do update stuff
    Serial.print("Detected: ");
    Serial.println(dir == 1 ? "Entry" : "Exit");
    Serial.print("Counter: ");
    if (dir == 1) counter += 1;
    else if (dir == 2) counter -= 1;
    lcd.clear();
    lcd.print("Counter: ");
    lcd.print(counter);
    Serial.println(counter);
    type = dir;
    trySend();
    time_diff = millis();
    delay(400);
  }

  currentDistance1 = calculateDistance(1);
  if (currentDistance1 < 50) {
    if (passedOne) return;
    passedOne = true;
    if (passedTwo) { //exit
      dir = 2; //exit
    }
    else { //entry

    }

  }
  currentDistance2 = calculateDistance(2);
  if (currentDistance2 < 50) {
    if (passedTwo) return;
    passedTwo = true;
    if (passedOne) { //entry
      dir = 1;
    }
    else { //exit

    }
  }

  if (millis() - time_diff > 10000) {
    type = 0;
    trySend();
    time_diff = millis();
  }

  delay(200);
}




int calculateDistance(int which)
{
  int trigPin = which == 1 ? trig1Pin : trig2Pin;
  int echoPin = which == 1 ? echo1Pin : echo2Pin;
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


void trySend() {
  if (!isConnected) {
    disconnectIfConnected();
    connectToServer();
    sendHttp();
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
  resp = send_cmd_delay(s, 10000); // give it 4 sec
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
  String r = "GET /api/gate/startDiscovery?type=";
  r += type;
  r += " HTTP/1.1\r\nHost: aast.voidbits.co\r\nConnection: keep-alive\r\n\r\n";
  String s = "AT+CIPSEND=";
  s += r.length();

  resp = send_cmd(s);
  //  Serial.println(resp);
  Serial.println("Sending update..");
  esp.print(r);
  Serial.println("Done sending");
  delay(1000);
  resp = "";
  boolean ok = false;
  char c;
  while (esp.available()) {
    c = esp.read();
    resp += c;
  }
  if (resp.indexOf("200 OK") != -1) {
    ok = true;
    Serial.println("Got OK response");
    return;
  }
  else if (resp.indexOf("400 Bad Request") != -1) {
    ok = true;
    Serial.println("Got Bad Request");
    return;
  }
  else if (resp.indexOf("404 Not Found") != -1) {
    ok = true;
    Serial.println("Got 404");
    return;
  }
  if (!ok) {
    Serial.println("No OK response...");
    Serial.println(resp);
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

