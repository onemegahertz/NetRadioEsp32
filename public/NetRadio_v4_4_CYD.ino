/*
 * NetRadio v.4.4 - TOUCH FIX VERSION
 * Исправлена работа кнопок тачскрина
 */

// WiFi CONFIGURATION
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SD.h>
#include "Audio.h"
#include <time.h>

// Pins
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST   -1
#define TFT_BL    21

#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

#define SD_MOSI   23
#define SD_MISO   19
#define SD_SCLK   18
#define SD_CS      5

#define I2S_BCLK  27
#define I2S_LRC   26
#define I2S_DOUT  25

#define BTN_VOL_DOWN 22

#define LED_RED    4
#define LED_GREEN 16
#define LED_BLUE  17

// Constants
#define MAX_STATIONS   20
#define MAX_NAME_LEN   16
#define MAX_URL_LEN    64
#define VOL_STEP       2
#define DEBOUNCE_MS  300  // Увеличил для стабильности
#define MAX_WIFI_NETWORKS 6

#define MODE_VOLUME      0
#define MODE_TEMPERATURE 1
#define MODE_TIME        2
#define MODE_WIFI        3

#define WEATHER_API_KEY "cf0cd0d160ba580cef69e35dfe3064c8"

// Globals
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
Audio audio;
WebServer server(80);
Preferences prefs;

struct Station {
  char name[MAX_NAME_LEN];
  char url[MAX_URL_LEN];
};

Station stations[MAX_STATIONS];
int stationCount = 0;
int currentStation = 0;
int currentVolume = 12;
bool isPlaying = false;
bool wifiConnected = false;
bool sdCardDetected = false;
char wifiSSID[20] = "";
char wifiIP[16] = "";
char curTime[9] = "";
char curDate[11] = "";
char weatherTemp[8] = "";
int displayMode = 0;

unsigned long lastBtn = 0, lastScr = 0, lastTime = 0, lastWeather = 0;
unsigned long lastTouch = 0;  // Для debounce тачскрина

struct WiFiNetwork {
  String ssid;
  int rssi;
  wifi_auth_mode_t encType;
};

WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
int wifiNetworkCount = 0;

enum MenuState { MENU_MAIN, MENU_WIFI_SCAN, MENU_WIFI_PASSWORD, MENU_WIFI_CONNECTING };
MenuState currentMenu = MENU_MAIN;
int selectedNetwork = -1;
String inputPassword = "";

// Forward declarations
void playStation(int);
void nextStation();
void prevStation();
void volumeUp();
void volumeDown();
void updateDisplay();
void loadStationsFromSD();
void saveStationsToSD();
void loadDefaultStations();
void updateTime();
void updateWeather();
void setupWebServer();
void initSDCard();
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void checkTouch();
void scanWiFiNetworks();
void connectToWiFi(String ssid, String password);
void drawWiFiMenu();
void drawKeyboard();
void drawMainScreen();

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println(F("\n=== NetRadio v.4.4 ==="));
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println(F("NetRadio v.4.4"));
  tft.setTextSize(1);
  tft.setCursor(10, 30);
  tft.println(F("Starting..."));
  
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  Serial.println(F("[OK] Touch"));
  
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setLEDColor(0, 0, 255);
  
  initSDCard();
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.println(F("Connecting WiFi..."));
  
  if (strlen(WIFI_SSID) == 0) {
    prefs.begin("netradio", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("password", "");
    prefs.end();
    
    if (ssid.length() == 0) {
      WiFi.softAP("NetRadio", "netradio123");
      strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
      currentMenu = MENU_WIFI_SCAN;
      scanWiFiNetworks();
    } else {
      WiFi.begin(ssid.c_str(), pass.c_str());
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        delay(1000);
        wifiConnected = true;
        strncpy(wifiSSID, ssid.c_str(), 19);
        strcpy(wifiIP, WiFi.localIP().toString().c_str());
        configTime(10800, 0, "pool.ntp.org");
        updateWeather();
        currentMenu = MENU_MAIN;
      } else {
        currentMenu = MENU_WIFI_SCAN;
        scanWiFiNetworks();
      }
    }
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      delay(1000);
      wifiConnected = true;
      strncpy(wifiSSID, WIFI_SSID, 19);
      strcpy(wifiIP, WiFi.localIP().toString().c_str());
      configTime(10800, 0, "pool.ntp.org");
      prefs.begin("netradio", false);
      prefs.putString("ssid", WIFI_SSID);
      prefs.putString("password", WIFI_PASSWORD);
      prefs.end();
      updateWeather();
      currentMenu = MENU_MAIN;
    } else {
      currentMenu = MENU_WIFI_SCAN;
      scanWiFiNetworks();
    }
  }
  
  loadStationsFromSD();
  setupWebServer();
  
  if (stationCount > 0 && currentMenu == MENU_MAIN) {
    playStation(currentStation);
  }
  
  setLEDColor(0, 255, 0);
  updateDisplay();
  
  Serial.println(F("=== Ready ==="));
  Serial.println(F("Touch screen to test buttons"));
  if (wifiConnected) Serial.printf("Web: http://%s\n", wifiIP);
}

