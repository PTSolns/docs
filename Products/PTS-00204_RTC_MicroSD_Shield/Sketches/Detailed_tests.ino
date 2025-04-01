// Testing RTC MicroSD Shield
// Last Update: Feb 21, 2025
//
// This sketch provides a detailed explanation of most features of the shield. It has several tests set up for the user to explore. Check the various tests below.
// 
// RTC MicroSD Shield Compatibility:
// The following boards where checked to be compatible, but other similar boards will likely also work:
//    PTSolns Uno R3+ :)
//    Arduino Uno R2 Wifi
//    Arduino Uno R3
//    Arduino Uno R4 Wifi
//    Arduino Uno R4 Minima
//    Arduino Leonardo
//    Arduino Mega 2560
// NOT Compatible with 3.3V boards, such as the Arduino Due.
//
// Battery Note:
// It is not requried to use the coin battery for the RTC. You can use the shield without the battery. However, if power goes out, the RTC will not keep the time.
// Use the CR2032 or the ML2032 coin battery with the RTC. 
// Although the RTC DS3231 is capable to trickle charge the ML2032 (NOT the CR2032), this feature is permanently disabled on this shield.


// Required libraries
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// Hardware configurations required for the below tests
#define OE_PIN 5            // [LLS] Disable = LOW, Enable = HIGH. Default is D5, but can be changed to D4 on board via jumper.
#define CD_Pin 7            // [SD Card] SD card detection pin. Default is D7, but can be changed to D6 on board via jumper.
#define WRITE_PROTECT_PIN 8 // [EEPROM] Digital pin controlling WP. Cannot be changed.
#define CS_PIN 10           // [SD Card] Chip Select pin. Default is D10, but can be changed to D9 on board via jumper.

// Other Hardware configurations (not used in this sketch, just as an FYI).
// 1) There is a jumper on the back labelled "RST", which is by default closed. 
//    This connects the reset of the microcontroller development board to the reset of the RTC. When pressed, it resets the RTC. Note it does NOT clear any initialization and time/date data are kept even with a reset.
//    If the jumper is cut (opened) then pressing the reset on the microcontroller development board no longer triggers a reset on the RTC.
//    What does the Reset on the RTC do:
//    A) I2C Bus Recovery: If the I2C bus locks up due to noise or a failed transmission, pulling RST low helps recover communication.
//    B) Interrupt Handling: If an alarm interrupt is triggered but not acknowledged, resetting can clear it.
//    C) SQW Output Reset: If the Square Wave (SQW) output is misbehaving, RST can restore proper operation.
// 2) There is a jumper on the back labelled "PWR LED", which is by default closed. Cutting (opening) this jumper disabled the power LED.
// 3) There are three jumpers on the back labelled "COPI", "CIPO", and "SCK", which are by default open. The SPI bus connections from the SD card are connected to the ICSP interface on the board.
//    However, if using a microcontroller development board that doesn't have this ICSP header, the jumpers can be closed to make the SPI bus connections.
// 4) There is a jumper on the back labelled "INT/SQR", which is by default set to pin D3. It can be changed to pin D2. 
//    A) Alarm Interrupts: The DS3231 has two alarms (Alarm 1 and Alarm 2). When an alarm condition is met, the INT pin goes LOW. It can be used to wake up a microcontroller from sleep mode.
//       An example sketch is provided demonstrating the use of the INT. See sketch "RTC_Alarm_Trigger_with_Interrupt.ide"
//    B) Square Wave Output: The INT pin can output a square wave at different frequencies (1 Hz, 1024 Hz, 4096 Hz, 8192 Hz). This is useful for timekeeping or clock synchronization.
//       An example sketch is provided demonstrating the square wave output. See sketch "SQW_output.ide".

