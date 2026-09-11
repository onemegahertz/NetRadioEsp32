/*
 * ============================================================
 *  NetRadio v.2 - Internet Radio for ESP32
 *  LCD 1602 I2C Display
 *  With Weather (OpenWeatherMap) & Multi-mode Display
 * ============================================================
 *
 *  Hardware:
 *  - ESP32 DevKit V1
 *  - LCD 1602 with I2C module (address 0x27 or 0x3F)
 *  - I2S DAC: MAX98357A / PCM5102
 *  - 5 buttons: PREV, NEXT, VOL_UP, VOL_DOWN, MODE
 *
 *  Libraries Required:
 *  - LiquidCrystal_I2C by Frank de Brabander
 *  - ESP32-audioI2S by schreibfaul1
 *  - WiFi (built-in)
 *  - WebServer (built-in)
 *  - Preferences (built-in)
 *  - HTTPClient (built-in)
 *
 *  Pin Connections:
 *  
 *  LCD 1602 I2C:
 *  - SDA -> GPIO21
 *  - SCL -> GPIO22
 *  - VCC -> 5V
 *  - GND -> GND
 *
 *  I2S DAC (MAX98357A):
 *  - BCLK -> GPIO26
 *  - LRC  -> GPIO25
 *  - DIN  -> GPIO27
 *  - VCC  -> 5V
 *  - GND  -> GND
 *
 *  Buttons (pin -> button -> GND):
 *  - BTN_PREV     -> GPIO32  (Previous station)
 *  - BTN_NEXT     -> GPIO33  (Next station)
 *  - BTN_VOL_UP   -> GPIO34  (Volume +)
 *  - BTN_VOL_DOWN -> GPIO14  (Volume -)
 *  - BTN_MODE     -> GPIO15  (Change display mode)
 *
 *  Display Modes:
 *  - Mode 1: Station name + Volume
 *  - Mode 2: Station name + Temperature (Moscow)
 *  - Mode 3: Station name + Time
 *  - Mode 4: IP address + WiFi status
 *
 *  Weather API: OpenWeatherMap
 *  City: Moscow, Russia
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Audio.h"
#include <time.h>
#include <HTTPClient.h>

// ==================== PIN DEFINITIONS ====================
#define LCD_ADDRESS  0x27  // Try 0x3F if not working
#define LCD_COLS     16
#define LCD_ROWS     2

#define I2S_BCLK  26
#define I2S_LRC   25
#define I2S_DOUT  27

#define BTN_PREV      32
#define BTN_NEXT      33
#define BTN_VOL_UP    34
#define BTN_VOL_DOWN  14
#define BTN_MODE      15

#define MAX_STATIONS  20
#define MAX_NAME_LEN  20
#define MAX_URL_LEN   96

#define VOL_STEP    2
#define DEBOUNCE_MS 200

// Display modes
#define MODE_VOLUME     0
#define MODE_TEMPERATURE 1
#define MODE_TIME       2
#define MODE_WIFI       3
#define TOTAL_MODES     4

// Weather API
#define WEATHER_API_KEY "cf0cd0d160ba580cef69e35dfe3064c8"
#define WEATHER_CITY "Moscow,RU"

// ==================== GLOBAL OBJECTS ====================
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
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
char wifiSSID[33] = "";
char wifiIP[16] = "";
char currentTime[9] = "";
char currentDate[11] = "";

// Weather
char weatherTemp[10] = "";
char weatherDesc[20] = "";
unsigned long lastWeatherUpdate = 0;
#define WEATHER_UPDATE_INTERVAL 1800000  // 30 minutes

// Display mode
int displayMode = MODE_VOLUME;
bool modeChanged = true;  // Force update on mode change

unsigned long lastBtnCheck = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastTimeUpdate = 0;
int scrollPos = 0;

// ==================== FORWARD DECLARATIONS ====================
void playStation(int idx);
void nextStation();
void prevStation();
void volumeUp();
void volumeDown();
void changeDisplayMode();
void updateDisplay();
void loadStations();
void saveStations();
void loadDefaultStations();
void deleteStation(int idx);
void updateTime();
void updateWeather();
void setupWebServer();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== NetRadio v.2 ===");
  Serial.println("ESP32 + LCD 1602 I2C + Weather");
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  NetRadio v.2  ");
  lcd.setCursor(0, 1);
  lcd.print(" Starting...    ");
  Serial.println("[OK] LCD");
  delay(1000);
  
  // Initialize buttons
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_VOL_UP, INPUT_PULLUP);
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  Serial.println("[OK] Buttons (5 buttons)");
  
  // Initialize I2S Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  Serial.println("[OK] I2S Audio");
  
  // Connect to WiFi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi ");
  
  prefs.begin("netradio", true);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("password", "");
  displayMode = prefs.getInt("displayMode", MODE_VOLUME);
  prefs.end();
  
  if (ssid.length() == 0) {
    // AP mode
    WiFi.softAP("NetRadio", "netradio123");
    strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
    Serial.println("[OK] WiFi AP Mode");
  } else {
    // Station mode
    WiFi.begin(ssid.c_str(), pass.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      lcd.setCursor(attempts % 16, 1);
      lcd.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      strncpy(wifiSSID, ssid.c_str(), 32);
      strcpy(wifiIP, WiFi.localIP().toString().c_str());
      configTime(10800, 0, "pool.ntp.org");  // Moscow UTC+3
      Serial.print("[OK] WiFi: ");
      Serial.println(wifiIP);
      
      // Get initial weather
      updateWeather();
    } else {
      WiFi.softAP("NetRadio", "netradio123");
      strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
      Serial.println("[FAIL] WiFi - AP Mode");
    }
  }
  
  // Load stations
  loadStations();
  
  // Setup web server
  setupWebServer();
  
  // Start playing
  if (stationCount > 0) {
    playStation(currentStation);
  }
  
  updateDisplay();
  Serial.println("=== NetRadio v.2 Ready ===");
  Serial.print("Web Interface: http://");
  Serial.println(wifiIP);
  Serial.println("Display modes: 0=Volume, 1=Temp, 2=Time, 3=WiFi");
}

// ==================== STATION MANAGEMENT ====================
void loadDefaultStations() {
  const char* names[] = {
    "Record", "RusMix", "90s", "Chill", "Deep",
    "Rock", "Rap", "Techno", "House", "EDM",
    "TM", "Pirate", "Dub", "Synth", "LoFi",
    "Euro", "Trap", "Hard", "Amb", "RusHits"
  };
  const char* urls[] = {
    "rr_320", "rusmix_320", "sd90_320", "chil_320", "deep_320",
    "rock_320", "rap_320", "techno320", "house_320", "edm_320",
    "tm_320", "ps_320", "dub_320", "synth_320", "lofi_320",
    "eurod_320", "trap_320", "hardst_320", "ambient_320", "rushits_320"
  };
  
  stationCount = 20;
  for (int i = 0; i < 20; i++) {
    strncpy(stations[i].name, names[i], MAX_NAME_LEN-1);
    snprintf(stations[i].url, MAX_URL_LEN, "https://radiorecord.hostingradio.ru/%s", urls[i]);
  }
  saveStations();
  Serial.println("[OK] Default stations loaded");
}

void loadStations() {
  prefs.begin("stations", true);
  stationCount = prefs.getInt("count", 0);
  
  if (stationCount == 0) {
    prefs.end();
    loadDefaultStations();
    return;
  }
  
  for (int i = 0; i < stationCount; i++) {
    char key[8];
    snprintf(key, 8, "s%d", i);
    char data[128] = "";
    prefs.getString(key, data, 128);
    char* sep = strchr(data, '|');
    if (sep) {
      *sep = 0;
      strncpy(stations[i].name, data, MAX_NAME_LEN-1);
      strncpy(stations[i].url, sep+1, MAX_URL_LEN-1);
    }
  }
  prefs.end();
  
  Serial.printf("[OK] Loaded %d stations\n", stationCount);
}

void saveStations() {
  prefs.begin("stations", false);
  prefs.putInt("count", stationCount);
  
  for (int i = 0; i < stationCount; i++) {
    char key[8];
    snprintf(key, 8, "s%d", i);
    char data[128];
    snprintf(data, 128, "%s|%s", stations[i].name, stations[i].url);
    prefs.putString(key, data);
  }
  prefs.end();
}

void deleteStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  for (int i = idx; i < stationCount-1; i++) {
    stations[i] = stations[i+1];
  }
  stationCount--;
  if (currentStation >= stationCount && stationCount > 0) {
    currentStation = 0;
  }
  saveStations();
}

// ==================== AUDIO CONTROL ====================
void playStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  
  currentStation = idx;
  isPlaying = true;
  audio.connecttohost(stations[idx].url);
  
  Serial.printf("[PLAY] %s\n", stations[idx].name);
  scrollPos = 0;
  modeChanged = true;
  updateDisplay();
}

void nextStation() {
  playStation((currentStation + 1) % stationCount);
}

void prevStation() {
  playStation((currentStation - 1 + stationCount) % stationCount);
}

void volumeUp() {
  currentVolume = min(21, currentVolume + VOL_STEP);
  audio.setVolume(currentVolume);
  modeChanged = true;
  updateDisplay();
  Serial.printf("[VOL+] %d/21\n", currentVolume);
}

void volumeDown() {
  currentVolume = max(0, currentVolume - VOL_STEP);
  audio.setVolume(currentVolume);
  modeChanged = true;
  updateDisplay();
  Serial.printf("[VOL-] %d/21\n", currentVolume);
}

void changeDisplayMode() {
  displayMode = (displayMode + 1) % TOTAL_MODES;
  modeChanged = true;
  
  // Save mode to preferences
  prefs.begin("netradio", false);
  prefs.putInt("displayMode", displayMode);
  prefs.end();
  
  const char* modeNames[] = {"Volume", "Temperature", "Time", "WiFi"};
  Serial.printf("[MODE] %s\n", modeNames[displayMode]);
  
  updateDisplay();
}

// ==================== WEATHER ====================
void updateWeather() {
  if (!wifiConnected) return;
  
  Serial.println("[WEATHER] Updating...");
  
  HTTPClient http;
  char url[200];
  snprintf(url, 200, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric&lang=ru",
           WEATHER_CITY, WEATHER_API_KEY);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    
    // Parse temperature
    int tempStart = payload.indexOf("\"temp\":") + 7;
    int tempEnd = payload.indexOf(",", tempStart);
    String tempStr = payload.substring(tempStart, tempEnd);
    float temp = tempStr.toFloat();
    snprintf(weatherTemp, 10, "%.1fC", temp);
    
    // Parse description
    int descStart = payload.indexOf("\"description\":\"") + 15;
    int descEnd = payload.indexOf("\"", descStart);
    String descStr = payload.substring(descStart, descEnd);
    strncpy(weatherDesc, descStr.c_str(), 19);
    weatherDesc[19] = '\0';
    
    Serial.printf("[WEATHER] %s %s\n", weatherTemp, weatherDesc);
  } else {
    Serial.printf("[WEATHER] HTTP error: %d\n", httpCode);
    strcpy(weatherTemp, "N/A");
    strcpy(weatherDesc, "Error");
  }
  
  http.end();
}

// ==================== DISPLAY ====================
void updateDisplay() {
  // Line 1: Station name (scrolling if needed) + Time in corner
  lcd.setCursor(0, 0);
  String name = String(stations[currentStation].name);
  
  // Reserve 8 characters for time display on the right
  int nameLen = 8;  // Show first 8 chars of name
  
  if (name.length() > nameLen) {
    // Scrolling text (shorter to make room for time)
    String scrollText = name + "   ";
    int len = scrollText.length();
    for (int i = 0; i < nameLen; i++) {
      lcd.print(scrollText[(scrollPos + i) % len]);
    }
    scrollPos = (scrollPos + 1) % len;
  } else {
    lcd.print(name);
    for (int i = name.length(); i < nameLen; i++) {
      lcd.print(' ');
    }
  }
  
  // Time in top right corner (always visible)
  if (strlen(currentTime) > 0) {
    lcd.setCursor(8, 0);
    lcd.print(currentTime);  // HH:MM:SS (8 chars)
  } else {
    lcd.setCursor(8, 0);
    lcd.print("--:--:--");
  }
  
  // Line 2: Different content based on mode
  lcd.setCursor(0, 1);
  
  switch (displayMode) {
    case MODE_VOLUME:
      // Volume display
      lcd.print("Vol:");
      if (currentVolume < 10) lcd.print(" ");
      lcd.print(currentVolume);
      lcd.print("/21     ");
      break;
      
    case MODE_TEMPERATURE:
      // Temperature display
      if (strlen(weatherTemp) > 0) {
        lcd.print("Temp:");
        lcd.print(weatherTemp);
        lcd.print("     ");
      } else {
        lcd.print("Temp:Loading.. ");
      }
      break;
      
    case MODE_TIME:
      // Date display (time already shown on line 1)
      if (strlen(currentDate) > 0) {
        lcd.print("Date:");
        lcd.print(currentDate);
        lcd.print("    ");
      } else {
        lcd.print("Date:--.--.---- ");
      }
      break;
      
    case MODE_WIFI:
      // WiFi status display
      if (wifiConnected) {
        lcd.print("IP:");
        lcd.print(wifiIP);
      } else {
        lcd.print("AP:192.168.4.1");
      }
      break;
  }
}

void updateTime() {
  if (!wifiConnected) return;
  
  struct tm t;
  if (getLocalTime(&t)) {
    strftime(currentTime, 9, "%H:%M:%S", &t);
    strftime(currentDate, 11, "%d.%m.%Y", &t);
  }
}

// ==================== BUTTON HANDLING ====================
void checkButtons() {
  unsigned long now = millis();
  if (now - lastBtnCheck < DEBOUNCE_MS) return;
  
  if (digitalRead(BTN_PREV) == LOW) {
    lastBtnCheck = now;
    prevStation();
    Serial.println("[BTN] Prev");
  }
  
  if (digitalRead(BTN_NEXT) == LOW) {
    lastBtnCheck = now;
    nextStation();
    Serial.println("[BTN] Next");
  }
  
  if (digitalRead(BTN_VOL_UP) == LOW) {
    lastBtnCheck = now;
    volumeUp();
    Serial.println("[BTN] Vol+");
  }
  
  if (digitalRead(BTN_VOL_DOWN) == LOW) {
    lastBtnCheck = now;
    volumeDown();
    Serial.println("[BTN] Vol-");
  }
  
  if (digitalRead(BTN_MODE) == LOW) {
    lastBtnCheck = now;
    changeDisplayMode();
    Serial.println("[BTN] Mode");
  }
}

// ==================== WEB HTML ====================
const char HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1'><title>NetRadio v.2</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0a0a1a;color:#e0e0e0}.h{background:linear-gradient(135deg,#064e3b,#0f766e);padding:15px;text-align:center;border-bottom:2px solid #10b981}.h h1{color:#34d399;font-size:20px}.h p{color:#888;font-size:11px}.c{max-width:400px;margin:0 auto;padding:10px}.i{background:#1a1a2e;border-radius:8px;padding:10px;margin-bottom:10px;border:1px solid #333}.r{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid #222}.l{color:#888;font-size:12px}.v{color:#34d399;font-weight:bold;font-size:12px}.g{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}.b{padding:8px;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold;color:#fff}.b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}.b5{background:#9C27B0}.s{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.s h3{color:#34d399;margin-bottom:6px;font-size:13px}.si{display:flex;align-items:center;padding:5px;margin:2px 0;background:#0d0d1a;border-radius:4px;border:1px solid #222}.si.a{border-color:#34d399;background:#0a2a1e}.n{width:20px;color:#666;font-size:10px}.m{flex:1;font-size:11px}.a{display:flex;gap:2px}.a button{padding:2px 5px;border:none;border-radius:3px;cursor:pointer;font-size:9px;color:#fff}.p{background:#4CAF50}.e{background:#2196F3}.d{background:#F44336}.f{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.f h3{color:#34d399;margin-bottom:6px;font-size:13px}.fr{margin-bottom:6px}.fr label{display:block;color:#888;font-size:10px;margin-bottom:2px}.fr input{width:100%;padding:6px;border:1px solid #333;border-radius:3px;background:#0d0d1a;color:#e0e0e0;font-size:11px}.sa{width:100%;padding:8px;background:#10b981;color:#fff;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold}</style></head><body><div class=h><h1>NetRadio v.2</h1><p>ESP32 + LCD 1602 + Weather</p></div><div class=c><div class=i><div class=r><span class=l>WiFi:</span><span class=v id=w>--</span></div><div class=r><span class=l>IP:</span><span class=v id=ip>--</span></div><div class=r><span class=l>Station:</span><span class=v id=s>--</span></div><div class=r><span class=l>Volume:</span><span class=v id=vl>--</span></div><div class=r><span class=l>Weather:</span><span class=v id=wt>--</span></div><div class=r><span class=l>Mode:</span><span class=v id=md>--</span></div></div><div class=g><button class=b b1 onclick=c('prev')>< Prev</button><button class=b b2 onclick=c('next')>Next ></button><button class=b b3 onclick=c('voldown')>Vol-</button><button class=b b4 onclick=c('volup')>Vol+</button><button class=b b5 onclick=c('mode') colspan=2>Mode</button></div><div class=s><h3>Stations (<span id=sc>0</span>/20)</h3><div id=sl></div></div><div class=f><h3 id=ft>+ Add Station</h3><div class=fr><label>Name:</label><input id=sn></div><div class=fr><label>URL:</label><input id=su></div><input type=hidden id=ei value=-1><button class=sa onclick=as()>Save</button></div><div class=f><h3>WiFi Settings</h3><div class=fr><label>SSID:</label><input id=wn></div><div class=fr><label>Password:</label><input type=password id=wp></div><button class=sa onclick=sw()>Save WiFi</button></div></div><script>function c(x){fetch('/api/'+x).then(r=>r.json()).then(u)}function ls(){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sc').textContent=d.stations.length;let h='';for(let i=0;i<d.stations.length;i++){let s=d.stations[i],a=i===d.current?'a':'';h+='<div class=si '+a+'><span class=n>'+(i+1)+'</span><span class=m>'+s.name+'</span><div class=a><button class=p onclick=p('+i+')>P</button><button class=e onclick=e('+i+')>E</button><button class=d onclick=dl('+i+')>X</button></div></div>'}document.getElementById('sl').innerHTML=h})}function p(i){c('play/'+i);setTimeout(ls,500)}function e(i){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sn').value=d.stations[i].name;document.getElementById('su').value=d.stations[i].url;document.getElementById('ei').value=i;document.getElementById('ft').textContent='Edit #'+(i+1)})}function dl(i){if(confirm('Delete?'))fetch('/api/delete/'+i).then(()=>ls())}function as(){let n=document.getElementById('sn').value,u=document.getElementById('su').value,x=document.getElementById('ei').value;if(!n||!u)return alert('Fill all!');fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(()=>{ls();document.getElementById('sn').value='';document.getElementById('su').value='';document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add Station'})}function sw(){let s=document.getElementById('wn').value,p=document.getElementById('wp').value;if(!s)return alert('Enter SSID!');fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,password:p})}).then(()=>alert('Saved! Rebooting...'))}function u(d){document.getElementById('w').textContent=d.ssid||'N/A';document.getElementById('ip').textContent=d.ip||'N/A';document.getElementById('vl').textContent=d.volume+'/21';document.getElementById('s').textContent=d.station||'N/A';document.getElementById('wt').textContent=d.weather||'N/A';let modes=['Volume','Temperature','Time','WiFi'];document.getElementById('md').textContent=modes[d.mode]||'Volume';ls()}fetch('/api/status').then(r=>r.json()).then(u);setInterval(()=>fetch('/api/status').then(r=>r.json()).then(u),3000)</script></body></html>)rawliteral";

// ==================== WEB SERVER ====================
void sendJsonOK() {
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handlePlay(int idx) {
  playStation(idx);
  sendJsonOK();
}

void handleDelete(int idx) {
  deleteStation(idx);
  sendJsonOK();
}

void setupWebServer() {
  // Serve main page
  server.on("/", HTTP_GET, []() {
    String html;
    char c;
    for (int i = 0; (c = pgm_read_byte(&HTML[i])); i++) {
      html += c;
    }
    server.send(200, "text/html", html);
  });
  
  // API: Status
  server.on("/api/status", HTTP_GET, []() {
    char weather[30];
    snprintf(weather, 30, "%s %s", weatherTemp, weatherDesc);
    
    char json[300];
    snprintf(json, 300, "{\"ssid\":\"%s\",\"ip\":\"%s\",\"volume\":%d,\"current\":%d,\"station\":\"%s\",\"playing\":%s,\"weather\":\"%s\",\"mode\":%d}",
      wifiSSID, wifiIP, currentVolume, currentStation, stations[currentStation].name, 
      isPlaying ? "true" : "false", weather, displayMode);
    server.send(200, "application/json", json);
  });
  
  // API: Stations list
  server.on("/api/stations", HTTP_GET, []() {
    String json = "{\"current\":" + String(currentStation) + ",\"stations\":[";
    for (int i = 0; i < stationCount; i++) {
      json += "{\"name\":\"" + String(stations[i].name) + "\",\"url\":\"" + String(stations[i].url) + "\"}";
      if (i < stationCount - 1) json += ",";
    }
    json += "]}";
    server.send(200, "application/json", json);
  });
  
  // API: Controls
  server.on("/api/next", HTTP_GET, []() { nextStation(); sendJsonOK(); });
  server.on("/api/prev", HTTP_GET, []() { prevStation(); sendJsonOK(); });
  server.on("/api/volup", HTTP_GET, []() { volumeUp(); sendJsonOK(); });
  server.on("/api/voldown", HTTP_GET, []() { volumeDown(); sendJsonOK(); });
  server.on("/api/mode", HTTP_GET, []() { changeDisplayMode(); sendJsonOK(); });
  
  // API: Play stations 0-19
  for (int i = 0; i < 20; i++) {
    server.on(("/api/play/" + String(i)).c_str(), HTTP_GET, [i]() { handlePlay(i); });
    server.on(("/api/delete/" + String(i)).c_str(), HTTP_GET, [i]() { handleDelete(i); });
  }
  
  // API: Add/Edit station
  server.on("/api/station", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"No data\"}");
      return;
    }
    
    String body = server.arg("plain");
    int ns = body.indexOf("\"name\":\"") + 8;
    int ne = body.indexOf("\"", ns);
    String name = body.substring(ns, ne);
    
    int us = body.indexOf("\"url\":\"") + 7;
    int ue = body.indexOf("\"", us);
    String url = body.substring(us, ue);
    
    int is = body.indexOf("\"index\":") + 8;
    int ie = body.indexOf("}", is);
    int idx = body.substring(is, ie).toInt();
    
    if (idx >= 0 && idx < stationCount) {
      strncpy(stations[idx].name, name.c_str(), MAX_NAME_LEN-1);
      strncpy(stations[idx].url, url.c_str(), MAX_URL_LEN-1);
    } else if (stationCount < MAX_STATIONS) {
      strncpy(stations[stationCount].name, name.c_str(), MAX_NAME_LEN-1);
      strncpy(stations[stationCount].url, url.c_str(), MAX_URL_LEN-1);
      stationCount++;
    } else {
      server.send(400, "application/json", "{\"error\":\"Full\"}");
      return;
    }
    
    saveStations();
    sendJsonOK();
  });
  
  // API: Save WiFi
  server.on("/api/wifi", HTTP_POST, []() {
    String body = server.arg("plain");
    int ss = body.indexOf("\"ssid\":\"") + 8;
    int se = body.indexOf("\"", ss);
    String ssid = body.substring(ss, se);
    
    int ps = body.indexOf("\"password\":\"") + 12;
    int pe = body.indexOf("\"", ps);
    String pass = body.substring(ps, pe);
    
    prefs.begin("netradio", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", pass);
    prefs.end();
    
    sendJsonOK();
    delay(1000);
    ESP.restart();
  });
  
  server.begin();
  Serial.println("[OK] Web server started");
}

// ==================== AUDIO CALLBACKS ====================
void audio_info(const char *info) {
  Serial.printf("[AUDIO] %s\n", info);
}

void audio_eof_mp3(const char *info) {
  Serial.printf("[AUDIO] EOF: %s\n", info);
  nextStation();
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();
  
  unsigned long now = millis();
  
  // Update display every 500ms (for scrolling)
  if (now - lastScreenUpdate > 500) {
    lastScreenUpdate = now;
    updateDisplay();
  }
  
  // Update time every second
  if (now - lastTimeUpdate > 1000) {
    lastTimeUpdate = now;
    updateTime();
  }
  
  // Update weather every 30 minutes
  if (now - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL) {
    lastWeatherUpdate = now;
    updateWeather();
  }
  
  delay(10);
}
