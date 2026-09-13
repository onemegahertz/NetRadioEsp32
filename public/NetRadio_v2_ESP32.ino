/*
 * NetRadio v.2 - OPTIMIZED for ESP32 2MB Flash
 * LCD 1602 I2C with LiquidCrystal_I2C library
 * Weather: OpenWeatherMap Moscow
 * 5 buttons: PREV, NEXT, VOL+, VOL-, MODE
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Audio.h"
#include <time.h>

// Pins
#define LCD_ADDR  0x27
#define I2S_BCLK  26
#define I2S_LRC   25
#define I2S_DOUT  27
#define BTN_PREV  32
#define BTN_NEXT  33
#define BTN_VOL_UP 34
#define BTN_VOL_DOWN 14
#define BTN_MODE  15

#define MAX_STATIONS 20
#define MAX_NAME_LEN 16
#define MAX_URL_LEN 80

// State
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Audio audio;
WebServer server(80);
Preferences prefs;

struct Station { char name[MAX_NAME_LEN]; char url[MAX_URL_LEN]; };
Station stations[MAX_STATIONS];
int stationCount = 0, currentStation = 0, currentVolume = 12;
bool isPlaying = false, wifiConnected = false;
char wifiSSID[24] = "", wifiIP[16] = "";
char curTime[9] = "", curDate[11] = "";
char weatherTemp[8] = "";
int displayMode = 0;
unsigned long lastBtn = 0, lastScr = 0, lastTime = 0, lastWeather = 0;
int scrollPos = 0;

// Forward declarations
void playStation(int);
void nextStation();
void prevStation();
void volumeUp();
void volumeDown();
void changeMode();
void updateDisplay();
void loadStations();
void saveStations();
void deleteStation(int);
void updateTime();
void updateWeather();
void setupWebServer();

// Stations
void loadDefaults() {
  const char* n[] = {"Record","RusMix","90s","Chill","Deep","Rock","Rap","Techno","House","EDM","TM","Pirate","Dub","Synth","LoFi","Euro","Trap","Hard","Amb","RusHits"};
  const char* u[] = {"rr_320","rusmix_320","sd90_320","chil_320","deep_320","rock_320","rap_320","techno320","house_320","edm_320","tm_320","ps_320","dub_320","synth_320","lofi_320","eurod_320","trap_320","hardst_320","ambient_320","rushits_320"};
  stationCount = 20;
  for (int i = 0; i < 20; i++) {
    strncpy(stations[i].name, n[i], MAX_NAME_LEN-1);
    snprintf(stations[i].url, MAX_URL_LEN, "https://radiorecord.hostingradio.ru/%s", u[i]);
  }
  saveStations();
}

void loadStations() {
  prefs.begin("stations", true);
  stationCount = prefs.getInt("count", 0);
  if (stationCount == 0) { prefs.end(); loadDefaults(); return; }
  for (int i = 0; i < stationCount; i++) {
    char k[8]; snprintf(k, 8, "s%d", i);
    char d[100] = ""; prefs.getString(k, d, 100);
    char* s = strchr(d, '|');
    if (s) { *s = 0; strncpy(stations[i].name, d, MAX_NAME_LEN-1); strncpy(stations[i].url, s+1, MAX_URL_LEN-1); }
  }
  prefs.end();
}

void saveStations() {
  prefs.begin("stations", false);
  prefs.putInt("count", stationCount);
  for (int i = 0; i < stationCount; i++) {
    char k[8]; snprintf(k, 8, "s%d", i);
    char d[100]; snprintf(d, 100, "%s|%s", stations[i].name, stations[i].url);
    prefs.putString(k, d);
  }
  prefs.end();
}

void deleteStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  for (int i = idx; i < stationCount-1; i++) stations[i] = stations[i+1];
  stationCount--;
  if (currentStation >= stationCount) currentStation = 0;
  saveStations();
}

// Audio
void playStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  currentStation = idx; isPlaying = true;
  audio.connecttohost(stations[idx].url);
  scrollPos = 0;
  updateDisplay();
}

void nextStation() { playStation((currentStation+1) % stationCount); }
void prevStation() { playStation((currentStation-1+stationCount) % stationCount); }
void volumeUp() { currentVolume = min(21, currentVolume+2); audio.setVolume(currentVolume); updateDisplay(); }
void volumeDown() { currentVolume = max(0, currentVolume-2); audio.setVolume(currentVolume); updateDisplay(); }
void changeMode() { displayMode = (displayMode+1) % 4; prefs.begin("netradio", false); prefs.putInt("mode", displayMode); prefs.end(); updateDisplay(); }

// Weather (direct TCP, no HTTPClient)
void updateWeather() {
  if (!wifiConnected) return;
  WiFiClient client;
  if (client.connect("api.openweathermap.org", 80)) {
    client.print("GET /data/2.5/weather?q=Moscow,RU&appid=cf0cd0d160ba580cef69e35dfe3064c8&units=metric&lang=ru HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n");
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

// Time
void updateTime() {
  if (!wifiConnected) return;
  struct tm t;
  if (getLocalTime(&t)) {
    strftime(curTime, 9, "%H:%M:%S", &t);
    strftime(curDate, 11, "%d.%m.%Y", &t);
  }
}

// Display
void updateDisplay() {
  // Line 1: Station (8 chars) + Time (8 chars)
  lcd.setCursor(0, 0);
  const char* name = stations[currentStation].name;
  int len = strlen(name);
  if (len > 8) {
    char buf[9];
    for (int i = 0; i < 8; i++) buf[i] = name[(scrollPos+i) % len];
    buf[8] = 0;
    lcd.print(buf);
    scrollPos = (scrollPos+1) % len;
  } else {
    lcd.print(name);
    for (int i = len; i < 8; i++) lcd.print(' ');
  }
  lcd.print(curTime[0] ? curTime : "--:--:--");
  
  // Line 2: Mode content
  lcd.setCursor(0, 1);
  switch (displayMode) {
    case 0: { char b[17]; snprintf(b, 17, "Vol:%d/21       ", currentVolume); lcd.print(b); break; }
    case 1: { char b[17]; snprintf(b, 17, "Temp:%-10s", weatherTemp[0] ? weatherTemp : "N/A"); lcd.print(b); break; }
    case 2: { char b[17]; snprintf(b, 17, "Date:%-10s", curDate[0] ? curDate : "N/A"); lcd.print(b); break; }
    case 3: { char b[17]; snprintf(b, 17, "IP:%-12s", wifiIP); lcd.print(b); break; }
  }
}

// Buttons
void checkButtons() {
  unsigned long now = millis();
  if (now - lastBtn < 200) return;
  if (digitalRead(BTN_PREV) == LOW) { lastBtn = now; prevStation(); }
  if (digitalRead(BTN_NEXT) == LOW) { lastBtn = now; nextStation(); }
  if (digitalRead(BTN_VOL_UP) == LOW) { lastBtn = now; volumeUp(); }
  if (digitalRead(BTN_VOL_DOWN) == LOW) { lastBtn = now; volumeDown(); }
  if (digitalRead(BTN_MODE) == LOW) { lastBtn = now; changeMode(); }
}

// Web HTML (minified)
const char HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1'><title>NetRadio</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0a0a1a;color:#e0e0e0}.h{background:linear-gradient(135deg,#064e3b,#0f766e);padding:15px;text-align:center}.h h1{color:#34d399}.c{max-width:400px;margin:0 auto;padding:10px}.i{background:#1a1a2e;border-radius:8px;padding:10px;margin-bottom:10px;border:1px solid #333}.r{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid #222}.l{color:#888;font-size:12px}.v{color:#34d399;font-weight:bold;font-size:12px}.g{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}.b{padding:8px;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold;color:#fff}.b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}.b5{background:#9C27B0}.s{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.s h3{color:#34d399;margin-bottom:6px;font-size:13px}.si{display:flex;align-items:center;padding:5px;margin:2px 0;background:#0d0d1a;border-radius:4px;border:1px solid #222}.si.a{border-color:#34d399}.n{width:20px;color:#666;font-size:10px}.m{flex:1;font-size:11px}.a{display:flex;gap:2px}.a button{padding:2px 5px;border:none;border-radius:3px;cursor:pointer;font-size:9px;color:#fff}.p{background:#4CAF50}.e{background:#2196F3}.d{background:#F44336}.f{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.f h3{color:#34d399;margin-bottom:6px;font-size:13px}.fr{margin-bottom:6px}.fr label{display:block;color:#888;font-size:10px;margin-bottom:2px}.fr input{width:100%;padding:6px;border:1px solid #333;border-radius:3px;background:#0d0d1a;color:#e0e0e0;font-size:11px}.sa{width:100%;padding:8px;background:#10b981;color:#fff;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold}</style></head><body><div class=h><h1>NetRadio v.2</h1></div><div class=c><div class=i><div class=r><span class=l>WiFi:</span><span class=v id=w>--</span></div><div class=r><span class=l>IP:</span><span class=v id=ip>--</span></div><div class=r><span class=l>Station:</span><span class=v id=s>--</span></div><div class=r><span class=l>Vol:</span><span class=v id=vl>--</span></div><div class=r><span class=l>Weather:</span><span class=v id=wt>--</span></div><div class=r><span class=l>Mode:</span><span class=v id=md>--</span></div></div><div class=g><button class=b b1 onclick=c('prev')>< Prev</button><button class=b b2 onclick=c('next')>Next ></button><button class=b b3 onclick=c('voldown')>Vol-</button><button class=b b4 onclick=c('volup')>Vol+</button><button class=b b5 onclick=c('mode') colspan=2>Mode</button></div><div class=s><h3>Stations (<span id=sc>0</span>/20)</h3><div id=sl></div></div><div class=f><h3 id=ft>+ Add</h3><div class=fr><label>Name:</label><input id=sn></div><div class=fr><label>URL:</label><input id=su></div><input type=hidden id=ei value=-1><button class=sa onclick=as()>Save</button></div><div class=f><h3>WiFi</h3><div class=fr><label>SSID:</label><input id=wn></div><div class=fr><label>Pass:</label><input type=password id=wp></div><button class=sa onclick=sw()>Save</button></div></div><script>function c(x){fetch('/api/'+x).then(r=>r.json()).then(u)}function ls(){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sc').textContent=d.stations.length;let h='';for(let i=0;i<d.stations.length;i++){let s=d.stations[i],a=i===d.current?'a':'';h+='<div class=si '+a+'><span class=n>'+(i+1)+'</span><span class=m>'+s.name+'</span><div class=a><button class=p onclick=p('+i+')>P</button><button class=e onclick=e('+i+')>E</button><button class=d onclick=dl('+i+')>X</button></div></div>'}document.getElementById('sl').innerHTML=h})}function p(i){c('play/'+i);setTimeout(ls,500)}function e(i){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sn').value=d.stations[i].name;document.getElementById('su').value=d.stations[i].url;document.getElementById('ei').value=i;document.getElementById('ft').textContent='Edit #'+(i+1)})}function dl(i){if(confirm('Delete?'))fetch('/api/delete/'+i).then(()=>ls())}function as(){let n=document.getElementById('sn').value,u=document.getElementById('su').value,x=document.getElementById('ei').value;if(!n||!u)return alert('Fill all!');fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(()=>{ls();document.getElementById('sn').value='';document.getElementById('su').value='';document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add'})}function sw(){let s=document.getElementById('wn').value,p=document.getElementById('wp').value;if(!s)return alert('Enter SSID!');fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,password:p})}).then(()=>alert('Saved!'))}function u(d){document.getElementById('w').textContent=d.ssid||'N/A';document.getElementById('ip').textContent=d.ip||'N/A';document.getElementById('vl').textContent=d.volume+'/21';document.getElementById('s').textContent=d.station||'N/A';document.getElementById('wt').textContent=d.weather||'N/A';let m=['Volume','Temp','Date','WiFi'];document.getElementById('md').textContent=m[d.mode]||'Volume';ls()}fetch('/api/status').then(r=>r.json()).then(u);setInterval(()=>fetch('/api/status').then(r=>r.json()).then(u),3000)</script></body></html>)rawliteral";

// Web server
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
    char j[256];
    snprintf(j, 256, "{\"ssid\":\"%s\",\"ip\":\"%s\",\"volume\":%d,\"current\":%d,\"station\":\"%s\",\"playing\":%s,\"weather\":\"%s\",\"mode\":%d}",
      wifiSSID, wifiIP, currentVolume, currentStation, stations[currentStation].name, isPlaying?"true":"false", weatherTemp, displayMode);
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
  
  for (int i = 0; i < 20; i++) {
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
    saveStations(); sendOK();
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

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("  NetRadio v.2  ");
  lcd.setCursor(0, 1); lcd.print(" Starting...    ");
  Serial.println("[OK] LCD");
  delay(1000);
  
  // Buttons
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_VOL_UP, INPUT_PULLUP);
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  Serial.println("[OK] Buttons");
  
  // I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  Serial.println("[OK] I2S");
  
  // WiFi
  prefs.begin("netradio", true);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("password", "");
  displayMode = prefs.getInt("mode", 0);
  prefs.end();
  
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Connecting WiFi ");
  
  if (ssid.length() == 0) {
    WiFi.softAP("NetRadio", "netradio123");
    strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
  } else {
    WiFi.begin(ssid.c_str(), pass.c_str());
    int a = 0;
    while (WiFi.status() != WL_CONNECTED && a < 20) { delay(500); a++; }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      strncpy(wifiSSID, ssid.c_str(), 23);
      strcpy(wifiIP, WiFi.localIP().toString().c_str());
      configTime(10800, 0, "pool.ntp.org");
      updateWeather();
    } else {
      WiFi.softAP("NetRadio", "netradio123");
      strcpy(wifiIP, WiFi.softAPIP().toString().c_str());
    }
  }
  
  loadStations();
  setupWebServer();
  if (stationCount > 0) playStation(currentStation);
  updateDisplay();
  Serial.println("=== Ready ===");
}

void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();
  unsigned long now = millis();
  if (now - lastScr > 500) { lastScr = now; updateDisplay(); }
  if (now - lastTime > 1000) { lastTime = now; updateTime(); }
  if (now - lastWeather > 1800000) { lastWeather = now; updateWeather(); }
  delay(10);
}
