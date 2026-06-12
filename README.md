# Cypher Box v2.0 Ultimate – ESP32-C3 Super Mini

2.4 GHz Jammer, Spektrum Analizör ve Spam Cihazı  
**Donanım:** ESP32-C3 Super Mini + 2x nRF24L01 + 128x32 OLED  
**Yazılım:** Arduino IDE (ESP32 board paketi)

---

## Özellikler

| Modül | Açıklama |
|-------|----------|
| **BT Jammer** | Bluetooth Classic kanallarını rastgele karıştırır |
| **Drone Jammer** | Drone frekanslarına karıştırma |
| **WiFi Jammer** | WiFi kanal 1,6,14 üzerinde jam |
| **Multi Jammer** | Rastgele kanal kombinasyonları |
| **Sweep Jammer** | Tüm 2.4 GHz bandını tarar (125 kanal) |
| **Channel Range** | 40‑80 arası Bluetooth kanal aralığı |
| **Spectrum** | Gerçek zamanlı waterfall spektrum analizörü |
| **BLE Spam** | 100+ rastgele cihaz adıyla BLE reklam paketi spam'i |
| **BLE Scanner** | BLE cihazlarını tarar (isim, MAC, RSSI) |
| **BLE Target Jam** | BLE reklam kanallarına (37,38,39) odaklı jam |
| **WiFi Deauth** | Tek bir AP’ye veya tüm çevre ağlara deauth saldırısı |
| **WiFi Beacon Flood** | Sahte WiFi ağ isimleriyle beacon seli |
| **WiFi Analyzer** | Kanal/RSSI grafikleri ve en güçlü AP listesi |
| **Zigbee Jammer** | Zigbee (802.15.4) 11‑26 kanallarına jam |
| **Test Radio** | nRF24L01 modüllerinin testi |
| **Settings** | Parlaklık ayarı, LED testi |
| **Help** | Temel kullanım bilgisi |

> **Not:** ESP32‑C3 yalnızca BLE (Bluetooth Low Energy) destekler; klasik Bluetooth özellikleri (SPP) bu kartta mevcut değildir. Menüde klasik Bluetooth seçenekleri bulunmaz.

---

## Donanım Gereksinimleri

- **ESP32-C3 Super Mini** (RISC‑V işlemci, 4 MB flash)
- **2 adet nRF24L01+** (PA+LNA tercih edilir)
- **128x32 OLED I2C ekran** (SSD1306 sürücülü)
- **3 adet buton** (UP, DOWN, SELECT), 1 adet LED
- 3.3V uyumlu bağlantılar

---

## Bağlantı Şeması

| Bileşen       | ESP32-C3 Pin |
|---------------|--------------|
| OLED SDA      | GPIO 8       |
| OLED SCL      | GPIO 9       |
| LED (ops.)    | GPIO 99      |
| UP Buton      | GPIO 2       |
| DOWN Buton    | GPIO 0       |
| SELECT Buton  | GPIO 1       |
| SPI SCK       | GPIO 4       |
| SPI MISO      | GPIO 5       |
| SPI MOSI      | GPIO 6       |
| Radio 1 CE    | GPIO 20      |
| Radio 1 CSN   | GPIO 21      |
| Radio 2 CE    | GPIO 7       |
| Radio 2 CSN   | GPIO 10      |

Tüm bileşenler **3.3V** ile beslenmelidir. nRF24 modüllerinin VCC ve GND bağlantılarını kısa tutun ve mümkünse ayrı bir 3.3V regülatör kullanın.

---

## Yazılım Gereksinimleri

1. **Arduino IDE 1.8.19** veya üstü
2. **ESP32 board paketi** (`https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)
3. Aşağıdaki kütüphaneleri Arduino Library Manager’dan yükleyin:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `U8g2_for_Adafruit_GFX` (manuel kurulum gerekebilir)
   - `RF24` (TMRh20)
   - `BLEDevice` (ESP32 BLE Arduino – genellikle board paketiyle gelir)
4. **Partition Scheme (Bölüm Şeması) Ayarı:**  
   Kod ~1.4 MB yer kaplar, bu nedenle **Tools → Partition Scheme → Huge APP (3 MB APP / 0 KB SPIFFS)** seçilmelidir. Eğer bu seçenek yoksa **Minimal SPIFFS (1.9 MB APP)** de kullanılabilir.

---

## Kurulum

1. Arduino IDE’de **Tools → Board → ESP32C3 Dev Module** seçin.
2. **Tools → USB CDC On Boot** → `Enabled` (seri port için)
3. **Tools → Partition Scheme** → `Huge APP (3 MB APP / 0 KB SPIFFS)`
4. `CypherBox_C3.ino` dosyasını açın ve yükleyin.
5. Açılışta `demonSHIT` splash ekranı görüntülendikten sonra ana menü gelir.

---

## Kullanım

- Menüde gezinmek için **UP** ve **DOWN** butonlarını kullanın.
- Bir işlevi başlatmak veya seçmek için **SELECT** butonuna basın.
- Aktif bir jam/spam modundan çıkmak için yine **SELECT** butonuna basın.
- Spectrum modunda dalga formu gerçek zamanlı güncellenir, çıkmak için SELECT.
- WiFi Deauth menüsünde `Single AP` veya `All APs` seçimi yapabilirsiniz.
- BLE Scanner tarama sonuçlarını listeler, UP/DOWN ile gezinebilirsiniz.
- Settings → Brightness ile OLED parlaklığını ayarlayın.

> **Önemli:** Her jammer modu başlatıldığında radyolar yeniden yapılandırılır. **Radio 2 FAIL** mesajı alırsanız cihaz tek radyo ile jamming yapmaya devam eder. Bu durum çoğunlukla ikinci nRF24 modülünün takılı olmamasından veya zayıf beslemeden kaynaklanır.

---

## Sık Karşılaşılan Sorunlar

| Sorun | Çözüm |
|-------|-------|
| **Sketch too big** | Partition Scheme'i `Huge APP` yapın |
| **OLED başlatılamadı** | SDA/SCL pinlerini ve 3.3V bağlantısını kontrol edin |
| **Radio 2 FAIL (ignored)** | Tek radyo ile devam eder, isterseniz ikinci modülü sökün |
| **WiFi deauth çalışmıyor** | `WiFi.mode(WIFI_AP_STA)` çağrıldığından emin olun |
| **BLE spam az cihaz gösteriyor** | nRF24 gücünü ve anten bağlantısını kontrol edin, PA+LNA modül önerilir |
| **Seri monitörde rastgele karakterler** | Baud rate 115200 olarak ayarlı değilse düzeltin |

---

## Uyarılar ve Yasal Not

Bu proje **yalnızca eğitim ve araştırma amaçlıdır**.  
Jammer ve deauth saldırıları birçok ülkede yasaktır.  
Cihazı yalnızca sahibi olduğunuz, izin aldığınız veya yasal test ortamlarında kullanın.  
Yazar, kötüye kullanımdan doğacak hukuki veya cezai sorumluluk kabul etmez.

---

MAIN IDEA FROM THE https://github.com/dkyazzentwatwa/cypher-jammer-mini by @dkyazzentwatwa

**Lisans:** MIT  
**Versiyon:** 2.0 Ultimate (ESP32-C3 Edition) 
