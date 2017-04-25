#include <SparkFunESP8266WiFi.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
#define LEVER_SWITCH_PIN 6
#define echoPin 3 // Echo Pin
#define trigPin 2 // Trigger Pin
#define SSID "ssid"
#define PASS "password"
#define SRC_PORT 54321
#define MiddleRate 25
#define FullRate 5
#define CANCapacity 100

const int BinId = 123;
int pressSwitch = 0;

ESP8266Server server = ESP8266Server(SRC_PORT);

void setup() {
  //LCD and switch
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Setting up..");
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

  serverSetup();
}


void serverSetup() {
  server.begin();
  Serial.print("Server started! Go to ");
  IPAddress localIp = esp8266.localIP();
  Serial.println(localIp);
  Serial.println();
  lcd.setCursor(0, 1);
  lcd.print(localIp[2] + "." + localIp[3]);
}

void serverReceive()
{
  // available() is an ESP8266Server function which will
  // return an ESP8266Client object for printing and reading.
  // available() has one parameter -- a timeout value. This
  // is the number of milliseconds the function waits,
  // checking for a connection.
  ESP8266Client client = server.available(500);

  if (client)
  {
    Serial.println("Client Connected!");
    while (client.connected())
    {
      sendCapacity(client);
      client.stop();
    }
  }

}

void sendCapacity(ESP8266Client client) {
  client.print(calculatePercentage());
  delay(2000);
}

void loop() {
  // Wait for client connections
  serverReceive();
  // Print on LCD
  pressSwitch = digitalRead(LEVER_SWITCH_PIN);
  if (pressSwitch)
  {
    Serial.println("pressed");
    int distance = calculateDInCM();
    if (distance <= FullRate)
    {
      render("Full", "Closed", distance);
    }
    else if (distance <= MiddleRate && distance > FullRate)
    {
      render("Available", "Closed", distance);
    }
    else if (distance > MiddleRate)
    {
      render("Available", "Closed", distance);
    }
  }
  else
  {
    render("Please close", "Opened", 0);
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
