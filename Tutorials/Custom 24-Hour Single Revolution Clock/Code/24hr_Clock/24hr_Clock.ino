// 24Hrs Clock
// Last Update: Jan 6, 2025

// DESCRIPTION
// First run the "offset_calc.ide" to calibrate the zero position.
// Upon powing up, the driver sends signals to stepper to move a few positions. This happens even when the stepper pins are all low.
// To disable this, a P-Channel MOSFET was added controlled by digital pin D3 to cut main power to driver upon startup. Power is manually turned ON when D3 is pulled LOW.
// 

#include <AccelStepper.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

// Constants
const int STEPS_PER_REV    = 2048;           // Steps for a full revolution (28BYJ-48)
const int EEPROM_ADDRESS   = 0;              // EEPROM address to store stepsCounter
const float HOURS_PER_REV  = 24.0;           // Full revolution = 24 hours
const float STEPS_PER_HOUR = STEPS_PER_REV / HOURS_PER_REV; // Steps per hour

// Global variables
int stepsCounter = 0;                       // Steps away from absolute zero

RTC_DS3231 rtc;                             // RTC object
AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 10, 9, 11); // Stepper pins

void setup() {
  pinMode(3, OUTPUT);    // Set pin D3 as an output
  digitalWrite(3, HIGH); // Set D3 to HIGH to disable power to the stepper driver

  Serial.begin(9600);

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Initialize stepper motor
  stepper.setMaxSpeed(500.0);
  stepper.setAcceleration(100.0);

  // Retrieve stepsCounter from EEPROM
  stepsCounter = readIntFromEEPROM(EEPROM_ADDRESS);
  Serial.print("Steps from zero position on startup: ");
  Serial.println(stepsCounter);

  digitalWrite(3, LOW); // Set D3 to LOW to enable power to the stepper driver

  // Move stepper to 24Hrs zero position to check
  if (stepsCounter != 0){
    stepper.move(-stepsCounter);
    stepper.runToPosition();
  };

  delay(3000);

  stepsCounter = 0; // Reset, since now we are at the zero position

  // Move stepper to correct position based on RTC time
  moveToCurrentTimePosition();
}

void loop() {
  // Continuously run the stepper
  stepper.run();

  // Update the position at regular intervals
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 60000) { // Update every minute
    lastUpdate = millis();
    moveToCurrentTimePosition();
  }
}

// Function to move the stepper to the correct position based on the RTC time
void moveToCurrentTimePosition() {
  DateTime now = rtc.now();

  // Calculate current hour fraction (e.g., 12:30 = 12.5)
  float hourFraction = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  // Calculate target steps based on current time
  long targetSteps = hourFraction * STEPS_PER_HOUR;

  // Calculate the number of steps to move
  long stepsToMove = targetSteps - stepsCounter;
  //long stepsToMove = targetSteps;

  // Wrap stepsToMove within one revolution (optional, for safety)
  stepsToMove = stepsToMove % STEPS_PER_REV;
  if (stepsToMove < 0) stepsToMove += STEPS_PER_REV;

  // Move the stepper and update stepsCounter
  stepper.move(stepsToMove);
  stepper.runToPosition();
  stepsCounter = targetSteps;

  // Save updated stepsCounter to EEPROM
  saveIntToEEPROM(EEPROM_ADDRESS, stepsCounter);

  // Print debugging information
  Serial.print("Current Time: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
  Serial.print("Steps moved to: ");
  Serial.println(stepsCounter);
}

// Function to save an integer to EEPROM
void saveIntToEEPROM(int address, int value) {
  byte lowByte = value & 0xFF;        // Extract the lower 8 bits
  byte highByte = (value >> 8) & 0xFF; // Extract the upper 8 bits
  EEPROM.write(address, lowByte);    // Write lower byte
  EEPROM.write(address + 1, highByte); // Write upper byte
}

// Function to read an integer from EEPROM
int readIntFromEEPROM(int address) {
  byte lowByte = EEPROM.read(address);      // Read lower byte
  byte highByte = EEPROM.read(address + 1); // Read upper byte
  return (highByte << 8) | lowByte;         // Combine bytes into an integer
}
