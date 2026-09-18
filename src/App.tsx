import { useState } from 'react';

type TabType = 'overview' | 'sketch' | 'wiring' | 'stations' | 'build';

function App() {
  const [activeTab, setActiveTab] = useState<TabType>('overview');

  const tabs: { id: TabType; label: string; icon: string }[] = [
    { id: 'overview', label: 'Обзор', icon: '📻' },
    { id: 'sketch', label: 'Скетч', icon: '💻' },
    { id: 'wiring', label: 'Распиновка', icon: '🔌' },
    { id: 'stations', label: 'Станции', icon: '📡' },
    { id: 'build', label: 'Сборка', icon: '⚙️' },
  ];

  return (
    <div className="min-h-screen bg-gray-950 text-gray-100">
      <header className="bg-gradient-to-r from-yellow-900 via-orange-900 to-red-900 border-b border-yellow-500/30">
        <div className="max-w-7xl mx-auto px-4 py-6">
          <div className="flex items-center justify-between flex-wrap gap-4">
            <div>
              <h1 className="text-3xl font-bold text-yellow-400 flex items-center gap-3">
                <span className="text-4xl">📻</span> NetRadio v.4
              </h1>
              <p className="text-gray-400 mt-1">ESP32-2432S028 (CYD) • 2.8" TFT • WiFi Scanner • Bluetooth (опционально)</p>
            </div>
            <div className="flex items-center gap-2 flex-wrap">
              <span className="px-3 py-1 bg-green-900/50 text-green-400 rounded-full text-xs border border-green-700">
                ✓ CYD Ready
              </span>
              <span className="px-3 py-1 bg-blue-900/50 text-blue-400 rounded-full text-xs border border-blue-700">
                v4.2.0
              </span>
              <span className="px-3 py-1 bg-purple-900/50 text-purple-400 rounded-full text-xs border border-purple-700">
                WiFi + Bluetooth
              </span>
            </div>
          </div>
        </div>
      </header>

      <nav className="bg-gray-900/80 border-b border-gray-800 sticky top-0 z-50 backdrop-blur-sm">
        <div className="max-w-7xl mx-auto px-4">
          <div className="flex overflow-x-auto gap-1 py-2">
            {tabs.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`px-4 py-2 rounded-lg text-sm font-medium whitespace-nowrap transition-all ${
                  activeTab === tab.id
                    ? 'bg-yellow-600 text-white shadow-lg shadow-yellow-500/20'
                    : 'text-gray-400 hover:text-white hover:bg-gray-800'
                }`}
              >
                <span className="mr-2">{tab.icon}</span>
                {tab.label}
              </button>
            ))}
          </div>
        </div>
      </nav>

      <main className="max-w-7xl mx-auto px-4 py-8">
        {activeTab === 'overview' && <OverviewTab />}
        {activeTab === 'sketch' && <SketchTab />}
        {activeTab === 'wiring' && <WiringTab />}
        {activeTab === 'stations' && <StationsTab />}
        {activeTab === 'build' && <BuildTab />}
      </main>

      <footer className="bg-gray-900 border-t border-gray-800 py-6 mt-12">
        <div className="max-w-7xl mx-auto px-4 text-center text-gray-500 text-sm">
          <p>NetRadio v.4.2 • ESP32-2432S028 (CYD) • WiFi Scanner • Bluetooth A2DP • 2MB Flash Optimized</p>
          <p className="mt-1">2.8" TFT ILI9341 • SD Card • Based on yoRadio</p>
        </div>
      </footer>
    </div>
  );
}

