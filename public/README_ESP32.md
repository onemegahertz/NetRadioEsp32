# NetRadio v.2 - ESP32 + LCD 1602 I2C

Интернет-радио на базе ESP32 с LCD дисплеем 1602 (I2C интерфейс)

## 📋 Необходимые компоненты

### Обязательные:
- **ESP32 DevKit V1** (или любая плата ESP32)
- **LCD 1602 с I2C модулем** (адрес 0x27 или 0x3F)
- **I2S DAC** (MAX98357A или PCM5102)
- **3 тактовые кнопки** (6x6mm)
- **Динамик 3W** (4-8 Ом)
- **Блок питания 5V 2A**
- **Провода подключения**

### Опциональные:
- Макетная плата (breadboard)
- Резисторы 10kΩ (для кнопок, если нет internal pull-up)

## 🔌 Схема подключения

### LCD 1602 I2C → ESP32
```
LCD I2C    →    ESP32
─────────────────────
GND        →    GND
VCC        →    5V
SDA        →    GPIO21
SCL        →    GPIO22
```

### I2S DAC (MAX98357A) → ESP32
```
DAC        →    ESP32
─────────────────────
VCC        →    5V
GND        →    GND
BCLK       →    GPIO26
LRC        →    GPIO25
DIN        →    GPIO27
```

### Кнопки → ESP32
```
Кнопка     →    ESP32    →    GND
─────────────────────────────────────
BTN_PREV   →    GPIO32   →    GND
BTN_NEXT   →    GPIO33   →    GND
BTN_VOL    →    GPIO34   →    GND
```

**Важно:** Кнопки подключаются между GPIO пином и GND. В коде используется INPUT_PULLUP, поэтому внешние резисторы не нужны.

## 📚 Необходимые библиотеки

Установите через Arduino Library Manager (Ctrl+Shift+I):

1. **LiquidCrystal_I2C** by Frank de Brabander (v1.1.2+)
2. **ESP32-audioI2S** by schreibfaul1 (v3.0.7+)
3. **ArduinoJson** by Benoit Blanchon (v7.0.4+) - опционально

## ⚙️ Настройка Arduino IDE

1. **Установите ESP32 Board Package:**
   - File → Preferences
   - Добавьте URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager
   - Найдите и установите "esp32 by Espressif Systems"

2. **Выберите плату:**
   - Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
   - Flash Size: "4MB (32Mb)"
   - Partition Scheme: "Default 4MB with spiffs"
   - Upload Speed: "921600"

3. **Установите библиотеки** (см. выше)

## 🔧 Определение адреса I2C LCD

Если LCD не работает, нужно определить его I2C адрес:

1. Загрузите скетч I2C Scanner:
```cpp
#include <Wire.h>

void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Scanning...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
  
  delay(5000);
}
```

2. Откройте Serial Monitor (115200 baud)
3. Найдите адрес (обычно 0x27 или 0x3F)
4. Измените `#define LCD_ADDRESS 0x27` в скетче на найденный адрес

## 🚀 Прошивка

1. Откройте файл `NetRadio_v2_ESP32.ino` в Arduino IDE
2. Подключите ESP32 через USB
3. Выберите правильный COM порт (Tools → Port)
4. Нажмите "Upload" (→)
5. Дождитесь завершения загрузки
6. Откройте Serial Monitor (115200 baud) для отладки

## 📡 Первое подключение

### Режим 1: WiFi не настроен (AP Mode)
1. После прошивки ESP32 создаст WiFi точку доступа:
   - **SSID:** `NetRadio`
   - **Пароль:** `netradio123`
2. Подключитесь к этой сети с телефона/компьютера
3. Откройте браузер и перейдите на `http://192.168.4.1`
4. В веб-интерфейсе настройте WiFi:
   - Введите SSID вашей домашней сети
   - Введите пароль
   - Нажмите "Save WiFi"
5. ESP32 перезагрузится и подключится к вашей сети

