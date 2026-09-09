export const arduinoSketch = `/*
 * ============================================================
 *  NetRadio v.1 - Internet Radio on ESP32
 *  TFT LCD 2.4" ILI9341 SPI 320x240
 *  I2S Audio Output (MAX98357A / PCM5102)
 *  Web Interface for Station Management
 * ============================================================
 *  
 *  Libraries Required (install via Arduino Library Manager):
 *  - TFT_eSPI by Bodmer (v2.5.43+)
 *  - ESP32-audioI2S by schreibfaul1 (v3.0.7+)
 *  - ArduinoJson by Benoit Blanchon (v7.0.4+)
 *  - Preferences (built-in ESP32)
 *  - WiFi (built-in ESP32)
 *  - WebServer (built-in ESP32)
 *  
 *  TFT_eSPI Configuration (User_Setup.h):
 *  #define ILI9341_DRIVER
 *  #define TFT_WIDTH  240
 *  #define TFT_HEIGHT 320
 *  #define TFT_MOSI   23
 *  #define TFT_SCLK   18
 *  #define TFT_CS     15
 *  #define TFT_DC      2
 *  #define TFT_RST     4
 *  #define SPI_FREQUENCY 40000000
 *  
 *  Hardware Connections:
 *  TFT ILI9341:  VCC->3.3V, GND->GND, CS->15, RESET->4, DC->2, 
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

// ==================== PIN DEFINITIONS ====================
// TFT Display Pins (SPI)
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST    4
#define TFT_MOSI  23
#define TFT_SCLK  18

// I2S Audio Pins
#define I2S_BCLK  26
#define I2S_LRC   25
#define I2S_DOUT  22

// Button Pins
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
#define SCREEN_UPDATE_MS 500

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
int currentVolume = 12;  // 0-21 range for audio library
bool isPlaying = false;
bool wifiConnected = false;
String wifiSSID = "";
String wifiIP = "";

// Diagnostic flags
bool diagTFT = false;
bool diagI2S = false;
bool diagWiFi = false;
bool diagButtons = false;

// Timing
unsigned long lastButtonCheck = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastStationName = 0;

// ==================== DIAGNOSTIC FUNCTIONS ====================
void diagPrint(String component, bool status, String details = "") {
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
  Serial.println("\\n========================================");
  Serial.println("  NetRadio v.1 - System Diagnostics");
  Serial.println("========================================");
  
  // 1. Check TFT Display
  Serial.println("\\n[1/4] Checking TFT Display (ILI9341 SPI)...");
  tft.init();
  tft.setRotation(0);  // Portrait 240x320
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v.1");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 40);
  tft.println("Running diagnostics...");
  diagTFT = true;  // If we got here, TFT is working
  diagPrint("TFT ILI9341", diagTFT, "240x320 SPI initialized");
  
  // 2. Check I2S Audio
  Serial.println("\\n[2/4] Checking I2S Audio Output...");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  diagI2S = true;  // I2S configured successfully
  diagPrint("I2S Audio", diagI2S, "BCLK=26 LRC=25 DOUT=22");
  
  // 3. Check Buttons
  Serial.println("\\n[3/4] Checking Buttons...");
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_VOL_UP, INPUT_PULLUP);
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  diagButtons = true;
  diagPrint("Buttons", diagButtons, "NEXT=32 PREV=33 VOL+=34 VOL-=35");
  
  // 4. Check WiFi
  Serial.println("\\n[4/4] Checking WiFi Connection...");
  tft.setCursor(10, 55);
  tft.println("Connecting to WiFi...");
  
  // Load WiFi credentials from preferences
  prefs.begin("netradio", false);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("password", "");
  
  if (ssid.length() == 0) {
    // Default AP mode if no credentials saved
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
    } else {
      // Fallback to AP mode
      WiFi.mode(WIFI_AP);
      WiFi.softAP("NetRadio_Setup", "netradio123");
      wifiIP = WiFi.softAPIP().toString();
      diagWiFi = false;
      diagPrint("WiFi", diagWiFi, "Connection failed! AP Mode: NetRadio_Setup");
    }
  }
  
  // Summary
  Serial.println("\\n========================================");
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
  
  // Update TFT with diagnostic results
  showDiagnosticScreen();
  delay(3000);
}

void showDiagnosticScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // Header
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(30, 5);
  tft.println("NetRadio v.1");
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 35);
  tft.println("--- Diagnostics ---");
  
  // Results
  int y = 55;
  int lineH = 18;
  
  // TFT Status
  tft.setTextColor(diagTFT ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagTFT ? "[OK] " : "[FAIL] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("TFT Display");
  y += lineH;
  
  // I2S Status
  tft.setTextColor(diagI2S ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagI2S ? "[OK] " : "[FAIL] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("I2S Audio");
  y += lineH;
  
  // Buttons Status
  tft.setTextColor(diagButtons ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagButtons ? "[OK] " : "[FAIL] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("Buttons");
  y += lineH;
  
  // WiFi Status
  tft.setTextColor(diagWiFi ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(10, y);
  tft.print(diagWiFi ? "[OK] " : "[!!] ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(diagWiFi ? "WiFi: " + wifiSSID : "AP: NetRadio_Setup");
  y += lineH;
  
  // IP Address
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, y + 10);
  tft.print("IP: " + wifiIP);
  
  // Web interface hint
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(10, y + 35);
  tft.print("Web UI: http://" + wifiIP);
}

// ==================== STATION MANAGEMENT ====================
void loadStations() {
  prefs.begin("stations", true);  // Read-only
  stationCount = prefs.getInt("count", 0);
  
  if (stationCount == 0) {
    // Load default stations
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
  
  Serial.printf("[INFO] Loaded %d stations from memory\\n", stationCount);
}

void saveStations() {
  prefs.begin("stations", false);  // Read-write
  prefs.putInt("count", stationCount);
  
  for (int i = 0; i < stationCount; i++) {
    String key = "s" + String(i);
    String data = String(stations[i].name) + "|" + String(stations[i].url);
    prefs.putString(key.c_str(), data);
  }
  prefs.end();
  
  Serial.printf("[INFO] Saved %d stations to memory\\n", stationCount);
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

// ==================== AUDIO CONTROL ====================
void playStation(int index) {
  if (index < 0 || index >= stationCount) return;
  
  currentStation = index;
  isPlaying = true;
  
  audio.connecttohost(stations[currentStation].url);
  
  Serial.printf("[AUDIO] Playing: %s (%s)\\n", stations[currentStation].name, stations[currentStation].url);
  
  // Save current station
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
  Serial.printf("[AUDIO] Volume: %d/21\\n", currentVolume);
}

void volumeDown() {
  currentVolume -= VOL_STEP;
  if (currentVolume < 0) currentVolume = 0;
  audio.setVolume(currentVolume);
  updateDisplay();
  Serial.printf("[AUDIO] Volume: %d/21\\n", currentVolume);
}

// ==================== DISPLAY FUNCTIONS ====================
void updateDisplay() {
  if (!diagTFT) return;
  
  tft.fillScreen(TFT_BLACK);
  
  // Header bar
  tft.fillRect(0, 0, 240, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(5, 5);
  tft.print("NetRadio v.1");
  
  // WiFi info
  tft.setTextColor(TFT_GREEN, TFT_DARKGREY);
  tft.setCursor(5, 17);
  if (wifiConnected) {
    tft.print("WiFi: " + wifiSSID);
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
    tft.print("AP: NetRadio_Setup");
  }
  
  // IP Address
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(5, 35);
  tft.print("IP: " + wifiIP);
  
  // Station info
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 65);
  // Truncate long names
  String name = String(stations[currentStation].name);
  if (name.length() > 14) name = name.substring(0, 14);
  tft.print(name);
  
  // Station number
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 90);
  tft.printf("Station %d/%d", currentStation + 1, stationCount);
  
  // Volume bar
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 115);
  tft.print("Volume:");
  
  int barX = 10;
  int barY = 130;
  int barW = 220;
  int barH = 15;
  
  // Background
  tft.fillRect(barX, barY, barW, barH, TFT_DARKGREY);
  // Fill
  int fillW = map(currentVolume, 0, 21, 0, barW);
  tft.fillRect(barX, barY, fillW, barH, TFT_GREEN);
  // Border
  tft.drawRect(barX, barY, barW, barH, TFT_WHITE);
  
  // Volume text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 150);
  tft.printf("%d/21", currentVolume);
  
  // Controls hint
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 180);
  tft.print("BTN: Next/Prev/Vol+/-");
  
  // Playing indicator
  if (isPlaying) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(180, 180);
    tft.print("PLAY");
  }
  
  // URL (truncated)
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 200);
  String url = String(stations[currentStation].url);
  if (url.length() > 30) url = url.substring(0, 30) + "...";
  tft.print(url);
  
  // Web UI hint
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 220);
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

// ==================== WEB SERVER ====================
const char WEB_INTERFACE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>NetRadio v.1 - Web Control</title>
<style>
* { margin:0; padding:0; box-sizing:border-box; }
body { font-family: 'Segoe UI', Arial, sans-serif; background: #0a0a1a; color: #e0e0e0; min-height: 100vh; }
.header { background: linear-gradient(135deg, #1a1a3e, #2d1b69); padding: 20px; text-align: center; border-bottom: 2px solid #6c3ecf; }
.header h1 { color: #00e5ff; font-size: 24px; margin-bottom: 5px; }
.header p { color: #888; font-size: 12px; }
.container { max-width: 600px; margin: 0 auto; padding: 15px; }
.info-panel { background: #1a1a2e; border-radius: 10px; padding: 15px; margin-bottom: 15px; border: 1px solid #333; }
.info-row { display: flex; justify-content: space-between; padding: 5px 0; border-bottom: 1px solid #222; }
.info-label { color: #888; }
.info-value { color: #00e5ff; font-weight: bold; }
.controls { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 15px; }
.btn { padding: 12px; border: none; border-radius: 8px; font-size: 14px; cursor: pointer; transition: all 0.2s; font-weight: bold; }
.btn-prev { background: #2196F3; color: white; }
.btn-next { background: #4CAF50; color: white; }
.btn-voldown { background: #FF9800; color: white; }
.btn-volup { background: #F44336; color: white; }
.btn:hover { transform: scale(1.05); opacity: 0.9; }
.btn:active { transform: scale(0.95); }
.station-list { background: #1a1a2e; border-radius: 10px; padding: 15px; border: 1px solid #333; }
.station-list h3 { color: #00e5ff; margin-bottom: 10px; }
.station-item { display: flex; align-items: center; padding: 8px; margin: 4px 0; background: #0d0d1a; border-radius: 6px; border: 1px solid #222; }
.station-item.active { border-color: #00e5ff; background: #1a2a3e; }
.station-num { width: 25px; color: #666; font-size: 12px; }
.station-name { flex: 1; font-size: 13px; }
.station-actions { display: flex; gap: 5px; }
.station-actions button { padding: 4px 8px; border: none; border-radius: 4px; cursor: pointer; font-size: 11px; }
.btn-play { background: #4CAF50; color: white; }
.btn-edit { background: #2196F3; color: white; }
.btn-del { background: #F44336; color: white; }
.add-form { background: #1a1a2e; border-radius: 10px; padding: 15px; margin-top: 15px; border: 1px solid #333; }
.add-form h3 { color: #00e5ff; margin-bottom: 10px; }
.form-row { margin-bottom: 10px; }
.form-row label { display: block; color: #888; font-size: 12px; margin-bottom: 3px; }
.form-row input { width: 100%; padding: 8px; border: 1px solid #333; border-radius: 5px; background: #0d0d1a; color: #e0e0e0; font-size: 13px; }
.btn-add { width: 100%; padding: 10px; background: linear-gradient(135deg, #6c3ecf, #00e5ff); color: white; border: none; border-radius: 8px; font-size: 14px; cursor: pointer; font-weight: bold; }
.btn-add:hover { opacity: 0.9; }
.volume-display { text-align: center; font-size: 18px; color: #00e5ff; margin: 10px 0; }
.wifi-form { background: #1a1a2e; border-radius: 10px; padding: 15px; margin-top: 15px; border: 1px solid #333; }
.wifi-form h3 { color: #00e5ff; margin-bottom: 10px; }
.btn-save-wifi { width: 100%; padding: 10px; background: #4CAF50; color: white; border: none; border-radius: 8px; font-size: 14px; cursor: pointer; font-weight: bold; margin-top: 10px; }
</style>
</head>
<body>
<div class="header">
<h1>&#128251; NetRadio v.1</h1>
<p>ESP32 Internet Radio Control Panel</p>
</div>
<div class="container">
<div class="info-panel">
<div class="info-row"><span class="info-label">WiFi Network:</span><span class="info-value" id="wifiSSID">Loading...</span></div>
<div class="info-row"><span class="info-label">IP Address:</span><span class="info-value" id="wifiIP">Loading...</span></div>
<div class="info-row"><span class="info-label">Current Station:</span><span class="info-value" id="currentStation">Loading...</span></div>
<div class="info-row"><span class="info-label">Status:</span><span class="info-value" id="status">Loading...</span></div>
</div>
<div class="volume-display">Volume: <span id="volume">0</span>/21</div>
<div class="controls">
<button class="btn btn-prev" onclick="sendCmd('prev')">&#9664; Prev</button>
<button class="btn btn-next" onclick="sendCmd('next')">Next &#9654;</button>
<button class="btn btn-voldown" onclick="sendCmd('voldown')">&#128264; Vol -</button>
<button class="btn btn-volup" onclick="sendCmd('volup')">&#128266; Vol +</button>
</div>
<div class="station-list">
<h3>&#128251; Stations (<span id="stationCount">0</span>/20)</h3>
<div id="stationList"></div>
</div>
<div class="add-form">
<h3 id="formTitle">+ Add Station</h3>
<div class="form-row"><label>Station Name:</label><input type="text" id="stationName" placeholder="e.g. Record"></div>
<div class="form-row"><label>Stream URL:</label><input type="text" id="stationURL" placeholder="https://..."></div>
<input type="hidden" id="editIndex" value="-1">
<button class="btn-add" onclick="addStation()">Save Station</button>
</div>
<div class="wifi-form">
<h3>&#128246; WiFi Settings</h3>
<div class="form-row"><label>SSID:</label><input type="text" id="wifiName" placeholder="Your WiFi name"></div>
<div class="form-row"><label>Password:</label><input type="password" id="wifiPass" placeholder="Your WiFi password"></div>
<button class="btn-save-wifi" onclick="saveWiFi()">Save & Reconnect</button>
</div>
</div>
<script>
function sendCmd(cmd) {
  fetch('/api/' + cmd).then(r => r.json()).then(d => updateUI(d));
}
function loadStations() {
  fetch('/api/stations').then(r => r.json()).then(data => {
    document.getElementById('stationCount').textContent = data.stations.length;
    let html = '';
    data.stations.forEach((s, i) => {
      let active = i === data.current ? 'active' : '';
      html += '<div class="station-item ' + active + '">' +
        '<span class="station-num">' + (i+1) + '</span>' +
        '<span class="station-name">' + s.name + '</span>' +
        '<div class="station-actions">' +
        '<button class="btn-play" onclick="playStation(' + i + ')">&#9654;</button>' +
        '<button class="btn-edit" onclick="editStation(' + i + ')">&#9998;</button>' +
        '<button class="btn-del" onclick="deleteStation(' + i + ')">&#10005;</button>' +
        '</div></div>';
    });
    document.getElementById('stationList').innerHTML = html;
  });
}
function playStation(i) { sendCmd('play/' + i); setTimeout(loadStations, 500); }
function editStation(i) {
  fetch('/api/stations').then(r => r.json()).then(data => {
    document.getElementById('stationName').value = data.stations[i].name;
    document.getElementById('stationURL').value = data.stations[i].url;
    document.getElementById('editIndex').value = i;
    document.getElementById('formTitle').textContent = 'Edit Station #' + (i+1);
  });
}
function deleteStation(i) {
  if (confirm('Delete this station?')) {
    fetch('/api/delete/' + i).then(r => r.json()).then(() => { loadStations(); });
  }
}
function addStation() {
  let name = document.getElementById('stationName').value;
  let url = document.getElementById('stationURL').value;
  let idx = document.getElementById('editIndex').value;
  if (!name || !url) { alert('Fill all fields!'); return; }
  let body = JSON.stringify({name: name, url: url, index: parseInt(idx)});
  fetch('/api/station', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: body})
    .then(r => r.json()).then(() => {
      document.getElementById('stationName').value = '';
      document.getElementById('stationURL').value = '';
      document.getElementById('editIndex').value = '-1';
      document.getElementById('formTitle').textContent = '+ Add Station';
      loadStations();
    });
}
function saveWiFi() {
  let ssid = document.getElementById('wifiName').value;
  let pass = document.getElementById('wifiPass').value;
  if (!ssid) { alert('Enter SSID!'); return; }
  fetch('/api/wifi', {method: 'POST', headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ssid: ssid, password: pass})})
    .then(r => r.json()).then(d => { alert('WiFi saved! Reconnecting...'); });
}
function updateUI(data) {
  document.getElementById('wifiSSID').textContent = data.ssid || 'N/A';
  document.getElementById('wifiIP').textContent = data.ip || 'N/A';
  document.getElementById('volume').textContent = data.volume || 0;
  document.getElementById('currentStation').textContent = data.station || 'N/A';
  document.getElementById('status').textContent = data.playing ? 'PLAYING' : 'STOPPED';
  document.getElementById('status').style.color = data.playing ? '#4CAF50' : '#F44336';
  loadStations();
}
// Initial load
fetch('/api/status').then(r => r.json()).then(d => updateUI(d));
setInterval(() => { fetch('/api/status').then(r => r.json()).then(d => updateUI(d)); }, 3000);
</script>
</body>
</html>
)rawliteral";

void setupWebServer() {
  // Serve main page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", WEB_INTERFACE_HTML);
  });
  
  // API: Get status
  server.on("/api/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["ssid"] = wifiSSID;
    doc["ip"] = wifiIP;
    doc["volume"] = currentVolume;
    doc["current"] = currentStation;
    doc["station"] = String(stations[currentStation].name);
    doc["playing"] = isPlaying;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  // API: Get stations list
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
  
  // API: Control commands
  server.on("/api/next", HTTP_GET, []() {
    nextStation();
    server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}");
  });
  
  server.on("/api/prev", HTTP_GET, []() {
    prevStation();
    server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}");
  });
  
  server.on("/api/volup", HTTP_GET, []() {
    volumeUp();
    server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}");
  });
  
  server.on("/api/voldown", HTTP_GET, []() {
    volumeDown();
    server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}");
  });
  
  // Play station by index - using a handler that parses the URI
  server.on("/api/play/0", HTTP_GET, []() { playStation(0); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/1", HTTP_GET, []() { playStation(1); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/2", HTTP_GET, []() { playStation(2); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/3", HTTP_GET, []() { playStation(3); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/4", HTTP_GET, []() { playStation(4); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/5", HTTP_GET, []() { playStation(5); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/6", HTTP_GET, []() { playStation(6); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/7", HTTP_GET, []() { playStation(7); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/8", HTTP_GET, []() { playStation(8); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/9", HTTP_GET, []() { playStation(9); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/10", HTTP_GET, []() { playStation(10); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/11", HTTP_GET, []() { playStation(11); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/12", HTTP_GET, []() { playStation(12); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/13", HTTP_GET, []() { playStation(13); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/14", HTTP_GET, []() { playStation(14); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/15", HTTP_GET, []() { playStation(15); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/16", HTTP_GET, []() { playStation(16); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/17", HTTP_GET, []() { playStation(17); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/18", HTTP_GET, []() { playStation(18); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  server.on("/api/play/19", HTTP_GET, []() { playStation(19); server.send(200, "application/json", "{\\\"status\\\":\\\"ok\\\"}"); });
  
  // API: Add/Edit station
  server.on("/api/station", HTTP_POST, []() {
    if (server.hasArg("plain") == false) {
      server.send(400, "application/json", "{\\"error\\":\\"No data\\"}");
      return;
    }
    
    String body = server.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      server.send(400, "application/json", "{\\"error\\":\\"JSON parse error\\"}");
      return;
    }
    
    const char* name = doc["name"];
    const char* url = doc["url"];
    int index = doc["index"];
    
    if (index >= 0 && index < stationCount) {
      // Edit existing
      strncpy(stations[index].name, name, MAX_NAME_LEN - 1);
      strncpy(stations[index].url, url, MAX_URL_LEN - 1);
      Serial.printf("[WEB] Edited station %d: %s\\n", index, name);
    } else {
      // Add new
      if (stationCount < MAX_STATIONS) {
        strncpy(stations[stationCount].name, name, MAX_NAME_LEN - 1);
        strncpy(stations[stationCount].url, url, MAX_URL_LEN - 1);
        stationCount++;
        Serial.printf("[WEB] Added station %d: %s\\n", stationCount - 1, name);
      } else {
        server.send(400, "application/json", "{\\"error\\":\\"Max stations reached\\"}");
        return;
      }
    }
    
    saveStations();
    server.send(200, "application/json", "{\\"status\\":\\"ok\\"}");
  });
  
  // API: Delete station
  server.on("/api/delete/0", HTTP_GET, []() { deleteStation(0); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/1", HTTP_GET, []() { deleteStation(1); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/2", HTTP_GET, []() { deleteStation(2); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/3", HTTP_GET, []() { deleteStation(3); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/4", HTTP_GET, []() { deleteStation(4); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/5", HTTP_GET, []() { deleteStation(5); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/6", HTTP_GET, []() { deleteStation(6); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/7", HTTP_GET, []() { deleteStation(7); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/8", HTTP_GET, []() { deleteStation(8); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/9", HTTP_GET, []() { deleteStation(9); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/10", HTTP_GET, []() { deleteStation(10); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/11", HTTP_GET, []() { deleteStation(11); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/12", HTTP_GET, []() { deleteStation(12); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/13", HTTP_GET, []() { deleteStation(13); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/14", HTTP_GET, []() { deleteStation(14); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/15", HTTP_GET, []() { deleteStation(15); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/16", HTTP_GET, []() { deleteStation(16); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/17", HTTP_GET, []() { deleteStation(17); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/18", HTTP_GET, []() { deleteStation(18); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  server.on("/api/delete/19", HTTP_GET, []() { deleteStation(19); server.send(200, "application/json", "{\\"status\\":\\"ok\\"}"); });
  
  // API: Save WiFi
  server.on("/api/wifi", HTTP_POST, []() {
    String body = server.arg("plain");
    JsonDocument doc;
    deserializeJson(doc, body);
    
    const char* ssid = doc["ssid"];
    const char* pass = doc["password"];
    
    prefs.begin("netradio", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", pass);
    prefs.end();
    
    server.send(200, "application/json", "{\\"status\\":\\"ok\\", \\"message\\":\\"WiFi saved. Restarting...\\"}");
    delay(1000);
    ESP.restart();
  });
  
  server.begin();
  Serial.println("[WEB] Server started on port 80");
}

void deleteStation(int index) {
  if (index < 0 || index >= stationCount) return;
  
  for (int i = index; i < stationCount - 1; i++) {
    stations[i] = stations[i + 1];
  }
  stationCount--;
  
  if (currentStation >= stationCount) {
    currentStation = 0;
  }
  
  saveStations();
  Serial.printf("[WEB] Deleted station %d, now %d stations\\n", index, stationCount);
}

// ==================== AUDIO CALLBACKS ====================
void audio_info(const char *info) {
  Serial.printf("[AUDIO] %s\\n", info);
}

void audio_id3data(const char *info) {
  Serial.printf("[ID3] %s\\n", info);
}

void audio_eof_mp3(const char *info) {
  Serial.printf("[AUDIO] EOF: %s\\n", info);
  nextStation();
}

void audio_showstation(const char *info) {
  Serial.printf("[STATION] %s\\n", info);
}

void audio_showstreamtitle(const char *info) {
  Serial.printf("[STREAM] %s\\n", info);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\\n*** NetRadio v.1 Starting ***");
  Serial.println("ESP32 Internet Radio with TFT Display");
  Serial.println("=====================================");
  
  // Run diagnostics
  runDiagnostics();
  
  // Load stations from memory
  loadStations();
  
  // Restore last state
  prefs.begin("netradio", true);
  currentStation = prefs.getInt("lastStation", 0);
  currentVolume = prefs.getInt("volume", 12);
  prefs.end();
  
  if (currentStation >= stationCount) currentStation = 0;
  
  // Setup web server
  setupWebServer();
  
  // Start playing
  if (wifiConnected && stationCount > 0) {
    playStation(currentStation);
  }
  
  // Show main screen
  updateDisplay();
  
  Serial.println("\\n*** NetRadio v.1 Ready ***");
  Serial.printf("Web Interface: http://%s\\n", wifiIP.c_str());
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();
  
  // Periodic screen update
  unsigned long now = millis();
  if (now - lastScreenUpdate > SCREEN_UPDATE_MS) {
    lastScreenUpdate = now;
    // Could update dynamic info here (e.g., stream title)
  }
}
`;

export const userSetupConfig = `
// ============================================
// TFT_eSPI User_Setup.h Configuration
// Add these lines to User_Setup.h in the 
// TFT_eSPI library folder
// ============================================

#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST    4
#define TFT_MISO  -1  // Not used

#define TFT_BL   -1   // LED backlight (connect to 3.3V)

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
`;
