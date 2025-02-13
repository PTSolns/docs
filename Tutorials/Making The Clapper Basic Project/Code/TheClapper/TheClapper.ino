// Tutorial: Making "The Clapper" with the digital sound sensor
// Last Update: Nov 5, 2024
//
// DESCRIPTION
// This quick tutorial uses the Nano Flip and the NTEA-LG to reproduce the popular "The Clapper".
//

// Pin definitions
const int soundSensorPin = 2;   // Digital sound sensor input pin
const int ledPin = 13;          // Built-in LED pin

// Timing variables
unsigned long firstClapTime = 0;
unsigned long secondClapTime = 0;
unsigned long lastClapTime = 0;
bool clapDetected = false;
bool ledState = false;

// Time limits in milliseconds
const unsigned long minClapInterval = 100;    // Minimum time between claps
const unsigned long maxClapInterval = 2000;   // Maximum time between claps
const unsigned long debounceTime = 300;       // Time to ignore subsequent signals after a clap

void setup() {
  pinMode(soundSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  // Read the sensor and check for clap
  if (digitalRead(soundSensorPin) == HIGH) {
    unsigned long currentTime = millis();
    
    // Check if enough time has passed since the last clap to consider it a new clap
    if (currentTime - lastClapTime >= debounceTime) {
      lastClapTime = currentTime; // Update the last clap time for debounce

      if (!clapDetected) {
        // First clap detected
        firstClapTime = currentTime;
        clapDetected = true;
      } else {
        // Second clap detected
        secondClapTime = currentTime;

        // Check if the second clap is within the valid time range
        if ((secondClapTime - firstClapTime) >= minClapInterval && 
            (secondClapTime - firstClapTime) <= maxClapInterval) {

          // Toggle LED state
          ledState = !ledState;
          digitalWrite(ledPin, ledState);
          clapDetected = false; // Reset clap detection

        } else if ((secondClapTime - firstClapTime) > maxClapInterval) {
          // Reset if time between claps exceeds the max interval
          clapDetected = false;
        }
      }
    }
  }
}