### Режим 2: WiFi уже настроен
1. ESP32 автоматически подключится к сохраненной сети
2. В Serial Monitor увидите IP адрес
3. Откройте веб-интерфейс по этому IP

## 🎵 Использование

### Физические кнопки:
- **BTN_PREV (GPIO32)** - предыдущая станция
- **BTN_NEXT (GPIO33)** - следующая станция
- **BTN_VOL (GPIO34)** - увеличить громкость (долгое нажатие - уменьшить)

### Веб-интерфейс:
Откройте `http://[IP-адрес-ESP32]` в браузере

**Функции:**
- Переключение станций
- Регулировка громкости
- Добавление/редактирование/удаление станций
- Изменение WiFi настроек
- Просмотр статуса

## 📻 Радиостанции по умолчанию

Прошивка включает 20 станций Radio Record:
1. Record
2. RusMix
3. 90s (Супердискотека 90-х)
4. Chill (Chill-Out)
5. Deep
6. Rock
7. Rap
8. Techno
9. House
10. EDM
11. TM (Trancemission)
12. Pirate (Pirate Station)
13. Dub (Dubstep)
14. Synth (Synthwave)
15. LoFi
16. Euro (Eurodance)
17. Trap
18. Hard (Hardstyle)
19. Amb (Ambient)
20. RusHits (Russian Hits)

Вы можете добавить свои станции через веб-интерфейс (максимум 20).

## 🖥️ LCD Дисплей

**Строка 1:** Название текущей станции (с прокруткой если длинное)
**Строка 2:** `V:[громкость] [время/IP]`

Пример:
```
Record          
V:12 14:35:22   
```

## 🔧 Решение проблем

### LCD не показывает текст
- Проверьте I2C адрес (0x27 или 0x3F)
- Проверьте подключение SDA (GPIO21) и SCL (GPIO22)
- Убедитесь что VCC = 5V (не 3.3V!)
- Попробуйте запустить I2C Scanner

### Нет звука
- Проверьте подключение I2S DAC
- Убедитесь что DAC получает 5V
- Проверьте динамик
- Увеличьте громкость через веб-интерфейс или кнопку

### WiFi не подключается
- Проверьте SSID и пароль
- ESP32 работает только с 2.4GHz WiFi (не 5GHz)
- Убедитесь что сеть в зоне покрытия
- Попробуйте перепрошить с очисткой EEPROM

### Станции не переключаются
- Проверьте подключение кнопок
- Убедитесь что кнопки замыкают на GND
- Попробуйте через веб-интерфейс

### Веб-интерфейс не открывается
- Проверьте что подключены к той же WiFi сети
- Проверьте IP адрес в Serial Monitor
- Попробуйте перезагрузить ESP32

## 💡 Советы

1. **Используйте качественный блок питания** (5V 2A минимум)
2. **Короткие провода** для I2S DAC (< 20cm)
3. **Конденсатор 100µF** между VCC и GND near ESP32
4. **Сохраняйте настройки** через веб-интерфейс
5. **Используйте станции с битрейтом 128-192 kbps** для стабильности

## 📊 Технические характеристики

- **Платформа:** ESP32 (4MB Flash)
- **Дисплей:** LCD 1602 I2C (16x2 символа)
- **Аудио:** I2S DAC (24-bit, 44.1kHz)
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Станции:** до 20 (сохраняются в NVS)
- **Громкость:** 0-21 (22 уровня)
- **Потребление:** ~200mA (без WiFi), ~350mA (с WiFi)

## 🆘 Дополнительная помощь

Если проблема не решена:
1. Проверьте все подключения
2. Используйте Serial Monitor для отладки
3. Попробуйте перепрошить с очисткой Flash
4. Проверьте напряжение питания

## 📝 Лицензия

Проект основан на идеях yoRadio by e2002
https://github.com/e2002/yoradio

---

**Удачи с вашим NetRadio проектом!** 🎵📻
