// BMP280 Temp Data Logging Example
// Last Update: Feb 21, 2025
//
// This example has the BMP280 sensor connected via I2C. It takes a temperature reading every minute and records it to the SD card.
// For this example the sensor is not so important. Change out the sensor with any sensor you have.
// It is recommended the user first get familiar with the shield by going through the sketch Detailed_tests.ide.
// NOTE: Make sure the RTC time is set already, if not you'll have to do that to get proper time reading.
// NOTE: Also make sure the RTC battery is plugged to keep the time during a power off!

#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>

#define OE_PIN 5           // [LLS] Disable = LOW, Enable = HIGH. Default is D5, but can be changed to D4 on board via jumper.
#define CS_PIN 10          // [SD Card] Chip Select pin. Default is D10, but can be changed to D9 on board via jumper.
#define BMP280_ADDR 0x76    
#define LOG_INTERVAL 60000 // Take a reading every minute

RTC_DS3231 rtc;
Adafruit_BMP280 bmp;
File logFile;

void setup() {
  // RTC set .. uncomment this only if the RTC has not been set yet.
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to compile time

  Serial.begin(9600);
  pinMode(OE_PIN, OUTPUT);
  digitalWrite(OE_PIN, HIGH); // Enable output

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  if (!bmp.begin(BMP280_ADDR)) {
    Serial.println("BMP280 not found!");
    while (1);
  }

  if (!SD.begin(CS_PIN)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }

  Serial.println("Logging started...");
}

void loop() {
  DateTime now = rtc.now();
  float temp = bmp.readTemperature();

  logFile = SD.open("log.txt", FILE_WRITE);
  if (logFile) {
    logFile.print(now.year(), DEC);
    logFile.print('/');
    logFile.print(now.month(), DEC);
    logFile.print('/');
    logFile.print(now.day(), DEC);
    logFile.print(" ");
    logFile.print(now.hour(), DEC);
    logFile.print(':');
    logFile.print(now.minute(), DEC);
    logFile.print(':');
    logFile.print(now.second(), DEC);
    logFile.print(" - Temp: ");
    logFile.print(temp);
    logFile.println(" C");
    logFile.close();
    Serial.println("Logged!");
  } else {
    Serial.println("SD Write Failed!");
  }

  delay(LOG_INTERVAL);
}
