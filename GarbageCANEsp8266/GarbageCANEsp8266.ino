#include <SparkFunESP8266WiFi.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
#define LEVER_SWITCH_PIN 6
#define echoPin 3 // Echo Pin
#define trigPin 2 // Trigger Pin
#define SSID "ssid"
#define PASS "password"
#define DST_IP "192.168.1.4"
#define DST_PORT 54321
#define MiddleRate 25
#define FullRate 5
#define CANCapacity 100

const int BinId = 123;
int pressSwitch = 0;
ESP8266Client client;

void setup() {
  //LCD and switch
  lcd.begin(16, 2);
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
  delay(1000);
  Serial.print("Connecting to ");
  Serial.println(DST_IP);

  //  connectToServer();
}

int connectToServer() {
  int retVal;
  while ((retVal = client.connect(DST_IP, DST_PORT)) <= 0)
  {
    Serial.print("Failed to connect to server. ");
    Serial.println((retVal == -1 ? "Timeout" : "Fail"));
  }
  Serial.println("Connected to server successfully");
  return 1;
}

void sendCapacity() {
  if (connectToServer() == 1) {
    client.print(calculatePercentage());
    client.stop();
  }
}

void loop() {

  pressSwitch = digitalRead(LEVER_SWITCH_PIN);
  if (pressSwitch)
  {
    Serial.println("pressed");
    int distance = calculateDInCM();
    if (distance <= FullRate)
    {
      render("Full", "Closed", distance);
      sendCapacity();
    }
    else if (distance <= MiddleRate && distance > FullRate)
    {
      render("Available", "Closed", distance);
      sendCapacity();
    }
    else if (distance > MiddleRate)
    {
      render("Available", "Closed", distance);
      sendCapacity();
    }
  }
  else
  {
    render("Please close", "Opened", 0);
    sendCapacity();
  }
  delay(5000);
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
