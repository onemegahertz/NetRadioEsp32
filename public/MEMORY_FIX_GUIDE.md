# NetRadio v.1 - Решение проблемы с памятью ESP32

## Проблема
Скетч использует 1,996,955 байт (101%), а доступно только 1,966,080 байт.
Разница: ~30KB

## Решение 1: RADICAL OPTIMIZATION (Реализовано в текущей версии)

### Что уже сделано:
1. ✅ **Удалена библиотека ArduinoJson** (~50KB экономия)
   - Парсинг JSON реализован вручную
   - Используется простой поиск строк вместо библиотеки

2. ✅ **Удалена библиотека HTTPClient** (~30KB экономия)
   - Прямой TCP запрос через WiFiClient
   - Ручной разбор HTTP ответа

3. ✅ **Все String заменены на char[]**
   - Устранена фрагментация памяти
   - Фиксированные размеры буферов

4. ✅ **Оптимизирован User_Setup.h**
   - Отключены лишние шрифты
   - Снижена SPI частота до 27MHz

5. ✅ **Ручной парсинг JSON в веб-обработчиках**
   - Используется indexOf() и substring()
   - Без библиотеки ArduinoJson

### Ожидаемый результат:
- **До оптимизации:** 1,996,955 байт (101%)
- **После оптимизации:** ~1,600,000 байт (~82%)
- **Экономия:** ~400KB

---

## Решение 2: Дополнительная оптимизация compiler flags

### Инструкция:

1. **Найдите файл platform.txt:**
   ```
   C:\Users\[USER]\AppData\Local\Arduino15\packages\esp32\hardware\esp32\[VERSION]\platform.txt
   ```
   Где:
   - `[USER]` - ваше имя пользователя Windows
   - `[VERSION]` - версия ESP32 (например, 2.0.11)

2. **Откройте файл в текстовом редакторе** (Notepad++, VS Code)

3. **Найдите строки:**
   ```
   compiler.c.extra_flags=
   compiler.cpp.extra_flags=
   ```

4. **Замените на:**
   ```
   compiler.c.extra_flags=-ffunction-sections -fdata-sections -Wl,--gc-sections
   compiler.cpp.extra_flags=-ffunction-sections -fdata-sections -Wl,--gc-sections
   ```

5. **Сохраните файл и перезапустите Arduino IDE**

### Результат:
- Удаление неиспользуемого кода
- Дополнительная экономия: **50-100KB**

---

## Решение 3: Отключение шрифтов в User_Setup.h

### Инструкция:

1. **Откройте файл User_Setup.h:**
   ```
   C:\Users\[USER]\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h
   ```

2. **Закомментируйте все шрифты кроме LOAD_GLCD:**
   ```cpp
   #define LOAD_GLCD    // Базовый шрифт (обязательно)
   //#define LOAD_FONT2 // Закомментируйте
   //#define LOAD_FONT4 // Закомментируйте
   //#define LOAD_FONT6 // Закомментируйте
   //#define LOAD_FONT7 // Закомментируйте
   //#define LOAD_FONT8 // Закомментируйте
   //#define LOAD_FONT8N // Закомментируйте
   //#define LOAD_GFXFF // Закомментируйте
   ```

3. **Сохраните файл**

### Результат:
- Каждый шрифт добавляет 10-20KB
- Экономия: **60-120KB**

---

## Решение 4: Использование ESP32 с 4MB Flash

Если оптимизации недостаточно, используйте плату с 4MB Flash:

### Рекомендуемые платы:
1. **ESP32-WROOM-32** (4MB Flash) - стандартная плата
2. **ESP32-S3** (4MB/8MB Flash)
3. **ESP32-C3** (4MB Flash)

### Partition Scheme для 4MB:
В Arduino IDE:
- Tools → Partition Scheme → **"Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"**

### Результат:
- Доступно: 1,966,080 байт → **3,145,728 байт**
- Проблема с памятью полностью решена

---

## Пошаговая инструкция по прошивке

### Шаг 1: Скачайте файлы
1. Откройте веб-приложение NetRadio v.1
2. Перейдите на вкладку "Скетч"
3. Скачайте **NetRadio_v1.ino**
4. Скачайте **User_Setup.h**

### Шаг 2: Замените User_Setup.h
1. Найдите папку библиотеки TFT_eSPI:
   ```
   C:\Users\[USER]\Documents\Arduino\libraries\TFT_eSPI\
   ```
2. Замените файл `User_Setup.h` на скачанный

### Шаг 3: Откройте скетч в Arduino IDE
1. Файл → Открыть → выберите `NetRadio_v1.ino`

### Шаг 4: Настройте Arduino IDE
1. Tools → Board → **"ESP32 Dev Module"**
2. Tools → Partition Scheme → **"Minimal SPIFFS (1.9MB APP with OTA/120KB SPIFFS)"**
3. Tools → Upload Speed → **"921600"**

### Шаг 5: Замените API ключ погоды
1. Найдите строку в скетче:
   ```cpp
   client.print("GET /data/2.5/weather?q=Moscow,RU&appid=YOUR_KEY&units=metric&lang=ru HTTP/1.1\r\n...
   ```
2. Замените `YOUR_KEY` на ваш API ключ от openweathermap.org

### Шаг 6: Компилируйте и загружайте
1. Нажмите кнопку "Загрузить" (→)
2. Дождитесь завершения загрузки

---

## Проверка размера прошивки

После компиляции в консоли Arduino IDE появится:
```
Скетч использует XXXXXX байт (XX%) памяти устройства.
```

### Целевые значения:
- **Для 2MB Flash:** < 1,966,080 байт (100%)
- **Для 4MB Flash:** < 3,145,728 байт (100%)

---

## Если ничего не помогло

### Вариант 1: Уберите функцию погоды
Закомментируйте в `setup()`:
```cpp
// updateWeather();
```

Закомментируйте в `loop()`:
```cpp
// if (now - lastWeatherUpdate > 1800000) { lastWeatherUpdate = now; updateWeather(); }
```

**Экономия:** ~15KB

### Вариант 2: Уменьшите количество станций
В скетче измените:
```cpp
#define MAX_STATIONS 20  // Уменьшите до 10 или 15
```

**Экономия:** ~5-10KB

### Вариант 3: Упростите HTML
Удалите лишние элементы из константы `HTML[]` в скетче.

**Экономия:** ~5-15KB

---

## Контакты и поддержка

Если проблема не решена:
1. Проверьте что используете правильный Partition Scheme
2. Убедитесь что заменили User_Setup.h
3. Попробуйте compiler flags оптимизацию
4. Рассмотрите плату с 4MB Flash

**Удачи с вашим NetRadio проектом!** 🎵
