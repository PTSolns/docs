// Uno R3 Get Started - Testing sketch default install 
// Last Update: March 10, 2024
// Sketch version: v1.0
// 
// Use this sketch to test various features quickly. The features are:
// 1) Onboard LED blinking
// 2) Reset makes onboard LED blink in fast pattern of 4 blinks.
// 3) I2C scanner 
//    - Plug an I2C device into either the 5V or 3.3V I2C bus, or the QWIIC connector and the device addresses are scanned and printed.
// 4) Fade LED on pin 9
//    - LED + Resistor from pin 9 to GND shows LED fading in and out. Ensure a resistor is put in series!
// 5) Analog read on pin A0
//    - Reads the value on A0 pin. Insert wire between A0 and 5V and 1023 is printed. Insert wire between A0 and GND and 0 is printed. Use a pot to change the A0 value from 0 to 1023.

#include <Wire.h>

const unsigned long LED_on_time  = 100; // Onboard LED ON time
const unsigned long LED_off_time = 200; // Onboard LED OFF time
int flag_LED = 0;
unsigned long LEDMillis = 0.0;

const unsigned long I2C_scanner_time = 5000; // Repeat I2C scan every 5000mS
unsigned long I2C_scannerMillis = 0.0;

int LED_fade = 9;
int brightness = 0;
int fadeAmount = 5;
const unsigned long fade_time = 30; // Fade brightness change every 30mS
unsigned long fadeMillis = 0.0;

int analog_pin = A0;
const unsigned long analog_time = 1000; // Read Analog pin every 1000mS
unsigned long analogMillis = 0.0;
int sensorValue;


void setup() {
  Wire.begin();
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_fade, OUTPUT);
  
  // Blink at reset
  delay(500);
  blinkReset(LED_BUILTIN, 4, 50, 50);
  delay(500);

  // Print instructions
  Serial.println("");
  Serial.println("Getting Started Default Sketch");
  Serial.println("- To test onboard LED.");
  Serial.println("    Observe onboard LED flash 100mS ON and 200mS OFF.");
  Serial.println("    Can also test Pin 13 (LED_BUILTIN) by connecting LED plus resistor from Pin 13 to GND. Flashing pattern should be the same as onboard LED.");
  Serial.println("- To test Reset(RST).");
  Serial.println("    Press reset and observe onboard LED blinking pattern.");
  Serial.println("    Should be some flashes (not coded by this test sketch), 500mS delay, four quick flashes (coded by this test sketch), 500mS delay, then start normal run loop().");
  Serial.println("- To test LED fade.");
  Serial.println("    Connect LED to Pin 9 (high voltage) and a resistor. Resistor to GND.");
  Serial.println("    Make sure polarity of LED is correct (flat side is towards GND).");
  Serial.println("- To test analog read.");
  Serial.println("    Connect wire to pin A0, observe Serial printout.");
  Serial.println("    Connect other end of wire to 5V, Serial printout should read 1023.");
  Serial.println("    Connect other end of wire to GND, Serial printout should read 0.");
  Serial.println("    Connect pot with A0 in middle, Serial printout should read between 0 and 1023.");
  Serial.println("- To test Serial Print.");
  Serial.println("    Observe TX LED flashing every approx 500mS.");
  Serial.println("    Open Serial Monitor and set 9600 baud.");
  Serial.println("");
}

void loop() {

  // Loops for onboard LED
  if (((millis() - LEDMillis) >= LED_on_time) && (flag_LED == 0)){
    flag_LED = 1;
    digitalWrite(LED_BUILTIN, LOW); // Turn OFF
  } else if (((millis() - LEDMillis) >= (LED_on_time + LED_off_time)) && (flag_LED == 1)){
    flag_LED = 0;
    digitalWrite(LED_BUILTIN, HIGH); // Turn ON
    LEDMillis = millis();
  }


  // I2C scanner
  if ((millis() - I2C_scannerMillis) >= I2C_scanner_time){
    I2C_scannerMillis = millis();
    I2Cscanner();
  }

  
  // Fading LED
  if ((millis() - fadeMillis) >= fade_time){
    fadeMillis = millis();
    analogWrite(LED_fade, brightness);

    brightness = brightness + fadeAmount;

    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  }


  // Read Analog
  if ((millis() - analogMillis) >= analog_time){
    analogMillis = millis();
    sensorValue = analogRead(analog_pin);
    Serial.print("Analog Read on A0 = ");
    Serial.println(sensorValue);
  }



}

void I2Cscanner(){
  int nDevices = 0;

  Serial.println("");
  Serial.println("Scanning...");

  for (byte address = 1; address < 127; ++address) {
    // The i2c_scanner uses the return value of
    // the Wire.endTransmission to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");

      ++nDevices;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }
 // delay(5000); // Wait 5 seconds for next scan
}



void blinkReset(int LED, int number_of_blink, int time_on_blink, int time_off_blink) {  
  int blink_counter = 0;

  while (blink_counter <= number_of_blink) {
		digitalWrite(LED, HIGH);
		delay(time_on_blink); 
		digitalWrite(LED, LOW);
		delay(time_off_blink); 

		blink_counter = blink_counter + 1;
	}
}