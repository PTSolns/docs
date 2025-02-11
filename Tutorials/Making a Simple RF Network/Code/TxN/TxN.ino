#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <SparkFunBME280.h>

// NRF24L01+ Configuration
#define CE_PIN 7    // Chip Enable
#define CSN_PIN 10   // Chip Select Not
#define CHANNEL 76   // Default channel

// Create NRF24L01+ instance
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// Create BME280 sensor instance
BME280 sensor;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    Serial.println("Initializing NRF24L01+");
    if (!radio.begin()) {
        Serial.println("NRF24L01+ initialization failed!");
        while (1);
    }
    radio.setChannel(CHANNEL);
    radio.setPALevel(RF24_PA_MIN); // Low power mode
    radio.openWritingPipe(address);
    radio.stopListening();
    Serial.println("NRF24L01+ initialized");
    
    // Initialize BME280 sensor
    Serial.println("Initializing BME280 sensor");
    if (sensor.beginI2C() == false) {
        Serial.println("BME280 not detected!");
        while (1);
    }
    Serial.println("BME280 sensor initialized");
}

void loop() {
    float temperature = sensor.readTempC();
    Serial.print("Sending Temp: ");
    Serial.println(temperature);
    bool success = radio.write(&temperature, sizeof(temperature));
    if (success) {
        Serial.println("Transmission successful");
    } else {
        Serial.println("Transmission failed");
    }
    delay(1000); // Send every second
}