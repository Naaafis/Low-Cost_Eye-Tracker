#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <Arduino_JSON.h>

WiFiMulti wifiMulti;
#define USE_SERIAL Serial

// WiFi credentials
const char* SSID = "*******";
const char* PASSWORD = "*******";
const char* serverUrl = "*******";

// Define CS Pins
#define CAM_CS 34
#define LED_CS 10
#define CLK_PIN 36   // or SCK
#define DATA_PIN 35  // or MOSI

#define DECODE_MODE 9
#define INTENSITY 0x0A
#define SCAN_LIMIT 0x0B
#define SHUTDOWN 0x0C
#define DISPLAY_TEST 0x0F

#define BMPIMAGEOFFSET 66
const char bmp_header[BMPIMAGEOFFSET] PROGMEM = {
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};

byte Up[8] = { B00001000,
               B00001100,
               B11111110,
               B11111111,
               B11111111,
               B11111110,
               B00001100,
               B00001000 };

byte Down[8] = {
  B00010000,
  B00110000,
  B01111111,
  B11111111,
  B11111111,
  B01111111,
  B00110000,
  B00010000
};

byte Left[8] = {
  B00011000,
  B00111100,
  B01111110,
  B11111111,
  B00111100,
  B00111100,
  B00111100,
  B00111100
};

byte Right[8] = {
  B00111100,
  B00111100,
  B00111100,
  B00111100,
  B11111111,
  B01111110,
  B00111100,
  B00011000
};

uint8_t image_data[1024 * 32];  // allocate a buffer to hold the image data (max 1024x32 bytes for OV2640)

bool is_header = false;
int mode = 0;
int imageSize = 0;
uint8_t start_capture = 0;
#if defined(OV2640_MINI_2MP)
ArduCAM myCAM(OV2640, CAM_CS);
#else
ArduCAM myCAM(OV5642, CAM_CS);
#endif
uint8_t read_fifo_burst(ArduCAM myCAM);

void SendData(uint8_t address, uint8_t value) {
  digitalWrite(LED_CS, LOW);
  SPI.transfer(address);       // Send address.
  SPI.transfer(value);         //   Send the value.
  digitalWrite(LED_CS, HIGH);  // Finish transfer.
}

void setup() {
  // put your setup code here, to run once:
  uint8_t vid, pid;
  uint8_t temp;

  pinMode(LED_CS, OUTPUT);

  Wire.begin();
  Serial.begin(115200);

  Serial.println(F("ACK CMD ArduCAM Start! END"));
  // set the CS as an output:
  pinMode(CAM_CS, OUTPUT);
  digitalWrite(CAM_CS, HIGH);

  // initialize SPI:
  SPI.begin();
  SendData(DISPLAY_TEST, 0x00);  // Finish test mode.
  SendData(DECODE_MODE, 0x00);   //   Disable BCD mode.
  SendData(INTENSITY, 0x0e);     // Use lowest intensity.
  SendData(SCAN_LIMIT, 0x0f);    // Scan all digits.
  SendData(SHUTDOWN, 0x01);      // Turn on chip.

  wifiMulti.addAP(SSID, PASSWORD);

  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      Serial.println(F("ACK CMD SPI interface Error! END"));
      delay(1000);
      continue;
    } else {
      Serial.println(F("ACK CMD SPI interface OK. END"));
      break;
    }
  }

#if defined(OV2640_MINI_2MP)
  while (1) {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) && ((pid != 0x41) || (pid != 0x42))) {
      Serial.println(F("ACK CMD Can't find OV2640 module! END"));
      delay(1000);
      continue;
    } else {
      Serial.println(F("ACK CMD OV2640 detected. END"));
      break;
    }
  }
#else
  while (1) {
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)) {
      Serial.println(F("ACK CMD Can't find OV5642 module! END"));
      delay(1000);
      continue;
    } else {
      Serial.println(F("ACK CMD OV5642 detected. END"));
      break;
    }
  }
#endif
  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
#if defined(OV2640_MINI_2MP)
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
#else
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);  //VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
#endif
  delay(1000);
  myCAM.clear_fifo_flag();
#if !(defined(OV2640_MINI_2MP))
  myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);
#endif

  // Connect to WiFi network
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}
void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp = 0xff, temp_last = 0;
  bool is_header = false;
  if (Serial.available()) {
    temp = Serial.read();
    switch (temp) {
      case 0x10:
        mode = 1;
        temp = 0xff;
        start_capture = 1;
        Serial.println(F("ACK CMD CAM start single shoot. END"));
        Serial.println("0x10");
        break;
    }
  }
  if (mode == 1) {
    if (start_capture == 1) {
      myCAM.flush_fifo();
      myCAM.clear_fifo_flag();
      //Start capture
      myCAM.start_capture();
      start_capture = 0;
    }
    if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
      Serial.println(F("ACK CMD CAM Capture Done. END"));
      delay(50);
      read_fifo_burst(myCAM);
      //Clear the capture done flag
      myCAM.clear_fifo_flag();
      delay(50);

      // Create HTTP client
      HTTPClient http;

      // Convert the array of integers to a string
      String dataString = "";
      for (int i = 0; i < imageSize; i++) {
        String hexString = String(image_data[i], HEX);
        while (hexString.length() < 2) {
          hexString = "0" + hexString;
        }
        dataString += hexString;
        // dataString += String(image_data[i]) + ",";
      }
      // Remove the last comma
      // dataString.remove(dataString.length() - 1);

      // Make the POST request
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpResponseCode = http.POST(dataString);

      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);

      // Check for successful request
      if (httpResponseCode > 0) {
        Serial.println("POST request successful");

      } else {
        Serial.println("POST request failed");
      }

      if (payload.equals("Up")) {
        for (int i = 1; i < 9; i++) SendData(i, Up[i - 1]);
      }

      if (payload.equals("Left")) {
        for (int i = 1; i < 9; i++) SendData(i, Left[i - 1]);
      }

      if (payload.equals("Right")) {
        for (int i = 1; i < 9; i++) SendData(i, Right[i - 1]);
      }

      if (payload.equals("Down")) {
        for (int i = 1; i < 9; i++) SendData(i, Down[i - 1]);
      }

      // Free resources
      http.end();

      // Wait 5 seconds before sending another request
      delay(5000);
    }
  }
}
uint8_t read_fifo_burst(ArduCAM myCAM) {
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  imageSize = 0;
  length = myCAM.read_fifo_length();
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE)  //512 kb
  {
    Serial.println(F("ACK CMD Over size. END"));
    return 0;
  }
  if (length == 0)  //0 kb
  {
    Serial.println(F("ACK CMD Size is 0. END"));
    return 0;
  }

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();  //Set fifo burst mode
  temp = SPI.transfer(0x00);
  image_data[imageSize++] = temp;
  length--;
  while (length--) {
    temp_last = temp;
    temp = SPI.transfer(0x00);
    image_data[imageSize++] = temp;
    if (is_header == true) {
      Serial.write(temp);
    } else if ((temp == 0xD8) & (temp_last == 0xFF)) {
      is_header = true;
      Serial.println(F("ACK IMG END"));
      Serial.write(temp_last);
      Serial.write(temp);
    }
    if ((temp == 0xD9) && (temp_last == 0xFF))  //If find the end ,break while,
      break;
    delayMicroseconds(15);
  }
  myCAM.CS_HIGH();
  is_header = false;
  return 1;
}
