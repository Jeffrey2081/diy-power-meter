// Function: Logs Data, Sends to BT Graph, Controls Reset

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

// --- CONFIG ---
const int chipSelect = 10;
const int buttonPin = 8;

// Comms
SoftwareSerial BTSerial(6, 7);    // RX=D6, TX=D7
SoftwareSerial LinkSerial(2, 3);  // RX=D2, TX=D3 (To Arduino B)

RTC_DS3231 rtc;
File dataFile;
char fileName[13];

// Data received from Arduino B
float volt = 0, amp = 0, power = 0, energy = 0;
String inputString = "";

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  LinkSerial.begin(4800); // Must match Arduino B

  pinMode(buttonPin, INPUT_PULLUP);
  
  Wire.begin();
  if (!rtc.begin()) Serial.println("RTC Fail");
  if (!SD.begin(chipSelect)) Serial.println("SD Fail");

  // Create Initial File
  createNewFile();
}

void loop() {
  // 1. LISTEN TO ARDUINO B (Get Data)
  LinkSerial.listen();
  unsigned long startWait = millis();
  
  // Listen for up to 200ms for a data packet
  while (millis() - startWait < 200) {
    while (LinkSerial.available()) {
      char inChar = (char)LinkSerial.read();
      if (inChar == '<') {
        inputString = ""; // Start packet
      } else if (inChar == '>') {
        parseData();      // End packet, process it
        logToSD();        // Save to card immediately
        sendToPhone();    // Update phone graph immediately
      } else {
        inputString += inChar;
      }
    }
  }

  // 2. CHECK BUTTON (Reset Logic)
  if (digitalRead(buttonPin) == LOW) {
    delay(50); // Debounce
    if (digitalRead(buttonPin) == LOW) {
      handleReset();
    }
  }
}

void parseData() {
  // Format: "230.00,1.20,276.00,0.0012"
  int idx1 = inputString.indexOf(',');
  int idx2 = inputString.indexOf(',', idx1 + 1);
  int idx3 = inputString.indexOf(',', idx2 + 1);

  if (idx3 > 0) {
    volt   = inputString.substring(0, idx1).toFloat();
    amp    = inputString.substring(idx1 + 1, idx2).toFloat();
    power  = inputString.substring(idx2 + 1, idx3).toFloat();
    energy = inputString.substring(idx3 + 1).toFloat();
  }
}

void logToSD() {
  if (dataFile) {
    DateTime now = rtc.now();
    
    // Excel CSV Format: Date,Time,V,I,W,kWh
    dataFile.print(now.year(), DEC); dataFile.print("/");
    dataFile.print(now.month(), DEC); dataFile.print("/");
    dataFile.print(now.day(), DEC); dataFile.print(",");
    
    dataFile.print(now.hour(), DEC); dataFile.print(":");
    dataFile.print(now.minute(), DEC); dataFile.print(":");
    dataFile.print(now.second(), DEC); dataFile.print(",");
    
    dataFile.print(volt, 2);   dataFile.print(",");
    dataFile.print(amp, 2);    dataFile.print(",");
    dataFile.print(power, 2);  dataFile.print(",");
    dataFile.println(energy, 4);
    
    dataFile.flush(); // Ensure data is saved physically
  }
}

void sendToPhone() {
  BTSerial.listen(); // Briefly switch to BT to talk
  
  // Format for Graphing App: "Value1,Value2,Value3"
  BTSerial.print(volt, 1);   BTSerial.print(",");
  BTSerial.print(amp, 2);    BTSerial.print(",");
  BTSerial.println(power, 1);
  
  // Switch back to listening to Sensor immediately
  LinkSerial.listen(); 
}

void handleReset() {
  Serial.println("Resetting...");
  
  // 1. Tell Arduino B to reset energy (Send 'R')
  LinkSerial.listen();
  LinkSerial.print("R");
  delay(100);

  // 2. Close old file and create new one
  dataFile.close();
  createNewFile();

  // 3. Wait for button release
  while(digitalRead(buttonPin) == LOW);
}

void createNewFile() {
  // Find next available filename (LOG000.CSV to LOG999.CSV)
  for (int i = 0; i < 999; i++) {
    sprintf(fileName, "LOG%03d.CSV", i);
    if (!SD.exists(fileName)) {
      break;
    }
  }
  
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Date,Time,Voltage(V),Current(A),Power(W),Energy(kWh)");
    dataFile.flush();
    Serial.print("New File: "); Serial.println(fileName);
  }
}
