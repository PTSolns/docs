// Square Wave output
// Last Update: Feb 21, 2025
//
// NOTE: An oscilloscope is required!
//
// This sketch uses EEPROM to store an integer (0–3) that determines the DS3231 square wave output frequency. On each reset:
// 1) The EEPROM value is read to get the last selected frequency mode.
// 2) The value increments (0 → 1 → 2 → 3 → 0), ensuring it cycles through all four modes.
// 3) The updated value is stored back in EEPROM to persist across resets.
// 4) The DS3231 square wave output is set based on the new mode:
//      0 → 1 Hz
//      1 → 1.024 kHz
//      2 → 4.096 kHz
//      3 → 8.192 kHz
// Each reset button press changes the frequency, cycling through the four options.
// Each mode also sets the onboard LED to blink. Mode 0 at 1 Hz, Mode 1 at 2 Hz, Mode 2 at 4 Hz, and Mode 3 at 8 Hz.
// Connect an oscilloscope to the square wave output on the screw terminal on the shield and observe the different square waves by cycling through with the reset button.
//
// It is recommended the user first get familiar with the shield by going through the sketch Detailed_tests.ide.
//
// TIP: When modifying this sketch or making your own, always make sure to disable write protect WP on the external EEPROM. If you don't do it manually in the code, the hardware will enable WP via an external pull-up.

#include <Wire.h>
#include <RTClib.h>

#define EEPROM_ADDR 0x57       // External EEPROM I2C address
#define EEPROM_MEM_ADDR 0x0000 // Memory address storing mode
#define WRITE_PROTECT_PIN 8    // Pin controlling EEPROM write protection

RTC_DS3231 rtc;
uint8_t mode;
unsigned long previousMillis = 0;
unsigned long interval = 0;
bool ledState = false;

void writeEEPROM(uint16_t memAddr, uint8_t data) {
  digitalWrite(WRITE_PROTECT_PIN, LOW); // Disable write protect

  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((uint8_t)(memAddr >> 8));  
  Wire.write((uint8_t)(memAddr & 0xFF)); 
  Wire.write(data);
  Wire.endTransmission();
  delay(10); 
}

uint8_t readEEPROM(uint16_t memAddr) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((uint8_t)(memAddr >> 8));  
  Wire.write((uint8_t)(memAddr & 0xFF)); 
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  pinMode(WRITE_PROTECT_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(WRITE_PROTECT_PIN, LOW);

  mode = (readEEPROM(EEPROM_MEM_ADDR) + 1) % 4;
  writeEEPROM(EEPROM_MEM_ADDR, mode);

  switch (mode) {
    case 0: 
      rtc.writeSqwPinMode(DS3231_SquareWave1Hz); 
      Serial.println("Square wave output: 1 Hz"); 
      interval = 500;  // 1 Hz blinking (500ms ON, 500ms OFF)
      break;
    case 1: 
      rtc.writeSqwPinMode(DS3231_SquareWave1kHz); 
      Serial.println("Square wave output: 1.024 kHz"); 
      interval = 250;  // 2 Hz blinking (250ms ON, 250ms OFF)
      break;
    case 2: 
      rtc.writeSqwPinMode(DS3231_SquareWave4kHz); 
      Serial.println("Square wave output: 4.096 kHz"); 
      interval = 125;  // 4 Hz blinking (125ms ON, 125ms OFF)
      break;
    case 3: 
      rtc.writeSqwPinMode(DS3231_SquareWave8kHz); 
      Serial.println("Square wave output: 8.192 kHz"); 
      interval = 62;   // 8 Hz blinking (62ms ON, 62ms OFF)
      break;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }
}