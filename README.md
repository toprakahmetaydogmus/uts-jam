# ⚡ UTS-JAM Ultimate — ESP32-C3 SuperMini Dual nRF24 Lab Suite

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![MCU: ESP32-C3](https://img.shields.io/badge/MCU-ESP32--C3%20SuperMini-brightgreen.svg)](https://www.espressif.com)
[![Display: OLED](https://img.shields.io/badge/Display-SSD1306%20128x32-orange.svg)](#)

> ⚠️ **DISCLAIMER**: Developed strictly for authorized cybersecurity research, RF signal testing, and hardware penetration testing in controlled laboratory environments.

---

## 🎯 1. System Overview
**UTS-JAM Ultimate** is a pocket-sized dual-transceiver RF exploration device built around the **ESP32-C3 SuperMini (RISC-V)** architecture, **dual nRF24L01+** radio modules with power amplification (PA/LNA), and an onboard **128x32 I2C OLED display**.

### Highlights:
- **Dual Radio Architecture:** Independent channel transmission and concurrent spectrum listening.
- **Spectrum Analyzer Mode:** Visual waterfall and RSSI signal level monitoring across 2.4GHz ISM bands.
- **Bluetooth BLE Advertisement Telemetry:** BLE beacon testing and airtag proximity simulation.
- **Physical Navigation:** Multi-mode hardware toggle buttons and OLED navigation menus.

---

## 🛠️ 2. Hardware Bill of Materials (BOM)
- 1x **ESP32-C3 SuperMini** RISC-V Microcontroller
- 2x **nRF24L01+ PA/LNA** with SMA High-Gain Antennas
- 1x **SSD1306 0.91" 128x32 I2C OLED Screen**
- 1x 3.3V LDO Voltage Regulator + Filter Capacitors (100µF)
- 3x Tactile Push Buttons (Up, Down, Select)

---

## 🚀 3. Getting Started
1. Flash firmware using PlatformIO or Arduino IDE with ESP32 board definitions (v2.0.14+).
2. Configure SPI buses for Dual nRF24 operation.
3. Power on with 3.7V LiPo or 5V Type-C connector.

---

## 📜 4. License
Licensed under the [MIT License](LICENSE).  
Developer: **Toprak Ahmet Aydoğmuş**.
