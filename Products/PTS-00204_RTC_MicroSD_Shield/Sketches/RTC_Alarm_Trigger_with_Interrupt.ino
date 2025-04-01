// RTC Alarm Trigger with Interrupt (The Final Countdown)
// Last Update: Feb 21, 2025
//
// This example uses only the RTC feature on the shield. 
// A countdown is started and after 10 seconds, an Alarm (alarm(1)) is triggered which causes an interrupt.
// During the interrupt the onboard LED flashes for 3 seconds. Then the alarm is cleared and the cycle is repeated.
// This example uses alarm(1), but the RTC also has a second alarm. There are key differences:
//    Alarm 1 (Higher Precision)
//      Can trigger per second (down to the exact second).
//      Can be set to match seconds, minutes, hours, day, or date.
//      Supports once-per-second triggers or at specific times.
//      Best for precise, timed events (e.g., triggering every 10 seconds).
//    Alarm 2 (Lower Precision)
//      Can trigger per minute (not per second).
//      Can be set to match minutes, hours, day, or date.
//      Cannot be set to trigger at an exact second, only whole minutes.
//      Best for less frequent events (e.g., every 5 minutes, hourly events).
//
// It is recommended the user first get familiar with the shield by going through the sketch Detailed_tests.ide.

#include <Wire.h>
#include <RTClib.h>

// Pin definitions
#define RTC_INT_PIN 3  // RTC Interrupt pin (D3)

// Variables for configuration
const unsigned long alarmDuration        = 10000; // 10 seconds until alarm
const unsigned long ledFlashDuration     = 3000;  // 3 seconds of LED flashing, then alarm is cleared
const unsigned long ledFlashInterval     = 50;    // 10Hz flashing (50ms on/off)
const unsigned long serialUpdateInterval = 1000;  // Update serial every second

// Global variables
volatile bool alarmTriggered = false;
unsigned long alarmStartTime = 0;
unsigned long lastSerialUpdate = 0;
unsigned long lastLedToggle = 0;
bool ledState = LOW;

RTC_DS3231 rtc;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Check if RTC lost power and set the time if necessary
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Configure RTC interrupt pin
  pinMode(RTC_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), alarmISR, FALLING);

  // Configure built-in LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Set the initial alarm
  setAlarm();
}

void loop() {
  unsigned long currentTime = millis();

  // Handle serial monitor updates
  if (currentTime - lastSerialUpdate >= serialUpdateInterval) {
    lastSerialUpdate = currentTime;

    if (!alarmTriggered) {
      // Display countdown to alarm (10 to 1)
      unsigned long secondsToAlarm = (alarmStartTime + alarmDuration - currentTime) / 1000;
      Serial.print("Seconds to alarm: ");
      Serial.println(secondsToAlarm + 1); // Add 1 to shift the countdown
    } else {
      // Display countdown to alarm clear (3 to 1)
      unsigned long secondsToClear = (ledFlashDuration - (currentTime - alarmStartTime)) / 1000;
      Serial.print("Seconds to clear alarm: ");
      Serial.println(secondsToClear + 1); // Add 1 to shift the countdown
    }
  }

  // Handle LED flashing when alarm is triggered
  if (alarmTriggered) {
    if (currentTime - lastLedToggle >= ledFlashInterval) {
      lastLedToggle = currentTime;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }

    // Clear the alarm after 3 seconds
    if (currentTime - alarmStartTime >= ledFlashDuration) {
      clearAlarm();
    }
  }
}

void setAlarm() {
  // Set the alarm to trigger in 10 seconds
  DateTime now = rtc.now();
  DateTime alarmTime = now + TimeSpan(alarmDuration / 1000);
  rtc.setAlarm1(alarmTime, DS3231_A1_Second); // Match on seconds
  rtc.clearAlarm(1); // Clear any previous alarm
  rtc.writeSqwPinMode(DS3231_OFF); // Disable square wave output

  // Record the start time for the countdown
  alarmStartTime = millis();
}

void clearAlarm() {
  // Clear the alarm and reset variables
  rtc.clearAlarm(1);
  alarmTriggered = false;
  digitalWrite(LED_BUILTIN, LOW);

  // Set the next alarm
  setAlarm();
}

void alarmISR() {
  // Handle the interrupt
  alarmTriggered = true;
  alarmStartTime = millis();
}