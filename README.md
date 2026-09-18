# NetRadio v.3 - ESP32-2432S028R (CYD) Internet Radio

Интернет-радио на базе платы ESP32-2432S028R (Cheap Yellow Display) с встроенным TFT дисплеем 2.8", SD картой и тачскрином.

## 🎯 Особенности

### Встроенные компоненты (не нужно подключать!):
- ✅ **TFT дисплей 2.8"** ILI9341 (320x240, RGB)
- ✅ **Слот SD карты** для хранения станций (до 50 станций)
- ✅ **Тачскрин** XPT2046 (резистивный)
- ✅ **Усилитель звука** (GPIO 26)
- ✅ **RGB LED** для индикации статуса
- ✅ **Датчик освещённости** (LDR, GPIO 34)
- ✅ **Кнопка BOOT** (GPIO 0)

### Функции:
- 📻 Воспроизведение интернет-радиостанций
- 💾 Хранение до 50 станций на SD карте
- 🌤️ Отображение погоды Москвы (OpenWeatherMap)
- ⏰ Отображение времени (NTP синхронизация)
- 🎨 4 режима дисплея (громкость, температура, дата, WiFi)
- 🌐 Веб-интерфейс для управления
- 🚦 RGB LED индикация (синий = старт, зелёный = работа)

## 🛠️ Необходимые компоненты

### Обязательные:
- **ESP32-2432S028R (CYD)** - ~$15
- **I2S DAC** (MAX98357A или PCM5102) - ~$3
- **Динамик 3W** (4-8 Ohm) - ~$2

### Опциональные:
- **microSD карта** (до 32GB, FAT32) - ~$3
- **2 тактовые кнопки** (6x6mm) - ~$0.50
- **2 резистора 10kΩ** (для кнопок) - ~$0.10

**Итого: ~$20-24**

## 🔌 Подключение

### I2S DAC (MAX98357A) → CYD (CN1 коннектор):
```
DAC        →    CYD
─────────────────────
BCLK       →    GPIO 25
LRC        →    GPIO 27
DIN        →    GPIO 22
VCC        →    3.3V
GND        →    GND
```

### Кнопки (опционально):
```
Кнопка     →    CYD         →    3.3V (через 10kΩ)
─────────────────────────────────────────────────────
BTN_PREV   →    GPIO 35     →    10kΩ → 3.3V
BTN_NEXT   →    GPIO 34     →    10kΩ → 3.3V
BTN_VOL_UP →    GPIO 0      →    (встроенная BOOT)
```

**Важно:** GPIO 34 и 35 не имеют внутреннего pull-up! Нужны внешние резисторы 10kΩ.

### Динамик:
Подключите динамик к разъёму J1 (Speaker) на плате CYD.

## 📚 Необходимые библиотеки

Установите через Arduino Library Manager (Ctrl+Shift+I):

1. **TFT_eSPI** by Bodmer (v2.5.43+)
2. **ESP32-audioI2S** by schreibfaul1 (**v3.0.8! НЕ 4.0.0**)
3. **SD** (встроена)
4. **SPI** (встроена)
5. **WiFi** (встроена)
6. **WebServer** (встроена)
7. **Preferences** (встроена)

### ⚠️ ВАЖНО: Версия ESP32-audioI2S

**НЕ используйте версию 4.0.0!** Она не работает с обычным ESP32 и вызывает ошибку OOM.

Установите **версию 3.0.8** через Library Manager.

## ⚙️ Настройка Arduino IDE

### 1. Установите ESP32 Board Package:
- File → Preferences
- Добавьте URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Tools → Board → Boards Manager
- Найдите и установите "esp32 by Espressif Systems"

### 2. Выберите плату:
- Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
- Flash Size: "4MB (32Mb)"
- Partition Scheme: "Default 4MB with spiffs"
- Upload Speed: "921600" (или "115200" если проблемы)

### 3. Настройте TFT_eSPI:

Откройте файл `User_Setup.h` в библиотеке TFT_eSPI и раскомментируйте:

```cpp
#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

#define SPI_FREQUENCY  40000000
```

**Важно:** Закомментируйте все другие определения драйверов!

## 💾 SD карта

### Форматирование:
Отформатируйте microSD карту в **FAT32**.

### Файл stations.txt:
Создайте файл `stations.txt` в корне SD карты:

```
Record|https://radiorecord.hostingradio.ru/rr_320
RusMix|https://radiorecord.hostingradio.ru/rusmix_320
90s|https://radiorecord.hostingradio.ru/sd90_320
Chill|https://radiorecord.hostingradio.ru/chil_320
Deep|https://radiorecord.hostingradio.ru/deep_320
```

**Формат:** `Название|URL` (одна станция на строку, максимум 50 станций)

Если файл не найден, загружаются 20 станций по умолчанию.

## 🚀 Прошивка

1. Откройте файл `NetRadio_v3_CYD.ino` в Arduino IDE
2. Подключите CYD через USB
3. Выберите правильный COM порт (Tools → Port)
4. Нажмите "Upload" (→)
5. Дождитесь завершения загрузки
6. Откройте Serial Monitor (115200 baud) для отладки

## 📡 Первое подключение

### Режим 1: WiFi не настроен (AP Mode)
1. После прошивки CYD создаст WiFi точку доступа:
   - **SSID:** `NetRadio`
   - **Пароль:** `netradio123`
2. Подключитесь к этой сети с телефона/компьютера
3. Откройте браузер и перейдите на `http://192.168.4.1`
4. В веб-интерфейсе настройте WiFi:
   - Введите SSID вашей домашней сети
   - Введите пароль
   - Нажмите "Save WiFi"
