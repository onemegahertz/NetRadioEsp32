/*
 * ============================================================
 *  NetRadio v.2 - Internet Radio
 *  LCD 1602 I2C Display
 *  Based on yoRadio project ideas
 *  Universal: NodeMCU (ESP8266) / ESP32
 * ============================================================
 *
 *  Libraries Required:
 *  - LiquidCrystal_I2C by Frank de Brabander
 *  - ESP8266Audio by Earle F. Philhower (ESP8266)
 *  - ESP32-audioI2S by schreibfaul1 (ESP32)
 *  - ArduinoJson by Benoit Blanchon
 *
 *  I2C LCD (address 0x27 or 0x3F):
 *  - SDA -> D2 (GPIO4) on ESP8266, GPIO21 on ESP32
 *  - SCL -> D1 (GPIO5) on ESP8266, GPIO22 on ESP32
 *  - VCC -> 5V, GND -> GND
 *
 *  I2S DAC (MAX98357A):
 *  - BCLK -> GPIO26, LRC -> GPIO25, DIN -> GPIO22
 *  - VCC -> 5V, GND -> GND
 *
 *  Buttons (pin -> button -> GND):
 *  - BTN_PREV -> GPIO14
 *  - BTN_NEXT -> GPIO12
 *  - BTN_VOL  -> GPIO13
 * ============================================================
 */

#include <Arduino.h>

// ==================== PLATFORM DETECTION ====================
#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Preferences.h>
  #include "Audio.h"
  #include <time.h>
  WebServer server(80);
  Audio audio;
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <AudioGeneratorMP3.h>
  #include <AudioOutputI2S.h>
  #include <AudioFileSourceHTTPStream.h>
  #include <AudioFileSourceBuffer.h>
  ESP8266WebServer server(80);
  AudioGeneratorMP3 *mp3 = nullptr;
  AudioOutputI2S *out = nullptr;
  AudioFileSourceHTTPStream *file = nullptr;
  AudioFileSourceBuffer *buff = nullptr;
#endif

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ==================== CONFIGURATION ====================
#define LCD_ADDRESS  0x27
#define LCD_COLS     16
#define LCD_ROWS     2

#define MAX_STATIONS  20
#define MAX_NAME_LEN  20
#define MAX_URL_LEN   96

#define BTN_PREV  14
#define BTN_NEXT  12
#define BTN_VOL   13

#define VOL_STEP    3
#define DEBOUNCE_MS 200

// ==================== GLOBAL OBJECTS ====================
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// ==================== STATE ====================
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
void updateDisplay();
void loadStations();
void saveStations();
void loadDefaultStations();
void deleteStation(int idx);
void updateTime();
void setupWebServer();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== NetRadio v.2 ===");

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  NetRadio v.2  ");
  lcd.setCursor(0, 1);
  lcd.print(" Starting...    ");
  Serial.println("[OK] LCD");
  delay(1000);

  // Buttons
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_VOL, INPUT_PULLUP);
  Serial.println("[OK] Buttons");

  // WiFi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi ");

  #if defined(ESP32)
    Preferences prefs;
    prefs.begin("netradio", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("password", "");
    prefs.end();

    if (ssid.length() == 0) {
      WiFi.softAP("NetRadio", "netradio123");
      strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
    } else {
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
        configTime(10800, 0, "pool.ntp.org");
      } else {
        WiFi.softAP("NetRadio", "netradio123");
        strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
      }
    }
    audio.setPinout(26, 25, 22);
    audio.setVolume(currentVolume);
  #elif defined(ESP8266)
    char ssid[33] = "";
    char pass[65] = "";
    // Read from EEPROM
    for (int i = 0; i < 32; i++) ssid[i] = EEPROM.read(i);
    ssid[32] = 0;
    for (int i = 0; i < 64; i++) pass[i] = EEPROM.read(32 + i);
    pass[64] = 0;

    if (strlen(ssid) == 0 || ssid[0] == 0xFF) {
      WiFi.softAP("NetRadio", "netradio123");
      strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
    } else {
      WiFi.begin(ssid, pass);
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        lcd.setCursor(attempts % 16, 1);
        lcd.print(".");
        attempts++;
      }
      if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        strncpy(wifiSSID, ssid, 32);
        strcpy(wifiIP, WiFi.localIP().toString().c_str());
      } else {
        WiFi.softAP("NetRadio", "netradio123");
        strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
      }
    }
    out = new AudioOutputI2S();
    out->SetGain(0.5);
  #endif

  Serial.print("[OK] WiFi: ");
  Serial.println(wifiIP);

  // Load stations
  loadStations();

  // Web server
  setupWebServer();

  // Start playing
  if (wifiConnected && stationCount > 0) {
    playStation(currentStation);
  }

  updateDisplay();
  Serial.println("=== NetRadio v.2 Ready ===");
}

