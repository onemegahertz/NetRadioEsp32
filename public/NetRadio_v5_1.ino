/*
 * NetRadio v5.1 - Bluetooth + Weather + Time
 * Partition Scheme: Huge APP (3MB No OTA) - ОБЯЗАТЕЛЬНО!
 */

// WiFi - ВВЕДИТЕ СВОИ ДАННЫЕ!
#define WIFI_SSID     "YourSSID"
#define WIFI_PASSWORD "YourPassword"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "Audio.h"
#include <BluetoothA2DPSink.h>
#include <time.h>

// Pins
#define TFT_BL    21
#define TOUCH_CS  33
#define TOUCH_IRQ 36
#define I2S_BCLK  27
#define I2S_LRC   26
#define I2S_DOUT  25

// Weather
#define WEATHER_KEY "cf0cd0d160ba580cef69e35dfe3064c8"

// Globals
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
Audio audio;
WebServer server(80);
BluetoothA2DPSink a2dp;

// Stations
const char* stationNames[] = {"Record", "RusMix", "90s", "Chill", "Rock", "Deep", "Techno", "House", "EDM", "Pirate"};
const char* stationURLs[] = {
  "https://radiorecord.hostingradio.ru/rr_320",
  "https://radiorecord.hostingradio.ru/rusmix_320",
  "https://radiorecord.hostingradio.ru/sd90_320",
  "https://radiorecord.hostingradio.ru/chil_320",
  "https://radiorecord.hostingradio.ru/rock_320",
  "https://radiorecord.hostingradio.ru/deep_320",
  "https://radiorecord.hostingradio.ru/techno320",
  "https://radiorecord.hostingradio.ru/house_320",
  "https://radiorecord.hostingradio.ru/edm_320",
  "https://radiorecord.hostingradio.ru/ps_320"
};
int numStations = 10;
int currentStation = 0;
int volume = 12;

// State
bool btEnabled = true;
bool btConnected = false;
char curTime[9] = "";
char curDate[11] = "";
char weatherTemp[10] = "";
char wifiSSID[20] = "";
char wifiIP[16] = "";

unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;
unsigned long lastTouch = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== NetRadio v5.1 ===");
  
  // TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v5.1");
  tft.setTextSize(1);
  tft.setCursor(10, 35);
  tft.println("Starting...");
  Serial.println("[OK] TFT");
  
  // Touch
  touch.begin(SPI);
  touch.setRotation(1);
  Serial.println("[OK] Touch");
  
  // Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume);
  Serial.println("[OK] Audio");
  
  // Bluetooth
  a2dp.set_volume(64);
  a2dp.start("NetRadio");
  Serial.println("[OK] Bluetooth");
  
  // WiFi
  tft.setCursor(10, 50);
  tft.println("Connecting WiFi...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    strncpy(wifiSSID, WIFI_SSID, 19);
    strcpy(wifiIP, WiFi.localIP().toString().c_str());
    configTime(10800, 0, "pool.ntp.org");  // Moscow UTC+3
    Serial.println("\n[OK] WiFi connected!");
    Serial.printf("SSID: %s\n", wifiSSID);
    Serial.printf("IP: %s\n", wifiIP);
    
    // Get weather immediately
    updateWeather();
  } else {
    Serial.println("\n[FAIL] WiFi failed!");
    strcpy(wifiSSID, "Not connected");
    strcpy(wifiIP, "0.0.0.0");
  }
  
  // Web server
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>NetRadio</title>";
    html += "<style>body{font-family:Arial;background:#1a1a2e;color:#e0e0e0;padding:20px}";
    html += "button{padding:10px 20px;margin:5px;font-size:16px;cursor:pointer;border:none;border-radius:5px;color:white}";
    html += ".prev{background:#2196F3}.next{background:#4CAF50}.vold{background:#FF9800}.volu{background:#F44336}</style></head>";
    html += "<body><h1>NetRadio v5.1</h1>";
    html += "<p>Station: <b>" + String(stationNames[currentStation]) + "</b></p>";
    html += "<p>Volume: <b>" + String(volume) + "/21</b></p>";
    html += "<p>WiFi: <b>" + String(wifiSSID) + "</b></p>";
    html += "<p>IP: <b>" + String(wifiIP) + "</b></p>";
    html += "<p>Time: <b>" + String(curTime) + "</b></p>";
    html += "<p>Weather: <b>" + String(weatherTemp) + "</b></p>";
    html += "<button class='prev' onclick=\"fetch('/prev')\">◀ Prev</button>";
    html += "<button class='next' onclick=\"fetch('/next')\">Next ▶</button><br>";
    html += "<button class='vold' onclick=\"fetch('/voldown')\">Vol-</button>";
    html += "<button class='volu' onclick=\"fetch('/volup')\">Vol+</button>";
    html += "<script>setInterval(()=>location.reload(),3000)</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  server.on("/prev", []() { 
    currentStation = (currentStation - 1 + numStations) % numStations;
    audio.connecttohost(stationURLs[currentStation]);
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/next", []() { 
    currentStation = (currentStation + 1) % numStations;
    audio.connecttohost(stationURLs[currentStation]);
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/volup", []() { 
    volume = min(21, volume + 2);
    audio.setVolume(volume);
    a2dp.set_volume(volume * 6);
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/voldown", []() { 
    volume = max(0, volume - 2);
    audio.setVolume(volume);
    a2dp.set_volume(volume * 6);
    server.send(200, "text/plain", "OK");
  });
  
  server.begin();
  Serial.println("[OK] Web server started");
  
  // Play first station
  audio.connecttohost(stationURLs[currentStation]);
  Serial.println("[OK] Playing");
  
  drawScreen();
  Serial.println("=== Ready ===");
  Serial.printf("Web: http://%s\n", wifiIP);
}

