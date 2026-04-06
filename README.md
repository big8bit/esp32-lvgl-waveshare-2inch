# ESP32 Waveshare LVGL Dashboard

Interactive ESP32 touchscreen dashboard built with LVGL and SquareLine Studio. This project features hardware-linked UI elements.

## Features
* **Relay Control:** On-screen buttons that change state (Red = Off, Green = On) based on hardware relay variables using LVGL states.
* **Temperature Gauge:** Animated UI gauge and numerical label simulating real-time temperature data.
* **UI Design:** Designed visually in SquareLine Studio for easy updates without modifying C logic.

## Hardware & Software
* **Microcontroller:** ESP32
* **Display:** Waveshare LCD Touch Screen
* **Graphics Library:** LVGL
* **UI Editor:** SquareLine Studio

## Setup
1. Clone this repository.
2. Open the project in your preferred IDE (Arduino IDE, PlatformIO, or ESP-IDF).
3. Ensure the LVGL library is installed and configured for your specific Waveshare display driver.
4. Compile and upload to your ESP32.
