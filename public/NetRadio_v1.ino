/*
 * NetRadio v.1 - ESP32 Internet Radio
 * Optimized for 2MB Flash ESP32
 * 
 * Libraries:
 * - TFT_eSPI by Bodmer
 * - ESP32-audioI2S by schreibfaul1
 * - ArduinoJson by Benoit Blanchon
 * 
 * TFT_eSPI User_Setup.h:
 * #define ILI9341_DRIVER
 * #define TFT_CS  5
 * #define TFT_DC  4
 * #define TFT_RST 15
 * #define SPI_FREQUENCY 40000000
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

// ==================== PINS ====================
#define TFT_CS     5
#define TFT_DC     4
#define TFT_RST   15
#define I2S_BCLK  26
#define I2S_LRC   25
#define I2S_DOUT  22
#define BTN_NEXT  32
#define BTN_PREV  33
#define BTN_VOL_UP 34
#define BTN_VOL_DOWN 35

// ==================== CONSTANTS ====================
#define MAX_STATIONS  20
#define MAX_NAME_LEN  24
#define MAX_URL_LEN   96
#define VOL_STEP      3

// ==================== GLOBALS ====================
TFT_eSPI tft = TFT_eSPI();
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
String wifiSSID = "";
String wifiIP = "";
String currentTime = "";
String currentDate = "";
String weatherTemp = "";
String weatherDesc = "";

unsigned long lastBtnCheck = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;

// ==================== FORWARD DECLARATIONS ====================
void playStation(int idx);
void nextStation();
void prevStation();
void volumeUp();
void volumeDown();
void updateDisplay();
void loadStations();
void saveStations();
void deleteStation(int idx);
void updateTime();
void updateWeather();
void sendJsonOK();

// ==================== DIAGNOSTICS ====================
void runDiagnostics() {
  Serial.println("\n=== NetRadio v.1 ===");
  
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v.1");
  Serial.println("[OK] TFT");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  Serial.println("[OK] I2S");

  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_VOL_UP, INPUT_PULLUP);
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  Serial.println("[OK] Buttons");

  prefs.begin("netradio", false);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("password", "");

  if (ssid.length() == 0) {
    WiFi.softAP("NetRadio", "netradio123");
    wifiIP = WiFi.softAPIP().toString();
  } else {
    WiFi.begin(ssid.c_str(), pass.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      wifiSSID = ssid;
      wifiIP = WiFi.localIP().toString();
      configTime(10800, 0, "pool.ntp.org");
    } else {
      WiFi.softAP("NetRadio", "netradio123");
      wifiIP = WiFi.softAPIP().toString();
    }
  }
  Serial.printf("[OK] WiFi: %s\n", wifiIP.c_str());
  delay(2000);
}

// ==================== STATIONS ====================
void loadStations() {
  prefs.begin("stations", true);
  stationCount = prefs.getInt("count", 0);
  
  if (stationCount == 0) {
    prefs.end();
    const char* names[] = {"Record","RusMix","90s","Chill","Deep","Rock","Rap","Techno","House","EDM","TM","Pirate","Dub","Synth","LoFi","Euro","Trap","Hard","Amb","RusHits"};
    const char* urls[] = {"rr_320","rusmix_320","sd90_320","chil_320","deep_320","rock_320","rap_320","techno320","house_320","edm_320","tm_320","ps_320","dub_320","synth_320","lofi_320","eurod_320","trap_320","hardst_320","ambient_320","rushits_320"};
    stationCount = 20;
    for (int i = 0; i < 20; i++) {
      strncpy(stations[i].name, names[i], MAX_NAME_LEN-1);
      String u = "https://radiorecord.hostingradio.ru/" + String(urls[i]);
      u.toCharArray(stations[i].url, MAX_URL_LEN);
    }
    saveStations();
  } else {
    for (int i = 0; i < stationCount; i++) {
      String data = prefs.getString(("s"+String(i)).c_str(), "");
      int sep = data.indexOf('|');
      if (sep > 0) {
        data.substring(0, sep).toCharArray(stations[i].name, MAX_NAME_LEN);
        data.substring(sep+1).toCharArray(stations[i].url, MAX_URL_LEN);
      }
    }
    prefs.end();
  }
}

void saveStations() {
  prefs.begin("stations", false);
  prefs.putInt("count", stationCount);
  for (int i = 0; i < stationCount; i++) {
    prefs.putString(("s"+String(i)).c_str(), String(stations[i].name)+"|"+String(stations[i].url));
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

// ==================== AUDIO ====================
void playStation(int idx) {
  if (idx < 0 || idx >= stationCount) return;
  currentStation = idx;
  isPlaying = true;
  audio.connecttohost(stations[idx].url);
  prefs.begin("netradio", false);
  prefs.putInt("lastStation", idx);
  prefs.putInt("volume", currentVolume);
  prefs.end();
  updateDisplay();
}

void nextStation() { playStation((currentStation+1) % stationCount); }
void prevStation() { playStation((currentStation-1+stationCount) % stationCount); }

void volumeUp() {
  currentVolume = min(21, currentVolume+VOL_STEP);
  audio.setVolume(currentVolume);
  updateDisplay();
}

void volumeDown() {
  currentVolume = max(0, currentVolume-VOL_STEP);
  audio.setVolume(currentVolume);
  updateDisplay();
}

// ==================== TIME & WEATHER ====================
void updateTime() {
  if (!wifiConnected) return;
  struct tm t;
  if (getLocalTime(&t)) {
    char buf[9];
    strftime(buf, 9, "%H:%M:%S", &t);
    currentTime = String(buf);
    strftime(buf, 9, "%d.%m.%Y", &t);
    currentDate = String(buf);
  }
}

void updateWeather() {
  if (!wifiConnected) return;
  HTTPClient http;
  http.begin("http://api.openweathermap.org/data/2.5/weather?q=Moscow,RU&appid=YOUR_KEY&units=metric&lang=ru");
  if (http.GET() == 200) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      weatherTemp = String(doc["main"]["temp"].as<float>(), 1) + "C";
      weatherDesc = doc["weather"][0]["description"].as<String>();
    }
  }
  http.end();
}

// ==================== DISPLAY ====================
void updateDisplay() {
  tft.fillScreen(TFT_BLACK);
  
  // Top bar
  tft.fillRect(0, 0, 240, 25, 0x2222);
  tft.setTextColor(TFT_CYAN, 0x2222);
  tft.setTextSize(1);
  tft.setCursor(5, 3);
  tft.print("NetRadio v.1");
  tft.setTextColor(TFT_WHITE, 0x2222);
  tft.setTextSize(2);
  tft.setCursor(5, 12);
  tft.print(currentTime.length() > 0 ? currentTime : "--:--:--");
  tft.setTextSize(1);
  tft.setTextColor(0x8888, 0x2222);
  tft.setCursor(150, 15);
  tft.print(currentDate.length() > 0 ? currentDate : "--.--.----");

  // Weather
  tft.fillRect(0, 30, 240, 35, 0x0A0A2E);
  tft.setTextColor(TFT_YELLOW, 0x0A0A2E);
  tft.setCursor(5, 33);
  tft.print("Moscow:");
  tft.setTextColor(TFT_WHITE, 0x0A0A2E);
  tft.setTextSize(2);
  tft.setCursor(70, 33);
  tft.print(weatherTemp.length() > 0 ? weatherTemp : "N/A");
  tft.setTextSize(1);
  tft.setTextColor(0x8888, 0x0A0A2E);
  tft.setCursor(5, 48);
  String d = weatherDesc.length() > 0 ? weatherDesc : "No data";
  tft.print(d.substring(0, 25));

  // Station
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 75);
  String n = String(stations[currentStation].name);
  tft.print(n.substring(0, 14));
  tft.setTextSize(1);
  tft.setTextColor(0x8888);
  tft.setCursor(10, 95);
  tft.printf("Station %d/%d", currentStation+1, stationCount);

  // Volume
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 115);
  tft.print("Volume:");
  tft.fillRect(10, 130, 220, 15, 0x3333);
  tft.fillRect(10, 130, map(currentVolume, 0, 21, 0, 220), 15, TFT_GREEN);
  tft.drawRect(10, 130, 220, 15, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 150);
  tft.printf("%d/21", currentVolume);

  // WiFi
  tft.setTextColor(0x8888);
  tft.setCursor(10, 170);
  tft.print(wifiConnected ? "WiFi: "+wifiSSID : "AP: NetRadio");
  tft.setCursor(10, 185);
  tft.print("IP: "+wifiIP);
  
  // Status
  tft.setCursor(10, 205);
  tft.print("BTN: Next/Prev/Vol+/-");
  tft.setTextColor(isPlaying ? TFT_GREEN : TFT_RED);
  tft.setCursor(180, 205);
  tft.print(isPlaying ? "PLAY" : "STOP");
  
  // URL
  tft.setTextColor(0x8888);
  tft.setCursor(10, 225);
  String u = String(stations[currentStation].url);
  tft.print(u.substring(0, 30));
  
  // Web
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(10, 240);
  tft.print("Web: http://"+wifiIP);
}

// ==================== BUTTONS ====================
void checkButtons() {
  unsigned long now = millis();
  if (now - lastBtnCheck < 200) return;
  if (digitalRead(BTN_NEXT) == LOW) { lastBtnCheck = now; nextStation(); }
  if (digitalRead(BTN_PREV) == LOW) { lastBtnCheck = now; prevStation(); }
  if (digitalRead(BTN_VOL_UP) == LOW) { lastBtnCheck = now; volumeUp(); }
  if (digitalRead(BTN_VOL_DOWN) == LOW) { lastBtnCheck = now; volumeDown(); }
}

// ==================== WEB HTML ====================
const char HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1'><title>NetRadio</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0a0a1a;color:#e0e0e0}.h{background:#1a1a3e;padding:15px;text-align:center;border-bottom:2px solid #6c3ecf}.h h1{color:#00e5ff;font-size:20px}.c{max-width:400px;margin:0 auto;padding:10px}.i{background:#1a1a2e;border-radius:8px;padding:10px;margin-bottom:10px;border:1px solid #333}.r{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid #222}.l{color:#888;font-size:12px}.v{color:#00e5ff;font-weight:bold;font-size:12px}.g{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}.b{padding:8px;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold;color:#fff}.b1{background:#2196F3}.b2{background:#4CAF50}.b3{background:#FF9800}.b4{background:#F44336}.s{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.s h3{color:#00e5ff;margin-bottom:6px;font-size:13px}.si{display:flex;align-items:center;padding:5px;margin:2px 0;background:#0d0d1a;border-radius:4px;border:1px solid #222}.si.a{border-color:#00e5ff}.n{width:20px;color:#666;font-size:10px}.m{flex:1;font-size:11px}.a{display:flex;gap:2px}.a button{padding:2px 5px;border:none;border-radius:3px;cursor:pointer;font-size:9px;color:#fff}.p{background:#4CAF50}.e{background:#2196F3}.d{background:#F44336}.f{background:#1a1a2e;border-radius:8px;padding:10px;border:1px solid #333;margin-bottom:10px}.f h3{color:#00e5ff;margin-bottom:6px;font-size:13px}.fr{margin-bottom:6px}.fr label{display:block;color:#888;font-size:10px;margin-bottom:2px}.fr input{width:100%;padding:6px;border:1px solid #333;border-radius:3px;background:#0d0d1a;color:#e0e0e0;font-size:11px}.sa{width:100%;padding:8px;background:#6c3ecf;color:#fff;border:none;border-radius:5px;font-size:12px;cursor:pointer;font-weight:bold}</style></head><body><div class=h><h1>NetRadio v.1</h1></div><div class=c><div class=i><div class=r><span class=l>WiFi:</span><span class=v id=w>--</span></div><div class=r><span class=l>IP:</span><span class=v id=i>--</span></div><div class=r><span class=l>Station:</span><span class=v id=s>--</span></div><div class=r><span class=l>Status:</span><span class=v id=t>--</span></div></div><div style=text-align:center;font-size:14px;color:#00e5ff;margin:6px 0>Vol: <span id=vl>0</span>/21</div><div class=g><button class=b b1 onclick=c('prev')>< Prev</button><button class=b b2 onclick=c('next')>Next ></button><button class=b b3 onclick=c('voldown')>Vol-</button><button class=b b4 onclick=c('volup')>Vol+</button></div><div class=s><h3>Stations (<span id=sc>0</span>/20)</h3><div id=sl></div></div><div class=f><h3 id=ft>+ Add</h3><div class=fr><label>Name:</label><input id=sn></div><div class=fr><label>URL:</label><input id=su></div><input type=hidden id=ei value=-1><button class=sa onclick=as()>Save</button></div><div class=f><h3>WiFi</h3><div class=fr><label>SSID:</label><input id=wn></div><div class=fr><label>Pass:</label><input type=password id=wp></div><button class=sa onclick=sw()>Save</button></div></div><script>function c(x){fetch('/api/'+x).then(r=>r.json()).then(u)}function ls(){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sc').textContent=d.stations.length;let h='';for(let i=0;i<d.stations.length;i++){let s=d.stations[i],a=i===d.current?'a':'';h+='<div class=si '+a+'><span class=n>'+(i+1)+'</span><span class=m>'+s.name+'</span><div class=a><button class=p onclick=p('+i+')>P</button><button class=e onclick=e('+i+')>E</button><button class=d onclick=dl('+i+')>X</button></div></div>'}document.getElementById('sl').innerHTML=h})}function p(i){c('play/'+i);setTimeout(ls,500)}function e(i){fetch('/api/stations').then(r=>r.json()).then(d=>{document.getElementById('sn').value=d.stations[i].name;document.getElementById('su').value=d.stations[i].url;document.getElementById('ei').value=i;document.getElementById('ft').textContent='Edit #'+(i+1)})}function dl(i){if(confirm('Delete?'))fetch('/api/delete/'+i).then(()=>ls())}function as(){let n=document.getElementById('sn').value,u=document.getElementById('su').value,x=document.getElementById('ei').value;if(!n||!u)return alert('Fill all!');fetch('/api/station',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:n,url:u,index:parseInt(x)})}).then(()=>{ls();document.getElementById('sn').value='';document.getElementById('su').value='';document.getElementById('ei').value=-1;document.getElementById('ft').textContent='+ Add'})}function sw(){let s=document.getElementById('wn').value,p=document.getElementById('wp').value;if(!s)return alert('Enter SSID!');fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,password:p})}).then(()=>alert('Saved!'))}function u(d){document.getElementById('w').textContent=d.ssid||'N/A';document.getElementById('i').textContent=d.ip||'N/A';document.getElementById('vl').textContent=d.volume||0;document.getElementById('s').textContent=d.station||'N/A';let t=document.getElementById('t');t.textContent=d.playing?'PLAYING':'STOPPED';t.style.color=d.playing?'#4CAF50':'#F44336';ls()}fetch('/api/status').then(r=>r.json()).then(u);setInterval(()=>fetch('/api/status').then(r=>r.json()).then(u),3000)</script></body></html>)rawliteral";

// ==================== WEB SERVER ====================
void sendJsonOK() {
  JsonDocument doc;
  doc["status"] = "ok";
  String r;
  serializeJson(doc, r);
  server.send(200, "application/json", r);
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
    JsonDocument doc;
    doc["ssid"] = wifiSSID;
    doc["ip"] = wifiIP;
    doc["volume"] = currentVolume;
    doc["current"] = currentStation;
    doc["station"] = String(stations[currentStation].name);
    doc["playing"] = isPlaying;
    String r;
    serializeJson(doc, r);
    server.send(200, "application/json", r);
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
    String r;
    serializeJson(doc, r);
    server.send(200, "application/json", r);
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
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) { server.send(400, "application/json", "{\"error\":\"JSON error\"}"); return; }
    const char* name = doc["name"];
    const char* url = doc["url"];
    int idx = doc["index"];
    if (idx >= 0 && idx < stationCount) {
      strncpy(stations[idx].name, name, MAX_NAME_LEN-1);
      strncpy(stations[idx].url, url, MAX_URL_LEN-1);
    } else if (stationCount < MAX_STATIONS) {
      strncpy(stations[stationCount].name, name, MAX_NAME_LEN-1);
      strncpy(stations[stationCount].url, url, MAX_URL_LEN-1);
      stationCount++;
    } else { server.send(400, "application/json", "{\"error\":\"Full\"}"); return; }
    saveStations();
    sendJsonOK();
  });

  server.on("/api/wifi", HTTP_POST, []() {
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) { server.send(400, "application/json", "{\"error\":\"JSON error\"}"); return; }
    prefs.begin("netradio", false);
    prefs.putString("ssid", doc["ssid"].as<const char*>());
    prefs.putString("password", doc["password"].as<const char*>());
    prefs.end();
    sendJsonOK();
    delay(1000);
    ESP.restart();
  });

  server.begin();
}

// ==================== AUDIO CALLBACKS ====================
void audio_info(const char *info) { Serial.printf("[AUDIO] %s\n", info); }
void audio_eof_mp3(const char *info) { nextStation(); }

// ==================== SETUP & LOOP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
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
    updateTime();
    updateWeather();
  }
  updateDisplay();
}

void loop() {
  server.handleClient();
  audio.loop();
  checkButtons();
  unsigned long now = millis();
  if (now - lastScreenUpdate > 1000) { lastScreenUpdate = now; updateDisplay(); }
  if (now - lastTimeUpdate > 60000) { lastTimeUpdate = now; updateTime(); }
  if (now - lastWeatherUpdate > 1800000) { lastWeatherUpdate = now; updateWeather(); }
}
