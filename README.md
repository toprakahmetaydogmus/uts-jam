# ⚡ UTS-JAM Ultimate — ESP32-C3 SuperMini Dual nRF24 Lab Suite

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![MCU: ESP32-C3](https://img.shields.io/badge/MCU-ESP32--C3%20SuperMini-brightgreen.svg)](https://www.espressif.com)
[![Display: OLED](https://img.shields.io/badge/Display-SSD1306%20128x64-orange.svg)](#)

> ⚠️ **UYARI (DISCLAIMER)**: Yalnızca yetkili siber güvenlik araştırmaları, RF sinyal testleri ve kontrollü laboratuvar ortamlarındaki donanım penetrasyon testleri için geliştirilmiştir.

---

## 🎯 1. Sistem Özeti (System Overview)
**UTS-JAM Ultimate**, cep boyutunda, **ESP32-C3 SuperMini (RISC-V)** mimarisine sahip çift alıcı-verici RF keşif cihazıdır. Güç amplifikatörlü (PA/LNA) **çift nRF24L01+** radyo modülleri ve yerleşik **128x64 I2C OLED ekrana** sahiptir.

### Öne Çıkan Özellikler:
- **Çift Radyo Mimarisi:** Bağımsız kanal iletimi ve eşzamanlı spektrum dinleme.
- **Spektrum Analizör Modu:** 2.4GHz ISM bantlarında görsel şelale ve RSSI sinyal seviyesi takibi.
- **Bluetooth BLE Telemetri:** BLE sinyal testi ve proximity (yakınlık) simülasyonu.
- **Gelişmiş Derin Uyku (Deep Sleep):** Pille kullanım için optimize edilmiş güç yönetimi.
- **Fiziksel Navigasyon:** Çok modlu donanım butonları ve OLED menü sistemi.
- **Oyun Motorları:** DOOM 3D raycaster ve diğer birçok klasik mini oyun desteği.

---

## 🔌 2. Donanım ve Pin Bağlantıları (Pinout & BOM)

### Gerekli Donanımlar
- 1x **ESP32-C3 SuperMini** RISC-V Mikrodenetleyici
- 2x **nRF24L01+ PA/LNA** (SMA Antenli)
- 1x **SSD1306 128x64 I2C OLED Ekran**
- 1x 3.3V LDO Voltaj Regülatörü + Filtre Kapasitörleri (100µF)
- 3x Push Buton (Yukarı, Aşağı, Seç)
- 1x LED (İsteğe Bağlı)

### Pin Bağlantı Haritası
| Bileşen | Pin Görevi | ESP32-C3 Pin Numarası |
|---|---|---|
| OLED Ekran | SDA | 8 |
| OLED Ekran | SCL | 9 |
| Buton | Yukarı (UP) | 2 |
| Buton | Aşağı (DOWN) | 0 |
| Buton | Seç (SELECT) | 1 |
| LED | Durum LED'i | 10 |
| SPI (nRF24 #1 ve #2 Ortak) | SCK | 4 |
| SPI (nRF24 #1 ve #2 Ortak) | MISO | 5 |
| SPI (nRF24 #1 ve #2 Ortak) | MOSI | 6 |
| nRF24 Radyo #1 | CE | 20 |
| nRF24 Radyo #1 | CSN | 21 |
| nRF24 Radyo #2 | CE | 7 |
| nRF24 Radyo #2 | CSN | 10 |

---

## 🛠️ 3. Kütüphane Gereksinimleri (Dependencies)
Projeyi derlemek için PlatformIO veya Arduino IDE üzerinde (ESP32 kart sürümü v2.0.14+ kullanılarak) aşağıdaki kütüphanelerin kurulu olması gereklidir:
- `RF24`
- `Adafruit_GFX`
- `Adafruit_SSD1306`
- `U8g2_for_Adafruit_GFX`
- `BLEDevice`, `BLEUtils`, `BLEScan`, `BLEAdvertisedDevice`, `BLEClient`

---

## 🚀 4. Kurulum ve Kullanım (Getting Started)
1. Yukarıdaki kütüphaneleri PlatformIO veya Arduino IDE üzerinden indirin.
2. ESP32 board tanımlarını (v2.0.14+) güncellediğinizden emin olun.
3. Donanımı "Pin Bağlantı Haritası"na göre birleştirin (Dual nRF24 modülleri için ortak SPI hatlarını kullanın).
4. `LASTVERSION.ino` (veya ilgili sürüm) dosyasını cihaza yükleyin.
5. Cihazı 3.7V LiPo pil veya 5V Tip-C bağlantısı üzerinden çalıştırın.

---

## 📱 5. Dahili Özellikler ve Menü (Features & Tools)
Cihaz, yazılımında toplam 46 adet entegre araç, saldırı simülatörü, analizör ve oyun barındırmaktadır:

**Siber Analiz ve RF Modülleri:**
- `BT Jam`, `DroneJam`, `WiFiJam`, `MultiJam`, `SweepJam`, `ChRange`
- `WiFiSpec` (WiFi Spektrum), `BLESpec` (BLE Spektrum)
- `BLESpam`, `Deauth`, `BLEScan`, `ZigbeeJam`, `TargJam`, `Beacon`, `WiFiAnaly`
- `DeauthDet` (Deauth Tespit Edici), `WIDS` (Kablosuz Saldırı Tespit Sistemi)
- `RFAnalyz`, `BLE GATT`, `BLETrack`, `MouseSnf` (Fare Sniffer)
- `Repeater` (RF Tekrarlayıcı), `Oscillosc` (RF Osiloskop), `BLEBadUSB`, `WiFiSniff`, `CW Jammer`
- `EvilPort` (Evil Portal - Phishing)

**Araçlar:**
- `TestRadio`, `Settings`, `Help`, `Deep Sleep`, `Sys Monitor`, `I2C Scanner`
- `Calc` (Hesap Makinesi), `Resistor` (Direnç Hesaplayıcı), `WiFiMon`, `PktCount` (Paket Sayacı)

**Mini Oyunlar:**
- `DOOM 3D`
- `Snake Deluxe`
- `SpaceInvaders`
- `Breakout`
- `Tetris Master`
- `Flappy Bird`
- `Dino Runner`
- `TicTacToe AI`
- `Pong Pro`

---

## 📜 6. Lisans (License)
Bu proje [MIT Lisansı](LICENSE) altında lisanslanmıştır.
Geliştirici: **Toprak Ahmet Aydoğmuş**.
