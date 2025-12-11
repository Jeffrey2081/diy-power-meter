#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // --- THIS LINE SETS THE TIME ---
  // It grabs the time from your computer at the moment you click "Upload"
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Serial.println("Time Set!");
}

void loop() {
  DateTime now = rtc.now();
  
  // Print time to Serial Monitor to verify
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  delay(1000);
}