void initSDCard() {
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  sdCardDetected = SD.begin(SD_CS);
}

void loadStationsFromSD() {
  if (!sdCardDetected) {
    loadDefaultStations();
    return;
  }
  
  File f = SD.open("/stations.txt", FILE_READ);
  if (!f) {
    loadDefaultStations();
    saveStationsToSD();
    return;
  }
  
  stationCount = 0;
  while (f.available() && stationCount < MAX_STATIONS) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      int sep = line.indexOf('|');
      if (sep > 0) {
        line.substring(0, sep).toCharArray(stations[stationCount].name, MAX_NAME_LEN);
        line.substring(sep + 1).toCharArray(stations[stationCount].url, MAX_URL_LEN);
        stationCount++;
      }
    }
  }
  f.close();
}

void saveStationsToSD() {
  if (!sdCardDetected) return;
  SD.remove("/stations.txt");
  File f = SD.open("/stations.txt", FILE_WRITE);
  if (!f) return;
  for (int i = 0; i < stationCount; i++) {
    f.printf("%s|%s\n", stations[i].name, stations[i].url);
  }
  f.close();
}

void loadDefaultStations() {
  const char* n[] = {"Record","RusMix","90s","Chill","Deep","Rock","Rap","Techno","House","EDM","TM","Pirate","Dub","Synth","LoFi","Euro","Trap","Hard","Amb","RusHits"};
  const char* u[] = {"rr_320","rusmix_320","sd90_320","chil_320","deep_320","rock_320","rap_320","techno320","house_320","edm_320","tm_320","ps_320","dub_320","synth_320","lofi_320","eurod_320","trap_320","hardst_320","ambient_320","rushits_320"};
  stationCount = 20;
  for (int i = 0; i < 20; i++) {
    strncpy(stations[i].name, n[i], MAX_NAME_LEN-1);
    snprintf(stations[i].url, MAX_URL_LEN, "https://radiorecord.hostingradio.ru/%s", u[i]);
  }
}

void playStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  currentStation = idx;
  isPlaying = true;
  audio.connecttohost(stations[idx].url);
  setLEDColor(0, 255, 0);
  updateDisplay();
}

void nextStation() { 
  Serial.println(F("[ACTION] Next station"));
  playStation((currentStation+1) % stationCount); 
}

void prevStation() { 
  Serial.println(F("[ACTION] Prev station"));
  playStation((currentStation-1+stationCount) % stationCount); 
}

void volumeUp() { 
  Serial.println(F("[ACTION] Volume +"));
  currentVolume = min(21, currentVolume+VOL_STEP); 
  audio.setVolume(currentVolume); 
  updateDisplay(); 
}

void volumeDown() { 
  Serial.println(F("[ACTION] Volume -"));
  currentVolume = max(0, currentVolume-VOL_STEP); 
  audio.setVolume(currentVolume); 
  updateDisplay(); 
}

void scanWiFiNetworks() {
  Serial.println(F("[ACTION] Scan WiFi"));
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println(F("Scanning WiFi..."));
  
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  wifiNetworkCount = 0;
  
  if (n == 0) {
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println(F("No networks"));
    delay(2000);
  } else {
    for (int i = 0; i < n && i < MAX_WIFI_NETWORKS; i++) {
      wifiNetworks[wifiNetworkCount].ssid = WiFi.SSID(i);
      wifiNetworks[wifiNetworkCount].rssi = WiFi.RSSI(i);
      wifiNetworks[wifiNetworkCount].encType = WiFi.encryptionType(i);
      wifiNetworkCount++;
    }
    
    for (int i = 0; i < wifiNetworkCount - 1; i++) {
      for (int j = i + 1; j < wifiNetworkCount; j++) {
        if (wifiNetworks[j].rssi > wifiNetworks[i].rssi) {
          WiFiNetwork temp = wifiNetworks[i];
          wifiNetworks[i] = wifiNetworks[j];
          wifiNetworks[j] = temp;
        }
      }
    }
    drawWiFiMenu();
  }
}

void drawWiFiMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 320, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.print(F("WiFi Networks"));
  
  tft.setTextSize(1);
  int startY = 40;
  int itemHeight = 25;
  
  for (int i = 0; i < wifiNetworkCount && i < 6; i++) {
    int y = startY + (i * itemHeight);
    tft.fillRect(0, y, 320, itemHeight, i == selectedNetwork ? TFT_BLUE : TFT_DARKGREY);
    tft.setTextColor(TFT_YELLOW, i == selectedNetwork ? TFT_BLUE : TFT_DARKGREY);
    tft.setCursor(10, y + 8);
    tft.print(wifiNetworks[i].encType != WIFI_AUTH_OPEN ? "L" : "O");
    tft.setTextColor(TFT_WHITE, i == selectedNetwork ? TFT_BLUE : TFT_DARKGREY);
    tft.setCursor(30, y + 8);
    String ssid = wifiNetworks[i].ssid;
    if (ssid.length() > 20) ssid = ssid.substring(0, 20);
    tft.print(ssid);
    tft.setTextColor(TFT_GREEN, i == selectedNetwork ? TFT_BLUE : TFT_DARKGREY);
    tft.setCursor(260, y + 8);
    tft.print(wifiNetworks[i].rssi);
  }
  
  tft.fillRoundRect(10, 210, 90, 25, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setTextSize(1);
  tft.setCursor(25, 218);
  tft.print(F("CONNECT"));
  
  tft.fillRoundRect(115, 210, 90, 25, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(130, 218);
  tft.print(F("REFRESH"));
  
  tft.fillRoundRect(220, 210, 90, 25, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(240, 218);
  tft.print(F("BACK"));
}

void drawKeyboard() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 320, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.print(F("Password"));
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 40);
  tft.print(F("Network: "));
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(wifiNetworks[selectedNetwork].ssid);
  
  tft.fillRect(10, 60, 300, 25, TFT_DARKGREY);
  tft.drawRect(10, 60, 300, 25, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setCursor(15, 68);
  tft.print(inputPassword);
  
  const char* kb[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM", "1234567890"};
  int keyWidth = 28, keyHeight = 28, startX = 10, startY = 100, spacing = 2;
  
  for (int row = 0; row < 4; row++) {
    int len = strlen(kb[row]);
    for (int i = 0; i < len; i++) {
      int x = startX + (i * (keyWidth + spacing));
      tft.fillRoundRect(x, startY + (row * (keyHeight + spacing)), keyWidth, keyHeight, 3, row < 3 ? TFT_BLUE : TFT_PURPLE);
      tft.setTextColor(TFT_WHITE, row < 3 ? TFT_BLUE : TFT_PURPLE);
      tft.setCursor(x + 10, startY + (row * (keyHeight + spacing)) + 10);
      tft.print(kb[row][i]);
    }
  }
  
  tft.fillRoundRect(10, 220, 60, 25, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(20, 228);
  tft.print(F("DEL"));
  
  tft.fillRoundRect(80, 220, 60, 25, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(90, 228);
  tft.print(F("CLEAR"));
  
  tft.fillRoundRect(250, 220, 60, 25, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setCursor(260, 228);
  tft.print(F("OK"));
}

void drawMainScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 320, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.print(F("NetRadio"));
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(220, 10);
  tft.print(curTime[0] ? curTime : "--:--:--");
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(10, 40);
  String name = String(stations[currentStation].name);
  if (name.length() > 12) name = name.substring(0, 12);
  tft.print(name);
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 70);
  tft.printf("Station %d/%d", currentStation+1, stationCount);
  
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  
  switch (displayMode) {
    case MODE_VOLUME:
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.printf("Volume: %d/21", currentVolume);
      tft.fillRect(10, 130, 300, 20, TFT_DARKGREY);
      tft.fillRect(10, 130, ::map(currentVolume, 0, 21, 0, 300), 20, TFT_GREEN);
      tft.drawRect(10, 130, 300, 20, TFT_WHITE);
      break;
    case MODE_TEMPERATURE:
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.printf("Moscow: %s", weatherTemp[0] ? weatherTemp : "N/A");
      break;
    case MODE_TIME:
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.printf("Date: %s", curDate[0] ? curDate : "N/A");
      break;
    case MODE_WIFI:
      tft.setTextColor(TFT_BLUE, TFT_BLACK);
      tft.printf("IP: %s", wifiIP);
      break;
  }
  
  // Кнопки управления - БОЛЬШЕ РАЗМЕР ДЛЯ ЛУЧШЕГО НАЖАТИЯ
  tft.fillRoundRect(5, 200, 75, 35, 5, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(15, 210);
  tft.print(F("PREV"));
  
  tft.fillRoundRect(85, 200, 75, 35, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setCursor(95, 210);
  tft.print(F("NEXT"));
  
  tft.fillRoundRect(165, 200, 75, 35, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(170, 210);
  tft.print(F("VOL-"));
  
  tft.fillRoundRect(245, 200, 75, 35, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(250, 210);
  tft.print(F("VOL+"));
  
  // WiFi кнопка
  tft.fillRoundRect(290, 5, 25, 20, 3, TFT_PURPLE);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);
  tft.setTextSize(1);
  tft.setCursor(295, 10);
  tft.print(F("W"));
  
  tft.setTextSize(1);
  tft.setCursor(10, 170);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.print(F("WiFi: "));
  tft.setTextColor(wifiConnected ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.print(wifiConnected ? wifiSSID : "Not connected");
  
  tft.setCursor(10, 185);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.print(F("SD: "));
  tft.setTextColor(sdCardDetected ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.print(sdCardDetected ? "OK" : "NOT FOUND");
}

void connectToWiFi(String ssid, String password) {
  Serial.printf("[ACTION] Connect to: %s\n", ssid.c_str());
  currentMenu = MENU_WIFI_CONNECTING;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println(F("Connecting..."));
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.print(F("SSID: "));
  tft.println(ssid);
  
  WiFi.begin(ssid.c_str(), password.c_str());
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    tft.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    delay(1000);
    wifiConnected = true;
    strncpy(wifiSSID, ssid.c_str(), 19);
    strcpy(wifiIP, WiFi.localIP().toString().c_str());
    configTime(10800, 0, "pool.ntp.org");
    prefs.begin("netradio", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println(F("Connected!"));
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.print(F("IP: "));
    tft.println(wifiIP);
    
    delay(2000);
    updateWeather();
    currentMenu = MENU_MAIN;
    if (stationCount > 0) playStation(currentStation);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println(F("Failed!"));
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.println(F("Check password"));
    delay(2000);
    currentMenu = MENU_WIFI_SCAN;
    drawWiFiMenu();
  }
}

void checkTouch() {
  // УПРОЩЁННАЯ ПРОВЕРКА КАСАНИЯ
  if (!touchscreen.touched()) return;
  
  // Debounce
  unsigned long now = millis();
  if (now - lastTouch < DEBOUNCE_MS) return;
  lastTouch = now;
  
  TS_Point p = touchscreen.getPoint();
  
  // Калибровка координат
  int x = ::map(p.x, 200, 3700, 0, 320);
  int y = ::map(p.y, 240, 3800, 0, 240);
  
  // Ограничение координат
  x = constrain(x, 0, 319);
  y = constrain(y, 0, 239);
  
  // Отладка
  Serial.printf("[TOUCH] X=%d Y=%d Menu=%d\n", x, y, currentMenu);
  
  // Обработка касаний в зависимости от меню
  if (currentMenu == MENU_MAIN) {
    // WiFi кнопка (правый верхний угол)
    if (x >= 280 && y <= 30) {
      Serial.println(F("[BTN] WiFi"));
      currentMenu = MENU_WIFI_SCAN;
      scanWiFiNetworks();
    }
    // PREV кнопка
    else if (x < 80 && y >= 200) {
      Serial.println(F("[BTN] PREV"));
      prevStation();
    }
    // NEXT кнопка
    else if (x >= 80 && x < 160 && y >= 200) {
      Serial.println(F("[BTN] NEXT"));
      nextStation();
    }
    // VOL- кнопка
    else if (x >= 160 && x < 240 && y >= 200) {
      Serial.println(F("[BTN] VOL-"));
      volumeDown();
    }
    // VOL+ кнопка
    else if (x >= 240 && y >= 200) {
      Serial.println(F("[BTN] VOL+"));
      volumeUp();
    }
  }
  else if (currentMenu == MENU_WIFI_SCAN) {
    // Список сетей
    for (int i = 0; i < wifiNetworkCount && i < 6; i++) {
      int itemY = 40 + (i * 25);
      if (y >= itemY && y < itemY + 25) {
        Serial.printf("[BTN] Network %d\n", i);
        selectedNetwork = i;
        drawWiFiMenu();
        return;
      }
    }
    // CONNECT
    if (x < 100 && y >= 210) {
      Serial.println(F("[BTN] CONNECT"));
      if (selectedNetwork >= 0) {
        if (wifiNetworks[selectedNetwork].encType == WIFI_AUTH_OPEN) {
          connectToWiFi(wifiNetworks[selectedNetwork].ssid, "");
        } else {
          inputPassword = "";
          currentMenu = MENU_WIFI_PASSWORD;
          drawKeyboard();
        }
      }
    }
    // REFRESH
    else if (x >= 100 && x < 220 && y >= 210) {
      Serial.println(F("[BTN] REFRESH"));
      scanWiFiNetworks();
    }
    // BACK
    else if (x >= 220 && y >= 210) {
      Serial.println(F("[BTN] BACK"));
      currentMenu = MENU_MAIN;
      drawMainScreen();
    }
  }
  else if (currentMenu == MENU_WIFI_PASSWORD) {
    const char* kb[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM", "1234567890"};
    int keyWidth = 28, keyHeight = 28, startX = 10, startY = 100, spacing = 2;
    
    for (int row = 0; row < 4; row++) {
      int len = strlen(kb[row]);
      for (int i = 0; i < len; i++) {
        int keyX = startX + (i * (keyWidth + spacing));
        int keyY = startY + (row * (keyHeight + spacing));
        if (x >= keyX && x < keyX + keyWidth && y >= keyY && y < keyY + keyHeight) {
          inputPassword += kb[row][i];
          Serial.printf("[KEY] %c\n", kb[row][i]);
          drawKeyboard();
          return;
        }
      }
    }
    
    // DEL
    if (x < 70 && y >= 220) {
      if (inputPassword.length() > 0) {
        inputPassword.remove(inputPassword.length() - 1);
        Serial.println(F("[KEY] DEL"));
        drawKeyboard();
      }
    }
    // CLEAR
    else if (x >= 80 && x < 140 && y >= 220) {
      inputPassword = "";
      Serial.println(F("[KEY] CLEAR"));
      drawKeyboard();
    }
    // OK
    else if (x >= 250 && y >= 220) {
      if (inputPassword.length() > 0) {
        Serial.println(F("[KEY] OK"));
        connectToWiFi(wifiNetworks[selectedNetwork].ssid, inputPassword);
      }
    }
  }
  
  // Ждём отпускания пальца
  while (touchscreen.touched()) {
    delay(5);
  }
}

void updateWeather() {
  if (!wifiConnected) return;
  WiFiClient client;
  if (client.connect("api.openweathermap.org", 80)) {
    client.printf("GET /data/2.5/weather?q=Moscow,RU&appid=%s&units=metric&lang=ru HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n", WEATHER_API_KEY);
    unsigned long t = millis();
    while (!client.available() && millis()-t < 5000) delay(1);
    String line;
    while (client.available()) {
      line = client.readStringUntil('\n');
      if (line.indexOf("\"temp\":") > 0) {
        int s = line.indexOf("\"temp\":")+7, e = line.indexOf(",", s);
        float temp = line.substring(s, e).toFloat();
        snprintf(weatherTemp, 8, "%.1fC", temp);
      }
    }
    client.stop();
  }
}

void updateTime() {
  if (!wifiConnected) return;
  struct tm t;
  if (getLocalTime(&t)) {
    strftime(curTime, 9, "%H:%M:%S", &t);
    strftime(curDate, 11, "%d.%m.%Y", &t);
  }
}

void updateDisplay() {
  switch (currentMenu) {
    case MENU_MAIN: drawMainScreen(); break;
    case MENU_WIFI_SCAN: drawWiFiMenu(); break;
    case MENU_WIFI_PASSWORD: drawKeyboard(); break;
    case MENU_WIFI_CONNECTING: break;
  }
}

void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  digitalWrite(LED_RED, r == 0 ? HIGH : LOW);
  digitalWrite(LED_GREEN, g == 0 ? HIGH : LOW);
  digitalWrite(LED_BLUE, b == 0 ? HIGH : LOW);
}

void checkButtons() {
  unsigned long now = millis();
  if (now - lastBtn < DEBOUNCE_MS) return;
  if (digitalRead(BTN_VOL_DOWN) == LOW) {
    lastBtn = now;
    volumeDown();
  }
}

const char HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1'><title>NetRadio</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0a0a1a;color:#e0e0e0}.h{background:linear-gradient(135deg,#064e3b,#0f766e);padding:15px;text-align:center}.h h1{color:#34d399}.c{max-width:400px;margin:0 auto;padding:10px}.i{background:#1a1a2e;border-radius:8px;padding:10px;margin-bottom:10px;border:1px solid #333}.r{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid #222}.l{color:#888;font-size:12px}.v{color:#34d399;font-weight:bold;font-size:12px}.g{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}.b{padding:8px;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold;color:#fff}.b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}.s{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.s h3{color:#34d399;margin-bottom:6px;font-size:13px}.si{display:flex;align-items:center;padding:5px;margin:2px 0;background:#0d0d1a;border-radius:4px;border:1px solid #222}.si.a{border-color:#34d399}.n{width:20px;color:#666;font-size:10px}.m{flex:1;font-size:11px}.a{display:flex;gap:2px}.a button{padding:2px 5px;border:none;border-radius:3px;cursor:pointer;font-size:9px;color:#fff}.p{background:#4CAF50}.e{background:#2196F3}.d{background:#F44336}.f{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.f h3{color:#34d399;margin-bottom:6px;font-size:13px}.fr{margin-bottom:6px}.fr label{display:block;color:#888;font-size:10px;margin-bottom:2px}.fr input{width:100%;padding:6px;border:1px solid #333;border-radius:3px;background:#0d0d1a;color:#e0e0e0;font-size:11px}.sa{width:100%;padding:8px;background:#10b981;color:#fff;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold}</style></head><body><div class=h><h1>NetRadio v.4.4</h1></div><div class=c><div class=i><div class=r><span class=l>WiFi:</span><span class=v id=w>--</span></div><div class=r><span class=l>IP:</span><span class=v id=ip>--</span></div><div class=r><span class=l>Station:</span><span class=v id=s>--</span></div><div class=r><span class=l>Vol:</span><span class=v id=vl>--</span></div><div class=r><span class=l>Weather:</span><span class=v id=wt>--</span></div></div><div class=g><button class=b b1 onclick=c('prev')>< Prev</button><button class=b b2 onclick=c('next')>Next ></button><button class=b b3 onclick=c('voldown')>Vol-</button><button class=b b4 onclick=c('volup')>Vol+</button></div><div class=s><h3>Stations (<span id=sc>0</span>/20)</h3><div id=sl></div></div><div class=f><h3 id=ft>+ Add</h3><div class=fr><label>Name:</label><input id=sn></div><div class=fr><label>URL:</label><input id=su></div><input type=hidden id=ei value=-1><button class=sa onclick=as()>Save</button></div></div><script>function c(x){fetch('/api/'+x).then(r=>r.json()).then(u)}function ls(){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sc').textContent=d.stations.length;let h='';for(let i=0;i<d.stations.length;i++){let s=d.stations[i],a=i===d.current?'a':'';h+='<div class=si '+a+'><span class=n>'+(i+1)+'</span><span class=m>'+s.name+'</span><div class=a><button class=p onclick=p('+i+')>P</button><button class=e onclick=e('+i+')>E</button><button class=d onclick=dl('+i+')>X</button></div></div>'}document.getElementById('sl').innerHTML=h})}function p(i){c('play/'+i);setTimeout(ls,500)}function e(i){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sn').value=d.stations[i].name;document.getElementById('su').value=d.stations[i].url;document.getElementById('ei').value=i;document.getElementById('ft').textContent='Edit #'+(i+1)})}function dl(i){if(confirm('Delete?'))fetch('/api/delete/'+i).then(()=>ls())}function as(){let n=document.getElementById('sn').value,u=document.getElementById('su').value,x=document.getElementById('ei').value;if(!n||!u)return alert('Fill all!');fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(()=>{ls();document.getElementById('sn').value='';document.getElementById('su').value='';document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add'})}function u(d){document.getElementById('w').textContent=d.ssid||'N/A';document.getElementById('ip').textContent=d.ip||'N/A';document.getElementById('vl').textContent=d.volume+'/21';document.getElementById('s').textContent=d.station||'N/A';document.getElementById('wt').textContent=d.weather||'N/A';ls()}fetch('/api/status').then(r=>r.json()).then(u);setInterval(()=>fetch('/api/status').then(r=>r.json()).then(u),3000)</script></body></html>)rawliteral";

void sendOK() { server.send(200, "application/json", "{\"status\":\"ok\"}"); }

void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String h; char c;
    for (int i = 0; (c = pgm_read_byte(&HTML[i])); i++) h += c;
    server.send(200, "text/html", h);
  });
  
  server.on("/api/status", HTTP_GET, []() {
    char j[200];
    snprintf(j, 200, "{\"ssid\":\"%s\",\"ip\":\"%s\",\"volume\":%d,\"current\":%d,\"station\":\"%s\",\"playing\":%s,\"weather\":\"%s\",\"stations\":%d}",
      wifiSSID, wifiIP, currentVolume, currentStation, stations[currentStation].name, 
      isPlaying?"true":"false", weatherTemp, stationCount);
    server.send(200, "application/json", j);
  });
  
  server.on("/api/stations", HTTP_GET, []() {
    String j = "{\"current\":" + String(currentStation) + ",\"stations\":[";
    for (int i = 0; i < stationCount; i++) {
      j += "{\"name\":\"" + String(stations[i].name) + "\",\"url\":\"" + String(stations[i].url) + "\"}";
      if (i < stationCount-1) j += ",";
    }
    j += "]}";
    server.send(200, "application/json", j);
  });
  
  server.on("/api/next", HTTP_GET, []() { nextStation(); sendOK(); });
  server.on("/api/prev", HTTP_GET, []() { prevStation(); sendOK(); });
  server.on("/api/volup", HTTP_GET, []() { volumeUp(); sendOK(); });
  server.on("/api/voldown", HTTP_GET, []() { volumeDown(); sendOK(); });
  
  for (int i = 0; i < 20; i++) {
    server.on(("/api/play/"+String(i)).c_str(), HTTP_GET, [i]() { playStation(i); sendOK(); });
    server.on(("/api/delete/"+String(i)).c_str(), HTTP_GET, [i]() {
      if (i < stationCount) {
        for (int j = i; j < stationCount-1; j++) stations[j] = stations[j+1];
        stationCount--;
        saveStationsToSD();
      }
      sendOK();
    });
  }
  
  server.on("/api/station", HTTP_POST, []() {
    if (!server.hasArg("plain")) { server.send(400, "application/json", "{\"error\":\"No data\"}"); return; }
    String b = server.arg("plain");
    int ns = b.indexOf("\"name\":\"")+8, ne = b.indexOf("\"", ns);
    String name = b.substring(ns, ne);
    int us = b.indexOf("\"url\":\"")+7, ue = b.indexOf("\"", us);
    String url = b.substring(us, ue);
    int is = b.indexOf("\"index\":")+8, ie = b.indexOf("}", is);
    int idx = b.substring(is, ie).toInt();
    if (idx >= 0 && idx < stationCount) {
      strncpy(stations[idx].name, name.c_str(), MAX_NAME_LEN-1);
      strncpy(stations[idx].url, url.c_str(), MAX_URL_LEN-1);
    } else if (stationCount < MAX_STATIONS) {
      strncpy(stations[stationCount].name, name.c_str(), MAX_NAME_LEN-1);
      strncpy(stations[stationCount].url, url.c_str(), MAX_URL_LEN-1);
      stationCount++;
    }
    saveStationsToSD(); sendOK();
  });
  
  server.begin();
}

void audio_info(const char *info) { Serial.printf("[AUDIO] %s\n", info); }
void audio_eof_mp3(const char *info) { nextStation(); }

void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();
  checkTouch();  // ВАЖНО: проверяем тачскрин!
  unsigned long now = millis();
  if (now - lastScr > 500) { lastScr = now; updateDisplay(); }
  if (now - lastTime > 1000) { lastTime = now; updateTime(); }
  if (now - lastWeather > 1800000) { lastWeather = now; updateWeather(); }
  delay(10);
}
