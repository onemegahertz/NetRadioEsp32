# 🔧 Решение проблемы мерцания монитора

## Симптомы:
- Монитор TFT включается, показывает изображение
- Затем экран гаснет или мерцает
- ESP32 может перезагружаться циклически

## ✅ Что уже исправлено в текущей версии:

### 1. Отключён Brownout Detector
```cpp
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  // ...
}
```
**Почему это важно:** При включении подсветки TFT происходит скачок тока, который может вызвать brownout reset ESP32.

### 2. Добавлены задержки при инициализации
```cpp
tft.init();
delay(100);  // Задержка после инициализации
tft.fillScreen(TFT_BLACK);
delay(200);  // Задержка перед следующим действием
```
**Почему это важно:** Даёт время для стабилизации питания и завершения операций SPI.

### 3. Увеличен интервал обновления дисплея
```cpp
if (now - lastScreenUpdate > 2000) {  // Было 1000, стало 2000
  lastScreenUpdate = now;
  updateDisplay();
}
```
**Почему это важно:** Снижает нагрузку на SPI шину и процессор.

### 4. Добавлен yield() в loop()
```cpp
void loop() {
  yield();  // Сбрасываем watchdog timer
  // ...
  delay(10);  // Небольшая задержка для стабильности
}
```
**Почему это важно:** Предотвращает срабатывание watchdog timer.

---

## 🔍 Если проблема не решена - проверьте:

### 1. Питание ESP32

**Проблема:** USB порт компьютера может не обеспечивать достаточный ток.

**Решение:**
- Используйте **отдельный блок питания 5V 2A** через пин VIN
- Или используйте USB хаб с собственным питанием
- Избегайте длинных USB кабелей

**Проверка:**
```cpp
void setup() {
  Serial.begin(115200);
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Min free heap: ");
  Serial.println(ESP.getMinFreeHeap());
}
```
Если "Min free heap" < 50000 байт - проблема с памятью.

### 2. Проверьте User_Setup.h

**Критические параметры:**
```cpp
#define ILI9341_DRIVER

#define TFT_CS     5   // Должен совпадать с физическим подключением
#define TFT_DC     4
#define TFT_RST   15
#define TFT_MOSI  23
#define TFT_SCLK  18

#define SPI_FREQUENCY  27000000  // НЕ 40000000!
```

**Частые ошибки:**
- ❌ `TFT_CS 15` вместо `TFT_CS 5`
- ❌ `SPI_FREQUENCY 40000000` (слишком высокая частота)
- ❌ Забыли закомментировать другие драйверы

### 3. Проверьте подключение TFT

**Схема подключения:**
```
TFT ILI9341    →    ESP32
─────────────────────────────
VCC            →    3.3V (НЕ 5V!)
GND            →    GND
CS             →    GPIO 5
RESET          →    GPIO 15
DC             →    GPIO 4
MOSI (SDA/DIN) →    GPIO 23
SCK (CLK)      →    GPIO 18
LED            →    3.3V (или через резистор 100Ω)
MISO           →    не подключен
```

**Проверьте:**
- ✅ Все провода надёжно подключены
- ✅ Нет коротких замыканий между пинами
- ✅ TFT питается от 3.3V (НЕ от 5V!)
- ✅ LED подсветка подключена через резистор или напрямую к 3.3V

### 4. Проверьте конфликт пинов

**Пины ESP32 с ограничениями:**
- GPIO 0, 2, 12, 15 - используются для boot mode
- GPIO 34-39 - только input (нельзя использовать для output)
- GPIO 6-11 - заняты flash памятью (НЕ ИСПОЛЬЗОВАТЬ!)

**Наши пины:**
- TFT: 5, 15, 4, 23, 18 ✅ (все безопасны)
- I2S: 26, 25, 22 ✅ (все безопасны)
- Buttons: 32, 33, 34, 35 ✅ (34, 35 - input only, но мы используем INPUT_PULLUP)

### 5. Отключите аудио временно

Если проблема сохраняется, закомментируйте аудио:

```cpp
void setup() {
  // ...
  
  // Закомментируйте эту строку:
  // audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // audio.setVolume(currentVolume);
  
  // ...
}

void loop() {
  // Закомментируйте эту строку:
  // audio.loop();
  
  // ...
}
```

Если мерцание прекратилось - проблема с I2S DAC или его подключением.