// ==================== STATION MANAGEMENT ====================
void loadDefaultStations() {
  const char* names[] = {"Record","RusMix","90s","Chill","Deep","Rock","Rap","Techno","House","EDM","TM","Pirate","Dub","Synth","LoFi","Euro","Trap","Hard","Amb","RusHits"};
  const char* urls[] = {"rr_320","rusmix_320","sd90_320","chil_320","deep_320","rock_320","rap_320","techno320","house_320","edm_320","tm_320","ps_320","dub_320","synth_320","lofi_320","eurod_320","trap_320","hardst_320","ambient_320","rushits_320"};

  stationCount = 20;
  for (int i = 0; i < 20; i++) {
    strncpy(stations[i].name, names[i], MAX_NAME_LEN-1);
    snprintf(stations[i].url, MAX_URL_LEN, "https://radiorecord.hostingradio.ru/%s", urls[i]);
  }
  saveStations();
}

void loadStations() {
  #if defined(ESP32)
    Preferences prefs;
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
  #elif defined(ESP8266)
    int count = EEPROM.read(200);
    if (count == 0 || count == 0xFF) {
      loadDefaultStations();
      return;
    }
    stationCount = count;
    for (int i = 0; i < stationCount; i++) {
      int addr = 201 + i * (MAX_NAME_LEN + MAX_URL_LEN);
      for (int j = 0; j < MAX_NAME_LEN; j++)
        stations[i].name[j] = EEPROM.read(addr + j);
      for (int j = 0; j < MAX_URL_LEN; j++)
        stations[i].url[j] = EEPROM.read(addr + MAX_NAME_LEN + j);
    }
  #endif
  Serial.printf("[OK] Loaded %d stations\n", stationCount);
}

void saveStations() {
  #if defined(ESP32)
    Preferences prefs;
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
  #elif defined(ESP8266)
    EEPROM.begin(2000);
    EEPROM.write(200, stationCount);
    for (int i = 0; i < stationCount; i++) {
      int addr = 201 + i * (MAX_NAME_LEN + MAX_URL_LEN);
      for (int j = 0; j < MAX_NAME_LEN; j++)
        EEPROM.write(addr + j, stations[i].name[j]);
      for (int j = 0; j < MAX_URL_LEN; j++)
        EEPROM.write(addr + MAX_NAME_LEN + j, stations[i].url[j]);
    }
    EEPROM.commit();
  #endif
}

void deleteStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  for (int i = idx; i < stationCount-1; i++) stations[i] = stations[i+1];
  stationCount--;
  if (currentStation >= stationCount && stationCount > 0) currentStation = 0;
  saveStations();
}

// ==================== AUDIO ====================
void playStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  currentStation = idx;
  isPlaying = true;

  #if defined(ESP32)
    audio.connecttohost(stations[idx].url);
  #elif defined(ESP8266)
    if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
    if (file) { delete file; file = nullptr; }
    if (buff) { delete buff; buff = nullptr; }
    file = new AudioFileSourceHTTPStream(stations[idx].url);
    buff = new AudioFileSourceBuffer(file, 2048);
    mp3 = new AudioGeneratorMP3();
    mp3->begin(buff, out);
  #endif

  Serial.printf("[PLAY] %s\n", stations[idx].name);
  scrollPos = 0;
  updateDisplay();
}