function OverviewTab() {
  return (
    <div className="space-y-8">
      <div className="bg-gradient-to-br from-yellow-900/50 to-orange-900/50 rounded-2xl p-8 border border-yellow-500/20">
        <h2 className="text-2xl font-bold text-white mb-4">📻 NetRadio v.4.2 - ESP32-2432S028 (CYD)</h2>
        <p className="text-gray-300 leading-relaxed mb-6">
          Полноценное интернет-радио на базе платы <strong>ESP32-2432S028</strong> (Cheap Yellow Display).
          Встроенный TFT дисплей 2.8", слот SD карты, WiFi сканер на экране.
          <strong className="text-green-400"> Bluetooth A2DP теперь работает даже на 2MB Flash!</strong>
          Полностью оптимизировано с помощью макроса F() и User_Setup.h.
        </p>
        
        <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
          <FeatureCard icon="🖥️" title="2.8&quot; TFT" desc="ILI9341, 320x240, RGB. Встроен в плату!" />
          <FeatureCard icon="💾" title="SD Card" desc="Хранение до 30 станций. Встроенный слот." />
          <FeatureCard icon="👆" title="Touch" desc="Сенсорные кнопки на экране" />
          <FeatureCard icon="🔊" title="Bluetooth A2DP" desc="Работает на 2MB Flash!" />
        </div>
        
        <div className="mt-4 bg-green-900/30 border border-green-500/50 rounded-lg p-4">
          <h4 className="text-green-400 font-bold mb-2">✨ v4.2 - Полная оптимизация!</h4>
          <ul className="text-gray-300 text-sm space-y-1">
            <li>✅ <strong>Bluetooth работает на 2MB Flash!</strong> (раньше требовалось 4MB)</li>
            <li>✅ Оптимизация User_Setup.h: закомментированы неиспользуемые шрифты (~100KB экономия)</li>
            <li>✅ Макрос F() для всех строк (~150KB экономия RAM)</li>
            <li>✅ Оптимизированный HTML (~10KB экономия)</li>
            <li>✅ <strong>Итого экономия: ~260KB!</strong></li>
          </ul>
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🎯 Преимущества ESP32-2432S028</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">✅ Всё в одном</h4>
            <ul className="text-gray-400 text-sm space-y-1">
              <li>• Встроенный TFT дисплей</li>
              <li>• Встроенный слот SD карты</li>
              <li>• <strong className="text-green-400">Встроенный тачскрин XPT2046</strong></li>
              <li>• Встроенный усилитель звука</li>
              <li>• RGB LED для индикации</li>
              <li>• Датчик освещённости (LDR)</li>
              <li>• Кнопка BOOT</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">💰 Экономия</h4>
            <ul className="text-gray-400 text-sm space-y-1">
              <li>• Не нужен внешний TFT (~$10)</li>
              <li>• Не нужен внешний SD слот (~$3)</li>
              <li>• Не нужен внешний DAC (~$5)</li>
              <li>• Не нужен внешний усилитель (~$3)</li>
              <li>• <strong>Итого экономия: ~$21</strong></li>
              <li>• Цена платы: ~$15</li>
            </ul>
          </div>
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🆕 Новые возможности v.4.1</h3>
        <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
          <div className="bg-green-900/20 border border-green-700/50 rounded-lg p-4">
            <h4 className="text-green-400 font-bold mb-2">💾 SD Card Storage</h4>
            <p className="text-gray-400 text-sm">До 30 станций на SD карте. Легко добавлять/удалять.</p>
          </div>
          <div className="bg-blue-900/20 border border-blue-700/50 rounded-lg p-4">
            <h4 className="text-blue-400 font-bold mb-2">📡 WiFi Scanner</h4>
            <p className="text-gray-400 text-sm">Сканирование сетей прямо на экране TFT!</p>
          </div>
          <div className="bg-purple-900/20 border border-purple-700/50 rounded-lg p-4">
            <h4 className="text-purple-400 font-bold mb-2">⌨️ Keyboard</h4>
            <p className="text-gray-400 text-sm">Экранная клавиатура для ввода паролей WiFi.</p>
          </div>
          <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-lg p-4">
            <h4 className="text-yellow-400 font-bold mb-2">🔊 Bluetooth</h4>
            <p className="text-gray-400 text-sm">Опционально! Вкл/выкл одной строкой в коде.</p>
          </div>
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">📊 Сравнение версий</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Параметр</th>
                <th className="text-left py-2 px-3 text-purple-400">v.1 (TFT 2.4")</th>
                <th className="text-left py-2 px-3 text-emerald-400">v.2 (LCD 1602)</th>
                <th className="text-left py-2 px-3 text-yellow-400">v.3 (CYD)</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Дисплей</td><td className="py-2 px-3">TFT 2.4"</td><td className="py-2 px-3">LCD 1602</td><td className="py-2 px-3">TFT 2.8"</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Разрешение</td><td className="py-2 px-3">320x240</td><td className="py-2 px-3">16x2</td><td className="py-2 px-3">320x240</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">SD карта</td><td className="py-2 px-3">❌ Нет</td><td className="py-2 px-3">❌ Нет</td><td className="py-2 px-3 text-green-400">✅ Да (50 станций)</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Тачскрин</td><td className="py-2 px-3">❌ Нет</td><td className="py-2 px-3">❌ Нет</td><td className="py-2 px-3 text-green-400">✅ XPT2046</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Кнопки</td><td className="py-2 px-3">4 внешние</td><td className="py-2 px-3">5 внешних</td><td className="py-2 px-3">3 внешние + BOOT</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Цена</td><td className="py-2 px-3">~$25</td><td className="py-2 px-3">~$13</td><td className="py-2 px-3 text-green-400">~$15</td></tr>
              <tr><td className="py-2 px-3">Сложность</td><td className="py-2 px-3">Средняя</td><td className="py-2 px-3">Простая</td><td className="py-2 px-3 text-green-400">Очень простая</td></tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}

function SketchTab() {
  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between flex-wrap gap-4">
        <div>
          <h2 className="text-2xl font-bold text-white">💻 Arduino Скетч для ESP32-2432S028</h2>
          <p className="text-gray-400 mt-1">Полный код прошивки с SD картой и TFT дисплеем</p>
        </div>
        <div className="flex gap-3 flex-wrap">
          <a
            href="/NetRadio_v4_2_CYD.ino"
            download="NetRadio_v4_2_CYD.ino"
            className="px-6 py-3 bg-green-600 hover:bg-green-700 text-white rounded-lg transition-colors flex items-center gap-2 font-bold"
          >
            ⬇️ Скачать скетч v.4.2 (с Bluetooth!)
          </a>
          <a
            href="/User_Setup_CYD.h"
            download="User_Setup_CYD.h"
            className="px-6 py-3 bg-orange-600 hover:bg-orange-700 text-white rounded-lg transition-colors flex items-center gap-2 font-bold"
          >
            ⬇️ Скачать User_Setup.h (оптимизированный)
          </a>
          <a
            href="/OPTIMIZATION_GUIDE.md"
            download="OPTIMIZATION_GUIDE.md"
            className="px-6 py-3 bg-purple-600 hover:bg-purple-700 text-white rounded-lg transition-colors flex items-center gap-2 font-bold"
          >
            📖 Руководство по оптимизации
          </a>
        </div>
      </div>

      <div className="bg-red-900/20 border border-red-700/50 rounded-xl p-4">
        <h4 className="text-red-400 font-bold mb-2">🚨 ВАЖНО: Версия библиотеки ESP32-audioI2S</h4>
        <p className="text-gray-300 text-sm mb-2">
          <strong>НЕ используйте версию 4.0.0!</strong> Она не работает с обычным ESP32.
        </p>
        <div className="bg-gray-900 rounded-lg p-3 font-mono text-xs">
          <p className="text-green-400">✅ Правильная версия: <strong>3.0.8</strong></p>
          <p className="text-red-400">❌ Неправильная версия: <strong>4.0.0</strong> (OOM error)</p>
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">📚 Необходимые библиотеки</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">Обязательные:</h4>
            <ul className="text-gray-400 text-sm space-y-2">
              <li><code className="text-yellow-400">TFT_eSPI</code> by Bodmer (v2.5.43+)</li>
              <li><code className="text-green-400">XPT2046_Touchscreen</code> by Paul Stoffregen</li>
              <li><code className="text-yellow-400">ESP32-audioI2S</code> by schreibfaul1 <strong className="text-red-400">(v3.0.8!)</strong></li>
              <li><code className="text-blue-400">ESP32-A2DP</code> by pschatzmann <strong className="text-green-400">(для Bluetooth!)</strong></li>
              <li><code className="text-yellow-400">SD</code> (встроена)</li>
              <li><code className="text-yellow-400">SPI</code> (встроена)</li>
              <li><code className="text-yellow-400">WiFi</code> (встроена)</li>
              <li><code className="text-yellow-400">WebServer</code> (встроена)</li>
              <li><code className="text-yellow-400">Preferences</code> (встроена)</li>
            </ul>
            <h4 className="text-white font-bold mb-2 mt-4">Опциональные (для Bluetooth):</h4>
            <ul className="text-gray-400 text-sm space-y-2">
              <li><code className="text-blue-400">ESP32-A2DP</code> by pschatzmann <span className="text-gray-500">(только если #define USE_BLUETOOTH)</span></li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">Настройка TFT_eSPI:</h4>
            <p className="text-gray-400 text-sm mb-2">Скачайте готовый User_Setup_CYD.h и замените файл в библиотеке TFT_eSPI, или настройте вручную:</p>
            <pre className="bg-black rounded p-2 text-xs text-green-400 overflow-x-auto">
{`#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

// ВАЖНО: Ориентация дисплея задаётся здесь!
// 0 = Portrait, 1 = Landscape (рекомендуется), 2 = Portrait (перевернуто), 3 = Landscape (перевернуто)
#define TFT_setRotation 1

#define SPI_FREQUENCY 40000000`}
            </pre>
            <p className="text-yellow-400 text-xs mt-2">⚠️ Не используйте tft.setRotation() в коде! Используйте #define TFT_setRotation в User_Setup.h</p>
          </div>
        </div>
      </div>

      <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-xl p-4">
        <h4 className="text-yellow-400 font-bold mb-2">💡 Особенности кода:</h4>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>• <strong>SD карта</strong> для хранения станций (до 30 станций)</li>
          <li>• <strong>Встроенный TFT</strong> 2.8" ILI9341 (320x240)</li>
          <li>• <strong>Тачскрин XPT2046</strong> с 4 сенсорными кнопками</li>
          <li>• <strong>WiFi Scanner</strong> прямо на экране TFT</li>
          <li>• <strong>Экранная клавиатура</strong> для ввода паролей</li>
          <li>• <strong>Опциональный Bluetooth</strong> - включается одной строкой!</li>
          <li>• <strong>Кнопка вкл/выкл Bluetooth</strong> на экране</li>
          <li>• <strong>4 режима дисплея</strong>: громкость, температура, дата, WiFi</li>
          <li>• <strong>Погода Москвы</strong> через OpenWeatherMap</li>
          <li>• <strong>Веб-интерфейс</strong> для управления</li>
        </ul>
      </div>

      <div className="bg-green-900/20 border border-green-700/50 rounded-xl p-4">
        <h4 className="text-green-400 font-bold mb-2">🔧 Исправление проблемы с кнопками:</h4>
        <p className="text-gray-300 text-sm mb-2">
          Если кнопки на экране не реагируют на касания:
        </p>
        <ul className="text-gray-300 text-sm space-y-1 mb-3">
          <li>• Загрузите обновлённый скетч с исправленными координатами</li>
          <li>• Откройте Serial Monitor (115200 baud)</li>
          <li>• Нажмите на кнопку и смотрите отладку: <code className="bg-gray-800 px-2 py-0.5 rounded">[TOUCH] PREV button</code></li>
          <li>• Если видите координаты но не кнопку - калибруйте тачскрин</li>
        </ul>
        <a
          href="/TOUCH_FIX_GUIDE.md"
          download="TOUCH_FIX_GUIDE.md"
          className="inline-block px-4 py-2 bg-green-600 hover:bg-green-700 text-white rounded-lg transition-colors text-sm font-bold"
        >
          📥 Скачать руководство по исправлению кнопок
        </a>
      </div>

      <div className="bg-green-900/20 border border-green-700/50 rounded-xl p-4">
        <h4 className="text-green-400 font-bold mb-2">✨ Новые возможности v.4:</h4>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>• <strong>WiFi Scanner</strong> - сканирование сетей и быстрое подключение</li>
          <li>• <strong>Bluetooth A2DP</strong> - подключение Bluetooth колонок и наушников</li>
          <li>• <strong>4 сенсорные кнопки</strong> - PREV, NEXT, VOL-, VOL+</li>
          <li>• <strong>WiFi настройки в коде</strong> - удобно для первоначальной настройки</li>
          <li>• <strong>Весь экран активен</strong> - нет пустого места</li>
        </ul>
        <a
          href="/NetRadio_v4_Guide.md"
          download="NetRadio_v4_Guide.md"
          className="inline-block mt-3 px-4 py-2 bg-green-600 hover:bg-green-700 text-white rounded-lg transition-colors text-sm font-bold"
        >
          📥 Скачать полное руководство v.4
        </a>
        <a
          href="/NetRadio_v4_FIXES.md"
          download="NetRadio_v4_FIXES.md"
          className="inline-block mt-3 ml-2 px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-lg transition-colors text-sm font-bold"
        >
          🔧 Скачать руководство по исправлениям
        </a>
      </div>

      <div className="bg-orange-900/20 border border-orange-700/50 rounded-xl p-4">
        <h4 className="text-orange-400 font-bold mb-2">🖥️ Проблемы с дисплеем (серая полоса)?</h4>
        <p className="text-gray-300 text-sm mb-2">
          Если на дисплее TPM408-2.8 появляется серая полоса внизу экрана:
        </p>
        <ul className="text-gray-300 text-sm space-y-1 mb-3">
          <li>• Проверьте настройки <code className="bg-gray-800 px-2 py-0.5 rounded">User_Setup.h</code></li>
          <li>• Попробуйте разные значения <code className="bg-gray-800 px-2 py-0.5 rounded">tft.setRotation(0-3)</code></li>
          <li>• Возможно нужен драйвер <strong>ST7789</strong> вместо ILI9341</li>
          <li>• Проверьте размер дисплея в Serial Monitor</li>
        </ul>
        <a
          href="/DISPLAY_FIX_GUIDE.md"
          download="DISPLAY_FIX_GUIDE.md"
          className="inline-block px-4 py-2 bg-orange-600 hover:bg-orange-700 text-white rounded-lg transition-colors text-sm font-bold"
        >
          📥 Скачать руководство по исправлению дисплея
        </a>
      </div>
    </div>
  );
}

function WiringTab() {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">🔌 Распиновка ESP32-2432S028 (CYD)</h2>
        <p className="text-gray-400 mt-1">Встроенные компоненты и доступные GPIO</p>
      </div>

      <div className="bg-green-900/20 border border-green-700/50 rounded-xl p-4">
        <h4 className="text-green-400 font-bold mb-2">✅ Встроенные компоненты (не нужно подключать!)</h4>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>• TFT дисплей 2.8" ILI9341</li>
          <li>• Слот SD карты</li>
          <li>• Тачскрин XPT2046</li>
          <li>• Усилитель звука</li>
          <li>• RGB LED</li>
          <li>• Датчик освещённости (LDR)</li>
          <li>• Кнопка BOOT</li>
        </ul>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🖥️ TFT Display (HSPI)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Пин</th>
                <th className="text-left py-2 px-3">GPIO</th>
                <th className="text-left py-2 px-3">Назначение</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">MOSI</td><td className="py-2 px-3">GPIO 13</td><td className="py-2 px-3">SPI Data Out</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">MISO</td><td className="py-2 px-3">GPIO 12</td><td className="py-2 px-3">SPI Data In</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">SCLK</td><td className="py-2 px-3">GPIO 14</td><td className="py-2 px-3">SPI Clock</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">CS</td><td className="py-2 px-3">GPIO 15</td><td className="py-2 px-3">Chip Select</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">DC</td><td className="py-2 px-3">GPIO 2</td><td className="py-2 px-3">Data/Command</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">RST</td><td className="py-2 px-3">-1</td><td className="py-2 px-3">Не подключен</td></tr>
              <tr><td className="py-2 px-3 font-mono text-yellow-400">BL</td><td className="py-2 px-3">GPIO 21</td><td className="py-2 px-3">Backlight</td></tr>
            </tbody>
          </table>
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">💾 SD Card (VSPI)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Пин</th>
                <th className="text-left py-2 px-3">GPIO</th>
                <th className="text-left py-2 px-3">Назначение</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">MOSI</td><td className="py-2 px-3">GPIO 23</td><td className="py-2 px-3">SPI Data Out</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">MISO</td><td className="py-2 px-3">GPIO 19</td><td className="py-2 px-3">SPI Data In</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">SCLK</td><td className="py-2 px-3">GPIO 18</td><td className="py-2 px-3">SPI Clock</td></tr>
              <tr><td className="py-2 px-3 font-mono text-yellow-400">CS</td><td className="py-2 px-3">GPIO 5</td><td className="py-2 px-3">Chip Select</td></tr>
            </tbody>
          </table>
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🔘 Кнопки (внешние)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Кнопка</th>
                <th className="text-left py-2 px-3">GPIO</th>
                <th className="text-left py-2 px-3">Функция</th>
                <th className="text-left py-2 px-3">Коннектор</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN_PREV</td><td className="py-2 px-3">GPIO 35</td><td className="py-2 px-3">◀ Предыдущая станция</td><td className="py-2 px-3">P3 (нужен pull-up 10k)</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN_NEXT</td><td className="py-2 px-3">GPIO 34</td><td className="py-2 px-3">▶ Следующая станция</td><td className="py-2 px-3">LDR (нужен pull-up 10k)</td></tr>
              <tr><td className="py-2 px-3 font-mono text-cyan-400">BTN_VOL_UP</td><td className="py-2 px-3">GPIO 0</td><td className="py-2 px-3">🔊 Громкость +</td><td className="py-2 px-3">BOOT (встроенная)</td></tr>
            </tbody>
          </table>
        </div>
        <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-lg p-3 mt-3">
          <p className="text-yellow-400 text-sm font-bold">⚠️ Важно: GPIO 34 и 35 не имеют внутреннего pull-up!</p>
          <p className="text-gray-400 text-xs mt-1">Подключите внешние резисторы 10kΩ между GPIO и 3.3V для кнопок PREV и NEXT.</p>
          <p className="text-gray-400 text-xs">Или используйте веб-интерфейс для управления.</p>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Подключение: GPIO → Кнопка → GND. Для GPIO 34/35 нужен внешний pull-up 10kΩ к 3.3V.</p>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🔊 I2S DAC (внешний, подключается к CN1)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">DAC Пин</th>
                <th className="text-left py-2 px-3">GPIO</th>
                <th className="text-left py-2 px-3">Назначение</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">BCLK</td><td className="py-2 px-3">GPIO 25</td><td className="py-2 px-3">Bit Clock</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">LRC</td><td className="py-2 px-3">GPIO 27</td><td className="py-2 px-3">Word Select</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">DIN</td><td className="py-2 px-3">GPIO 22</td><td className="py-2 px-3">Serial Data</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">VCC</td><td className="py-2 px-3">3.3V</td><td className="py-2 px-3">Питание</td></tr>
              <tr><td className="py-2 px-3 font-mono text-green-400">GND</td><td className="py-2 px-3">GND</td><td className="py-2 px-3">Земля</td></tr>
            </tbody>
          </table>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Рекомендуется MAX98357A или PCM5102. Подключается к коннектору CN1.</p>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🎨 RGB LED (встроенный)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Цвет</th>
                <th className="text-left py-2 px-3">GPIO</th>
                <th className="text-left py-2 px-3">Логика</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-red-400">Red</td><td className="py-2 px-3">GPIO 4</td><td className="py-2 px-3">Active LOW (0=ON, 1=OFF)</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">Green</td><td className="py-2 px-3">GPIO 16</td><td className="py-2 px-3">Active LOW</td></tr>
              <tr><td className="py-2 px-3 font-mono text-blue-400">Blue</td><td className="py-2 px-3">GPIO 17</td><td className="py-2 px-3">Active LOW</td></tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}

function StationsTab() {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">📡 Радиостанции на SD карте</h2>
        <p className="text-gray-400 mt-1">До 50 станций хранятся на SD карте</p>
      </div>

      <div className="bg-green-900/20 border border-green-700/50 rounded-xl p-4">
        <h4 className="text-green-400 font-bold mb-2">💾 Формат файла stations.txt</h4>
        <p className="text-gray-300 text-sm mb-2">Создайте файл <code className="bg-gray-800 px-2 py-0.5 rounded">stations.txt</code> в корне SD карты:</p>
        <pre className="bg-black rounded-lg p-3 text-xs text-gray-300 overflow-x-auto">
{`Record|https://radiorecord.hostingradio.ru/rr_320
RusMix|https://radiorecord.hostingradio.ru/rusmix_320
90s|https://radiorecord.hostingradio.ru/sd90_320
Chill|https://radiorecord.hostingradio.ru/chil_320
Deep|https://radiorecord.hostingradio.ru/deep_320`}
        </pre>
        <p className="text-gray-400 text-sm mt-2">Формат: <code className="bg-gray-800 px-2 py-0.5 rounded">Название|URL</code> (одна станция на строку)</p>
      </div>

      <div className="bg-blue-900/20 border border-blue-700/50 rounded-xl p-4">
        <h4 className="text-blue-400 font-bold mb-2">📋 Станции по умолчанию</h4>
        <p className="text-gray-300 text-sm mb-2">Если SD карта не найдена или файл stations.txt отсутствует, загружаются 20 станций по умолчанию:</p>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-2 text-sm">
          {['Record', 'RusMix', '90s', 'Chill', 'Deep', 'Rock', 'Rap', 'Techno', 'House', 'EDM', 'TM', 'Pirate', 'Dub', 'Synth', 'LoFi', 'Euro', 'Trap', 'Hard', 'Amb', 'RusHits'].map((name, i) => (
            <div key={i} className="bg-gray-800/50 rounded px-2 py-1 text-gray-300">
              {i+1}. {name}
            </div>
          ))}
        </div>
      </div>

      <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-xl p-4">
        <h4 className="text-yellow-400 font-bold mb-2">🌐 Управление через веб-интерфейс</h4>
        <p className="text-gray-300 text-sm">
          Вы можете добавлять, редактировать и удалять станции через веб-интерфейс.
          Все изменения автоматически сохраняются на SD карту в файл <code className="bg-gray-800 px-2 py-0.5 rounded">stations.txt</code>.
        </p>
      </div>
    </div>
  );
}

function BuildTab() {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">⚙️ Сборка и прошивка</h2>
        <p className="text-gray-400 mt-1">Пошаговая инструкция для ESP32-2432S028</p>
      </div>

      <div className="space-y-4">
        <StepCard
          num={1}
          title="Подготовьте SD карту"
          content="Отформатируйте microSD карту в FAT32.\nСоздайте файл stations.txt со списком станций (опционально).\nВставьте карту в слот на плате."
        />
        <StepCard
          num={2}
          title="Установите Arduino IDE и ESP32 Core"
          content="Скачайте Arduino IDE 2.x.\nДобавьте URL в настройки: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json\nУстановите 'esp32 by Espressif Systems' через Boards Manager."
        />
        <StepCard
          num={3}
          title="Установите библиотеки"
          content="Library Manager (Ctrl+Shift+I):\n• TFT_eSPI by Bodmer (v2.5.43+)\n• ESP32-audioI2S by schreibfaul1 (v3.0.8! НЕ 4.0.0)\n\nНастройте User_Setup.h для TFT_eSPI (см. вкладка 'Скетч')."
        />
        <StepCard
          num={4}
          title="Подключите кнопки"
          content="Подключите 2 внешние кнопки (или используйте веб-интерфейс):\n• GPIO 35 → Кнопка PREV → GND (нужен pull-up 10kΩ к 3.3V)\n• GPIO 34 → Кнопка NEXT → GND (нужен pull-up 10kΩ к 3.3V)\n\nКнопка VOL+ - встроенная (BOOT).\n\nИли используйте веб-интерфейс для управления!"
        />
        <StepCard
          num={5}
          title="Загрузите прошивку"
          content="Откройте NetRadio_v4_1_CYD.ino в Arduino IDE.\n\nВАЖНО: Для включения Bluetooth раскомментируйте строку:\n#define USE_BLUETOOTH\n(только для плат с 4MB Flash!)\n\nВыберите плату: ESP32 Dev Module.\nPartition Scheme: Default 4MB with spiffs.\nUpload Speed: 921600 (или 115200 если проблемы).\nНажмите Upload."
        />
        <StepCard
          num={6}
          title="Первый запуск"
          content="После прошивки:\n• RGB LED загорится синим (старт)\n• TFT покажет 'Connecting WiFi...'\n• Если WiFi не настроен, создастся AP 'NetRadio' (пароль: netradio123)\n• Подключитесь и откройте http://192.168.4.1"
        />
        <StepCard
          num={7}
          title="Настройка WiFi"
          content="В веб-интерфейсе укажите SSID и пароль вашей сети.\nУстройство перезагрузится и подключится.\nRGB LED загорится зелёным (готово).\nTFT покажет IP адрес и статус."
        />
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">🔧 Решение проблем</h3>
        <div className="space-y-4">
          <TroubleItem
            problem="TFT дисплей не работает"
            solution="Проверьте User_Setup.h для TFT_eSPI. Убедитесь что указаны правильные пины (13, 12, 14, 15, 2)."
          />
          <TroubleItem
            problem="SD карта не обнаружена"
            solution="Проверьте что карта отформатирована в FAT32. Попробуйте другую карту (до 32GB). Проверьте контакты слота."
          />
          <TroubleItem
            problem="Нет звука"
            solution="Проверьте подключение динамика к разъёму J1 (Speaker). Убедитесь что версия библиотеки ESP32-audioI2S = 3.0.8."
          />
          <TroubleItem
            problem="OOM: failed to allocate 720896 bytes"
            solution="Вы используете версию 4.0.0 библиотеки ESP32-audioI2S. Удалите её и установите версию 3.0.8!"
          />
          <TroubleItem
            problem="Кнопки не работают"
            solution="GPIO 34 и 35 не имеют внутреннего pull-up! Подключите внешние резисторы 10kΩ между GPIO и 3.3V. Или используйте веб-интерфейс."
          />
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-yellow-400 mb-4">💰 Стоимость компонентов</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Компонент</th>
                <th className="text-left py-2 px-3">Цена</th>
                <th className="text-left py-2 px-3">Примечание</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3">ESP32-2432S028R (CYD)</td><td className="py-2 px-3">$15</td><td className="py-2 px-3">Всё в одном!</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">I2S DAC (MAX98357A)</td><td className="py-2 px-3">$3</td><td className="py-2 px-3">Для качественного звука</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">microSD карта 8GB</td><td className="py-2 px-3">$3</td><td className="py-2 px-3">Опционально</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Динамик 3W</td><td className="py-2 px-3">$2</td><td className="py-2 px-3">4-8 Ohm</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">2 кнопки + резисторы</td><td className="py-2 px-3">$1</td><td className="py-2 px-3">Опционально</td></tr>
              <tr><td className="py-2 px-3 font-bold">ИТОГО</td><td className="py-2 px-3 font-bold text-green-400">~$24</td><td className="py-2 px-3">Всё включено!</td></tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}

// Helper components
function FeatureCard({ icon, title, desc }: { icon: string; title: string; desc: string }) {
  return (
    <div className="bg-gray-900/50 rounded-xl p-4 border border-gray-700">
      <div className="text-3xl mb-2">{icon}</div>
      <h4 className="text-white font-bold mb-1">{title}</h4>
      <p className="text-gray-400 text-sm">{desc}</p>
    </div>
  );
}

function StepCard({ num, title, content }: { num: number; title: string; content: string }) {
  return (
    <div className="flex gap-4 bg-gray-900 rounded-xl p-5 border border-gray-800">
      <div className="flex-shrink-0 w-10 h-10 bg-yellow-600 rounded-full flex items-center justify-center text-white font-bold">
        {num}
      </div>
      <div>
        <h4 className="text-white font-bold mb-2">{title}</h4>
        <p className="text-gray-400 text-sm whitespace-pre-line">{content}</p>
      </div>
    </div>
  );
}

function TroubleItem({ problem, solution }: { problem: string; solution: string }) {
  return (
    <div className="bg-gray-800/50 rounded-lg p-3">
      <p className="text-red-400 font-medium text-sm">❌ {problem}</p>
      <p className="text-gray-400 text-sm mt-1">✅ {solution}</p>
    </div>
  );
}

export default App;
