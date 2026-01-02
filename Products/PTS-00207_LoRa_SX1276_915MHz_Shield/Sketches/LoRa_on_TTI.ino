// Example: Push Temp and Hum on LoRa Shield to TTI cloud
// Last Update: Dec 29, 2025
//
// DESCRIPTION
// This basic example shows how to push temperature and humidity data from the AHT20 sensor onboard the PTSolns LoRa SX1276 915MHz Shield to The Things Industries (TTI) cloud.
// To make this example work, the below prerequisites must be completed and working.
// The PTSolns LoRa SX1276 915MHz Shield works on 915MHz (North America) and hence this example is exclusively designed for that frequency.
// Tested compatibility for Uno R3+ or similar board.
//
// HARDWARE
// - PTSolns LoRa SX1276 915MHz Shield
// - PTSolns Uno R3+, or similar Uno microcontroller development board
//
// PREREQUISITES
// - The PTSolns LoRa SX1276 915MHz Shield + Uno (call it the "LoRa device") must be within range of a gateway.
//   Check here to see if you have coverage: https://www.thethingsnetwork.org/map
//   If you do not have coverage, this example is not going to work as no gateway is within range to pick up the messages from the LoRa device.
//   If that is the case, the only way to make this work is by getting and configuring your own gateway. This is beyond the scope of this tutorial. There are many tutorials online that can help.
//   We have set up our own gateway for testing and used the SenseCap M2 by Seeed Studio. However, the user can try any other equivalent technology.
// - Have an account with The Things Industries (TTI). The free account is sufficient.
//   An application must be added (+ Add application), and then add a device.
//   To add a device press "+ Add -> Register end device in an application".
//   The following details are important to set correctly:
//     Frequency plan: United States 902-928 MHz, FSB 2 (used by TTN)
//     LoRaWAN version: LoRaWAN Specification 1.0.2
//     Regional Parameters version: RP001 Regional Parameters 1.0.2 revision B
//     Under advanced settings, YOU MUST SELECT Activation mode: Activation by personalization (ABP). DO NOT SELECT Over the air activation (OTAA).
//     Let the Device address, NwkSKey, and AppSKey be generated. Note these down and enter them below in the respective variables in the section marked "Configuration".
// - Install the libraries PTSolns_AHTx and TinyLoRa from within the Library Manager.
// - Payload formatters
//   In the device on TTI, go to Payload formatters, select Custom Javascript formatter and paste the following (then press "Save changes"):
//     function decodeUplink(input) {
//       var data = {};
//
//       var tempRaw = (input.bytes[0] << 8) | input.bytes[1];
// 
//       if (tempRaw > 0x7FFF) {
//         tempRaw -= 0x10000;
//       }
//       data.temperature = tempRaw / 100.0;
//
//       var humRaw = (input.bytes[2] << 8) | input.bytes[3];
//       data.humidity = humRaw / 100.0;
//
//       return {
//         data: data,
//         warnings: [],
//         errors: []
//       };
//     }
//
// TROUBLESHOOTING
// 1) The device was pushing data to the TTI cloud, but now it has stopped working.
//    This can happen if the frame counter is not matching, caused by uploading the sketch again to the Uno after it was already communicating with the cloud.
//    The simplest way to fix this is to enter the device settings, expand the options in the "Network Layer", and press "Reset session and MAC state".
// 2) The LoRa device is not able to connect to the TTI cloud.
//    There are many reasons why this might happen. One of the most common is that the activation mode is not set to Activation by personalization (ABP).
//    This is set when the device is registered and the default option for OTAA is used. The only way to fix this is to register a new device. See the above instructions for details.
//    Also make sure the Device address, NwkSKey, and AppSKey are properly copied and pasted into the variables below. These must match the Device Session information and the sketch below.


#include <PTSolns_AHTx.h>
#include <TinyLoRa.h>
#include <SPI.h>
#include <Wire.h>


/************************** Configuration ***********************************/

// AFTER SETTING UP THE DEVICE IN TTI GET THESE THREE KEYS AND ENTER THEM HERE.
// TTI Session Keys
unsigned char NwkSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char AppSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char DevAddr[4]  = { 0x00, 0x00, 0x00, 0x00 };

/************************** END OF Configuration *****************************/


TinyLoRa lora = TinyLoRa(2, 10, 9); // Pinouts for the PTSolns LoRa SX1276 915MHz Shield. DO NOT CHANGE
PTSolns_AHTx aht;

unsigned char loraData[4]; // Data Packet (4 bytes: 2 for Temp, 2 for Humidity)
const unsigned int sendInterval = 60; // Time in seconds to set the sending interval.


void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize AHT20
  Serial.print(F("Starting AHT20..."));
  if (!aht.begin()) { 
    Serial.println(F("Failed to initialize AHT20!")); 
    while(1); 
  }
  Serial.println(F("OK"));

  // Initialize LoRa
  Serial.print(F("Starting LoRa..."));
  lora.setChannel(MULTI);
  lora.setDatarate(SF7BW125);

  if(!lora.begin()) {
    Serial.println(F("Failed to initialize LoRa!"));
    while(true);
  }
  Serial.println(F("OK"));
}

void loop() {
  float t = 0;
  float h = 0;

  AHTxStatus st = aht.readTemperatureHumidity(t, h, 120);

  if (st == AHTX_OK) {
    Serial.print(F("Temp: ")); Serial.print(t);
    Serial.print(F(" C, Hum: ")); Serial.print(h); Serial.println(F(" %"));

    // --- Encoding Payload ---
    // Multiply by 100 to preserve 2 decimal places in an integer
    int16_t tempInt = round(t * 100);
    int16_t humidInt = round(h * 100);

    // Pack into 4-byte array
    loraData[0] = highByte(tempInt);
    loraData[1] = lowByte(tempInt);
    loraData[2] = highByte(humidInt);
    loraData[3] = lowByte(humidInt);

    // --- Sending ---
    Serial.print(F("Sending Frame: ")); Serial.println(lora.frameCounter);
    lora.sendData(loraData, sizeof(loraData), lora.frameCounter);
    lora.frameCounter++;

    // Visual confirmation
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);

  } else {
    Serial.print(F("Sensor Error Code: ")); Serial.println((int)st);
  }

  Serial.println(F("Waiting for next interval..."));
  delay(sendInterval * 1000);
}