void nextStation() { playStation((currentStation+1) % stationCount); }
void prevStation() { playStation((currentStation-1+stationCount) % stationCount); }

void volumeUp() {
  #if defined(ESP32)
    currentVolume = min(21, currentVolume+VOL_STEP);
    audio.setVolume(currentVolume);
  #elif defined(ESP8266)
    currentVolume = min(21, currentVolume+VOL_STEP);
    if (out) out->SetGain(currentVolume / 21.0);
  #endif
  updateDisplay();
}

void volumeDown() {
  #if defined(ESP32)
    currentVolume = max(0, currentVolume-VOL_STEP);
    audio.setVolume(currentVolume);
  #elif defined(ESP8266)
    currentVolume = max(0, currentVolume-VOL_STEP);
    if (out) out->SetGain(currentVolume / 21.0);
  #endif
  updateDisplay();
}

// ==================== DISPLAY ====================
void updateDisplay() {
  lcd.clear();

  // Line 1: Station name (scrolling)
  lcd.setCursor(0, 0);
  String name = String(stations[currentStation].name);
  if (name.length() > 16) {
    String scrollText = name + "   ";
    int len = scrollText.length();
    for (int i = 0; i < 16; i++) {
      lcd.print(scrollText[(scrollPos + i) % len]);
    }
    scrollPos = (scrollPos + 1) % len;
  } else {
    lcd.print(name);
    // Pad with spaces
    for (int i = name.length(); i < 16; i++) lcd.print(' ');
  }

  // Line 2: Volume + Time/IP
  lcd.setCursor(0, 1);
  lcd.print("V:");
  lcd.print(currentVolume);
  lcd.print(" ");
  if (strlen(currentTime) > 0) {
    lcd.print(currentTime);
  } else {
    lcd.print(wifiIP);
  }
}

void updateTime() {
  #if defined(ESP32)
    if (!wifiConnected) return;
    struct tm t;
    if (getLocalTime(&t)) {
      strftime(currentTime, 9, "%H:%M:%S", &t);
    }
  #endif
}

// ==================== BUTTONS ====================
void checkButtons() {
  unsigned long now = millis();
  if (now - lastBtnCheck < DEBOUNCE_MS) return;

  if (digitalRead(BTN_PREV) == LOW) {
    lastBtnCheck = now;
    prevStation();
  }
  if (digitalRead(BTN_NEXT) == LOW) {
    lastBtnCheck = now;
    nextStation();
  }
  if (digitalRead(BTN_VOL) == LOW) {
    lastBtnCheck = now;
    volumeUp();
  }
}

