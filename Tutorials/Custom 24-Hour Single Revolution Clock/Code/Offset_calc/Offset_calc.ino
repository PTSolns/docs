// Offset Calculator
// Last Update: Jan 8, 2025

// DESCRIPTION
// Use this sketch to move the hour hand to 24hrs position.
// Increment the hour hand until it is at 24hrs. This is the zero position.
// This zero position is to act like an absolute zero, which the stepper inherently does not have. 
// The zero position is reset in the EEPROM.
// When running the full Clock sketch, each time the stepper is moved, the counter is updated in the EEPROM.

// In the event of a power outage, the stepper will be able to remember how many steps it was away 
// from the absolute zero (24hrs) and recalibrate itself to the proper position.



// User-specified time for calibration
int targetSteps = -5; // {0-2048} Change this value so that after several runs of this sketch the hour hand is exactly on 24hrs.
                       // Use the Nano Flip reset button to slowly increment the hour hand forward (clockwise) to the 24hrs position.
                       // If the hour hand is far away from the 24hrs position, then increase the targetSteps to a large value.

int stepsCounter = 0; // The goal of this sketch is to align stepsCounter = 0 to the hour hand to point at 24hrs.
                      // In the full Clock sketch the stepsCounter is incremented every time the stepper moves a step.


#include <AccelStepper.h>
#include <EEPROM.h>
#include <RTClib.h>

const int EEPROM_ADDRESS = 0; // Starting address to store the integer

RTC_DS3231 rtc;                             // RTC object
AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 10, 9, 11); // IN1, IN3, IN2, IN4 on ULN2003


void setup() {
  pinMode(3, OUTPUT);    // Set pin D3 as an output
  digitalWrite(3, HIGH); // Set D3 to HIGH to disable power to the stepper driver

  // Set up the stepper motor
  stepper.setMaxSpeed(500.0); // Maximum speed in steps per second
  stepper.setAcceleration(100.0); // Acceleration in steps per second^2

  digitalWrite(3, LOW); // Set D3 to LOW to enable power to the stepper driver

  stepper.moveTo(targetSteps);
  stepper.runToPosition(); // This is a blocking function.

  Serial.begin(9600);

  // Save the integer to EEPROM
  saveIntToEEPROM(EEPROM_ADDRESS, stepsCounter);
  Serial.println("Absolute zero calibrated when stepsCounter = 0 and hour hand points to 24hrs.");

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.println("RTC Date and Time Set.");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to compile time
}

void loop() {
  // Nothing needed in the loop
}

// Function to save an integer to EEPROM
void saveIntToEEPROM(int address, int value) {
  byte lowByte = value & 0xFF;        // Extract the lower 8 bits
  byte highByte = (value >> 8) & 0xFF; // Extract the upper 8 bits
  EEPROM.write(address, lowByte);    // Write lower byte
  EEPROM.write(address + 1, highByte); // Write upper byte
}