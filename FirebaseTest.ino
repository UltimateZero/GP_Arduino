#include <SparkFunESP8266WiFi.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
#define LEVER_SWITCH_PIN 6
#define echoPin 3 // Echo Pin
#define trigPin 2 // Trigger Pin
#define SSID "ssid"
#define PASS "password"
#define DST_IP "5.10.124.141" //bluemix server
#define DST_PORT 80
#define MiddleRate 25
#define FullRate 5
#define CANCapacity 100



const int BinId = 123;
int pressSwitch = 0;

ESP8266Client client;
void setup() {
  //LCD and switch
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Setting up.. Id=123");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEVER_SWITCH_PIN, INPUT);
  //

  Serial.begin(115200);
  while (esp8266.begin() != true)
  {
    Serial.println("Error connecting to ESP8266.");
    delay(1000);
  }
  Serial.println("Connected to ESP8266.");

  if (esp8266.status() <= 0)
  {
    while (esp8266.connect(SSID, PASS) < 0)
      delay(1000);
    Serial.println("Connected to WiFi");
  }
  else {
    Serial.println("Already Connected to WiFi");
  }
  delay(1000);

//  clientSetup();
}


void sendCapacity() {
  // ESP8266Client connect([server], [port]) is used to 
  // connect to a server (const char * or IPAddress) on
  // a specified port.
  // Returns: 1 on success, 2 on already connected,
  // negative on fail (-1=TIMEOUT, -3=FAIL).
  int retVal = client.connect(DST_IP, DST_PORT);
  if (retVal <= 0)
  {
    Serial.print(F("Failed to connect to server. Code: "));
    Serial.println(retVal);
    return;
  }
  Serial.println("Connect to server successfully");
  // print and write can be used to send data to a connected
  // client connection.
  client.print(getCapacityHttp());
  delay(3000);
    // available() will return the number of characters
  // currently in the receive buffer.
  receivePackets();
  
  // connected() is a boolean return value - 1 if the 
  // connection is active, 0 if it's closed.
//  if (client.connected())
//    client.stop(); // stop() closes a TCP connection.
}

void receivePackets() {
    while (client.available())
      Serial.write(client.read()); // read() gets the FIFO char
  
}


String getCapacityHttp() {
   String httpRequest = "GET /getBinUpdate.php?id=1&capacity=";
   httpRequest += calculatePercentage();
   httpRequest += " HTTP/1.1\r\n";
   httpRequest += "Host: firebase-test.eu-gb.mybluemix.net\r\n";
   httpRequest += "Connection: close\r\n\r\n";
   return httpRequest;
}


void loop() {
  // Wait for client connections
//  serverReceive();
//  // Print on LCD
//  pressSwitch = digitalRead(LEVER_SWITCH_PIN);
//  if (pressSwitch)
//  {
//    Serial.println("pressed");
//    int distance = calculateDInCM();
//    if (distance <= FullRate)
//    {
//      render("Full", "Closed", distance);
//    }
//    else if (distance <= MiddleRate && distance > FullRate)
//    {
//      render("Available", "Closed", distance);
//    }
//    else if (distance > MiddleRate)
//    {
//      render("Available", "Closed", distance);
//    }
//  }
//  else
//  {
//    render("Please close", "Opened", 0);
//  }
  if(client.connected()) {
    receivePackets();
  }
  else {
    sendCapacity(); //start
  }
  
  delay(1000);
}


int calculatePercentage() {
  int distance = calculateDInCM();
  return ((distance * CANCapacity) / 100);
}

void render(String state, String pos, int distance)
{
  Serial.println("render called");
  lcd.clear();
  lcd.setCursor(0, 0);
  if (state.equals("Available"))
  {
    lcd.print(state + ((distance * CANCapacity) / 100) + "%");
    lcd.setCursor(0, 1);
    lcd.print(pos);
  }
  else
  {
    lcd.print(state);
    lcd.setCursor(0, 1);
    lcd.print(pos);
  }
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
