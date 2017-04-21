  //#include <SoftwareSerial.h>
  //use mega Serial 2 for serial monitor; Serial 1 on pins 19 (RX) and 18 (TX);// Serial2 on pins 17 (RX) and 16 (TX), Serial3 on pins 15 (RX) and 14 (TX).
  #include <LiquidCrystal.h>
  LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
  #define LEVER_SWITCH_PIN 6
  #define echoPin 3 // Echo Pin
  #define trigPin 2 // Trigger Pin
  #define SSID "Santos"
  #define PASS "midomaney93"
  #define DST_IP "192.168.1.4"
  #define DST_PORT 54321
  #define MiddleRate 25
  #define FullRate 5
  #define CANCapacity 100
  int pressSwitch = 0;
  
  void setup() 
  {
     lcd.begin(16, 2);
     pinMode(trigPin, OUTPUT);
     pinMode(echoPin, INPUT);
     pinMode(LEVER_SWITCH_PIN,INPUT);
     Serial2.begin(115200);
     Serial.begin(115200);
     while(!Serial); 
     while(!Serial2);
     while(Serial2.available()>0)
     Serial2.read();
     //setupESP();
     delay(1000);
     Serial.println("setup called");
     lcd.print("Setting up..");
     while(!connectWiFi());
  }

  void loop() 
  {
    pressSwitch = digitalRead(LEVER_SWITCH_PIN);
    if(pressSwitch)
    {
      Serial.println("pressed");
      int distance = calculateDInCM();
      if(distance<=FullRate)
      {
        render("Full","Closed",distance);
        while(!sendMSG("full"));
      }
      else if(distance<=MiddleRate && distance > FullRate)
      {
        render("Available","Closed",distance);
        while(!sendMSG("middle"));
      }
      else if(distance>MiddleRate)
      {
        render("Available","Closed",distance);
      }
    }
    else
    {
      render("Please close","Opened",0);
    }
    delay(500);
  }

  boolean sendMSG(String msg)
  {
    String cmd = "AT+CIPSTART=\"TCP\",\"";
     cmd += DST_IP;
     cmd += "\","+DST_PORT;
     Serial2.println(cmd);
     //dbgSerial.println(cmd);
     Serial.println(cmd);
     if(Serial2.find("Error"))
     {
      return false; 
     } 
     cmd = msg+"\r\n\r\n";
     Serial2.print("AT+CIPSEND=");
     Serial2.println(cmd.length());  
     if(Serial2.find(">"))
     {
       //dbgSerial.print(">");
       Serial.print(">");
       //return true;
     }
     else
     {
       Serial2.println("AT+CIPCLOSE");
       Serial.println("connect timeout");
       delay(1000);
       return false;
     } 
     Serial2.print(cmd);
     delay(2000);
     //Serial.find("+IPD");
     while (Serial2.available())
     {
       char c = Serial2.read();
       //dbgSerial.write(c);
       Serial.write(c);
       //if(c=='\r') dbgSerial.print('\n');
       if(c=='\r') Serial.print('\n');
       if(Serial2.find("SEND OK"))
       {
        delay(1000);
        return true; 
       }else return false;
     }
  }
  
  boolean connectWiFi()
     {
       Serial2.println("AT+CWMODE=1");
       String cmd="AT+CWJAP=\"";
       cmd+=SSID;
       cmd+="\",\"";
       cmd+=PASS;
       cmd+="\"";
       Serial2.println(cmd);
       Serial.println(cmd);
       delay(2000);
       if(Serial2.find("OK"))
       {
         Serial.println("OK, Connected to WiFi.");
         return true;
       }
       else
       {
         Serial.println("Can not connect to the WiFi.");
         return false;
       }
     }
     
  //dont use this function its useless
  void setupESP()
   {
    while(1)
    {
      Serial2.println("AT+RST");
      delay(5000);
      Serial2.flush();
      if(Serial2.find("Ready")||Serial2.find("ready"))
       {
         Serial.println("Module is ready");
         break;
       }
       else
       {
          Serial.println("Esp isn't ready!");
       }
    }
  }

  void render(String state, String pos, int distance)
  {
    Serial.println("render called");
    lcd.clear();
    lcd.setCursor(0, 0);
    if(state.equals("Available"))
    {
      lcd.print(state+((distance*CANCapacity)/100)+"%");
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
    double duration=0.0, distance=0.0;
    digitalWrite(trigPin,LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin,HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    duration = pulseIn(echoPin, HIGH);
     
     //Calculate the distance (in cm) based on the speed of sound.
     distance = (duration/2) / 29.1;
     int output = distance;
     return output;
  }
