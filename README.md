# Akvariumas — ESP32 Aquarium Controller

> **Work in progress.** This project is functional but incomplete. Known missing pieces: settings are lost on reboot (no flash storage), time is lost on reboot if the RTC battery is dead or not fitted, no input validation on the web UI, no authentication on the access point. Use at your own risk.

---

A simple aquarium automation controller built around an ESP32. It hosts its own Wi-Fi hotspot so you can open a browser on your phone and control the lighting and pumps without needing a router or any app.

The ESP32 handles the web server, the clock, and the NeoPixel strip directly. For the RGBW LED channels and the two pumps, it talks to a secondary Arduino over a serial connection — that part is a separate sketch not included here.

---

## What it does

- RGBW LED control with configurable sunrise/sunset fade (the lights ramp up and down gradually at the start and end of the day rather than switching on/off hard)
- WS2812 NeoPixel strip with its own color and on/off schedule
- Two pumps, each triggered once a day at a set hour, running for a configurable number of seconds at a configurable power level
- Real-time clock so the schedule survives power cuts (as long as the RTC module has a working battery)
- Web interface accessible from any browser on the local network

---

## What is missing / not finished

- **Settings are not saved.** Every reboot resets all sliders and values back to defaults. The `Preferences` library or EEPROM would fix this but it hasn't been implemented yet.
- **RTC time is not saved across cold starts** if the DS3231 backup battery is missing or flat. You would need to re-set the time manually via the web UI.
- **No password on the Wi-Fi hotspot.** Anyone nearby can connect and change settings.
- **No input validation.** Setting end hour earlier than start hour, or a transition longer than the day window, will produce wrong behavior without any warning.
- **Arduino sketch not included.** The ESP32 sends commands over UART but the receiving Arduino code is a separate project and not in this repo.
- **No OTA updates.** Flashing new firmware requires a USB cable.

---

## Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | Any standard 38-pin board should work |
| DS3231 RTC module | Connected over I2C |
| WS2812B LED strip | Up to 50 LEDs |
| RGBW LED strip + driver | Driven by the secondary Arduino |
| Arduino (any) | Receives commands from ESP32 via UART |
| 2x pumps | Wired to the Arduino |

---

## Wiring

| ESP32 pin | Goes to |
|---|---|
| GPIO 21 | DS3231 SDA |
| GPIO 22 | DS3231 SCL |
| GPIO 23 | WS2812B data |
| GPIO 17 (TX2) | Arduino RX |
| GPIO 16 (RX2) | Arduino TX |

---

## How to connect

1. Power on the ESP32.
2. On your phone or computer, connect to the Wi-Fi network **akvariumas** (no password).
3. Open a browser and go to **http://192.168.0.1**.

---

## Web interface

**LED section** — sets the color (R/G/B/W as percentages) for the main lighting, the start and end hours, and the fade transition time in minutes.

**WS2812 section** — controls the NeoPixel strip independently with its own color (0–255 per channel) and a separate on/off schedule.

**Pump 1 / Pump 2** — each pump fires once per day at the configured hour (at minute :00), runs for the set number of seconds, at the set power percentage.

**Set time** — manually sets the hour, minute, and second on the RTC. Has to be done after every cold start if the RTC battery is not fitted.

---

## Lighting schedule logic

Every second the ESP32 looks at the current time and calculates what brightness to send:

```
off --> [start hour] --fade in--> full brightness ---> [end hour - transition] --fade out--> [end hour] --> off
```

The transition time applies to both the fade-in and fade-out. If you set a 30 minute transition, the lights spend 30 minutes ramping up at the start of the day and 30 minutes ramping down at the end.

---

## Arduino communication

Every second the ESP32 prints one line to Serial2 (9600 baud):

```
R:120,G:80,B:200,W:50,P1:0,P2:128
```

Values are 0–255. The Arduino on the other end reads this and drives the actual hardware. That sketch is not part of this repo yet.

---

## Libraries required

Install these through the Arduino Library Manager:

- Adafruit NeoPixel
- RTClib (Adafruit)
- ESPAsyncWebServer
- AsyncTCP

Board: ESP32 Dev Module (or whatever matches your board).

---

## Network details

| | |
|---|---|
| SSID | akvariumas |
| Password | none |
| IP | 192.168.0.1 |