// 4-pin Screw Terminal output (not used in this sketch, just as an FYI).
// Pin 1 (closest to SD card): Labelled "CD". This outputs the SD card detect signal. If HIGH, no SD Card is detected. If LOW, SD Cad is detected.
// Pin 2: Labelled "WP". This outputs the write protect signal for the SD card. WP = HIGH -> Enable write protect -> Read only. WP = LOW -> Disable write protect -> Read and write.
// Pin 3: Labelled "32 kHz". This outputs a high-precision clock signal at 32.768 kHz, providing stable frequency for external circuits that need accurate timing. Always enabled by default, unless disabled manually.
// Pin 4: Labelled "SQW INT". This outputs either the interrupt signal or a square wave from the RTC. See point 5) above for more details.

// ****************

// I2C Scanner Test
// Run this test to check the RTC and EEPROM can be found and see their I2C addresses. 
// By default the RTC address is 0x68, the EEPROM address is 0x57 (but can be changed via jumpers A0, A1, and A2 from 0x50 to 0x57).
// NOTE: Even if the RTC or the EEPROM have not been initialized via the BEGIN function, their addresses should still scan doing this test.
int I2C_scan = 1; // Turn "ON" (=1) to run the I2C scanner.

// ****************

// RTC Test
// Test 1) This test checks that RTC can be found and set. Set: RTC_test = 1, RTC_init = 1. Upload sketch. Check Serial Monitor. RTC time should be set and displaying correctly.
// Test 2) This test checks that RTC can display proper time. Set: RTC_test = 1, RTC_init = 0. Upload sketch. Check Serial Monitor. RTC time should be displaying correctly.
// Test 3) This test checks that RTC backup battery is working and keeping the time. Set: RTC_test = 1, RTC_init = 0. Unplug board (power off) and plug back in (power on). RTC time should be displaying correctly. 
// NOTE: Only time is checked here, but data should work equally well.
int RTC_test = 0;  // Turn "ON" (=1) to check RTC test.
int RTC_init = 0;  // Turn "ON" (=1) to set RTC to computer time. Turn "OFF" (0) after doing so once.

// ****************

// EEPROM Test
// Test 1) This tests does NOT write the testData to the memoryAddress location and it simply reads what is at the memoryAddress. It likely will read 0xFF. Set: EEPROM_test = 1, EEPROM_write = 0.
// Test 2) Now write the testData to memoryAddress and read it back. It should read back exactly what was written in testData. Set: EPROM_test = 1, EEPROM_write = 1.
// Test 3) Change the testData and repeat Test 2. You can write any value between 0x00 and 0xFF. Maybe try 0x5A (the bitwise inverse of 0xA5).
// Test 4) Change the memoryAddress and repeat Test 2. You can use any address between 0x0000 to 0x0FFF.
// Given Test 3 and Test 4, it can now be seen that data (e.g. integers) ranging in values 0x00 to 0xFF can be stored anywhere between addresses 0x0000 to 0x0FFF. Each address holds one 1 byte (uint8_t, or 8-bit).
// Test 5) This test checks that the testData remains in EEPROM even when power is off. Make sure to turn OFF the EEPROM_write so to not write it again after powering back ON. We only want to read. Set: EEPROM_test = 1, EEPROM_write = 0. Upload sketch. Unplug board (power off) and plug back in (power on). testData should read the previous value. 
// Test 6) Try to write to the card with write-protect enabled (EEPROM_WP = 1). The sketch will still attempt to write to EEPROM and there is no error message. 
//         To test this first write a value to the card (EEPROM_write = 1, EEPROM_WP = 0). Then change the value and try to write it again but enable WP (EEPROM_write = 1, EEPROM_WP = 1). When reading back the value, it reads the previous one when WP was disabled.
// Finally note that WP has a pull-up to 5V. So if it is not defined in software and left alone, it will turn WP ON by default (good to keep data from accidentally being over-written!).
#define EEPROM_ADDRESS 0x57       // I2C address of the EEPROM. Set jumpers A0, A1, and A2 to change the address from 0x50 to 0x57 (default).
int      EEPROM_test   = 0;       // Turn "ON" (=1) to check EEPROM test.
int      EEPROM_write  = 1;       // Turn "ON" (=1) to write the testData to memoryAddress. Turn "OFF" (=0) to NOT write the testData.
uint16_t memoryAddress = 0x0000;  // Memory location to test. This EEPROM has 32kBit of space (4 times that of the ATmega328P-PU!), therefore any address in the following range is acceptable 0x0000 to 0x0FFF.
uint8_t  testData      = 0x5A;    // Data to write (example: 0xA5).
int      EEPROM_WP     = 0;       // Write-protect. If "OFF" (=0), the write-protect is disabled, and the card can read and write. If "ON" (=1), the write-protect is enabled, and the card can only read.

