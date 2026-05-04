# 🐠 Akvariumas — ESP32 Aquarium Controller

A Wi-Fi controlled aquarium automation system built on an **ESP32** microcontroller. Control your LED lighting, NeoPixel strip, and two dosing/circulation pumps from any phone or browser — no internet required.

---

## ✨ Features

- **📱 Web interface** — Control everything from your phone's browser (no app needed)
- **💡 RGBW LED lighting** — Set custom colors with automatic sunrise/sunset transitions
- **🌈 WS2812 NeoPixel strip** — Independently controlled ambient lighting with a schedule
- **💧 Two pumps** — Schedule each pump to run at a specific hour with adjustable duration and power
- **🕐 Real-time clock (RTC)** — Keeps accurate time even after power cuts
- **📡 Own Wi-Fi hotspot** — Creates its own `akvariumas` network, no router needed

---

## 🛒 Hardware Required

| Component | Details |
|---|---|
| ESP32 board | Any standard ESP32 dev board |
| DS3231 RTC module | Connected via I2C |
| WS2812B LED strip | Up to 50 LEDs, connected to pin 23 |
| RGBW LED strip + driver | Controlled via Arduino over Serial2 |
| Arduino (secondary MCU) | Receives RGBW + pump commands via UART |
| 2× pumps | Controlled by the Arduino |

---

## 🔌 Wiring

| ESP32 Pin | Connected To |
|---|---|
| GPIO 21 (SDA) | DS3231 SDA |
| GPIO 22 (SCL) | DS3231 SCL |
| GPIO 23 | WS2812B data line |
| GPIO 17 (TX) | Arduino RX |
| GPIO 16 (RX) | Arduino TX |

---

## 📲 How to Connect & Use

1. **Power on** the ESP32
2. On your phone or laptop, connect to the Wi-Fi network: **`akvariumas`** (no password)
3. Open a browser and go to: **`http://192.168.0.1`**
4. The control panel will load — adjust everything from there

---

## 🖥️ Web Interface Sections

### 🔆 LED
Set the **R, G, B, W** (red, green, blue, white) percentages for your main lighting.
- **Start / End hours** — When the lights turn on and off each day
- **Transition** — Fade duration in minutes (the "sunrise" and "sunset" effect)

### ✨ WS2812
Control the NeoPixel strip separately.
- Enable/disable with a checkbox
- Set its own color (0–255 per channel) and on/off schedule

### 💧 Pump #1 and Pump #2
Each pump has:
- **Hour** — What hour of the day it activates (runs at minute 00)
- **Seconds** — How long it runs
- **Power** — Speed/intensity as a percentage

### 🕐 Set Time
Manually set the current time on the RTC clock (hour, minute, second).

---

## ⚙️ How the Lighting Works

The ESP32 calculates the current lighting values every second based on the time:

```
[Start hour] ──fade in──> [Full brightness] ──────> [fade out]──> [End hour]
```

- Before start hour → lights off
- During fade-in → brightness ramps up gradually over the transition duration
- During the day → full brightness at your set color
- During fade-out → brightness ramps down
- After end hour → lights off

This creates a natural sunrise/sunset effect automatically.

---

## 📡 How the Arduino Communication Works

Every second, the ESP32 sends a line over UART (Serial2 at 9600 baud) to an Arduino:

```
R:120,G:80,B:200,W:50,P1:0,P2:128
```

The Arduino is responsible for driving the actual RGBW LED channels and the two pumps based on these values.

---

## 🛠️ Software Setup

### Libraries needed (install via Arduino Library Manager):

- `Adafruit NeoPixel`
- `RTClib` (by Adafruit)
- `ESPAsyncWebServer`
- `AsyncTCP`
- `WiFi` (built-in with ESP32 core)

### Board:
Select **ESP32 Dev Module** (or your specific board) in Arduino IDE.

---

## 📁 File Overview

```
pats_paskutinis_kodas.ino   ← Main ESP32 firmware (this file)
```

The Arduino sketch for the secondary MCU (RGBW + pump control) is a separate project.

---

## 🌐 Network Details

| Setting | Value |
|---|---|
| Network name | `akvariumas` |
| Password | *(none — open)* |
| ESP32 IP address | `192.168.0.1` |
| Web UI address | `http://192.168.0.1` |

---

## 📝 Notes

- Settings are **not saved to flash** — if the ESP32 restarts, sliders return to defaults. Consider adding `Preferences` library support if persistence is needed.
- The RTC holds time through power cuts (as long as its battery is good).
- The pump trigger fires once per hour at minute `:00` and runs for the configured number of seconds.

---

## 📜 License

Do whatever you want with this. It's an aquarium controller. 🐟
