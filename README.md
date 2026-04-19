# ESP32-Based RFID Access Control System

Detailed presence control system developed for the Microcontrollers II course at **UNICAMP**.

## 📝 Description
This system manages entry and exit events using RFID technology, calculating session times and cumulative presence. It was designed to demonstrate integration between microcontrollers, local displays, and remote communication.

## ⚙️ Features
* **Dual-Mode Communication:**
    * **Remote:** Telegram Bot integration for real-time notifications and reports via Wi-Fi.
    * **Local:** Bluetooth Terminal management and OLED display feedback.
* **Data Handling:** Real-time log generation and individual presence calculation.

## 🛠️ Hardware & Tech Stack
* **Microcontroller:** ESP32 DEVKIT V1
* **Peripherals:** MFRC522 RFID, I2C OLED Display
* **Connectivity:** Wi-Fi (Telegram API), Bluetooth Classic
* **Language:** C++ (Arduino Framework)

---
*Developed as part of the School of Technology (FT-UNICAMP) curriculum.*