void updateWeather() {
  WiFiClient client;
  if (client.connect("api.openweathermap.org", 80)) {
    client.printf("GET /data/2.5/weather?q=Moscow,RU&appid=%s&units=metric&lang=ru HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n", WEATHER_KEY);
    unsigned long timeout = millis();
    while (!client.available() && millis() - timeout < 5000) delay(1);
    
    String response = "";
    while (client.available()) {
      response += client.readStringUntil('\n');
    }
    client.stop();
    
    int tempStart = response.indexOf("\"temp\":") + 7;
    int tempEnd = response.indexOf(",", tempStart);
    if (tempStart > 7 && tempEnd > tempStart) {
      float temp = response.substring(tempStart, tempEnd).toFloat();
      snprintf(weatherTemp, 10, "%.1f°C", temp);
      Serial.printf("[WEATHER] %s\n", weatherTemp);
    }
  }
}

void updateTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(curTime, 9, "%H:%M:%S", &timeinfo);
    strftime(curDate, 11, "%d.%m.%Y", &timeinfo);
  }
}

void drawScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // Header with time
  tft.fillRect(0, 0, 320, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.print("NetRadio v5.1");
  
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(230, 10);
  tft.print(curTime);
  
  // Station name (large)
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 45);
  tft.print(stationNames[currentStation]);
  
  // Station number
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(10, 80);
  tft.printf("Station %d/%d", currentStation + 1, numStations);
  
  // WiFi info
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(10, 100);
  tft.printf("WiFi: %s", wifiSSID);
  
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(10, 115);
  tft.printf("IP: %s", wifiIP);
  
  // Weather
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 130);
  tft.printf("Moscow: %s", weatherTemp);
  
  // Volume
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 155);
  tft.printf("Vol: %d/21", volume);
  
  // Volume bar
  tft.fillRect(10, 180, 300, 15, TFT_DARKGREY);
  tft.fillRect(10, 180, ::map(volume, 0, 21, 0, 300), 15, TFT_GREEN);
  tft.drawRect(10, 180, 300, 15, TFT_WHITE);
  
  // Buttons - BIG for easy touch
  tft.fillRoundRect(5, 205, 75, 30, 5, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(12, 213);
  tft.print("PREV");
  
  tft.fillRoundRect(85, 205, 75, 30, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setCursor(92, 213);
  tft.print("NEXT");
  
  tft.fillRoundRect(165, 205, 75, 30, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(170, 213);
  tft.print("VOL-");
  
  tft.fillRoundRect(245, 205, 75, 30, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(250, 213);
  tft.print("VOL+");
}

void loop() {
  server.handleClient();
  audio.loop();
  
  unsigned long now = millis();
  
  // Update time every second
  if (now - lastTimeUpdate > 1000) {
    lastTimeUpdate = now;
    updateTime();
    drawScreen();
  }
  
  // Update weather every 30 minutes
  if (now - lastWeatherUpdate > 1800000) {
    lastWeatherUpdate = now;
    updateWeather();
  }
  
  // Touch
  if (touch.touched()) {
    if (now - lastTouch < 300) {
      while (touch.touched()) delay(5);
      return;
    }
    lastTouch = now;
    
    TS_Point p = touch.getPoint();
    int x = ::map(p.x, 200, 3700, 0, 320);
    int y = ::map(p.y, 240, 3800, 0, 240);
    
    x = constrain(x, 0, 319);
    y = constrain(y, 0, 239);
    
    Serial.printf("[TOUCH] X=%d Y=%d\n", x, y);
    
    // PREV (x: 5-80, y: 205-235)
    if (x >= 5 && x <= 80 && y >= 205 && y <= 235) {
      Serial.println("[BTN] PREV");
      currentStation = (currentStation - 1 + numStations) % numStations;
      audio.connecttohost(stationURLs[currentStation]);
      drawScreen();
    }
    // NEXT (x: 85-160, y: 205-235)
    else if (x >= 85 && x <= 160 && y >= 205 && y <= 235) {
      Serial.println("[BTN] NEXT");
      currentStation = (currentStation + 1) % numStations;
      audio.connecttohost(stationURLs[currentStation]);
      drawScreen();
    }
    // VOL- (x: 165-240, y: 205-235)
    else if (x >= 165 && x <= 240 && y >= 205 && y <= 235) {
      Serial.println("[BTN] VOL-");
      volume = max(0, volume - 2);
      audio.setVolume(volume);
      a2dp.set_volume(volume * 6);
      drawScreen();
    }
    // VOL+ (x: 245-320, y: 205-235)
    else if (x >= 245 && x <= 320 && y >= 205 && y <= 235) {
      Serial.println("[BTN] VOL+");
      volume = min(21, volume + 2);
      audio.setVolume(volume);
      a2dp.set_volume(volume * 6);
      drawScreen();
    }
    
    while (touch.touched()) delay(5);
  }
  
  delay(10);
}
