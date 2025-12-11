# ⚡ Dual-Arduino AC Power Meter & Data Logger

A comprehensive AC Energy Monitor built using two Arduino units. It measures Voltage, Current, Real Power, and Energy consumption (kWh) of home appliances. 

The system features an OLED dashboard, CSV data logging to an SD card, and real-time Bluetooth plotting on an Android phone.

## 🚀 Features
* **Real-time Monitoring:** Displays V, I, W, and kWh on an OLED screen.
* **Dual-Core Architecture:** * **Arduino A (Master):** Handles Storage, Timekeeping, Bluetooth, and User Input.
    * **Arduino B (Slave):** Dedicated to high-speed sampling of AC sensors and Display drivers.
* **Data Logging:** Saves data to SD Card in `.CSV` format (Excel compatible) with precise timestamps.
* **Wireless Graphing:** Sends live data to Android via Bluetooth for real-time charting.
* **Session Management:** Physical button to reset energy counters and generate a new log file instantly.
* **Noise Filtering:** Software noise gates to prevent ghost readings when idle.

---

## 🛠️ Hardware Required

### Core Components
* 2x Arduino Uno (or Nano)
* 1x 0.96" OLED Display (I2C)
* 1x Micro SD Card Module
* 1x DS3231 RTC Module (Real Time Clock)
* 1x HC-05 Bluetooth Module
* 1x Push Button

### Sensors & AC Circuit
* **Current:** SCT-013-000 (100A/50mA) or SCT-013-1V (Voltage Output type)
* **Voltage:** 9V AC Transformer (Iron Core, NOT DC adapter)
* **Bias Circuit:** 2x 10kΩ Resistors, 1x 10µF Capacitor (Creates 2.5V DC Offset)
* **Voltage Divider:** 100kΩ and 10kΩ Resistors

---

## 🔌 Pinout & Wiring

### 1. Inter-Arduino Connection (The Bridge)
*The two Arduinos communicate via SoftwareSerial.*

| Arduino B (Sensor Unit) | Arduino A (Storage Unit) | Function |
| :--- | :--- | :--- |
| **Pin D3 (TX)** | **Pin D2 (RX)** | Sends Sensor Data (B -> A) |
| **Pin D2 (RX)** | **Pin D3 (TX)** | Receives Reset Command (A -> B) |
| **GND** | **GND** | **Common Ground (Crucial)** |

### 2. Arduino A: Master (Storage & Comms)

| Component | Pin | Arduino Pin |
| :--- | :--- | :--- |
| **SD Card** | CS | D10 |
| | MOSI | D11 |
| | MISO | D12 |
| | SCK | D13 |
| **RTC (DS3231)** | SDA | A4 |
| | SCL | A5 |
| **Bluetooth (HC-05)**| TX | D6 |
| | RX | D7 |
| **Button** | Pin 1 | D8 |
| | Pin 2 | GND |

### 3. Arduino B: Slave (Sensors & Display)

| Component | Pin | Arduino Pin |
| :--- | :--- | :--- |
| **OLED Display** | SDA | A4 |
| | SCL | A5 |
| **Voltage Sensor** | Out | A0 |
| **Current Sensor** | Out | A1 |

---

## 💾 Libraries Required
Install these via the Arduino IDE Library Manager:
1.  **EmonLib** (by OpenEnergyMonitor) - *For AC sampling*
2.  **Adafruit SSD1306** - *For OLED Display*
3.  **Adafruit GFX** - *Graphics Core*
4.  **RTClib** (by Adafruit) - *For DS3231 Clock*
5.  **SPI** & **Wire** - *Built-in*
6.  **SoftwareSerial** - *Built-in*

---

## 📊 Usage Guide

### 1. Calibration
Before installing, adjust the calibration values in **Arduino B Code** to match your local voltage and sensor type.

```cpp
// Inside setup() of Arduino B
emon1.voltage(A0, 94.0, 1.7);  // Adjust 94.0 until Display matches Multimeter
emon1.current(A1, 100);        // 100 for SCT-013-1V, 60.6 for SCT-013-000
```
### 2. Viewing Data on Phone
1. Download **"Serial Bluetooth Terminal"** on Android.
2. Connect to **HC-05**.
3. Switch input mode to **GRAPH**.
4. You will see 3 plotted lines: **Green** (Voltage), **Blue** (Current), **Red** (Power).

### 3. Importing Data to Excel
1. Remove the SD card and plug it into a PC.
2. Files are named `LOG000.CSV`, `LOG001.CSV`, etc.
3. Open directly in Excel to see columns: `Date`, `Time`, `Voltage`, `Current`, `Power`, `Energy`.

### 4. Reset Button
1. **Press and Hold** the button on Arduino A.
2. The OLED will flash **"RESET!"**.
3. Energy (kWh) resets to 0.
4. The old CSV file is saved and closed.
5. A new CSV file is created automatically.

---

## ⚠️ Safety Warning
**DANGER: HIGH VOLTAGE**

This project involves connecting components to Mains AC Voltage (110V/220V).

* **Do not touch** the AC transformer wiring while powered.
* Ensure the AC transformer is isolated.
* Clamp the CT sensor around **ONE** wire only (Live or Neutral).
* **The author is not responsible for any damage or injury caused by this project.**
