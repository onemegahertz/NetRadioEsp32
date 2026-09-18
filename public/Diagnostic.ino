/*
 * NetRadio v.4.1 - DIAGNOSTIC VERSION
 * Проверка всех компонентов
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// WiFi CONFIGURATION
#define WIFI_SSID     "YourSSID"      // <-- Введите ваш WiFi
#define WIFI_PASSWORD "YourPassword"  // <-- Введите пароль

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

// Globals
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  NetRadio v.4.1 - DIAGNOSTIC");
  Serial.println("========================================\n");
  
  // 1. TFT Test
  Serial.println("[1/4] Testing TFT Display...");
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("TFT: OK");
  Serial.println("[OK] TFT initialized");
  
  // 2. Touch Test
  Serial.println("\n[2/4] Testing Touchscreen...");
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  
  if (touchscreen.begin(touchscreenSPI)) {
    touchscreen.setRotation(1);
    Serial.println("[OK] Touch initialized");
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Touch: OK");
  } else {
    Serial.println("[FAIL] Touch initialization failed!");
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Touch: FAIL");
  }
  
  // 3. WiFi Test
  Serial.println("\n[3/4] Testing WiFi...");
  tft.setCursor(10, 70);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.println("WiFi: Connecting...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi connected!");
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("WiFi: OK");
    
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("SSID: %s", WiFi.SSID().c_str());
    
    tft.setCursor(10, 60);
    tft.printf("IP: %s", WiFi.localIP().toString().c_str());
    
    tft.setCursor(10, 80);
    tft.printf("RSSI: %d dBm", WiFi.RSSI());
  } else {
    Serial.println("\n[FAIL] WiFi connection failed!");
    Serial.printf("Status: %d\n", WiFi.status());
    
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("WiFi: FAIL");
    
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Check SSID/Password");
  }
  
  // 4. Touch Test - Interactive
  Serial.println("\n[4/4] Testing Touch (touch the screen)...");
  tft.setCursor(10, 110);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.println("Touch the screen to test");
  
  unsigned long startTime = millis();
  int touchCount = 0;
  
  while (millis() - startTime < 10000) {  // 10 seconds test
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
      TS_Point p = touchscreen.getPoint();
      
      int x = ::map(p.x, 200, 3700, 0, 320);
      int y = ::map(p.y, 240, 3800, 0, 240);
      
      Serial.printf("Touch detected: X=%d, Y=%d (Raw: X=%d, Y=%d, Z=%d)\n", 
                    x, y, p.x, p.y, p.z);
      
      tft.fillCircle(x, y, 5, TFT_RED);
      
      touchCount++;
      delay(100);
      
      while (touchscreen.touched()) {
        delay(10);
      }
    }
  }
  
  // Summary
  Serial.println("\n========================================");
  Serial.println("  DIAGNOSTIC SUMMARY");
  Serial.println("========================================");
  Serial.printf("TFT: OK\n");
  Serial.printf("Touch: %s (%d touches detected)\n", 
                touchCount > 0 ? "OK" : "FAIL", touchCount);
  Serial.printf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "OK" : "FAIL");
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("  SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  RSSI: %d dBm\n", WiFi.RSSI());
  }
  
  Serial.println("========================================\n");
  
  // Final screen
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.println("DIAGNOSTIC DONE");
  
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Check Serial Monitor");
  tft.println("for full report");
  
  if (touchCount > 0) {
    tft.setCursor(10, 80);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("Touch: OK (%d)", touchCount);
  } else {
    tft.setCursor(10, 80);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Touch: FAIL");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    tft.setCursor(10, 100);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("WiFi: OK");
    tft.setCursor(10, 120);
    tft.printf("IP: %s", WiFi.localIP().toString().c_str());
  } else {
    tft.setCursor(10, 100);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("WiFi: FAIL");
  }
}

void loop() {
  // Stay here
  delay(1000);
}