5. CYD перезагрузится и подключится к вашей сети

### Режим 2: WiFi уже настроен
1. CYD автоматически подключится к сохраненной сети
2. В Serial Monitor увидите IP адрес
3. Откройте веб-интерфейс по этому IP

## 🎵 Использование

### Физические кнопки (если подключены):
- **BTN_PREV (GPIO 35)** - предыдущая станция
- **BTN_NEXT (GPIO 34)** - следующая станция
- **BTN_VOL_UP (GPIO 0)** - увеличить громкость (BOOT кнопка)

### Веб-интерфейс:
Откройте `http://[IP-адрес-CYD]` в браузере

**Функции:**
- Переключение станций
- Регулировка громкости
- Добавление/редактирование/удаление станций
- Изменение WiFi настроек
- Просмотр статуса

## 🖥️ TFT Дисплей

### Режим 1: Громкость
```
NetRadio v.3
              14:35:22

Record
Station 1/20

Volume: 12/21
[========      ]

WiFi: MyNetwork
SD Card: OK
Mode: Volume (press BOOT to change)
Web: http://192.168.1.100
```

### Режим 2: Температура
```
NetRadio v.3
              14:35:22

Record
Station 1/20

Moscow: -5.2C

WiFi: MyNetwork
SD Card: OK
Mode: Temp (press BOOT to change)
Web: http://192.168.1.100
```

### Режим 3: Дата
```
NetRadio v.3
              14:35:22

Record
Station 1/20

Date: 15.01.2024

WiFi: MyNetwork
SD Card: OK
Mode: Date (press BOOT to change)
Web: http://192.168.1.100
```

### Режим 4: WiFi
```
NetRadio v.3
              14:35:22

Record
Station 1/20

IP: 192.168.1.100

WiFi: MyNetwork
SD Card: OK
Mode: WiFi (press BOOT to change)
Web: http://192.168.1.100
```

## 🚦 RGB LED индикация

- **Синий** - запуск, подключение к WiFi
- **Зелёный** - готово, воспроизведение
- **Красный** - ошибка

## 🌤️ Погода

API ключ OpenWeatherMap уже встроен в код:
```cpp
#define WEATHER_API_KEY "cf0cd0d160ba580cef69e35dfe3064c8"
#define WEATHER_CITY "Moscow,RU"
```

Погода обновляется каждые 30 минут.

## 🔧 Решение проблем

### TFT дисплей не работает
- Проверьте User_Setup.h для TFT_eSPI
- Убедитесь что указаны правильные пины (13, 12, 14, 15, 2)
- Проверьте что закомментированы другие драйверы

### SD карта не обнаружена
- Проверьте что карта отформатирована в FAT32
- Попробуйте другую карту (до 32GB)
- Проверьте контакты слота

### Нет звука
- Проверьте подключение I2S DAC к GPIO 25, 27, 22
- Убедитесь что DAC получает питание 3.3V
- Проверьте подключение динамика к J1
- Убедитесь что версия библиотеки ESP32-audioI2S = 3.0.8

### OOM: failed to allocate 720896 bytes
- Вы используете версию 4.0.0 библиотеки ESP32-audioI2S
- Удалите её и установите версию 3.0.8!

### Кнопки не работают
- GPIO 34 и 35 не имеют внутреннего pull-up
- Подключите внешние резисторы 10kΩ между GPIO и 3.3V
- Или используйте веб-интерфейс для управления

### WiFi не подключается
- Проверьте SSID и пароль
- ESP32 работает только с 2.4GHz WiFi (не 5GHz)
- Убедитесь что сеть в зоне покрытия

## 📊 Технические характеристики

- **Плата:** ESP32-2432S028R (CYD)
- **Процессор:** ESP32-WROOM-32 (Dual Core, 240MHz)
- **Flash:** 4MB
- **RAM:** 520KB
- **Дисплей:** TFT 2.8" ILI9341 (320x240, RGB)
- **Тачскрин:** XPT2046 (резистивный)
- **SD карта:** microSD до 32GB (FAT32)
- **Аудио:** I2S DAC (внешний) + встроенный усилитель
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Станции:** до 50 (на SD карте)
- **Громкость:** 0-21 (22 уровня)

## 🆘 Дополнительная помощь

Если проблема не решена:
1. Проверьте все подключения
2. Используйте Serial Monitor для отладки
3. Попробуйте перепрошить с очисткой Flash
4. Проверьте напряжение питания (5V через USB)
5. Убедитесь что используете правильную версию библиотеки ESP32-audioI2S (3.0.8)

## 📝 Лицензия

Проект основан на идеях yoRadio by e2002
https://github.com/e2002/yoradio

## 🎉 Благодарности

- [yoRadio](https://github.com/e2002/yoradio) - оригинальный проект
- [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) - аудио библиотека
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) - библиотека для TFT дисплея
- [OpenWeatherMap](https://openweathermap.org/) - API погоды
- [Random Nerd Tutorials](https://randomnerdtutorials.com/esp32-cheap-yellow-display-cyd-pinout-esp32-2432s028r/) - распиновка CYD

---

**Удачи с вашим NetRadio проектом!** 🎵📻

Если у вас возникли вопросы или проблемы, проверьте:
1. Все подключения согласно схеме
2. Правильность настроек User_Setup.h
3. Наличие всех библиотек (правильные версии!)
4. Правильность настроек платы в Arduino IDE
5. Формат SD карты (FAT32)
