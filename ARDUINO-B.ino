// Function: Reads AC, Drives OLED, Calculates Energy, Sends to Arduino A 
// Listens for 'R' command to reset Energy to 0.

#include <EmonLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// --- CONFIG ---
EnergyMonitor emon1;
// Communication with Arduino A (RX=D2, TX=D3)
SoftwareSerial LinkSerial(2, 3); 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables
float voltage = 0;
float current = 0;
float power = 0;
float energy_kWh = 0;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);     
  LinkSerial.begin(4800);
  
  // 1. Initialize Sensors 
  emon1.voltage(A0, 94, 1.7); 
  emon1.current(A1, 100);

  // 2. Initialize Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED Error")); 
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Sensor Ready...");
  display.display();
  delay(1000);
}

void loop() {
  // 1. Calculate Power (Sampling)
  emon1.calcVI(20, 2000); // 20 wavelengths, 2s timeout
  
  voltage = emon1.Vrms;
  current = emon1.Irms;
  power   = emon1.apparentPower;

  // Filter Noise (Force zero if unplugged)
  if (voltage < 50) voltage = 0;
  if (current < 2.2) { current = 0; power = 0; }

  // 2. Calculate Energy (kWh)
  // Power (W) * Time (ms) / (1000 * 3600000)
  unsigned long currentMillis = millis();
  if (lastMillis > 0) {
     unsigned long duration = currentMillis - lastMillis;
     energy_kWh += (power * duration) / 3600000000.0;
  }
  lastMillis = currentMillis;

  // 3. Update OLED Display
  updateScreen();

  // 4. Send Data to Arduino A (Format: <V,I,W,E>)
  sendDataToMaster();

  // 5. Check for "Reset" Command from Arduino A
  checkResetCommand();
}

void updateScreen() {
  display.clearDisplay();
  
  // Voltage & Current
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("V: "); display.print(voltage, 1);
  display.setCursor(64, 0);
  display.print("I: "); display.print(current, 2);

  // Power (Big Text)
  display.setCursor(0, 16);
  display.setTextSize(2);
  display.print(power, 1); display.print(" W");

  // Energy
  display.setCursor(0, 40);
  display.setTextSize(1);
  display.print("Energy: ");
  display.setCursor(0, 52);
  display.print(energy_kWh, 4); display.print(" kWh");
  
  display.display();
}

void sendDataToMaster() {
  // Sends packet like: <230.50,1.20,276.00,0.0012>
  LinkSerial.print("<");
  LinkSerial.print(voltage, 2); LinkSerial.print(",");
  LinkSerial.print(current, 2); LinkSerial.print(",");
  LinkSerial.print(power, 2);   LinkSerial.print(",");
  LinkSerial.print(energy_kWh, 5);
  LinkSerial.println(">");
}

void checkResetCommand() {
  if (LinkSerial.available()) {
    char c = LinkSerial.read();
    if (c == 'R') {
      // RESET COMMAND RECEIVED!
      energy_kWh = 0.0;
      display.clearDisplay();
      display.setCursor(20, 20);
      display.setTextSize(2);
      display.println("RESET!");
      display.display();
      delay(1000); // Show "Reset" message for 1 sec
    }
  }
}
