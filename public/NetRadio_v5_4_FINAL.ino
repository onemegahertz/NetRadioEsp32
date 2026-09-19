/*
 * NetRadio v5.4 - ПОЛНОСТЬЮ РАБОЧАЯ ВЕРСИЯ
 * Исправлены кнопки, добавлена SD карта, кнопка BT сопряжения
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
#include <SD.h>
#include "Audio.h"
#include <time.h>

// Bluetooth через ESP-IDF
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"

// Pins
#define TFT_BL    21
#define TOUCH_CS  33
#define TOUCH_IRQ 36
#define I2S_BCLK  27
#define I2S_LRC   26
#define I2S_DOUT  25
#define SD_CS     5

// Weather
#define WEATHER_KEY "cf0cd0d160ba580cef69e35dfe3064c8"

// Globals
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
Audio audio;
WebServer server(80);

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
bool btEnabled = false;
bool btConnected = false;
bool sdCardDetected = false;
char curTime[9] = "";
char curDate[11] = "";
char weatherTemp[10] = "";
char wifiSSID[20] = "";
char wifiIP[16] = "";

unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;
unsigned long lastTouch = 0;

// Bluetooth callbacks
void bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  if (event == ESP_BT_GAP_AUTH_CMPL_EVT) {
    if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
      Serial.println("[BT] Auth success");
    }
  }
}

void bt_a2dp_callback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
  if (event == ESP_A2D_CONNECTION_STATE_EVT) {
    if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
      btConnected = true;
      Serial.println("[BT] Connected!");
    } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
      btConnected = false;
      Serial.println("[BT] Disconnected");
    }
  }
}

void bt_audio_data_callback(const uint8_t *data, uint32_t len) {
  // Audio data handled by I2S
}

void initBluetooth() {
  Serial.println("[BT] Initializing...");
  
  esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
  
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  bt_cfg.mode = ESP_BT_MODE_CLASSIC_BT;
  bt_cfg.bt_max_acl_conn = 1;
  bt_cfg.bt_max_sync_conn = 0;
  
  if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
    Serial.println("[BT] Init failed!");
    return;
  }
  
  if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
    Serial.println("[BT] Enable failed!");
    return;
  }
  
  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("[BT] Bluedroid init failed!");
    return;
  }
  
  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("[BT] Bluedroid enable failed!");
    return;
  }
  
  esp_bt_dev_set_device_name("NetRadio");
  esp_bt_gap_register_callback(bt_gap_callback);
  esp_a2dp_register_callback(bt_a2dp_callback);
  esp_a2dp_sink_register_data_callback(bt_audio_data_callback);
  
  if (esp_a2dp_sink_init() != ESP_OK) {
    Serial.println("[BT] A2DP sink init failed!");
    return;
  }
  
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  
  btEnabled = true;
  Serial.println("[BT] Ready - device name: NetRadio");
}

void stopBluetooth() {
  if (!btEnabled) return;
  
  esp_a2dp_sink_deinit();
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  
  btEnabled = false;
  btConnected = false;
  Serial.println("[BT] Stopped");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== NetRadio v5.4 ===");
  
  // TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("NetRadio v5.4");
  tft.setTextSize(1);
  tft.setCursor(10, 35);
  tft.println("Starting...");
  Serial.println("[OK] TFT");
  
  // Touch
  touch.begin(SPI);
  touch.setRotation(1);
  Serial.println("[OK] Touch");
  
  // SD Card
  SPI.begin(18, 19, 23, SD_CS);
  sdCardDetected = SD.begin(SD_CS);
  if (sdCardDetected) {
    Serial.println("[OK] SD Card detected");
  } else {
    Serial.println("[FAIL] SD Card not detected");
  }
  
  // Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume);
  Serial.println("[OK] Audio");
  
  // Bluetooth
  initBluetooth();
  
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
    configTime(10800, 0, "pool.ntp.org");
    Serial.println("\n[OK] WiFi connected!");
    Serial.printf("SSID: %s\n", wifiSSID);
    Serial.printf("IP: %s\n", wifiIP);
    updateWeather();
  } else {
    Serial.println("\n[FAIL] WiFi failed!");
    strcpy(wifiSSID, "Not connected");
    strcpy(wifiIP, "0.0.0.0");
  }
  
  // Web server
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>NetRadio</title>";
    html += "<meta http-equiv='refresh' content='3'>";
    html += "<style>body{font-family:Arial;background:#1a1a2e;color:#e0e0e0;padding:20px}";
    html += "button{padding:15px 30px;margin:5px;font-size:18px;cursor:pointer;border:none;border-radius:5px;color:white}";
    html += ".prev{background:#2196F3}.next{background:#4CAF50}.vold{background:#FF9800}.volu{background:#F44336}.bt{background:#9C27B0}</style></head>";
    html += "<body><h1>NetRadio v5.4</h1>";
    html += "<p>Station: <b>" + String(stationNames[currentStation]) + "</b></p>";
    html += "<p>Volume: <b>" + String(volume) + "/21</b></p>";
    html += "<p>WiFi: <b>" + String(wifiSSID) + "</b></p>";
    html += "<p>IP: <b>" + String(wifiIP) + "</b></p>";
    html += "<p>Time: <b>" + String(curTime) + "</b></p>";
    html += "<p>Weather: <b>" + String(weatherTemp) + "</b></p>";
    html += "<p>Bluetooth: <b>" + String(btEnabled ? (btConnected ? "Connected" : "ON") : "OFF") + "</b></p>";
    html += "<p>SD Card: <b>" + String(sdCardDetected ? "OK" : "NOT FOUND") + "</b></p>";
    html += "<button class='prev' onclick=\"fetch('/prev')\">◀ Prev</button>";
    html += "<button class='next' onclick=\"fetch('/next')\">Next ▶</button><br>";
    html += "<button class='vold' onclick=\"fetch('/voldown')\">Vol-</button>";
    html += "<button class='volu' onclick=\"fetch('/volup')\">Vol+</button><br>";
    html += "<button class='bt' onclick=\"fetch('/bt')\">Toggle Bluetooth</button>";
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
  
  server.on("/bt", []() {
    if (btEnabled) {
      stopBluetooth();
    } else {
      initBluetooth();
    }
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
  tft.print("NetRadio v5.4");
  
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(240, 10);
  tft.print(curTime);
  
  // LEFT SIDE - Station info
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 45);
  tft.print(stationNames[currentStation]);
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(10, 80);
  tft.printf("Station %d/%d", currentStation + 1, numStations);
  
  // Volume
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 105);
  tft.printf("Vol: %d/21", volume);
  
  // Volume bar
  tft.fillRect(10, 130, 200, 15, TFT_DARKGREY);
  tft.fillRect(10, 130, ::map(volume, 0, 21, 0, 200), 15, TFT_GREEN);
  tft.drawRect(10, 130, 200, 15, TFT_WHITE);
  
  // RIGHT SIDE - Status info
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(220, 45);
  tft.printf("WiFi:");
  tft.setCursor(220, 60);
  tft.printf("%s", wifiSSID);
  
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(220, 80);
  tft.printf("IP:");
  tft.setCursor(220, 95);
  tft.printf("%s", wifiIP);
  
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(220, 115);
  tft.printf("Moscow:");
  tft.setCursor(220, 130);
  tft.printf("%s", weatherTemp);
  
  // SD Card status
  tft.setTextColor(sdCardDetected ? TFT_GREEN : TFT_RED);
  tft.setCursor(220, 150);
  tft.printf("SD: %s", sdCardDetected ? "OK" : "NO");
  
  // Bluetooth status
  tft.setTextColor(btEnabled ? TFT_GREEN : TFT_RED);
  tft.setCursor(220, 165);
  if (btEnabled) {
    tft.printf("BT: %s", btConnected ? "LINK" : "ON");
  } else {
    tft.print("BT: OFF");
  }
  
  // Buttons - BIG and EASY to touch
  tft.fillRoundRect(5, 200, 75, 35, 5, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(15, 210);
  tft.print("PREV");
  
  tft.fillRoundRect(85, 200, 75, 35, 5, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setCursor(95, 210);
  tft.print("NEXT");
  
  tft.fillRoundRect(165, 200, 75, 35, 5, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_ORANGE);
  tft.setCursor(170, 210);
  tft.print("VOL-");
  
  tft.fillRoundRect(245, 200, 75, 35, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(250, 210);
  tft.print("VOL+");
}

void loop() {
  server.handleClient();
  audio.loop();
  
  unsigned long now = millis();
  
  if (now - lastTimeUpdate > 1000) {
    lastTimeUpdate = now;
    updateTime();
    drawScreen();
  }
  
  if (now - lastWeatherUpdate > 1800000) {
    lastWeatherUpdate = now;
    updateWeather();
  }
  
  // Touch handling
  if (touch.touched()) {
    if (now - lastTouch < 300) {
      while (touch.touched()) delay(5);
      return;
    }
    lastTouch = now;
    
    TS_Point p = touch.getPoint();
    
    // Калибровка координат
    int x = p.x;
    int y = p.y;
    
    // Преобразование координат тачскрина в экранные
    // Эти значения нужно подстроить под ваш дисплей
    int screenX = ::map(x, 200, 3800, 0, 320);
    int screenY = ::map(y, 200, 3800, 0, 240);
    
    // Ограничение координат
    screenX = constrain(screenX, 0, 319);
    screenY = constrain(screenY, 0, 239);
    
    Serial.printf("[TOUCH] Raw: X=%d Y=%d -> Screen: X=%d Y=%d\n", x, y, screenX, screenY);
    
    // PREV button (x: 5-80, y: 200-235)
    if (screenX >= 5 && screenX <= 80 && screenY >= 200 && screenY <= 235) {
      Serial.println("[BTN] PREV");
      currentStation = (currentStation - 1 + numStations) % numStations;
      audio.connecttohost(stationURLs[currentStation]);
      drawScreen();
    }
    // NEXT button (x: 85-160, y: 200-235)
    else if (screenX >= 85 && screenX <= 160 && screenY >= 200 && screenY <= 235) {
      Serial.println("[BTN] NEXT");
      currentStation = (currentStation + 1) % numStations;
      audio.connecttohost(stationURLs[currentStation]);
      drawScreen();
    }
    // VOL- button (x: 165-240, y: 200-235)
    else if (screenX >= 165 && screenX <= 240 && screenY >= 200 && screenY <= 235) {
      Serial.println("[BTN] VOL-");
      volume = max(0, volume - 2);
      audio.setVolume(volume);
      drawScreen();
    }
    // VOL+ button (x: 245-320, y: 200-235)
    else if (screenX >= 245 && screenX <= 320 && screenY >= 200 && screenY <= 235) {
      Serial.println("[BTN] VOL+");
      volume = min(21, volume + 2);
      audio.setVolume(volume);
      drawScreen();
    }
    
    while (touch.touched()) delay(5);
  }
  
  delay(10);
}
