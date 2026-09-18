/*
 * NetRadio v.3 - Internet Radio for ESP32-2432S028 (CYD)
 * ============================================================
 * Built-in features:
 * - 2.8" TFT ILI9341 (320x240)
 * - MicroSD card slot
 * - Speaker amplifier (GPIO 26)
 * - RGB LED
 * - LDR light sensor
 * Note: No touchscreen (ESP32-2432S028 without R)
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SD.h>
#include "Audio.h"
#include <time.h>

// ==================== PIN DEFINITIONS ====================
// TFT Display (HSPI) - Built-in
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST   -1
#define TFT_BL    21

// SD Card (VSPI) - Built-in
#define SD_MOSI   23
#define SD_MISO   19
#define SD_SCLK   18
#define SD_CS      5

// I2S DAC - External (connect MAX98357A or PCM5102 to CYD)
// Use free GPIO pins from CN1 connector
#define I2S_BCLK    25  // Was Touch CLK (not used for touch)
#define I2S_LRC     27  // CN1 connector
#define I2S_DOUT    22  // CN1 connector

// Buttons - External (connect to GND)
// Note: GPIO 34-39 are input-only, no internal pull-up
#define BTN_PREV    35  // P3 connector (input only, needs external pull-up)
#define BTN_NEXT    34  // LDR pin (input only, needs external pull-up, disables LDR)
#define BTN_VOL_UP   0  // BOOT button (built-in)

// RGB LED - Built-in
#define LED_RED    4
#define LED_GREEN 16
#define LED_BLUE  17

// LDR - Built-in
#define LDR_PIN   34

// ==================== CONSTANTS ====================
#define MAX_STATIONS   50  // SD card can store many stations
#define MAX_NAME_LEN   32
#define MAX_URL_LEN   128
#define VOL_STEP       2
#define DEBOUNCE_MS  200

// Display modes
#define MODE_VOLUME      0
#define MODE_TEMPERATURE 1
#define MODE_TIME        2
#define MODE_WIFI        3
#define TOTAL_MODES      4

// Weather API
#define WEATHER_API_KEY "cf0cd0d160ba580cef69e35dfe3064c8"
#define WEATHER_CITY "Moscow,RU"

// ==================== GLOBAL OBJECTS ====================
TFT_eSPI tft = TFT_eSPI();
Audio audio;
WebServer server(80);
Preferences prefs;
File stationFile;

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
bool sdCardDetected = false;
char wifiSSID[24] = "";
char wifiIP[16] = "";
char curTime[9] = "";
char curDate[11] = "";
char weatherTemp[8] = "";
int displayMode = 0;

unsigned long lastBtn = 0, lastScr = 0, lastTime = 0, lastWeather = 0;
int scrollPos = 0;

// ==================== FORWARD DECLARATIONS ====================
void playStation(int);
void nextStation();
void prevStation();
void volumeUp();
void volumeDown();
void changeMode();
void updateDisplay();
void loadStationsFromSD();
void saveStationsToSD();
void loadDefaultStations();
void deleteStation(int);
void updateTime();
void updateWeather();
void setupWebServer();
void initSDCard();
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void initTouch();
void checkTouch();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== NetRadio v.3 ===");
  Serial.println("ESP32-2432S028 (CYD)");
  
  // Initialize TFT
  // NOTE: Rotation is set in User_Setup.h via #define TFT_setRotation 1
  tft.init();
  tft.fillScreen(TFT_BLACK);
  
  // Initialize touch screen
  initTouch();
  
  // Check if display is working correctly
  Serial.println("[OK] TFT initialized");
  Serial.printf("TFT size: %dx%d\n", tft.width(), tft.height());
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v.3");
  tft.setTextSize(1);
  tft.setCursor(10, 30);
  tft.println("Starting...");
  
  // Enable backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  // Initialize RGB LED (active LOW)
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setLEDColor(0, 0, 255);  // Blue = starting
  Serial.println("[OK] RGB LED");
  
  // Initialize SD Card
  initSDCard();
  
  // Initialize buttons
  pinMode(BTN_PREV, INPUT);  // GPIO 35 - input only, no internal pull-up
  pinMode(BTN_NEXT, INPUT);  // GPIO 34 - input only, no internal pull-up
  pinMode(BTN_VOL_UP, INPUT_PULLUP);  // GPIO 0 - BOOT button
  Serial.println("[OK] Buttons");
  
  // Initialize I2S Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  Serial.println("[OK] Audio");
  
  // WiFi connection
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.println("Connecting WiFi...");
  
  prefs.begin("netradio", true);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("password", "");
  displayMode = prefs.getInt("mode", 0);
  prefs.end();
  
  if (ssid.length() == 0) {
    WiFi.softAP("NetRadio", "netradio123");
    strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
    Serial.println("[OK] WiFi AP Mode");
  } else {
    WiFi.begin(ssid.c_str(), pass.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      delay(1000);
      IPAddress ip = WiFi.localIP();
      if (ip.toString() != "0.0.0.0") {
        wifiConnected = true;
        strncpy(wifiSSID, ssid.c_str(), 23);
        strcpy(wifiIP, ip.toString().c_str());
        configTime(10800, 0, "pool.ntp.org");
        Serial.printf("\n[OK] WiFi: %s\n", wifiIP);
        updateWeather();
      } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("NetRadio", "netradio123");
        strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
      }
    } else {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("NetRadio", "netradio123");
      strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
    }
  }
  
  // Load stations
  loadStationsFromSD();
  
  // Setup web server
  setupWebServer();
  
  // Start playing
  if (stationCount > 0) {
    playStation(currentStation);
  }
  
  setLEDColor(0, 255, 0);  // Green = ready
  updateDisplay();
  
  Serial.println("=== NetRadio v.3 Ready ===");
  Serial.printf("Web: http://%s\n", wifiIP);
}

// ==================== SD CARD ====================
void initSDCard() {
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("[FAIL] SD Card not detected");
    sdCardDetected = false;
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("SD Card: NOT FOUND");
    return;
  }
  
  sdCardDetected = true;
  uint8_t cardType = SD.cardType();
  Serial.println("[OK] SD Card detected");
  
  if (cardType == CARD_MMC) {
    Serial.println("SD Card Type: MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SD Card Type: SD");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SD Card Type: SDHC");
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void loadStationsFromSD() {
  if (!sdCardDetected) {
    loadDefaultStations();
    return;
  }
  
  stationFile = SD.open("/stations.txt", FILE_READ);
  if (!stationFile) {
    Serial.println("[SD] stations.txt not found, loading defaults");
    loadDefaultStations();
    saveStationsToSD();
    return;
  }
  
  stationCount = 0;
  while (stationFile.available() && stationCount < MAX_STATIONS) {
    String line = stationFile.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      int sep = line.indexOf('|');
      if (sep > 0) {
        String name = line.substring(0, sep);
        String url = line.substring(sep + 1);
        name.toCharArray(stations[stationCount].name, MAX_NAME_LEN);
        url.toCharArray(stations[stationCount].url, MAX_URL_LEN);
        stationCount++;
      }
    }
  }
  stationFile.close();
  
  Serial.printf("[SD] Loaded %d stations\n", stationCount);
}

void saveStationsToSD() {
  if (!sdCardDetected) return;
  
  SD.remove("/stations.txt");
  stationFile = SD.open("/stations.txt", FILE_WRITE);
  if (!stationFile) {
    Serial.println("[SD] Failed to create stations.txt");
    return;
  }
  
  for (int i = 0; i < stationCount; i++) {
    stationFile.printf("%s|%s\n", stations[i].name, stations[i].url);
  }
  stationFile.close();
  
  Serial.printf("[SD] Saved %d stations\n", stationCount);
}

void loadDefaultStations() {
  const char* names[] = {"Record","RusMix","90s","Chill","Deep","Rock","Rap","Techno","House","EDM","TM","Pirate","Dub","Synth","LoFi","Euro","Trap","Hard","Amb","RusHits"};
  const char* urls[] = {"rr_320","rusmix_320","sd90_320","chil_320","deep_320","rock_320","rap_320","techno320","house_320","edm_320","tm_320","ps_320","dub_320","synth_320","lofi_320","eurod_320","trap_320","hardst_320","ambient_320","rushits_320"};
  
  stationCount = 20;
  for (int i = 0; i < 20; i++) {
    strncpy(stations[i].name, names[i], MAX_NAME_LEN-1);
    snprintf(stations[i].url, MAX_URL_LEN, "https://radiorecord.hostingradio.ru/%s", urls[i]);
  }
  Serial.println("[OK] Default stations loaded");
}

void deleteStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  for (int i = idx; i < stationCount-1; i++) stations[i] = stations[i+1];
  stationCount--;
  if (currentStation >= stationCount && stationCount > 0) currentStation = 0;
  saveStationsToSD();
}

// ==================== AUDIO ====================
void playStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  currentStation = idx;
  isPlaying = true;
  audio.connecttohost(stations[idx].url);
  scrollPos = 0;
  setLEDColor(0, 255, 0);  // Green = playing
  updateDisplay();
}

void nextStation() { playStation((currentStation+1) % stationCount); }
void prevStation() { playStation((currentStation-1+stationCount) % stationCount); }
void volumeUp() { currentVolume = min(21, currentVolume+VOL_STEP); audio.setVolume(currentVolume); updateDisplay(); }
void volumeDown() { currentVolume = max(0, currentVolume-VOL_STEP); audio.setVolume(currentVolume); updateDisplay(); }
void changeMode() { displayMode = (displayMode+1) % TOTAL_MODES; prefs.begin("netradio", false); prefs.putInt("mode", displayMode); prefs.end(); updateDisplay(); }

// ==================== WEATHER ====================
void updateWeather() {
  if (!wifiConnected) return;
  WiFiClient client;
  if (client.connect("api.openweathermap.org", 80)) {
    client.printf("GET /data/2.5/weather?q=%s&appid=%s&units=metric&lang=ru HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n", WEATHER_CITY, WEATHER_API_KEY);
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

// ==================== TIME ====================
void updateTime() {
  if (!wifiConnected) return;
  struct tm t;
  if (getLocalTime(&t)) {
    strftime(curTime, 9, "%H:%M:%S", &t);
    strftime(curDate, 11, "%d.%m.%Y", &t);
  }
}

// ==================== TOUCH ====================
// Калибровка тачскрина из проекта CYD-ESP32Marauder
uint16_t touchCalData[5] = { 350, 3465, 188, 3431, 2 };

void initTouch() {
  tft.setTouch(touchCalData);
  Serial.println("[OK] Touch initialized");
}

void checkTouch() {
  uint16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Debounce
    delay(50);
    
    // Определяем какая кнопка нажата
    // Кнопка PREV: x=20-80, y=160-200
    if (x >= 20 && x <= 80 && y >= 160 && y <= 200) {
      prevStation();
      Serial.println("[TOUCH] Prev station");
    }
    // Кнопка NEXT: x=100-160, y=160-200
    else if (x >= 100 && x <= 160 && y >= 160 && y <= 200) {
      nextStation();
      Serial.println("[TOUCH] Next station");
    }
    // Кнопка VOL+: x=180-240, y=160-200
    else if (x >= 180 && x <= 240 && y >= 160 && y <= 200) {
      volumeUp();
      Serial.println("[TOUCH] Volume +");
    }
    // Кнопка MODE: x=260-320, y=160-200
    else if (x >= 260 && x <= 320 && y >= 160 && y <= 200) {
      changeMode();
      Serial.println("[TOUCH] Mode change");
    }
    
    // Ждём отпускания пальца
    while (tft.getTouch(&x, &y)) {
      delay(10);
    }
  }
}

// ==================== DISPLAY ====================
void updateDisplay() {
  tft.fillScreen(TFT_BLACK);
  
  // Header
  tft.fillRect(0, 0, 320, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.print("NetRadio v.3");
  
  // Time in header
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(220, 10);
  tft.print(curTime[0] ? curTime : "--:--:--");
  
  // Station name (large)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(10, 40);
  String name = String(stations[currentStation].name);
  if (name.length() > 12) name = name.substring(0, 12);
  tft.print(name);
  
  // Station number
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(10, 70);
  tft.printf("Station %d/%d", currentStation+1, stationCount);
  
  // Mode content
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  
  switch (displayMode) {
    case MODE_VOLUME:
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.printf("Volume: %d/21", currentVolume);
      // Volume bar
      tft.fillRect(10, 130, 300, 20, TFT_DARKGREY);
      tft.fillRect(10, 130, map(currentVolume, 0, 21, 0, 300), 20, TFT_GREEN);
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
  
  // Touch buttons
  // PREV button
  tft.fillRoundRect(20, 160, 60, 40, 5, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(30, 175);
  tft.print("PREV");
  
  // NEXT button
  tft.fillRoundRect(100, 160, 60, 40, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setCursor(110, 175);
  tft.print("NEXT");
  
  // VOL+ button
  tft.fillRoundRect(180, 160, 60, 40, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(190, 175);
  tft.print("VOL+");
  
  // MODE button
  tft.fillRoundRect(260, 160, 60, 40, 5, TFT_PURPLE);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);
  tft.setCursor(270, 175);
  tft.print("MODE");
  
  // Status
  tft.setTextSize(1);
  tft.setCursor(10, 210);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.print("WiFi: ");
  tft.setTextColor(wifiConnected ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.print(wifiConnected ? wifiSSID : "AP Mode");
  
  tft.setCursor(10, 225);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.print("SD Card: ");
  tft.setTextColor(sdCardDetected ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.print(sdCardDetected ? "OK" : "NOT FOUND");
  
  // Web interface hint
  tft.setCursor(10, 240);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.printf("Web: http://%s", wifiIP);
}

// ==================== LED ====================
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  digitalWrite(LED_RED, r == 0 ? HIGH : LOW);
  digitalWrite(LED_GREEN, g == 0 ? HIGH : LOW);
  digitalWrite(LED_BLUE, b == 0 ? HIGH : LOW);
}

// ==================== BUTTONS ====================
void checkButtons() {
  unsigned long now = millis();
  if (now - lastBtn < DEBOUNCE_MS) return;
  
  if (digitalRead(BTN_PREV) == LOW) { lastBtn = now; prevStation(); }
  if (digitalRead(BTN_NEXT) == LOW) { lastBtn = now; nextStation(); }
  if (digitalRead(BTN_VOL_UP) == LOW) { lastBtn = now; volumeUp(); }
  
  // Note: GPIO 34 and 35 are input-only, no internal pull-up
  // Connect external 10k pull-up resistors to 3.3V
}

// ==================== WEB SERVER ====================
const char HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1'><title>NetRadio v.3</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0a0a1a;color:#e0e0e0}.h{background:linear-gradient(135deg,#064e3b,#0f766e);padding:15px;text-align:center}.h h1{color:#34d399}.c{max-width:400px;margin:0 auto;padding:10px}.i{background:#1a1a2e;border-radius:8px;padding:10px;margin-bottom:10px;border:1px solid #333}.r{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid #222}.l{color:#888;font-size:12px}.v{color:#34d399;font-weight:bold;font-size:12px}.g{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}.b{padding:8px;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold;color:#fff}.b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}.b5{background:#9C27B0}.s{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.s h3{color:#34d399;margin-bottom:6px;font-size:13px}.si{display:flex;align-items:center;padding:5px;margin:2px 0;background:#0d0d1a;border-radius:4px;border:1px solid #222}.si.a{border-color:#34d399}.n{width:20px;color:#666;font-size:10px}.m{flex:1;font-size:11px}.a{display:flex;gap:2px}.a button{padding:2px 5px;border:none;border-radius:3px;cursor:pointer;font-size:9px;color:#fff}.p{background:#4CAF50}.e{background:#2196F3}.d{background:#F44336}.f{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.f h3{color:#34d399;margin-bottom:6px;font-size:13px}.fr{margin-bottom:6px}.fr label{display:block;color:#888;font-size:10px;margin-bottom:2px}.fr input{width:100%;padding:6px;border:1px solid #333;border-radius:3px;background:#0d0d1a;color:#e0e0e0;font-size:11px}.sa{width:100%;padding:8px;background:#10b981;color:#fff;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold}</style></head><body><div class=h><h1>NetRadio v.3</h1><p>ESP32-2432S028 (CYD)</p></div><div class=c><div class=i><div class=r><span class=l>WiFi:</span><span class=v id=w>--</span></div><div class=r><span class=l>IP:</span><span class=v id=ip>--</span></div><div class=r><span class=l>Station:</span><span class=v id=s>--</span></div><div class=r><span class=l>Vol:</span><span class=v id=vl>--</span></div><div class=r><span class=l>Weather:</span><span class=v id=wt>--</span></div><div class=r><span class=l>Mode:</span><span class=v id=md>--</span></div><div class=r><span class=l>SD Card:</span><span class=v id=sd>--</span></div></div><div class=g><button class=b b1 onclick=c('prev')>< Prev</button><button class=b b2 onclick=c('next')>Next ></button><button class=b b3 onclick=c('voldown')>Vol-</button><button class=b b4 onclick=c('volup')>Vol+</button><button class=b b5 onclick=c('mode') colspan=2>Mode</button></div><div class=s><h3>Stations (<span id=sc>0</span>/50)</h3><div id=sl></div></div><div class=f><h3 id=ft>+ Add Station</h3><div class=fr><label>Name:</label><input id=sn></div><div class=fr><label>URL:</label><input id=su></div><input type=hidden id=ei value=-1><button class=sa onclick=as()>Save</button></div><div class=f><h3>WiFi Settings</h3><div class=fr><label>SSID:</label><input id=wn></div><div class=fr><label>Password:</label><input type=password id=wp></div><button class=sa onclick=sw()>Save WiFi</button></div></div><script>function c(x){fetch('/api/'+x).then(r=>r.json()).then(u)}function ls(){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sc').textContent=d.stations.length;let h='';for(let i=0;i<d.stations.length;i++){let s=d.stations[i],a=i===d.current?'a':'';h+='<div class=si '+a+'><span class=n>'+(i+1)+'</span><span class=m>'+s.name+'</span><div class=a><button class=p onclick=p('+i+')>P</button><button class=e onclick=e('+i+')>E</button><button class=d onclick=dl('+i+')>X</button></div></div>'}document.getElementById('sl').innerHTML=h})}function p(i){c('play/'+i);setTimeout(ls,500)}function e(i){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sn').value=d.stations[i].name;document.getElementById('su').value=d.stations[i].url;document.getElementById('ei').value=i;document.getElementById('ft').textContent='Edit #'+(i+1)})}function dl(i){if(confirm('Delete?'))fetch('/api/delete/'+i).then(()=>ls())}function as(){let n=document.getElementById('sn').value,u=document.getElementById('su').value,x=document.getElementById('ei').value;if(!n||!u)return alert('Fill all!');fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(()=>{ls();document.getElementById('sn').value='';document.getElementById('su').value='';document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add Station'})}function sw(){let s=document.getElementById('wn').value,p=document.getElementById('wp').value;if(!s)return alert('Enter SSID!');fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,password:p})}).then(()=>alert('Saved!'))}function u(d){document.getElementById('w').textContent=d.ssid||'N/A';document.getElementById('ip').textContent=d.ip||'N/A';document.getElementById('vl').textContent=d.volume+'/21';document.getElementById('s').textContent=d.station||'N/A';document.getElementById('wt').textContent=d.weather||'N/A';document.getElementById('sd').textContent=d.sd?'OK ('+d.stations+' stations)':'NOT FOUND';let m=['Volume','Temp','Date','WiFi'];document.getElementById('md').textContent=m[d.mode]||'Volume';ls()}fetch('/api/status').then(r=>r.json()).then(u);setInterval(()=>fetch('/api/status').then(r=>r.json()).then(u),3000)</script></body></html>)rawliteral";

void sendOK() { server.send(200, "application/json", "{\"status\":\"ok\"}"); }
void handlePlay(int i) { playStation(i); sendOK(); }
void handleDel(int i) { deleteStation(i); sendOK(); }

void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String h; char c;
    for (int i = 0; (c = pgm_read_byte(&HTML[i])); i++) h += c;
    server.send(200, "text/html", h);
  });
  
  server.on("/api/status", HTTP_GET, []() {
    char j[300];
    snprintf(j, 300, "{\"ssid\":\"%s\",\"ip\":\"%s\",\"volume\":%d,\"current\":%d,\"station\":\"%s\",\"playing\":%s,\"weather\":\"%s\",\"mode\":%d,\"sd\":%s,\"stations\":%d}",
      wifiSSID, wifiIP, currentVolume, currentStation, stations[currentStation].name, 
      isPlaying?"true":"false", weatherTemp, displayMode, sdCardDetected?"true":"false", stationCount);
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
  server.on("/api/mode", HTTP_GET, []() { changeMode(); sendOK(); });
  
  for (int i = 0; i < 50; i++) {
    server.on(("/api/play/"+String(i)).c_str(), HTTP_GET, [i]() { handlePlay(i); });
    server.on(("/api/delete/"+String(i)).c_str(), HTTP_GET, [i]() { handleDel(i); });
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
    } else { server.send(400, "application/json", "{\"error\":\"Full\"}"); return; }
    saveStationsToSD(); sendOK();
  });
  
  server.on("/api/wifi", HTTP_POST, []() {
    String b = server.arg("plain");
    int ss = b.indexOf("\"ssid\":\"")+8, se = b.indexOf("\"", ss);
    String ssid = b.substring(ss, se);
    int ps = b.indexOf("\"password\":\"")+12, pe = b.indexOf("\"", ps);
    String pass = b.substring(ps, pe);
    prefs.begin("netradio", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", pass);
    prefs.end();
    sendOK(); delay(1000); ESP.restart();
  });
  
  server.begin();
}

void audio_info(const char *info) { Serial.printf("[AUDIO] %s\n", info); }
void audio_eof_mp3(const char *info) { nextStation(); }

void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();
  checkTouch();  // Check touch screen
  unsigned long now = millis();
  if (now - lastScr > 500) { lastScr = now; updateDisplay(); }
  if (now - lastTime > 1000) { lastTime = now; updateTime(); }
  if (now - lastWeather > 1800000) { lastWeather = now; updateWeather(); }
  delay(10);
}