// ****************

// SD Card Test
// The SD Card works on 3.3V logic using the SPI signals. The Shield works on 5V and hence an onboard logic level shifter (LLS) is used to level shift the signals.
// The LLS can be disabled (put into a tri-state/high impedance), completely disabling the SD Card. This is useful if using the SPI bus elsewhere and don't want the SD Card to interfere.
// Note if LLS is OFF then that also disables the QWIIC interface.
// Test 1) This checks if the SD card has been inserted or not. First, do not insert it, upload the sketch and read the Serial Monitor. It should read "No SD card detected!".
//         Then insert the SD card (you should hear a click when it is inserted). Run the sketch again (just press the reset button on the microcontroller development board). It should read "SD card detected!".
// Test 2) This test checks the LLS. Set: SDcard_test = 1, LLS = 0. Upload sketch and read Serial Monitor. It should read "SD card initialization failed!".
//         Now turn ON the LLS by setting: SDcard_test = 1, LLS = 1. Upload sketch and read Serial Monitor. It should read "SD card initialized successfully!".
// Test 3) This tests writes to the SD card and reads back from it. First prepare the SD card on your computer. Afterwards insert the SD card into the shield.
//         A) Format the microSD Card
//            On Windows:
//            Insert the microSD card.
//            Open File Explorer → Right-click the SD card → Format.
//            Choose FAT32 (or FAT16 if the card is ≤2GB).
//            Click Start.
//          
//            On Mac/Linux:
//            sudo mkfs.vfat -F 32 /dev/sdX
//         B) Create test.txt
//            Open Notepad (Windows) or TextEdit (Mac), or anything similar.
//            Type some sample text, for example: "SD Card test, 3.14159!"
//            Save it as test.txt on the SD card.
//         Now run the sketch and observe the Serial Monitor. It should read the first line that you entered when setting up the card in part B) above, and then also the string that was written onto the card from this sketch.
//         You can change the string that is written in the sketch below, myString.
//         Press the reset button and observe the Serial Monitor. This sketch adds one line to the test.txt every time. Therefore you should see this in the Serial Monitor.
int SDcard_test       = 0;
int LLS               = 1;
const char myString[] = "Hello World :)"; // This is the line written to the test.txt every time the sketch is started.

// ****************


// RTC object
RTC_DS3231 rtc;   


