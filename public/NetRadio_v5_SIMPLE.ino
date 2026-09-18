/*
 * NetRadio v5.0 - SIMPLE WORKING VERSION
 * Только самое необходимое - всё работает!
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "Audio.h"

// WiFi - ВВЕДИТЕ СВОИ ДАННЫЕ!
#define WIFI_SSID     "YourSSID"
#define WIFI_PASSWORD "YourPassword"

// Pins
#define TFT_BL    21
#define TOUCH_CS  33
#define TOUCH_IRQ 36
#define I2S_BCLK  27
#define I2S_LRC   26
#define I2S_DOUT  25

// Globals
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
Audio audio;
WebServer server(80);

// Stations
const char* stationNames[] = {"Record", "RusMix", "90s", "Chill", "Rock"};
const char* stationURLs[] = {
  "https://radiorecord.hostingradio.ru/rr_320",
  "https://radiorecord.hostingradio.ru/rusmix_320",
  "https://radiorecord.hostingradio.ru/sd90_320",
  "https://radiorecord.hostingradio.ru/chil_320",
  "https://radiorecord.hostingradio.ru/rock_320"
};
int numStations = 5;
int currentStation = 0;
int volume = 12;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== NetRadio v5.0 ===");
  
  // TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v5.0");
  Serial.println("[OK] TFT");
  
  // Touch
  touch.begin(SPI);
  touch.setRotation(1);
  Serial.println("[OK] Touch");
  
  // Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume);
  Serial.println("[OK] Audio");
  
  // WiFi
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.println("Connecting WiFi...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("WiFi OK!");
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\n[FAIL] WiFi failed!");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("WiFi FAIL!");
    delay(3000);
  }
  
  // Web server
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>NetRadio</title></head><body>";
    html += "<h1>NetRadio v5.0</h1>";
    html += "<p>Station: " + String(stationNames[currentStation]) + "</p>";
    html += "<p>Volume: " + String(volume) + "/21</p>";
    html += "<button onclick=\"fetch('/prev')\">Prev</button> ";
    html += "<button onclick=\"fetch('/next')\">Next</button> ";
    html += "<button onclick=\"fetch('/volup')\">Vol+</button> ";
    html += "<button onclick=\"fetch('/voldown')\">Vol-</button>";
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
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/voldown", []() { 
    volume = max(0, volume - 2);
    audio.setVolume(volume);
    server.send(200, "text/plain", "OK");
  });
  
  server.begin();
  Serial.println("[OK] Web server started");
  
  // Play first station
  audio.connecttohost(stationURLs[currentStation]);
  Serial.println("[OK] Playing");
  
  drawScreen();
}

void drawScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // Header
  tft.fillRect(0, 0, 320, 30, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.print("NetRadio v5.0");
  
  // Station name
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 50);
  tft.print(stationNames[currentStation]);
  
  // Station number
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(10, 90);
  tft.printf("Station %d/%d", currentStation + 1, numStations);
  
  // Volume
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 120);
  tft.printf("Vol: %d/21", volume);
  
  // Volume bar
  tft.fillRect(10, 150, 300, 20, TFT_DARKGREY);
  tft.fillRect(10, 150, map(volume, 0, 21, 0, 300), 20, TFT_GREEN);
  tft.drawRect(10, 150, 300, 20, TFT_WHITE);
  
  // Buttons
  tft.fillRoundRect(10, 200, 70, 35, 5, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(20, 210);
  tft.print("PREV");
  
  tft.fillRoundRect(90, 200, 70, 35, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setCursor(100, 210);
  tft.print("NEXT");
  
  tft.fillRoundRect(170, 200, 70, 35, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(175, 210);
  tft.print("VOL-");
  
  tft.fillRoundRect(250, 200, 70, 35, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(255, 210);
  tft.print("VOL+");
}

void loop() {
  server.handleClient();
  audio.loop();
  
  // Touch check
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    int x = map(p.x, 200, 3700, 0, 320);
    int y = map(p.y, 240, 3800, 0, 240);
    
    Serial.printf("Touch: X=%d Y=%d\n", x, y);
    
    // PREV button (x: 10-80, y: 200-235)
    if (x >= 10 && x <= 80 && y >= 200 && y <= 235) {
      Serial.println("PREV pressed!");
      currentStation = (currentStation - 1 + numStations) % numStations;
      audio.connecttohost(stationURLs[currentStation]);
      drawScreen();
      delay(500);
    }
    // NEXT button (x: 90-160, y: 200-235)
    else if (x >= 90 && x <= 160 && y >= 200 && y <= 235) {
      Serial.println("NEXT pressed!");
      currentStation = (currentStation + 1) % numStations;
      audio.connecttohost(stationURLs[currentStation]);
      drawScreen();
      delay(500);
    }
    // VOL- button (x: 170-240, y: 200-235)
    else if (x >= 170 && x <= 240 && y >= 200 && y <= 235) {
      Serial.println("VOL- pressed!");
      volume = max(0, volume - 2);
      audio.setVolume(volume);
      drawScreen();
      delay(500);
    }
    // VOL+ button (x: 250-320, y: 200-235)
    else if (x >= 250 && x <= 320 && y >= 200 && y <= 235) {
      Serial.println("VOL+ pressed!");
      volume = min(21, volume + 2);
      audio.setVolume(volume);
      drawScreen();
      delay(500);
    }
    
    while (touch.touched()) delay(10);
  }
  
  delay(10);
}
