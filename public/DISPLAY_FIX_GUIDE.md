# 🔧 Исправление серой полосы на дисплее TPM408-2.8

## ❌ Проблема

Серая полоса внизу экрана, занимающая 25-50% дисплея.

## 🔍 Причина

Проблема в неправильной настройке дисплея в библиотеке TFT_eSPI. Маркировка TPM408-2.8 указывает на дисплей с контроллером ILI9341, но нужно правильно настроить разрешение и ориентацию.

## ✅ Решение

### Шаг 1: Проверьте User_Setup.h

Откройте файл `User_Setup.h` в библиотеке TFT_eSPI и убедитесь что указаны правильные настройки:

```cpp
// ВАЖНО: Используйте ILI9341_DRIVER
#define ILI9341_DRIVER

// Правильное разрешение для 2.8" дисплея
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Пины CYD
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

#define SPI_FREQUENCY  40000000
```

### Шаг 2: Попробуйте разные значения rotation

В скетче найдите строку:
```cpp
tft.setRotation(1);
```

Попробуйте разные значения:
- `tft.setRotation(0);` - Portrait (вертикально)
- `tft.setRotation(1);` - Landscape (горизонтально) ← обычно для CYD
- `tft.setRotation(2);` - Portrait (перевернуто)
- `tft.setRotation(3);` - Landscape (перевернуто)

### Шаг 3: Если проблема сохраняется - попробуйте ST7789

Некоторые дисплеи TPM408-2.8 используют контроллер ST7789 вместо ILI9341.

В `User_Setup.h` замените:
```cpp
// Закомментируйте ILI9341
// #define ILI9341_DRIVER

// Раскомментируйте ST7789
#define ST7789_DRIVER

// Для ST7789 нужно указать дополнительные параметры
#define TFT_RGB_ORDER TFT_RGB  // Порядок цветов

#define TFT_WIDTH  240
#define TFT_HEIGHT 320
```

### Шаг 4: Проверьте размер дисплея

Загрузите скетч и откройте Serial Monitor (115200 baud). Вы увидите:
```
[OK] TFT initialized
TFT size: 320x240
```

Если размер неправильный (например, 240x240), значит проблема в настройках.

### Шаг 5: Альтернативное решение - используйте TFT_eSPI с автоматическим определением

Некоторые версии библиотеки TFT_eSPI поддерживают автоматическое определение дисплея. Попробуйте обновить библиотеку до последней версии.

## 📋 Полная настройка User_Setup.h для CYD

```cpp
// ============================================
// User_Setup.h для ESP32-2432S028 (CYD)
// ============================================

// Вариант 1: ILI9341 (обычно для CYD)
#define ILI9341_DRIVER

// Вариант 2: ST7789 (если ILI9341 не работает)
// #define ST7789_DRIVER
// #define TFT_RGB_ORDER TFT_RGB

// Размер дисплея
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Пины CYD
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

// Подсветка
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// Шрифты
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// SPI частота
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
```

## 🔧 Если ничего не помогло

### Проверьте версию библиотеки TFT_eSPI

Убедитесь что используете последнюю версию библиотеки TFT_eSPI (v2.5.43 или новее).

### Попробуйте другой пример

Загрузите пример из библиотеки TFT_eSPI:
- File → Examples → TFT_eSPI → 320 x 240 → TFT_Print_Test

Если пример работает правильно, значит проблема в настройках скетча NetRadio.

### Проверьте подключение

Убедитесь что все пины подключены правильно:
- MOSI → GPIO 13
- MISO → GPIO 12
- SCLK → GPIO 14
- CS → GPIO 15
- DC → GPIO 2
- BL → GPIO 21

## 📊 Диагностика

Загрузите скетч и проверьте Serial Monitor:

**Правильный вывод:**
```
[OK] TFT initialized
TFT size: 320x240
```

**Неправильный вывод:**
```
[OK] TFT initialized
TFT size: 240x240  ← Неправильно!
```

Если размер неправильный, попробуйте:
1. Изменить rotation (0, 1, 2, 3)
2. Использовать ST7789 вместо ILI9341
3. Проверить User_Setup.h

## 🎯 Рекомендуемое решение

Для дисплея TPM408-2.8 на плате CYD:

1. **Используйте ILI9341_DRIVER**
2. **Установите rotation(1)** для landscape режима
3. **Убедитесь что TFT_WIDTH = 240 и TFT_HEIGHT = 320**
4. **SPI_FREQUENCY = 40000000**

Если серая полоса остается, попробуйте **ST7789_DRIVER** с теми же пинами.

---

**Удачи с настройкой дисплея!** 🖥️

Если проблема не решена, напишите:
1. Версию библиотеки TFT_eSPI
2. Значение rotation
3. Вывод Serial Monitor (размер дисплея)
4. Фотографию дисплея с серой полосой
