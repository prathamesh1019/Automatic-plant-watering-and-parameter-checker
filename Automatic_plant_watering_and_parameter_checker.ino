#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// RS485
#define MAX485_DE 8
#define MAX485_RE 8

SoftwareSerial modbusSerial(10, 11); // RO, DI
ModbusMaster node;

// Soil sensor
#define SOIL_PIN A0

void preTransmission() {
  digitalWrite(MAX485_DE, 1);
  digitalWrite(MAX485_RE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE, 0);
}

void setup() {
  Serial.begin(9600);
  modbusSerial.begin(9600);

  pinMode(MAX485_DE, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE, 0);

  node.begin(1, modbusSerial); // Slave ID = 1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(10, 20);
  display.println("NPK Monitor");
  display.display();
  delay(2000);
}

void loop() {

  // Read NPK
  uint8_t result;
  result = node.readHoldingRegisters(0x0000, 3);

  int nitrogen = 0;
  int phosphorus = 0;
  int potassium = 0;

  if (result == node.ku8MBSuccess) {
    nitrogen   = node.getResponseBuffer(0);
    phosphorus = node.getResponseBuffer(1);
    potassium  = node.getResponseBuffer(2);
  }

  // Read Soil Moisture
  int soilRaw = analogRead(SOIL_PIN);
  int soilPercent = map(soilRaw, 1023, 300, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // Serial Monitor
  Serial.print("N: "); Serial.print(nitrogen);
  Serial.print(" P: "); Serial.print(phosphorus);
  Serial.print(" K: "); Serial.print(potassium);
  Serial.print(" Soil: "); Serial.print(soilPercent);
  Serial.println("%");

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Soil Nutrients");

  display.setCursor(0, 15);
  display.print("Nitrogen: ");
  display.print(nitrogen);

  display.setCursor(0, 25);
  display.print("Phosphor: ");
  display.print(phosphorus);

  display.setCursor(0, 35);
  display.print("Potassium: ");
  display.print(potassium);

  display.setCursor(0, 50);
  display.print("Moisture: ");
  display.print(soilPercent);
  display.print("%");

  display.display();

  delay(2000);
}