void setup() {
  Serial.begin(9600);  // Set Serial Monitor Baud to 9600

  Wire.begin();        // Initialize I2C communication


  // *** RTC TEST **********************
  if (RTC_test) {

     // Initialize RTC
    if (!rtc.begin()) { // Default RTC address is 0x68. This cannot be changed.
      Serial.println("Couldn't find RTC");
      while (1);
    }

    if (RTC_init) {
      Serial.println("");
      Serial.println("RTC Date and Time Set.");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to compile time
    }

  }
  // *** End of RTC TEST ****************


  // *** EEPROM TEST **********************
  if (EEPROM_test) {

    Serial.println("");
    Serial.println("Starting EEPROM test...");

    // Configure the write-protect pin
    pinMode(WRITE_PROTECT_PIN, OUTPUT);

    if (EEPROM_WP == 1) {
      Serial.println("Write-protect enabled. Read Only.");
      digitalWrite(WRITE_PROTECT_PIN, HIGH); // Enable write protect. Read only.
    } else if (EEPROM_WP == 0) {
      Serial.println("Write-protect disabled. Read and write.");
      digitalWrite(WRITE_PROTECT_PIN, LOW); // Disable write protect. Read and write.
    }

    // Write test data
    if (EEPROM_write) {
      Serial.print("Writing data 0x");
      Serial.print(testData, HEX);
      Serial.print(" to address 0x");
      Serial.println(memoryAddress, HEX);
      writeEEPROM(memoryAddress, testData);
    }

    // Read back the data
    uint8_t readData = readEEPROM(memoryAddress);
    Serial.print("Read data 0x");
    Serial.print(readData, HEX);
    Serial.print(" from address 0x");
    Serial.println(memoryAddress, HEX);
  }
  // *** End of EEPROM TEST ****************


  // *** SD Card TEST **********************
  if (SDcard_test){
    pinMode(OE_PIN, OUTPUT);
    pinMode(CD_Pin, INPUT_PULLUP); // CD pin usually pulls LOW when a card is inserted

    Serial.println("");

    if (digitalRead(CD_Pin) == HIGH) {
      Serial.println("No SD card detected!");
      while (1);
    } else {
      Serial.println("SD card detected!");
    }

    if (LLS) {
      digitalWrite(OE_PIN, HIGH); // Disable = LOW, Enable = HIGH
    } else {
      digitalWrite(OE_PIN, LOW); // Disable = LOW, Enable = HIGH
    }

    if (!SD.begin(CS_PIN)) {
      Serial.println("SD card initialization failed!");
      return;
    }

    Serial.println("SD card initialized successfully!");

    // Test write operation
    File testFile = SD.open("test.txt", FILE_WRITE);
    if (testFile) {
      Serial.println("Writing to test.txt...");
      testFile.println(myString);
      testFile.close();
      Serial.println("Write successful!");
    } else {
      Serial.println("Failed to open test.txt for writing.");
    }

    // Test read operation
    testFile = SD.open("test.txt");
    if (testFile) {
      Serial.println("Reading from test.txt...");
      while (testFile.available()) {
        Serial.write(testFile.read());
      }
      testFile.close();
      Serial.println("\nRead successful!");
    } else {
      Serial.println("Failed to open test.txt for reading.");
    }
  }
  // *** END of SD Card TEST **********************






}

void loop(){
  if (I2C_scan) {
    Serial.println("");
    Serial.println("Scanning for I2C devices...");

    for (uint8_t address = 1; address < 127; address++) {
      Wire.beginTransmission(address);
      if (Wire.endTransmission() == 0) {
        Serial.print("Found I2C device at 0x");
        Serial.println(address, HEX);
      }
    }

    Serial.println("Scan complete.\n");
    delay(5000); // Wait before the next scan
  }

  if (RTC_test) {
    Serial.println("");
    DateTime now = rtc.now();

    // Print debugging information
    Serial.print("Current Time: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
  }

  if (!(I2C_scan || RTC_test || EEPROM_test || SDcard_test)) {
    Serial.println("No tests are selected...");
    while (1);
  }

  delay(1000);
}





// Function to write a byte to the EEPROM
void writeEEPROM(uint16_t address, uint8_t data) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((address >> 8) & 0xFF); // High byte of memory address
  Wire.write(address & 0xFF);        // Low byte of memory address
  Wire.write(data);                  // Data to write
  Wire.endTransmission();

  delay(5); // Allow time for the write cycle to complete
}

// Function to read a byte from the EEPROM
uint8_t readEEPROM(uint16_t address) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((address >> 8) & 0xFF); // High byte of memory address
  Wire.write(address & 0xFF);        // Low byte of memory address
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read(); // Read and return the byte
  }

  return 0xFF; // Return 0xFF if no data is available (indicates error)
}