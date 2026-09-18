// ============================================
// User_Setup.h для ESP32-2432S028 (CYD)
// Настройки из проекта CYD-ESP32Marauder
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

// ============================================
// ТАЧСКРИН XPT2046 (SoftSPI)
// ============================================
#define TOUCH_CS 33        // Chip select тачскрина

// SoftSPI для тачскрина (отдельная шина)
#define SOFTSPI
#define TOUCH_MOSI 32      // Отдельный MOSI для тачскрина
#define TOUCH_MISO 39      // Отдельный MISO для тачскрина
#define TOUCH_CLK 25       // Отдельный CLK для тачскрина

// ОРИЕНТАЦИЯ ДИСПЛЕЯ
// 0 = Portrait (вертикально, 240x320)
// 1 = Landscape (горизонтально, 320x240) ← РЕКОМЕНДУЕТСЯ для CYD
// 2 = Portrait (перевернуто, 240x320)
// 3 = Landscape (перевернуто, 320x240)
#define TFT_setRotation 1

// Шрифты
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

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
// 1. Откройте папку библиотеки TFT_eSPI:
//    C:\Users\[USER]\Documents\Arduino\libraries\TFT_eSPI\
//
// 2. Замените файл User_Setup.h на этот файл
//
// 3. Перезапустите Arduino IDE
//
// 4. Если дисплей показывает серую полосу:
//    - Убедитесь что используется ILI9341_2_DRIVER (не ILI9341_DRIVER!)
//    - Проверьте что TFT_INVERSION_ON включен
//    - Попробуйте изменить TFT_setRotation на 0, 2 или 3
// ============================================
