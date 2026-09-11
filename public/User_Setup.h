// =====================================================
// User_Setup.h - ULTRA OPTIMIZED for 2MB Flash ESP32
// =====================================================
// ВАЖНО: Закомментируйте ВСЕ другие драйверы!
// Только ILI9341_DRIVER должен быть активен
// =====================================================

#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS     5
#define TFT_DC     4
#define TFT_RST   15
#define TFT_MISO  -1

#define TFT_BL   -1

// КРИТИЧНО: Загрузите только необходимые шрифты!
// Каждый шрифт добавляет ~10-20KB к размеру прошивки
#define LOAD_GLCD    // Базовый шрифт (обязательно)
//#define LOAD_FONT2 // Закомментируйте если не нужен
//#define LOAD_FONT4 // Закомментируйте если не нужен
//#define LOAD_FONT6 // Закомментируйте если не нужен
//#define LOAD_FONT7 // Закомментируйте если не нужен
//#define LOAD_FONT8 // Закомментируйте если не нужен
//#define LOAD_FONT8N // Закомментируйте если не нужен
//#define LOAD_GFXFF // Закомментируйте если не нужен

#define SMOOTH_FONT

// Оптимизация SPI частоты
#define SPI_FREQUENCY  27000000  // Снижено с 40MHz для экономии
//#define SPI_READ_FREQUENCY  20000000  // Закомментируйте если не нужно

// =====================================================
// ДОПОЛНИТЕЛЬНЫЕ ОПТИМИЗАЦИИ В platform.txt:
// =====================================================
// Найдите файл platform.txt в папке ESP32:
// C:\Users\[USER]\AppData\Local\Arduino15\packages\esp32\hardware\esp32\[VERSION]\platform.txt
// 
// Найдите строку:
// compiler.c.extra_flags=
// Замените на:
// compiler.c.extra_flags=-ffunction-sections -fdata-sections -Wl,--gc-sections
//
// Найдите строку:
// compiler.cpp.extra_flags=
// Замените на:
// compiler.cpp.extra_flags=-ffunction-sections -fdata-sections -Wl,--gc-sections
//
// Это удалит неиспользуемый код и может сэкономить 50-100KB!
// =====================================================

// =====================================================
// АЛЬТЕРНАТИВА: Используйте ESP32 с 4MB Flash
// =====================================================
// Если оптимизации недостаточно, рассмотрите:
// - ESP32-WROOM-32 (4MB Flash) - стандартная плата
// - ESP32-S3 (4MB/8MB Flash)
// - ESP32-C3 (4MB Flash)
// 
// Partition Scheme для 4MB:
// "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
// =====================================================
