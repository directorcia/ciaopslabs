/*
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control
For use with the Adafruit Motor Shield v2
----> http://www.adafruit.com/products/1438
*/
#include <Adafruit_MotorShield.h>
#include <Arduino.h>
#include <SPI.h>
#include "DFRobot_RGBLCD1602.h"
#include "iot_configs.h"
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "Adafruit_VL53L0X.h"   // Distance Sensor

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);
// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *myMotor1 = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS.getMotor(2);
Adafruit_DCMotor *myMotor3 = AFMS.getMotor(3);
Adafruit_DCMotor *myMotor4 = AFMS.getMotor(4);
// You can also make another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);

DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x6B ,/*lcdCols*/16,/*lcdRows*/2);  //16 characters and 2 lines of show

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoffbutton");
void MQTT_connect();

Adafruit_VL53L0X lox = Adafruit_VL53L0X();  // default address = 0x30

void setup() {
  Serial.begin(115200);           // set up Serial library at 9600 bps
  lcd.init();
  lcd.setCursor(0,0);     // Column 0, Row 0
  lcd.print("                   ");
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");
  
  if (!AFMS.begin()) {         // create with the default frequency 1.6KHz
  // if (!AFMS.begin(1000)) {  // OR with a different frequency, say 1KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");
  lcd.print("Motor found");

lcd.setCursor(0,0);     // Column 0, Row 0
lcd.print("Motor run     ");
WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
while (WiFi.status() != WL_CONNECTED){
  delay(250);
  Serial.print(".");
}
Serial.println("WiFi connected");
lcd.setCursor(0,1);     // Column 0, Row 1
lcd.print("WiFi connect     ");

mqtt.subscribe(&onoffbutton);

if (!lox.begin()) {
  Serial.print("Failed to boot VL53L0X");
  while(1);
}

}

uint32_t speed = 0;

void loop() {
VL53L0X_RangingMeasurementData_t measure;

MQTT_connect();
Adafruit_MQTT_Subscribe *subscription;
lcd.setCursor(0,1);     // Column 0, Row 0
lcd.print("                  ");
lcd.setCursor(0,0);     // Column 0, Row 0
lcd.print("Speed:          ");
lcd.setCursor(7,0);     // Column 0, Row 0
lcd.print(speed);
Serial.println(F("Looping"));
myMotor1->setSpeed(speed);
myMotor2->setSpeed(speed);
myMotor3->setSpeed(speed);
myMotor4->setSpeed(speed);
lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
int distance = measure.RangeMilliMeter; // Save range as an integer
if (measure.RangeStatus != 4) {  // phase failures have incorrect data
  Serial.print("Distance (mm): "); 
  Serial.println(measure.RangeMilliMeter);
  int distance = measure.RangeMilliMeter; // Save range as an integer
  lcd.setCursor(0,1);     // Column 1, Row 2;

  if (distance >= 325) {
    lcd.print("Dist(mm):     ");
  }
  else if (distance >= 200 && distance < 325) {
    lcd.print("SLOW(mm):     ");
    if (speed > 20) {
      speed = speed - 20;
    }
    else {
      speed = 0;
    }
  }
  else if (distance < 200) {
    lcd.print("STOP(mm):     ");
    speed = 0;
  }

  lcd.setCursor(9,1);    // Column 10, Row 2 
  lcd.print(distance);   // Display distance in mm
}
while (subscription = mqtt.readSubscription(2000)) {
    Serial.println(F("Received button data"));
    lcd.setCursor(0,1);     // Column 0, Row 0
//    lcd.print("Button press       ");
    if (subscription == &onoffbutton) {
      String ledstatus = (char *)onoffbutton.lastread;
      if (ledstatus == "*") { // Start
        speed = speed - 5;
        if (speed < 75) {
          speed = 75;
        }
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Slower           ");
      }
      if (ledstatus == "0") { // Start
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Turn left           ");
        myMotor1->run(FORWARD);
        myMotor2->run(BACKWARD);
        myMotor3->run(FORWARD);
        myMotor4->run(BACKWARD);
      }
      else if (ledstatus == "#") { // Stop
        speed = speed + 5;
        if (speed > 255){
          speed = 255;
        }
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Faster           ");
      }
      else if (ledstatus == "1") { // Left Forward
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Foward Left       ");
        myMotor1->run(FORWARD);
        myMotor2->run(RELEASE);
        myMotor3->run(RELEASE);
        myMotor4->run(FORWARD);
      }
      else if (ledstatus == "2") { // Straight Forward
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Foward           ");
        myMotor1->run(FORWARD);
        myMotor2->run(FORWARD);
        myMotor3->run(FORWARD);
        myMotor4->run(FORWARD);
      }
      else if (ledstatus == "3") { // Right Forward
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Forward Right           ");
        myMotor1->run(RELEASE);
        myMotor2->run(FORWARD);
        myMotor3->run(FORWARD);
        myMotor4->run(RELEASE);
      }
      else if (ledstatus == "4") { // Left Forward
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Left           ");
        myMotor1->run(FORWARD);
        myMotor2->run(BACKWARD);
        myMotor3->run(BACKWARD);
        myMotor4->run(FORWARD);
      }
      else if (ledstatus == "5") { // Right Forward
        lcd.setCursor(0,0);     // Column 0, Row 0
        if (speed >= 75) {
          lcd.print("Stop           ");
          speed = 0;
          myMotor1->run(RELEASE);
          myMotor2->run(RELEASE);
          myMotor3->run(RELEASE);
          myMotor4->run(RELEASE);
        }
        else {
          lcd.print("Start           ");
          speed=100;
        }
      }
      else if (ledstatus == "6") { // Right Forward
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Right           ");
        myMotor1->run(BACKWARD);
        myMotor2->run(FORWARD);
        myMotor3->run(FORWARD);
        myMotor4->run(BACKWARD);
      }
      else if (ledstatus == "7") { //Left Back
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Back Left           ");
        myMotor1->run(RELEASE);
        myMotor2->run(BACKWARD);
        myMotor3->run(BACKWARD);
        myMotor4->run(RELEASE);
      }
      else if (ledstatus == "8") { // Straight Back
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Back           ");
        myMotor1->run(BACKWARD);
        myMotor2->run(BACKWARD);
        myMotor3->run(BACKWARD);
        myMotor4->run(BACKWARD);
      }
      else if (ledstatus == "9") { // Right Back
        lcd.setCursor(0,0);     // Column 0, Row 0
        lcd.print("Back Right           ");
        myMotor1->run(BACKWARD);
        myMotor2->run(RELEASE);
        myMotor3->run(RELEASE);
        myMotor4->run(BACKWARD);
      }
    }
  }
}

void MQTT_connect() {
  if (mqtt.connected()) 
  {
    return;
  }
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }
    if(ret >= 0)
      mqtt.disconnect();
    Serial.println(F("Retrying connection..."));
    delay(5000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}


/*
Change the RGBaddr value based on the hardware version
-----------------------------------------
       Moudule        | Version| RGBAddr|
-----------------------------------------
  LCD1602 Module      |  V1.0  | 0x60   |
-----------------------------------------
  LCD1602 Module      |  V1.1  | 0x6B   |
-----------------------------------------
  LCD1602 RGB Module  |  V1.0  | 0x60   |
-----------------------------------------
  LCD1602 RGB Module  |  V2.0  | 0x2D   |
-----------------------------------------
*/