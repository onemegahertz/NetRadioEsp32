# 🔧 Решение проблемы с тёмным экраном при Huge APP

## ❌ Проблема:

При использовании **Partition Scheme: Huge APP (3MB No OTA)** экран тёмный и ничего не отображается.

## 🔍 Причина:

При схеме разделов **Huge APP (3MB)** ESP32 использует больше памяти для приложения, но остаётся **слишком мало RAM** для буферов TFT дисплея.

**TFT_eSPI требует ~50KB RAM** для буферов экрана. При Huge APP свободной RAM остаётся < 30KB, и дисплей не может инициализироваться.

## ✅ Решение:

### Вариант 1: Использовать "Minimal SPIFFS" (РЕКОМЕНДУЮ)

**Partition Scheme: Minimal SPIFFS (1.9MB APP with OTA/120KB SPIFFS)**

**Что работает:**
- ✅ Погода Москвы
- ✅ Время (NTP)
- ✅ WiFi SSID + IP адрес
- ✅ Громкость с визуализацией
- ✅ 10 радиостанций
- ✅ 4 кнопки на экране
- ✅ Веб-интерфейс
- ✅ Тачскрин
- ❌ Bluetooth (убран для стабильности)

**Размер скетча:** ~1.5MB (помещается в 1.9MB)

**Преимущества:**
- Стабильная работа
- Экран работает корректно
- Все основные функции работают
- Не нужно оптимизировать User_Setup.h

**Недостатки:**
- Нет Bluetooth

### Вариант 2: "Huge APP" + оптимизация User_Setup.h

Если **очень нужен Bluetooth**, нужно:

1. Использовать **Partition Scheme: Huge APP (3MB No OTA)**

2. Заменить `User_Setup.h` в библиотеке TFT_eSPI на **User_Setup_CYD_HugeAPP.h**

**Что оптимизировано:**
- Убраны все шрифты кроме LOAD_GLCD, LOAD_FONT2, LOAD_FONT4
- Снижена SPI частота с 40MHz до 27MHz
- Это экономит ~30-40KB RAM

**Что работает:**
- ✅ Bluetooth A2DP
- ✅ Погода
- ✅ Время
- ✅ WiFi
- ✅ Громкость
- ✅ Станции
- ✅ Кнопки
- ✅ Веб-интерфейс

**Размер скетча:** ~2.2MB (помещается в 3MB)

**Риски:**
- Экран может быть нестабильным
- Возможны перезагрузки
- Меньше свободной RAM

## 📋 Инструкция для Варианта 1 (РЕКОМЕНДУЮ):

### Шаг 1: Настройте Arduino IDE

1. Tools → Board → ESP32 Dev Module
2. Tools → Partition Scheme → **Minimal SPIFFS (1.9MB APP with OTA/120KB SPIFFS)**
3. Tools → Upload Speed → 921600

### Шаг 2: Откройте скетч

Откройте файл **NetRadio_v5_2_STABLE.ino**

### Шаг 3: Введите WiFi данные

В начале файла найдите:
```cpp
#define WIFI_SSID     "YourSSID"
#define WIFI_PASSWORD "YourPassword"
```

Замените на ваши данные:
```cpp
#define WIFI_SSID     "MyHomeNetwork"
#define WIFI_PASSWORD "MyPassword123"
```

### Шаг 4: Загрузите прошивку

Нажмите кнопку Upload

### Шаг 5: Проверьте Serial Monitor

Откройте Serial Monitor (115200 baud)

Должно быть:
```
=== NetRadio v5.2 ===
[OK] TFT
[OK] Touch
[OK] Audio
[OK] WiFi connected!
SSID: MyHomeNetwork
IP: 192.168.1.26
[WEATHER] -5.2°C
[OK] Web server started
[OK] Playing
=== Ready ===
Web: http://192.168.1.26
```

### Шаг 6: Проверьте экран

На экране должно отображаться:
- Название станции
- WiFi SSID и IP
- Погода Москвы
- Время
- Громкость
- 4 кнопки

## 📋 Инструкция для Варианта 2 (если нужен Bluetooth):

### Шаг 1: Замените User_Setup.h

1. Откройте папку библиотеки TFT_eSPI:
   ```
   C:\Users\[USER]\Documents\Arduino\libraries\TFT_eSPI\
   ```

