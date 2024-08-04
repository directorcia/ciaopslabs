
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "io_configs.h"
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
// Camera and WiFi settings
#define CS_PIN 16
ArduCAM myCAM(OV3640, CS_PIN); 
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish imageFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/feed1");
// connect to adafruit io via MQTT
void connect() {
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
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}
void captureAndSendImage() {
  myCAM.start_capture();
  Serial.println("Start capture");
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  Serial.println("Capture done");
  uint32_t length = myCAM.read_fifo_length();
  if (length == 0) {
    Serial.println("Image length is 0, capture failed");
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  uint8_t buf[256];
  int buf_pos = 0;
  while (length--) {
    buf[buf_pos++] = SPI.transfer(0x00);
    if (buf_pos == 256) {
      imageFeed.publish(buf, 256);
      buf_pos = 0;
    }
  }
  if (buf_pos > 0) {
    imageFeed.publish(buf, buf_pos);
  }
  myCAM.CS_HIGH();
  Serial.println("Image sent to Adafruit IO");
}
void setup() {
 
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Init serial port");
  Serial.print("Connecting to wifi");
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(250);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  // connect to adafruit
  connect();
  Wire.begin();
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);
   // Initialize the camera
//  myCAM.begin();
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV3640_set_JPEG_size(OV3640_320x240); // Adjust resolution as needed
 }
void loop() {
  mqtt.processPackets(10000);
  // Capture and send image
  captureAndSendImage();
  delay(60000); // Capture and send every minute
}
