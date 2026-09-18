// ============================================
// User_Setup.h для ESP32-2432S028 (CYD)
// ОПТИМИЗИРОВАНО для Huge APP (3MB No OTA)
// ============================================

// Драйвер дисплея
#define ILI9341_2_DRIVER

// Инверсия цветов
#define TFT_INVERSION_ON

// Размер дисплея
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Пины дисплея
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

// Подсветка
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// ОРИЕНТАЦИЯ
#define TFT_setRotation 1

// ============================================
// ОПТИМИЗАЦИЯ RAM - ВАЖНО для Huge APP!
// ============================================

// Оставляем ТОЛЬКО базовые шрифты
#define LOAD_GLCD    // Базовый шрифт
#define LOAD_FONT2   // Маленький шрифт
#define LOAD_FONT4   // Средний шрифт

// ВСЁ ОСТАЛЬНОЕ ЗАКОММЕНТИРОВАНО для экономии RAM!
// #define LOAD_FONT6
// #define LOAD_FONT7
// #define LOAD_FONT8
// #define LOAD_GFXFF
// #define SMOOTH_FONT

// ============================================
// ОПТИМИЗАЦИЯ SPI для стабильности
// ============================================

// Снижена частота для стабильности при нехватке RAM
#define SPI_FREQUENCY  27000000  // Было 40MHz, стало 27MHz
#define SPI_READ_FREQUENCY  16000000

// ============================================
// ТАЧСКРИН
// ============================================
#define TOUCH_CS 33
#define SOFTSPI
#define TOUCH_MOSI 32
#define TOUCH_MISO 39
#define TOUCH_CLK 25
#define SPI_TOUCH_FREQUENCY  2500000

// ============================================
// ВАЖНО: Если экран тёмный при Huge APP:
// ============================================
// 1. Убедитесь что закомментированы все лишние шрифты
// 2. SPI_FREQUENCY снижен до 27MHz
// 3. Если всё равно не работает - используйте Minimal SPIFFS без Bluetooth