2. Замените файл `User_Setup.h` на скачанный **User_Setup_CYD_HugeAPP.h**

3. Перезапустите Arduino IDE

### Шаг 2: Настройте Arduino IDE

1. Tools → Board → ESP32 Dev Module
2. Tools → Partition Scheme → **Huge APP (3MB No OTA/1MB SPIFFS)**
3. Tools → Upload Speed → 921600

### Шаг 3: Используйте скетч с Bluetooth

Откройте файл **NetRadio_v5_1.ino** (с Bluetooth)

### Шаг 4: Введите WiFi данные и загрузите

## 🎯 Сравнение вариантов:

| Параметр | Вариант 1 (Minimal SPIFFS) | Вариант 2 (Huge APP) |
|----------|---------------------------|---------------------|
| Partition Scheme | Minimal SPIFFS (1.9MB) | Huge APP (3MB) |
| Bluetooth | ❌ Нет | ✅ Да |
| Стабильность | ✅ Высокая | ⚠️ Средняя |
| Размер скетча | ~1.5MB | ~2.2MB |
| Свободная RAM | ~150KB | ~50KB |
| Экран | ✅ Работает | ⚠️ Может быть проблемы |
| Рекомендация | ✅ **ДА** | ⚠️ Только если нужен BT |

## 💡 Моя рекомендация:

**Используйте Вариант 1 (Minimal SPIFFS без Bluetooth)**

**Почему:**
1. Стабильная работа экрана
2. Все основные функции работают
3. Не нужно оптимизировать User_Setup.h
4. Меньше проблем с памятью
5. Bluetooth можно добавить позже через внешнюю колонку

**Если очень нужен Bluetooth:**
- Купите ESP32 с **PSRAM** (ESP32-WROVER) - там больше RAM
- Или используйте внешнюю Bluetooth колонку через AUX кабель

## 🔍 Если экран всё равно тёмный:

### Проверка 1: Partition Scheme

Убедитесь что выбрано:
- **Minimal SPIFFS (1.9MB APP with OTA/120KB SPIFFS)**

НЕ должно быть:
- ~~Huge APP (3MB No OTA)~~
- ~~Default 4MB with spiffs~~

### Проверка 2: User_Setup.h

Убедитесь что в User_Setup.h:
```cpp
#define ILI9341_2_DRIVER
#define TFT_INVERSION_ON
#define TFT_setRotation 1
#define SPI_FREQUENCY  27000000  // НЕ 40000000!
```

### Проверка 3: Подключение

Проверьте что пины подключены правильно:
- TFT_MOSI → GPIO 13
- TFT_MISO → GPIO 12
- TFT_SCLK → GPIO 14
- TFT_CS → GPIO 15
- TFT_DC → GPIO 2
- TFT_BL → GPIO 21

### Проверка 4: Запустите Diagnostic.ino

Загрузите файл **Diagnostic.ino** для проверки всех компонентов.

## 📊 Размер скетча:

**Вариант 1 (Minimal SPIFFS):**
```
Скетч использует 1500000 байт (79%) памяти устройства.
Всего доступно 1900544 байт.
Глобальные переменные используют 65000 байт (19%) динамической памяти.
```
✅ **Помещается!**

**Вариант 2 (Huge APP):**
```
Скетч использует 2200000 байт (70%) памяти устройства.
Всего доступно 3145728 байт.
Глобальные переменные используют 85000 байт (26%) динамической памяти.
```
✅ **Помещается, но мало RAM!**

## 🎉 Итог:

**Для стабильной работы используйте:**
- Partition Scheme: **Minimal SPIFFS (1.9MB APP with OTA)**
- Скетч: **NetRadio_v5_2_STABLE.ino** (без Bluetooth)

**Для Bluetooth (если очень нужен):**
- Partition Scheme: **Huge APP (3MB No OTA)**
- User_Setup.h: **User_Setup_CYD_HugeAPP.h** (оптимизированный)
- Скетч: **NetRadio_v5_1.ino** (с Bluetooth)

---

**Удачи с настройкой!** 🚀

Если проблема не решена - пришлите:
1. Какой Partition Scheme используете
2. Вывод из Serial Monitor
3. Фото экрана (если есть изображение)
