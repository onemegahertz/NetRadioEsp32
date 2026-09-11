export const arduinoSketch = String.raw`/*
 * ============================================================
 *  NetRadio v.1 - Internet Radio on ESP32
 *  TFT LCD 2.4" ILI9341 SPI 320x240
 *  I2S Audio Output (MAX98357A / PCM5102)
 *  Web Interface for Station Management
 *  Time & Weather Display
 * ============================================================
 *
 *  Libraries Required (install via Arduino Library Manager):
 *  - TFT_eSPI by Bodmer (v2.5.43+)
 *  - ESP32-audioI2S by schreibfaul1 (v3.0.7+)
 *  - ArduinoJson by Benoit Blanchon (v7.0.4+)
 *  - Preferences (built-in ESP32)
 *  - WiFi (built-in ESP32)
 *  - WebServer (built-in ESP32)
 *  - time by Paul Stoffregen (v1.6.2+)
 *  - HTTPClient (built-in ESP32)
 *
 *  IMPORTANT: TFT_eSPI User_Setup.h Configuration:
 *  #define ILI9341_DRIVER
 *  #define TFT_WIDTH  240
 *  #define TFT_HEIGHT 320
 *  #define TFT_MOSI   23
 *  #define TFT_SCLK   18
 *  #define TFT_CS      5    // Changed from 15
 *  #define TFT_DC      4    // Changed from 2
 *  #define TFT_RST    15    // Changed from 4
 *  #define SPI_FREQUENCY 40000000
 *
 *  Hardware Connections:
 *  TFT ILI9341:  VCC->3.3V, GND->GND, CS->5, RESET->15, DC->4,
 *                MOSI->23, SCK->18, LED->3.3V
 *  I2S DAC:      BCLK->26, LRC->25, DIN->22, VCC->5V, GND->GND
 *  Buttons:      BTN_NEXT->32, BTN_PREV->33, BTN_VOL_UP->34, BTN_VOL_DOWN->35
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Audio.h"
#include <time.h>
#include <HTTPClient.h>

// ==================== PIN DEFINITIONS ====================
#define TFT_CS     5
#define TFT_DC     4
#define TFT_RST   15
#define TFT_MOSI  23
#define TFT_SCLK  18

#define I2S_BCLK  26
#define I2S_LRC   25
#define I2S_DOUT  22

#define BTN_NEXT      32
#define BTN_PREV      33
#define BTN_VOL_UP    34
#define BTN_VOL_DOWN  35

// ==================== CONSTANTS ====================
#define MAX_STATIONS    20
#define MAX_NAME_LEN    32
#define MAX_URL_LEN     128
#define VOL_STEP        3
#define DEBOUNCE_MS     200
#define SCREEN_UPDATE_MS 1000
#define TIME_UPDATE_MS 60000
#define WEATHER_UPDATE_MS 1800000

// NTP Settings
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 10800  // Moscow UTC+3
#define DAYLIGHT_OFFSET_SEC 0

// Weather API (OpenWeatherMap - free)
#define WEATHER_API_KEY "YOUR_API_KEY_HERE"  // Get free key from openweathermap.org
#define WEATHER_CITY "Moscow,RU"

// ==================== GLOBAL OBJECTS ====================
TFT_eSPI tft = TFT_eSPI();
Audio audio;
WebServer server(80);
Preferences prefs;

// ==================== STATE VARIABLES ====================
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
String wifiSSID = "";
String wifiIP = "";

// Time & Weather
String currentTime = "";
String currentDate = "";
String weatherTemp = "";
String weatherDesc = "";
String weatherIcon = "";
unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;

bool diagTFT = false;
bool diagI2S = false;
bool diagWiFi = false;
bool diagButtons = false;

unsigned long lastButtonCheck = 0;
unsigned long lastScreenUpdate = 0;

// ==================== FORWARD DECLARATIONS ====================
void playStation(int index);
void nextStation();
void prevStation();
void volumeUp();
void volumeDown();
void updateDisplay();
void loadStations();
void saveStations();
void loadDefaultStations();
void deleteStation(int index);
void setupWebServer();
void showDiagnosticScreen();
void runDiagnostics();
void checkButtons();
void sendJsonOK();
void updateTime();
void updateWeather();
void drawWeatherIcon(int x, int y, String icon);

// ==================== DIAGNOSTIC FUNCTIONS ====================
void diagPrint(String component, bool status, String details) {
  Serial.print("[DIAG] ");
  Serial.print(component);
  Serial.print(": ");
  if (status) {
    Serial.print("OK");
  } else {
    Serial.print("FAIL");
  }
  if (details.length() > 0) {
    Serial.print(" - ");
    Serial.print(details);
  }
  Serial.println();
}

void runDiagnostics() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("  NetRadio v.1 - System Diagnostics");
  Serial.println("========================================");

  // 1. Check TFT Display
  Serial.println();
  Serial.println("[1/4] Checking TFT Display (ILI9341 SPI)...");
  
  // CRITICAL: Proper TFT initialization
  tft.init();
  tft.setRotation(0);  // Portrait mode
  
  // Test fill screen to verify communication
  tft.fillScreen(TFT_RED);
  delay(200);
  tft.fillScreen(TFT_GREEN);
  delay(200);
  tft.fillScreen(TFT_BLUE);
  delay(200);
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v.1");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 40);
  tft.println("Running diagnostics...");
  
  diagTFT = true;
  diagPrint("TFT ILI9341", diagTFT, "240x320 SPI initialized");

  // 2. Check I2S Audio
  Serial.println();
  Serial.println("[2/4] Checking I2S Audio Output...");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  diagI2S = true;
  diagPrint("I2S Audio", diagI2S, "BCLK=26 LRC=25 DOUT=22");

  // 3. Check Buttons
  Serial.println();
  Serial.println("[3/4] Checking Buttons...");
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_VOL_UP, INPUT_PULLUP);
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  diagButtons = true;
  diagPrint("Buttons", diagButtons, "NEXT=32 PREV=33 VOL+=34 VOL-=35");

  // 4. Check WiFi
  Serial.println();
  Serial.println("[4/4] Checking WiFi Connection...");
  tft.setCursor(10, 55);
  tft.println("Connecting to WiFi...");

  prefs.begin("netradio", false);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("password", "");

  if (ssid.length() == 0) {
    ssid = "NetRadio_Setup";
    pass = "netradio123";
    WiFi.softAP(ssid.c_str(), pass.c_str());
    wifiIP = WiFi.softAPIP().toString();
    diagWiFi = true;
    diagPrint("WiFi AP Mode", diagWiFi, "SSID: " + ssid + " IP: " + wifiIP);
  } else {
    WiFi.begin(ssid.c_str(), pass.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      wifiSSID = ssid;
      wifiIP = WiFi.localIP().toString();
      diagWiFi = true;
      diagPrint("WiFi Station", diagWiFi, "SSID: " + ssid + " IP: " + wifiIP);
      
      // Initialize NTP
      configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
      Serial.println("[NTP] Time sync started");
    } else {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("NetRadio_Setup", "netradio123");
      wifiIP = WiFi.softAPIP().toString();
      diagWiFi = false;
      diagPrint("WiFi", diagWiFi, "Connection failed! AP Mode: NetRadio_Setup");
    }
  }

  // Summary
  Serial.println();
  Serial.println("========================================");
  Serial.println("  Diagnostics Summary:");
  Serial.print("  TFT Display:    ");
  Serial.println(diagTFT ? "PASS" : "FAIL");
  Serial.print("  I2S Audio:      ");
  Serial.println(diagI2S ? "PASS" : "FAIL");
  Serial.print("  Buttons:        ");
  Serial.println(diagButtons ? "PASS" : "FAIL");
  Serial.print("  WiFi:           ");
  Serial.println(diagWiFi ? "PASS" : "FAIL (AP Mode)");
  Serial.println("========================================");

  showDiagnosticScreen();
  delay(3000);
}

void showDiagnosticScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(30, 5);
  tft.println("NetRadio v.1");

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 35);
  tft.println("--- Diagnostics ---");

  int y = 55;
  int lineH = 18;

  tft.setTextColor(diagTFT ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagTFT ? "[OK] " : "[FAIL] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("TFT Display");
  y += lineH;

  tft.setTextColor(diagI2S ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagI2S ? "[OK] " : "[FAIL] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("I2S Audio");
  y += lineH;

  tft.setTextColor(diagButtons ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagButtons ? "[OK] " : "[FAIL] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("Buttons");
  y += lineH;

  tft.setTextColor(diagWiFi ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagWiFi ? "[OK] " : "[!!] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (wifiConnected) {
    tft.print("WiFi: " + wifiSSID);
  } else {
    tft.print("AP: NetRadio_Setup");
  }
  y += lineH;

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, y + 10);
  tft.print("IP: " + wifiIP);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(10, y + 35);
  tft.print("Web: http://" + wifiIP);
}

// ==================== TIME & WEATHER ====================
void updateTime() {
  if (!wifiConnected) return;
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[NTP] Failed to obtain time");
    return;
  }
  
  char timeStr[9];
  char dateStr[11];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  strftime(dateStr, sizeof(dateStr), "%d.%m.%Y", &timeinfo);
  
  currentTime = String(timeStr);
  currentDate = String(dateStr);
  
  Serial.printf("[TIME] %s %s\n", dateStr, timeStr);
}

void updateWeather() {
  if (!wifiConnected) return;
  if (strcmp(WEATHER_API_KEY, "YOUR_API_KEY_HERE") == 0) {
    Serial.println("[WEATHER] API key not configured");
    weatherTemp = "N/A";
    weatherDesc = "No API key";
    return;
  }
  
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += WEATHER_CITY;
  url += "&appid=";
  url += WEATHER_API_KEY;
  url += "&units=metric&lang=ru";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      float temp = doc["main"]["temp"];
      const char* desc = doc["weather"][0]["description"];
      const char* icon = doc["weather"][0]["icon"];
      
      weatherTemp = String(temp, 1) + "°C";
      weatherDesc = String(desc);
      weatherIcon = String(icon);
      
      Serial.printf("[WEATHER] %s %s\n", weatherTemp.c_str(), weatherDesc.c_str());
    }
  } else {
    Serial.printf("[WEATHER] HTTP error: %d\n", httpCode);
  }
  
  http.end();
}

void drawWeatherIcon(int x, int y, String icon) {
  // Simple weather icons using text
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  
  if (icon.indexOf("01") >= 0) {  // Clear
    tft.setCursor(x, y);
    tft.print("*");
  } else if (icon.indexOf("02") >= 0 || icon.indexOf("03") >= 0 || icon.indexOf("04") >= 0) {  // Clouds
    tft.setCursor(x, y);
    tft.print("~");
  } else if (icon.indexOf("09") >= 0 || icon.indexOf("10") >= 0 || icon.indexOf("11") >= 0) {  // Rain
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(x, y);
    tft.print("/");
  } else if (icon.indexOf("13") >= 0) {  // Snow
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(x, y);
    tft.print("*");
  } else if (icon.indexOf("50") >= 0) {  // Mist
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(x, y);
    tft.print("=");
  }
}

// ==================== STATION MANAGEMENT ====================
void loadStations() {
  prefs.begin("stations", true);
  stationCount = prefs.getInt("count", 0);

  if (stationCount == 0) {
    prefs.end();
    loadDefaultStations();
    return;
  }

  for (int i = 0; i < stationCount && i < MAX_STATIONS; i++) {
    String key = "s" + String(i);
    String data = prefs.getString(key.c_str(), "");
    int sep = data.indexOf('|');
    if (sep > 0) {
      String name = data.substring(0, sep);
      String url = data.substring(sep + 1);
      name.toCharArray(stations[i].name, MAX_NAME_LEN);
      url.toCharArray(stations[i].url, MAX_URL_LEN);
    }
  }
  prefs.end();

  Serial.printf("[INFO] Loaded %d stations from memory\n", stationCount);
}

void saveStations() {
  prefs.begin("stations", false);
  prefs.putInt("count", stationCount);

  for (int i = 0; i < stationCount; i++) {
    String key = "s" + String(i);
    String data = String(stations[i].name) + "|" + String(stations[i].url);
    prefs.putString(key.c_str(), data);
  }
  prefs.end();

  Serial.printf("[INFO] Saved %d stations to memory\n", stationCount);
}

void loadDefaultStations() {
  const char* defaultNames[] = {
    "Record", "Russian Mix", "Super 90s", "Chill-Out", "Deep",
    "Rock", "Rap Hits", "Techno", "House Hits", "EDM",
    "Trancemission", "Pirate St.", "Dubstep", "Synthwave", "Lo-Fi",
    "Eurodance", "Trap", "Hardstyle", "Ambient", "Russian Hits"
  };
  const char* defaultUrls[] = {
    "https://radiorecord.hostingradio.ru/rr_320",
    "https://radiorecord.hostingradio.ru/rusmix_320",
    "https://radiorecord.hostingradio.ru/sd90_320",
    "https://radiorecord.hostingradio.ru/chil_320",
    "https://radiorecord.hostingradio.ru/deep_320",
    "https://radiorecord.hostingradio.ru/rock_320",
    "https://radiorecord.hostingradio.ru/rap_320",
    "https://radiorecord.hostingradio.ru/techno320",
    "https://radiorecord.hostingradio.ru/house_320",
    "https://radiorecord.hostingradio.ru/edm_320",
    "https://radiorecord.hostingradio.ru/tm_320",
    "https://radiorecord.hostingradio.ru/ps_320",
    "https://radiorecord.hostingradio.ru/dub_320",
    "https://radiorecord.hostingradio.ru/synth_320",
    "https://radiorecord.hostingradio.ru/lofi_320",
    "https://radiorecord.hostingradio.ru/eurod_320",
    "https://radiorecord.hostingradio.ru/trap_320",
    "https://radiorecord.hostingradio.ru/hardst_320",
    "https://radiorecord.hostingradio.ru/ambient_320",
    "https://radiorecord.hostingradio.ru/rushits_320"
  };

  stationCount = 20;
  for (int i = 0; i < stationCount; i++) {
    strcpy(stations[i].name, defaultNames[i]);
    strcpy(stations[i].url, defaultUrls[i]);
  }

  saveStations();
  Serial.println("[INFO] Default stations loaded and saved");
}

void deleteStation(int index) {
  if (index < 0 || index >= stationCount) return;

  for (int i = index; i < stationCount - 1; i++) {
    stations[i] = stations[i + 1];
  }
  stationCount--;

  if (currentStation >= stationCount && stationCount > 0) {
    currentStation = 0;
  }

  saveStations();
  Serial.printf("[WEB] Deleted station %d, now %d stations\n", index, stationCount);
}

// ==================== AUDIO CONTROL ====================
void playStation(int index) {
  if (index < 0 || index >= stationCount) return;

  currentStation = index;
  isPlaying = true;

  audio.connecttohost(stations[currentStation].url);

  Serial.printf("[AUDIO] Playing: %s\n", stations[currentStation].name);

  prefs.begin("netradio", false);
  prefs.putInt("lastStation", currentStation);
  prefs.putInt("volume", currentVolume);
  prefs.end();

  updateDisplay();
}

void nextStation() {
  currentStation++;
  if (currentStation >= stationCount) currentStation = 0;
  playStation(currentStation);
}

void prevStation() {
  currentStation--;
  if (currentStation < 0) currentStation = stationCount - 1;
  playStation(currentStation);
}

void volumeUp() {
  currentVolume += VOL_STEP;
  if (currentVolume > 21) currentVolume = 21;
  audio.setVolume(currentVolume);
  updateDisplay();
  Serial.printf("[AUDIO] Volume: %d/21\n", currentVolume);
}

void volumeDown() {
  currentVolume -= VOL_STEP;
  if (currentVolume < 0) currentVolume = 0;
  audio.setVolume(currentVolume);
  updateDisplay();
  Serial.printf("[AUDIO] Volume: %d/21\n", currentVolume);
}

// ==================== DISPLAY FUNCTIONS ====================
void updateDisplay() {
  if (!diagTFT) return;

  tft.fillScreen(TFT_BLACK);

  // ===== TOP BAR: Time & Date =====
  tft.fillRect(0, 0, 240, 25, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(5, 3);
  tft.print("NetRadio v.1");
  
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(5, 12);
  if (currentTime.length() > 0) {
    tft.print(currentTime);
  } else {
    tft.print("--:--:--");
  }
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_DARKGREY);
  tft.setCursor(150, 15);
  if (currentDate.length() > 0) {
    tft.print(currentDate);
  } else {
    tft.print("--.--.----");
  }

  // ===== WEATHER SECTION =====
  tft.fillRect(0, 30, 240, 35, 0x0A0A2E);
  tft.setTextColor(TFT_YELLOW, 0x0A0A2E);
  tft.setTextSize(1);
  tft.setCursor(5, 33);
  tft.print("Moscow:");
  
  if (weatherTemp.length() > 0) {
    tft.setTextColor(TFT_WHITE, 0x0A0A2E);
    tft.setTextSize(2);
    tft.setCursor(70, 33);
    tft.print(weatherTemp);
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY, 0x0A0A2E);
    tft.setCursor(5, 48);
    String desc = weatherDesc;
    if (desc.length() > 25) desc = desc.substring(0, 25);
    tft.print(desc);
  } else {
    tft.setTextColor(TFT_DARKGREY, 0x0A0A2E);
    tft.setTextSize(1);
    tft.setCursor(70, 38);
    tft.print("No data");
  }

  // ===== STATION INFO =====
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 75);
  String name = String(stations[currentStation].name);
  if (name.length() > 14) name = name.substring(0, 14);
  tft.print(name);

  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 95);
  tft.printf("Station %d/%d", currentStation + 1, stationCount);

  // ===== VOLUME BAR =====
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 115);
  tft.print("Volume:");

  int barX = 10;
  int barY = 130;
  int barW = 220;
  int barH = 15;

  tft.fillRect(barX, barY, barW, barH, TFT_DARKGREY);
  int fillW = map(currentVolume, 0, 21, 0, barW);
  tft.fillRect(barX, barY, fillW, barH, TFT_GREEN);
  tft.drawRect(barX, barY, barW, barH, TFT_WHITE);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 150);
  tft.printf("%d/21", currentVolume);

  // ===== WIFI INFO =====
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 170);
  if (wifiConnected) {
    tft.print("WiFi: " + wifiSSID);
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print("AP: NetRadio_Setup");
  }

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 185);
  tft.print("IP: " + wifiIP);

  // ===== STATUS =====
  tft.setCursor(10, 205);
  tft.print("BTN: Next/Prev/Vol+/-");

  if (isPlaying) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(180, 205);
    tft.print("PLAY");
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(180, 205);
    tft.print("STOP");
  }

  // ===== URL (truncated) =====
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 225);
  String url = String(stations[currentStation].url);
  if (url.length() > 30) url = url.substring(0, 30) + "...";
  tft.print(url);

  // ===== WEB INTERFACE HINT =====
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 240);
  tft.print("Web: http://" + wifiIP);
}

// ==================== BUTTON HANDLING ====================
void checkButtons() {
  unsigned long now = millis();
  if (now - lastButtonCheck < DEBOUNCE_MS) return;

  if (digitalRead(BTN_NEXT) == LOW) {
    lastButtonCheck = now;
    nextStation();
    Serial.println("[BTN] Next station");
  }

  if (digitalRead(BTN_PREV) == LOW) {
    lastButtonCheck = now;
    prevStation();
    Serial.println("[BTN] Previous station");
  }

  if (digitalRead(BTN_VOL_UP) == LOW) {
    lastButtonCheck = now;
    volumeUp();
    Serial.println("[BTN] Volume Up");
  }

  if (digitalRead(BTN_VOL_DOWN) == LOW) {
    lastButtonCheck = now;
    volumeDown();
    Serial.println("[BTN] Volume Down");
  }
}

// ==================== WEB INTERFACE HTML (PROGMEM) ====================
const char WEB_HTML_PART1[] PROGMEM =
"<!DOCTYPE html><html><head><meta charset=UTF-8>"
"<meta name=viewport content='width=device-width,initial-scale=1'>"
"<title>NetRadio v.1</title><style>"
"*{margin:0;padding:0;box-sizing:border-box}"
"body{font-family:Arial,sans-serif;background:#0a0a1a;color:#e0e0e0}"
".hd{background:linear-gradient(135deg,#1a1a3e,#2d1b69);padding:20px;text-align:center;border-bottom:2px solid #6c3ecf}"
".hd h1{color:#00e5ff;font-size:22px}.hd p{color:#888;font-size:11px}"
".ct{max-width:500px;margin:0 auto;padding:12px}"
".ip{background:#1a1a2e;border-radius:8px;padding:12px;margin-bottom:12px;border:1px solid #333}"
".ir{display:flex;justify-content:space-between;padding:4px 0;border-bottom:1px solid #222}"
".il{color:#888;font-size:13px}.iv{color:#00e5ff;font-weight:bold;font-size:13px}"
".ct2{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-bottom:12px}"
".bt{padding:10px;border:none;border-radius:6px;font-size:13px;cursor:pointer;font-weight:bold;color:#fff}"
".b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}"
".bt:active{opacity:0.7}"
".vd{text-align:center;font-size:16px;color:#00e5ff;margin:8px 0}"
".sl{background:#1a1a2e;border-radius:8px;padding:12px;border:1px solid #333;margin-bottom:12px}"
".sl h3{color:#00e5ff;margin-bottom:8px;font-size:14px}"
".si{display:flex;align-items:center;padding:6px;margin:3px 0;background:#0d0d1a;border-radius:5px;border:1px solid #222}"
".si.a{border-color:#00e5ff;background:#1a2a3e}"
".sn{width:22px;color:#666;font-size:11px}.nm{flex:1;font-size:12px}"
".sa{display:flex;gap:3px}"
".sa button{padding:3px 6px;border:none;border-radius:3px;cursor:pointer;font-size:10px;color:#fff}"
".sp{background:#4CAF50}.se{background:#2196F3}.sd{background:#F44336}"
".af{background:#1a1a2e;border-radius:8px;padding:12px;border:1px solid #333;margin-bottom:12px}"
".af h3{color:#00e5ff;margin-bottom:8px;font-size:14px}"
".fr{margin-bottom:8px}.fr label{display:block;color:#888;font-size:11px;margin-bottom:2px}"
".fr input{width:100%;padding:7px;border:1px solid #333;border-radius:4px;background:#0d0d1a;color:#e0e0e0;font-size:12px}"
".ba{width:100%;padding:9px;background:linear-gradient(135deg,#6c3ecf,#00e5ff);color:#fff;border:none;border-radius:6px;font-size:13px;cursor:pointer;font-weight:bold}"
".wf{background:#1a1a2e;border-radius:8px;padding:12px;border:1px solid #333}"
".wf h3{color:#00e5ff;margin-bottom:8px;font-size:14px}"
".bw{width:100%;padding:9px;background:#4CAF50;color:#fff;border:none;border-radius:6px;font-size:13px;cursor:pointer;font-weight:bold;margin-top:8px}"
"</style></head><body>";

const char WEB_HTML_PART2[] PROGMEM =
"<div class=hd><h1>NetRadio v.1</h1>"
"<p>ESP32 Internet Radio</p></div><div class=ct>"
"<div class=ip>"
"<div class=ir><span class=il>WiFi:</span><span class=iv id=ws>--</span></div>"
"<div class=ir><span class=il>IP:</span><span class=iv id=wi>--</span></div>"
"<div class=ir><span class=il>Station:</span><span class=iv id=cs>--</span></div>"
"<div class=ir><span class=il>Status:</span><span class=iv id=st>--</span></div>"
"</div>"
"<div class=vd>Vol: <span id=vl>0</span>/21</div>"
"<div class=ct2>"
"<button class=bt b1 onclick=sc('prev')>< Prev</button>"
"<button class=bt b2 onclick=sc('next')>Next ></button>"
"<button class=bt b3 onclick=sc('voldown')>Vol-</button>"
"<button class=bt b4 onclick=sc('volup')>Vol+</button>"
"</div>"
"<div class=sl><h3>Stations (<span id=sc2>0</span>/20)</h3>"
"<div id=slist></div></div>"
"<div class=af><h3 id=ft>+ Add Station</h3>"
"<div class=fr><label>Name:</label><input id=sn2 placeholder='Station name'></div>"
"<div class=fr><label>URL:</label><input id=su placeholder='https://...'></div>"
"<input type=hidden id=ei value=-1>"
"<button class=ba onclick=addSt()>Save</button></div>"
"<div class=wf><h3>WiFi Settings</h3>"
"<div class=fr><label>SSID:</label><input id=wn placeholder='WiFi name'></div>"
"<div class=fr><label>Password:</label><input type=password id=wp placeholder='Password'></div>"
"<button class=bw onclick=saveWifi()>Save WiFi</button></div>"
"</div>";

const char WEB_HTML_PART3[] PROGMEM =
"<script>"
"function sc(c){fetch('/api/'+c).then(function(r){return r.json()}).then(function(d){uui(d)})}"
"function ldSt(){fetch('/api/stations').then(function(r){return r.json()}).then(function(d){"
"document.getElementById('sc2').textContent=d.stations.length;"
"var h='';for(var i=0;i<d.stations.length;i++){"
"var s=d.stations[i];var ac=i===d.current?'a':'';"
"h+='<div class=si '+ac+'><span class=sn>'+(i+1)+'</span><span class=nm>'+s.name+'</span>'"
"+'<div class=sa><button class=sp onclick=plSt('+i+')>P</button>'"
"+'<button class=se onclick=edSt('+i+')>E</button>'"
"+'<button class=sd onclick=dlSt('+i+')>X</button></div></div>'}"
"document.getElementById('slist').innerHTML=h})}"
"function plSt(i){sc('play/'+i);setTimeout(ldSt,500)}"
"function edSt(i){fetch('/api/stations').then(function(r){return r.json()}).then(function(d){"
"document.getElementById('sn2').value=d.stations[i].name;"
"document.getElementById('su').value=d.stations[i].url;"
"document.getElementById('ei').value=i;"
"document.getElementById('ft').textContent='Edit #'+(i+1)})}"
"function dlSt(i){if(confirm('Delete?')){fetch('/api/delete/'+i).then(function(){ldSt()})}}"
"function addSt(){var n=document.getElementById('sn2').value;var u=document.getElementById('su').value;"
"var x=document.getElementById('ei').value;if(!n||!u){alert('Fill all!');return}"
"fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},"
"body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(function(){ldSt();"
"document.getElementById('sn2').value='';document.getElementById('su').value='';"
"document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add Station'})}"
"function saveWifi(){var s=document.getElementById('wn').value;var p=document.getElementById('wp').value;"
"if(!s){alert('Enter SSID!');return}"
"fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},"
"body:JSON.stringify({ssid:s,password:p})}).then(function(){alert('Saved! Rebooting...')})}"
"function uui(d){document.getElementById('ws').textContent=d.ssid||'N/A';"
"document.getElementById('wi').textContent=d.ip||'N/A';"
"document.getElementById('vl').textContent=d.volume||0;"
"document.getElementById('cs').textContent=d.station||'N/A';"
"var st=document.getElementById('st');"
"st.textContent=d.playing?'PLAYING':'STOPPED';"
"st.style.color=d.playing?'#4CAF50':'#F44336';ldSt()}"
"fetch('/api/status').then(function(r){return r.json()}).then(function(d){uui(d)});"
"setInterval(function(){fetch('/api/status').then(function(r){return r.json()}).then(function(d){uui(d)})},3000);"
"</script></body></html>";

String readProgmemStr(const char* progmemStr) {
  String result = "";
  char c;
  while ((c = pgm_read_byte(progmemStr++))) {
    result += c;
  }
  return result;
}

// ==================== JSON HELPERS ====================
void sendJsonOK() {
  JsonDocument doc;
  doc["status"] = "ok";
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void sendJsonError(String msg) {
  JsonDocument doc;
  doc["error"] = msg;
  String response;
  serializeJson(doc, response);
  server.send(400, "application/json", response);
}

// ==================== WEB SERVER ====================
void handlePlay(int idx) {
  playStation(idx);
  sendJsonOK();
}

void handleDelete(int idx) {
  deleteStation(idx);
  sendJsonOK();
}

void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String html = readProgmemStr(WEB_HTML_PART1);
    html += readProgmemStr(WEB_HTML_PART2);
    html += readProgmemStr(WEB_HTML_PART3);
    server.send(200, "text/html", html);
  });

  server.on("/api/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["ssid"] = wifiSSID;
    doc["ip"] = wifiIP;
    doc["volume"] = currentVolume;
    doc["current"] = currentStation;
    doc["station"] = String(stations[currentStation].name);
    doc["playing"] = isPlaying;
    doc["time"] = currentTime;
    doc["date"] = currentDate;
    doc["weather"] = weatherTemp + " " + weatherDesc;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  server.on("/api/stations", HTTP_GET, []() {
    JsonDocument doc;
    doc["current"] = currentStation;
    JsonArray arr = doc["stations"].to<JsonArray>();
    for (int i = 0; i < stationCount; i++) {
      JsonObject obj = arr.add<JsonObject>();
      obj["name"] = stations[i].name;
      obj["url"] = stations[i].url;
    }
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  server.on("/api/next", HTTP_GET, []() {
    nextStation();
    sendJsonOK();
  });

  server.on("/api/prev", HTTP_GET, []() {
    prevStation();
    sendJsonOK();
  });

  server.on("/api/volup", HTTP_GET, []() {
    volumeUp();
    sendJsonOK();
  });

  server.on("/api/voldown", HTTP_GET, []() {
    volumeDown();
    sendJsonOK();
  });

  server.on("/api/play/0", HTTP_GET, []() { handlePlay(0); });
  server.on("/api/play/1", HTTP_GET, []() { handlePlay(1); });
  server.on("/api/play/2", HTTP_GET, []() { handlePlay(2); });
  server.on("/api/play/3", HTTP_GET, []() { handlePlay(3); });
  server.on("/api/play/4", HTTP_GET, []() { handlePlay(4); });
  server.on("/api/play/5", HTTP_GET, []() { handlePlay(5); });
  server.on("/api/play/6", HTTP_GET, []() { handlePlay(6); });
  server.on("/api/play/7", HTTP_GET, []() { handlePlay(7); });
  server.on("/api/play/8", HTTP_GET, []() { handlePlay(8); });
  server.on("/api/play/9", HTTP_GET, []() { handlePlay(9); });
  server.on("/api/play/10", HTTP_GET, []() { handlePlay(10); });
  server.on("/api/play/11", HTTP_GET, []() { handlePlay(11); });
  server.on("/api/play/12", HTTP_GET, []() { handlePlay(12); });
  server.on("/api/play/13", HTTP_GET, []() { handlePlay(13); });
  server.on("/api/play/14", HTTP_GET, []() { handlePlay(14); });
  server.on("/api/play/15", HTTP_GET, []() { handlePlay(15); });
  server.on("/api/play/16", HTTP_GET, []() { handlePlay(16); });
  server.on("/api/play/17", HTTP_GET, []() { handlePlay(17); });
  server.on("/api/play/18", HTTP_GET, []() { handlePlay(18); });
  server.on("/api/play/19", HTTP_GET, []() { handlePlay(19); });

  server.on("/api/delete/0", HTTP_GET, []() { handleDelete(0); });
  server.on("/api/delete/1", HTTP_GET, []() { handleDelete(1); });
  server.on("/api/delete/2", HTTP_GET, []() { handleDelete(2); });
  server.on("/api/delete/3", HTTP_GET, []() { handleDelete(3); });
  server.on("/api/delete/4", HTTP_GET, []() { handleDelete(4); });
  server.on("/api/delete/5", HTTP_GET, []() { handleDelete(5); });
  server.on("/api/delete/6", HTTP_GET, []() { handleDelete(6); });
  server.on("/api/delete/7", HTTP_GET, []() { handleDelete(7); });
  server.on("/api/delete/8", HTTP_GET, []() { handleDelete(8); });
  server.on("/api/delete/9", HTTP_GET, []() { handleDelete(9); });
  server.on("/api/delete/10", HTTP_GET, []() { handleDelete(10); });
  server.on("/api/delete/11", HTTP_GET, []() { handleDelete(11); });
  server.on("/api/delete/12", HTTP_GET, []() { handleDelete(12); });
  server.on("/api/delete/13", HTTP_GET, []() { handleDelete(13); });
  server.on("/api/delete/14", HTTP_GET, []() { handleDelete(14); });
  server.on("/api/delete/15", HTTP_GET, []() { handleDelete(15); });
  server.on("/api/delete/16", HTTP_GET, []() { handleDelete(16); });
  server.on("/api/delete/17", HTTP_GET, []() { handleDelete(17); });
  server.on("/api/delete/18", HTTP_GET, []() { handleDelete(18); });
  server.on("/api/delete/19", HTTP_GET, []() { handleDelete(19); });

  server.on("/api/station", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      sendJsonError("No data");
      return;
    }

    String body = server.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      sendJsonError("JSON parse error");
      return;
    }

    const char* name = doc["name"];
    const char* url = doc["url"];
    int index = doc["index"];

    if (index >= 0 && index < stationCount) {
      strncpy(stations[index].name, name, MAX_NAME_LEN - 1);
      stations[index].name[MAX_NAME_LEN - 1] = '\0';
      strncpy(stations[index].url, url, MAX_URL_LEN - 1);
      stations[index].url[MAX_URL_LEN - 1] = '\0';
      Serial.printf("[WEB] Edited station %d: %s\n", index, name);
    } else {
      if (stationCount < MAX_STATIONS) {
        strncpy(stations[stationCount].name, name, MAX_NAME_LEN - 1);
        stations[stationCount].name[MAX_NAME_LEN - 1] = '\0';
        strncpy(stations[stationCount].url, url, MAX_URL_LEN - 1);
        stations[stationCount].url[MAX_URL_LEN - 1] = '\0';
        stationCount++;
        Serial.printf("[WEB] Added station %d: %s\n", stationCount - 1, name);
      } else {
        sendJsonError("Max stations reached (20)");
        return;
      }
    }

    saveStations();
    sendJsonOK();
  });

  server.on("/api/wifi", HTTP_POST, []() {
    String body = server.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      sendJsonError("JSON parse error");
      return;
    }

    const char* ssid = doc["ssid"];
    const char* pass = doc["password"];

    prefs.begin("netradio", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", pass);
    prefs.end();

    sendJsonOK();
    delay(1000);
    ESP.restart();
  });

  server.begin();
  Serial.println("[WEB] Server started on port 80");
}

// ==================== AUDIO CALLBACKS ====================
void audio_info(const char *info) {
  Serial.printf("[AUDIO] %s\n", info);
}

void audio_id3data(const char *info) {
  Serial.printf("[ID3] %s\n", info);
}

void audio_eof_mp3(const char *info) {
  Serial.printf("[AUDIO] EOF: %s\n", info);
  nextStation();
}

void audio_showstation(const char *info) {
  Serial.printf("[STATION] %s\n", info);
}

void audio_showstreamtitle(const char *info) {
  Serial.printf("[STREAM] %s\n", info);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("*** NetRadio v.1 Starting ***");
  Serial.println("ESP32 Internet Radio with TFT Display");
  Serial.println("=====================================");

  runDiagnostics();
  loadStations();

  prefs.begin("netradio", true);
  currentStation = prefs.getInt("lastStation", 0);
  currentVolume = prefs.getInt("volume", 12);
  prefs.end();

  if (currentStation >= stationCount) currentStation = 0;

  setupWebServer();

  if (wifiConnected && stationCount > 0) {
    playStation(currentStation);
    
    // Initial time and weather update
    updateTime();
    updateWeather();
  }

  updateDisplay();

  Serial.println();
  Serial.println("*** NetRadio v.1 Ready ***");
  Serial.printf("Web Interface: http://%s\n", wifiIP.c_str());
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();

  unsigned long now = millis();
  
  // Update display every second
  if (now - lastScreenUpdate > SCREEN_UPDATE_MS) {
    lastScreenUpdate = now;
    updateDisplay();
  }
  
  // Update time every minute
  if (now - lastTimeUpdate > TIME_UPDATE_MS) {
    lastTimeUpdate = now;
    updateTime();
  }
  
  // Update weather every 30 minutes
  if (now - lastWeatherUpdate > WEATHER_UPDATE_MS) {
    lastWeatherUpdate = now;
    updateWeather();
  }
}
`;

export const userSetupConfig = String.raw`
// ============================================
// TFT_eSPI User_Setup.h Configuration
// CRITICAL: These pins are different from previous version!
// ============================================

#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS     5    // CHANGED from 15
#define TFT_DC     4    // CHANGED from 2
#define TFT_RST   15    // CHANGED from 4
#define TFT_MISO  -1

#define TFT_BL   -1

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000

// IMPORTANT: Comment out ALL other driver definitions!
// Only ILI9341_DRIVER should be active
`;
