// ============================================
// User_Setup.h для ESP32-2432S028 (CYD)
// Настройки из Random Nerd Tutorials + CYD-ESP32Marauder
// ============================================

// Драйвер дисплея - ВАЖНО: используйте ILI9341_2_DRIVER!
// #define ILI9341_DRIVER
#define ILI9341_2_DRIVER     // Альтернативный драйвер для CYD

// Инверсия цветов (обязательно для CYD!)
#define TFT_INVERSION_ON

// Размер дисплея
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Пины дисплея (HSPI)
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1  // Не подключен к GPIO

// Подсветка дисплея
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// ОРИЕНТАЦИЯ ДИСПЛЕЯ
// 0 = Portrait (вертикально, 240x320)
// 1 = Landscape (горизонтально, 320x240) ← РЕКОМЕНДУЕТСЯ для CYD
// 2 = Portrait (перевернуто, 240x320)
// 3 = Landscape (перевернуто, 320x240)
#define TFT_setRotation 1

// ============================================
// ТАЧСКРИН XPT2046 (отдельная SPI шина VSPI)
// ============================================
// Пины тачскрина (из Random Nerd Tutorials)
#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

// Калибровка тачскрина (из Random Nerd Tutorials)
// Используйте эти значения в коде:
// x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
// y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);

// Шрифты (ОПТИМИЗИРОВАНО для экономии памяти!)
#define LOAD_GLCD   // Базовый шрифт - оставить
#define LOAD_FONT2  // Маленький шрифт - оставить
#define LOAD_FONT4  // Средний шрифт - оставить
// #define LOAD_FONT6  // ЗАКОММЕНТИРОВАНО для экономии ~15KB
// #define LOAD_FONT7  // ЗАКОММЕНТИРОВАНО для экономии ~20KB
// #define LOAD_FONT8  // ЗАКОММЕНТИРОВАНО для экономия ~25KB
// #define LOAD_GFXFF  // ЗАКОММЕНТИРОВАНО для экономии ~30KB

// #define SMOOTH_FONT  // ЗАКОММЕНТИРОВАНО для экономии ~10KB

// SPI частоты
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  16000000
#define SPI_TOUCH_FREQUENCY  2500000

// ============================================
// ВАЖНО: Закомментируйте все другие драйверы!
// ============================================
// Убедитесь что в вашем User_Setup.h закомментированы:
// #define ST7735_DRIVER
// #define ST7789_DRIVER
// #define ILI9163_DRIVER
// #define ILI9341_DRIVER  (используйте ILI9341_2_DRIVER!)
// #define S6D02A1_DRIVER
// и все другие драйверы

// ============================================
// ИНСТРУКЦИЯ ПО УСТАНОВКЕ:
// ============================================
// 1. Установите библиотеку XPT2046_Touchscreen by Paul Stoffregen
//    Library Manager → XPT2046_Touchscreen
//
// 2. Откройте папку библиотеки TFT_eSPI:
//    C:\Users\[USER]\Documents\Arduino\libraries\TFT_eSPI\
//
// 3. Замените файл User_Setup.h на этот файл
//
// 4. Перезапустите Arduino IDE
//
// 5. Если дисплей показывает серую полосу:
//    - Убедитесь что используется ILI9341_2_DRIVER
//    - Проверьте что TFT_INVERSION_ON включен
//    - Попробуйте изменить TFT_setRotation на 0, 2 или 3
//
// 6. Если тачскрин не работает:
//    - Убедитесь что установлена библиотека XPT2046_Touchscreen
//    - Проверьте что пины тачскрина указаны правильно
// ============================================