### 6. Проверьте I2S DAC

**Схема подключения I2S DAC (MAX98357A / PCM5102):**
```
I2S DAC        →    ESP32
─────────────────────────────
VIN / VCC      →    5V
GND            →    GND
BCLK           →    GPIO 26
LRC / WS       →    GPIO 25
DIN            →    GPIO 22
SD / Gain      →    не подключен (или GND для 9dB)
```

**Проверьте:**
- ✅ DAC питается от 5V (НЕ от 3.3V!)
- ✅ Все пины подключены правильно
- ✅ Нет коротких замыканий
- ✅ Динамик подключён к DAC

---

## 🛠️ Диагностика через Serial Monitor

Добавьте в начало `setup()`:

```cpp
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== DIAGNOSTIC INFO ===");
  Serial.print("Chip model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU freq: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Min free heap: ");
  Serial.println(ESP.getMinFreeHeap());
  Serial.print("Flash size: ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("Sketch size: ");
  Serial.println(ESP.getSketchSize());
  Serial.print("Free sketch space: ");
  Serial.println(ESP.getFreeSketchSpace());
  Serial.println("======================\n");
  
  // ... остальной код
}
```

**Нормальные значения:**
- Free heap: > 100000 байт
- Min free heap: > 50000 байт
- Flash size: 2097152 (2MB) или 4194304 (4MB)

---

## 🎯 Пошаговое решение проблемы:

### Шаг 1: Скачайте обновлённый скетч
Скачайте `NetRadio_v1.ino` с исправлениями для brownout и watchdog.

### Шаг 2: Проверьте User_Setup.h
Убедитесь что пины указаны правильно:
```cpp
#define TFT_CS     5
#define TFT_DC     4
#define TFT_RST   15
#define SPI_FREQUENCY  27000000
```

### Шаг 3: Проверьте подключение TFT
- VCC → 3.3V (НЕ 5V!)
- CS → GPIO 5
- RESET → GPIO 15
- DC → GPIO 4
- MOSI → GPIO 23
- SCK → GPIO 18

### Шаг 4: Используйте отдельное питание
Подключите ESP32 через блок питания 5V 2A, а не через USB компьютера.

### Шаг 5: Загрузите скетч и откройте Serial Monitor
- Скорость: 115200 baud
- Проверьте диагностическую информацию

### Шаг 6: Если проблема сохраняется
- Отключите аудио временно (закомментируйте audio.loop())
- Увеличьте задержки в коде
- Попробуйте снизить SPI_FREQUENCY до 20000000

---

## 📊 Типичные значения для диагностики:

### Работает правильно:
```
=== DIAGNOSTIC INFO ===
Chip model: ESP32
Free heap: 200000
Min free heap: 150000
Flash size: 2097152
======================
[OK] TFT initialized
[OK] I2S
[OK] Buttons
[OK] WiFi: 192.168.1.100
```

### Проблема с памятью:
```
Free heap: 50000
Min free heap: 20000  ← Слишком мало!
```
**Решение:** Используйте ESP32 с 4MB Flash или оптимизируйте код.

### Проблема с питанием:
```
[OK] TFT initialized
Guru Meditation Error: Core 1 panic'ed (Interrupt wdt timeout on CPU1)
```
**Решение:** Используйте отдельный блок питания, отключите brownout detector.

### Проблема с SPI:
```
[OK] TFT initialized
(экран мерцает или показывает артефакты)
```
**Решение:** Снижайте SPI_FREQUENCY, проверяйте провода.

---

## 💡 Дополнительные советы:

1. **Не используйте USB hub без питания** - ESP32 + TFT требуют много тока
2. **Добавьте конденсатор 100µF** между VCC и GND near ESP32
3. **Используйте короткие провода** для SPI подключения (< 10cm)
4. **Проверьте качество пайки** - плохой контакт вызывает мерцание
5. **Попробуйте другую плату ESP32** - некоторые платы имеют дефекты

---

## 🆘 Если ничего не помогло:

1. Попробуйте **ESP32 с 4MB Flash** - больше памяти = стабильнее работа
2. Используйте **отдельный блок питания 5V 3A**
3. Замените **TFT дисплей** - возможно брак
4. Попробуйте **другую версию библиотеки TFT_eSPI** (2.5.0 или 2.4.0)

**Удачи! Если проблема решена - поделитесь результатом!** 🎉