// ==================== WEB HTML ====================
const char HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1'><title>NetRadio v.2</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0a0a1a;color:#e0e0e0}.h{background:linear-gradient(135deg,#064e3b,#0f766e);padding:15px;text-align:center;border-bottom:2px solid #10b981}.h h1{color:#34d399;font-size:20px}.h p{color:#888;font-size:11px}.c{max-width:400px;margin:0 auto;padding:10px}.i{background:#1a1a2e;border-radius:8px;padding:10px;margin-bottom:10px;border:1px solid #333}.r{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid #222}.l{color:#888;font-size:12px}.v{color:#34d399;font-weight:bold;font-size:12px}.g{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}.b{padding:8px;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold;color:#fff}.b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}.s{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.s h3{color:#34d399;margin-bottom:6px;font-size:13px}.si{display:flex;align-items:center;padding:5px;margin:2px 0;background:#0d0d1a;border-radius:4px;border:1px solid #222}.si.a{border-color:#34d399;background:#0a2a1e}.n{width:20px;color:#666;font-size:10px}.m{flex:1;font-size:11px}.a{display:flex;gap:2px}.a button{padding:2px 5px;border:none;border-radius:3px;cursor:pointer;font-size:9px;color:#fff}.p{background:#4CAF50}.e{background:#2196F3}.d{background:#F44336}.f{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.f h3{color:#34d399;margin-bottom:6px;font-size:13px}.fr{margin-bottom:6px}.fr label{display:block;color:#888;font-size:10px;margin-bottom:2px}.fr input{width:100%;padding:6px;border:1px solid #333;border-radius:3px;background:#0d0d1a;color:#e0e0e0;font-size:11px}.sa{width:100%;padding:8px;background:#10b981;color:#fff;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold}</style></head><body><div class=h><h1>NetRadio v.2</h1><p>LCD 1602 I2C Radio</p></div><div class=c><div class=i><div class=r><span class=l>WiFi:</span><span class=v id=w>--</span></div><div class=r><span class=l>IP:</span><span class=v id=ip>--</span></div><div class=r><span class=l>Station:</span><span class=v id=s>--</span></div><div class=r><span class=l>Status:</span><span class=v id=t>--</span></div></div><div style=text-align:center;font-size:14px;color:#34d399;margin:6px 0>Vol: <span id=vl>0</span>/21</div><div class=g><button class=b b1 onclick=c('prev')>< Prev</button><button class=b b2 onclick=c('next')>Next ></button><button class=b b3 onclick=c('voldown')>Vol-</button><button class=b b4 onclick=c('volup')>Vol+</button></div><div class=s><h3>Stations (<span id=sc>0</span>/20)</h3><div id=sl></div></div><div class=f><h3 id=ft>+ Add Station</h3><div class=fr><label>Name:</label><input id=sn></div><div class=fr><label>URL:</label><input id=su></div><input type=hidden id=ei value=-1><button class=sa onclick=as()>Save</button></div><div class=f><h3>WiFi Settings</h3><div class=fr><label>SSID:</label><input id=wn></div><div class=fr><label>Password:</label><input type=password id=wp></div><button class=sa onclick=sw()>Save WiFi</button></div></div><script>function c(x){fetch('/api/'+x).then(r=>r.json()).then(u)}function ls(){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sc').textContent=d.stations.length;let h='';for(let i=0;i<d.stations.length;i++){let s=d.stations[i],a=i===d.current?'a':'';h+='<div class=si '+a+'><span class=n>'+(i+1)+'</span><span class=m>'+s.name+'</span><div class=a><button class=p onclick=p('+i+')>P</button><button class=e onclick=e('+i+')>E</button><button class=d onclick=dl('+i+')>X</button></div></div>'}document.getElementById('sl').innerHTML=h})}function p(i){c('play/'+i);setTimeout(ls,500)}function e(i){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sn').value=d.stations[i].name;document.getElementById('su').value=d.stations[i].url;document.getElementById('ei').value=i;document.getElementById('ft').textContent='Edit #'+(i+1)})}function dl(i){if(confirm('Delete?'))fetch('/api/delete/'+i).then(()=>ls())}function as(){let n=document.getElementById('sn').value,u=document.getElementById('su').value,x=document.getElementById('ei').value;if(!n||!u)return alert('Fill all!');fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(()=>{ls();document.getElementById('sn').value='';document.getElementById('su').value='';document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add Station'})}function sw(){let s=document.getElementById('wn').value,p=document.getElementById('wp').value;if(!s)return alert('Enter SSID!');fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,password:p})}).then(()=>alert('Saved! Rebooting...'))}function u(d){document.getElementById('w').textContent=d.ssid||'N/A';document.getElementById('ip').textContent=d.ip||'N/A';document.getElementById('vl').textContent=d.volume||0;document.getElementById('s').textContent=d.station||'N/A';let t=document.getElementById('t');t.textContent=d.playing?'PLAYING':'STOPPED';t.style.color=d.playing?'#4CAF50':'#F44336';ls()}fetch('/api/status').then(r=>r.json()).then(u);setInterval(()=>fetch('/api/status').then(r=>r.json()).then(u),3000)</script></body></html>)rawliteral";

// ==================== WEB SERVER ====================
void sendJsonOK() {
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handlePlay(int idx) { playStation(idx); sendJsonOK(); }
void handleDelete(int idx) { deleteStation(idx); sendJsonOK(); }

void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String html;
    char c;
    for (int i = 0; (c = pgm_read_byte(&HTML[i])); i++) html += c;
    server.send(200, "text/html", html);
  });

  server.on("/api/status", HTTP_GET, []() {
    char json[256];
    snprintf(json, 256, "{\"ssid\":\"%s\",\"ip\":\"%s\",\"volume\":%d,\"current\":%d,\"station\":\"%s\",\"playing\":%s}",
      wifiSSID, wifiIP, currentVolume, currentStation, stations[currentStation].name, isPlaying ? "true" : "false");
    server.send(200, "application/json", json);
  });

  server.on("/api/stations", HTTP_GET, []() {
    String json = "{\"current\":" + String(currentStation) + ",\"stations\":[";
    for (int i = 0; i < stationCount; i++) {
      json += "{\"name\":\"" + String(stations[i].name) + "\",\"url\":\"" + String(stations[i].url) + "\"}";
      if (i < stationCount-1) json += ",";
    }
    json += "]}";
    server.send(200, "application/json", json);
  });

  server.on("/api/next", HTTP_GET, []() { nextStation(); sendJsonOK(); });
  server.on("/api/prev", HTTP_GET, []() { prevStation(); sendJsonOK(); });
  server.on("/api/volup", HTTP_GET, []() { volumeUp(); sendJsonOK(); });
  server.on("/api/voldown", HTTP_GET, []() { volumeDown(); sendJsonOK(); });

  for (int i = 0; i < 20; i++) {
    server.on(("/api/play/"+String(i)).c_str(), HTTP_GET, [i]() { handlePlay(i); });
    server.on(("/api/delete/"+String(i)).c_str(), HTTP_GET, [i]() { handleDelete(i); });
  }

  server.on("/api/station", HTTP_POST, []() {
    if (!server.hasArg("plain")) { server.send(400, "application/json", "{\"error\":\"No data\"}"); return; }
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
    } else { server.send(400, "application/json", "{\"error\":\"Full\"}"); return; }
    saveStations();
    sendJsonOK();
  });

  server.on("/api/wifi", HTTP_POST, []() {
    String body = server.arg("plain");
    int ss = body.indexOf("\"ssid\":\"") + 8;
    int se = body.indexOf("\"", ss);
    String ssid = body.substring(ss, se);
    int ps = body.indexOf("\"password\":\"") + 12;
    int pe = body.indexOf("\"", ps);
    String pass = body.substring(ps, pe);

    #if defined(ESP32)
      Preferences prefs;
      prefs.begin("netradio", false);
      prefs.putString("ssid", ssid);
      prefs.putString("password", pass);
      prefs.end();
    #elif defined(ESP8266)
      EEPROM.begin(200);
      for (int i = 0; i < 32 && i < ssid.length(); i++) EEPROM.write(i, ssid[i]);
      for (int i = 0; i < 64 && i < pass.length(); i++) EEPROM.write(32 + i, pass[i]);
      EEPROM.commit();
    #endif

    sendJsonOK();
    delay(1000);
    ESP.restart();
  });

  server.begin();
  Serial.println("[OK] Web server started");
}

// ==================== AUDIO CALLBACKS (ESP32) ====================
#if defined(ESP32)
void audio_info(const char *info) { Serial.printf("[AUDIO] %s\n", info); }
void audio_eof_mp3(const char *info) { nextStation(); }
#endif

// ==================== LOOP ====================
void loop() {
  server.handleClient();

  #if defined(ESP32)
    audio.loop();
  #elif defined(ESP8266)
    if (mp3 && mp3->isRunning()) {
      if (!mp3->loop()) mp3->stop();
    }
  #endif

  checkButtons();

  unsigned long now = millis();
  if (now - lastScreenUpdate > 500) {
    lastScreenUpdate = now;
    updateDisplay();
  }
  if (now - lastTimeUpdate > 1000) {
    lastTimeUpdate = now;
    updateTime();
  }

  delay(10);
}
