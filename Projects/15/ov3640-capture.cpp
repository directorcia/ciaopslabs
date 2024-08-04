#include <ESP8266WiFi.h>
#include <SPI.h>
#include <memorysaver.h>
const int CS = 16; // Chip select pin (GPIO16)
ArduCAM myCAM(OV3640, CS);
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  // Set CS as OUTPUT
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  // Initialize SPI
  SPI.begin();
  // Initialize ArduCAM
  Serial.println(F("ArduCAM Start!"));
  // Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  // Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  uint8_t temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("SPI interface Error!"));
    while (1);
  }
  // Initialize OV3640 sensor
  myCAM.InitCAM();
//  if (myCAM.InitCAM() != 0) {
//    Serial.println(F("OV3640 detected."));
//  } else {
//    Serial.println(F("OV3640 detect failed"));
//    while (1);
//  }
}
void captureImage() {
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  Serial.println(F("Start Capture"));
  
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  Serial.println(F("Capture Done."));
  
  uint32_t length = myCAM.read_fifo_length();
  if (length >= MAX_FIFO_SIZE) {
    Serial.println(F("Over size."));
    return;
  }
  if (length == 0) {
    Serial.println(F("Size is 0."));
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  
  // You can add code here to send the image data over WiFi or save it
  // For now, we'll just print the first few bytes
  for (int i = 0; i < 32; i++) {
    Serial.print(SPI.transfer(0x00), HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  myCAM.CS_HIGH();
}
void loop() {
  // Your main code here
  // For example, you can add code to capture an image:
  captureImage();
  delay(5000); // Wait for 5 seconds before next capture
}