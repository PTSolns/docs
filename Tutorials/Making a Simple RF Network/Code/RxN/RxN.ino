#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <PTSolns_I2CBackpack.h>

// NRF24L01+ Configuration
#define CE_PIN 8    // Chip Enable
#define CSN_PIN 9   // Chip Select Not (corrected)
#define CHANNEL 76   // Default channel

// Create NRF24L01+ instance
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// Create LCD instance
I2C_LCD LCD;

// Buffer to store last 4 temperatures
float tempBuffer[4] = {0, 0, 0, 0};

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
    radio.openReadingPipe(0, address);
    radio.startListening();
    Serial.println("NRF24L01+ initialized and listening");
    
    // Initialize LCD
    Serial.println("Initializing LCD");
    LCD.begin(0x3F);
    LCD.print("Waiting for data...");
    Serial.println("LCD initialized");
}

void loop() {
    if (radio.available()) {
        Serial.println("Data available");
        float temperature;
        radio.read(&temperature, sizeof(temperature));
        Serial.print("Received Temp: ");
        Serial.println(temperature);
        
        // Shift old data down
        tempBuffer[3] = tempBuffer[2];
        tempBuffer[2] = tempBuffer[1];
        tempBuffer[1] = tempBuffer[0];
        tempBuffer[0] = temperature;
        
        // Display on LCD
        LCD.clear();
        for (int i = 0; i < 4; i++) {
            LCD.setCursor(0, i);
            LCD.print("Temp: ");
            LCD.print(tempBuffer[i]);
            LCD.print(" C");
        }
    } else {
        Serial.println("No data available");
    }
    delay(500);
}
