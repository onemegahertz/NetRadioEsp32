import { useState } from 'react';
import { radioRecordStations, defaultStations } from './data/stations';

type TabType = 'overview' | 'sketch' | 'wiring' | 'stations' | 'build' | 'v1';

function App() {
  const [activeTab, setActiveTab] = useState<TabType>('overview');

  const tabs: { id: TabType; label: string; icon: string }[] = [
    { id: 'overview', label: 'Обзор', icon: '📻' },
    { id: 'sketch', label: 'Скетч', icon: '💻' },
    { id: 'wiring', label: 'Схема', icon: '🔌' },
    { id: 'stations', label: 'Станции', icon: '📡' },
    { id: 'build', label: 'Сборка', icon: '⚙️' },
  ];

  return (
    <div className="min-h-screen bg-gray-950 text-gray-100">
      {/* Header */}
      <header className="bg-gradient-to-r from-emerald-900 via-teal-900 to-cyan-900 border-b border-emerald-500/30">
        <div className="max-w-7xl mx-auto px-4 py-6">
          <div className="flex items-center justify-between flex-wrap gap-4">
            <div>
              <h1 className="text-3xl font-bold text-emerald-400 flex items-center gap-3">
                <span className="text-4xl">📻</span> NetRadio v.2
              </h1>
              <p className="text-gray-400 mt-1">ESP32 • LCD 1602 I2C • Based on yoRadio</p>
            </div>
            <div className="flex items-center gap-2 flex-wrap">
              <span className="px-3 py-1 bg-green-900/50 text-green-400 rounded-full text-xs border border-green-700">
                ✓ ESP32 Ready
              </span>
              <span className="px-3 py-1 bg-blue-900/50 text-blue-400 rounded-full text-xs border border-blue-700">
                v2.0.0
              </span>
              <span className="px-3 py-1 bg-purple-900/50 text-purple-400 rounded-full text-xs border border-purple-700">
                LCD 1602 I2C
              </span>
            </div>
          </div>
        </div>
      </header>

      {/* Navigation */}
      <nav className="bg-gray-900/80 border-b border-gray-800 sticky top-0 z-50 backdrop-blur-sm">
        <div className="max-w-7xl mx-auto px-4">
          <div className="flex overflow-x-auto gap-1 py-2">
            {tabs.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`px-4 py-2 rounded-lg text-sm font-medium whitespace-nowrap transition-all ${
                  activeTab === tab.id
                    ? 'bg-emerald-600 text-white shadow-lg shadow-emerald-500/20'
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

      {/* Content */}
      <main className="max-w-7xl mx-auto px-4 py-8">
        {activeTab === 'overview' && <OverviewTab />}
        {activeTab === 'sketch' && <SketchTab />}
        {activeTab === 'wiring' && <WiringTab />}
        {activeTab === 'stations' && <StationsTab />}
        {activeTab === 'build' && <BuildTab />}
      </main>

      {/* Footer */}
      <footer className="bg-gray-900 border-t border-gray-800 py-6 mt-12">
        <div className="max-w-7xl mx-auto px-4 text-center text-gray-500 text-sm">
          <p>NetRadio v.2 • ESP32 + LCD 1602 I2C Internet Radio • Based on yoRadio project</p>
          <p className="mt-1">ESP32 • LiquidCrystal_I2C • ESP32-audioI2S • Web Interface</p>
        </div>
      </footer>
    </div>
  );
}

function OverviewTab() {
  return (
    <div className="space-y-8">
      {/* Hero */}
      <div className="bg-gradient-to-br from-emerald-900/50 to-teal-900/50 rounded-2xl p-8 border border-emerald-500/20">
        <h2 className="text-2xl font-bold text-white mb-4">📻 NetRadio v.2 - ESP32 Internet Radio</h2>
        <p className="text-gray-300 leading-relaxed mb-6">
          Интернет-радио на базе ESP32 с LCD дисплеем 1602 (I2C интерфейс). Создано на основе идей проекта <a href="https://github.com/e2002/yoradio" className="text-emerald-400 hover:underline" target="_blank">yoRadio</a>.
          Простая сборка, красивый веб-интерфейс, 20 радиостанций по умолчанию.
        </p>
        
        <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
          <FeatureCard icon="🖥️" title="LCD 1602 I2C" desc="Синий дисплей 16x2 с I2C модулем. 4 режима отображения." />
          <FeatureCard icon="⚡" title="ESP32 Power" desc="Мощный процессор, WiFi. 4MB Flash, 520KB RAM." />
          <FeatureCard icon="🌤️" title="Погода" desc="Температура Москвы с OpenWeatherMap. Обновление каждые 30 мин." />
          <FeatureCard icon="🎛️" title="5 кнопок" desc="Prev, Next, Vol+, Vol-, Mode. Переключение режимов дисплея." />
        </div>
      </div>

      {/* Platform support */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🔧 Поддерживаемые платформы</h3>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          <PlatformCard
            name="NodeMCU / Wemos (ESP8266)"
            icon="📡"
            features={[
              "Встроенный WiFi",
              "I2S DAC (MAX98357A)",
              "Прямое подключение LCD",
              "Рекомендуется!",
            ]}
            recommended={true}
          />
          <PlatformCard
            name="Arduino UNO / Nano"
            icon="🔌"
            features={[
              "Нужен WiFi модуль (ESP01)",
              "I2C LCD напрямую",
              "Ограниченная память",
              "Для простых проектов",
            ]}
            recommended={false}
          />
          <PlatformCard
            name="STM32 (Blue/Black Pill)"
            icon="⚡"
            features={[
              "Нужен WiFi модуль",
              "Мощный процессор",
              "I2C LCD напрямую",
              "Для продвинутых",
            ]}
            recommended={false}
          />
        </div>
      </div>

      {/* Features from yoRadio */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">✨ Взято из yoRadio</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">🌐 Веб-интерфейс</h4>
            <ul className="text-gray-400 text-sm space-y-1">
              <li>• Добавление/удаление станций</li>
              <li>• Редактирование списка</li>
              <li>• Управление воспроизведением</li>
              <li>• Настройка WiFi</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">💾 Сохранение настроек</h4>
            <ul className="text-gray-400 text-sm space-y-1">
              <li>• Список станций в EEPROM/Preferences</li>
              <li>• WiFi настройки</li>
              <li>• Последняя станция</li>
              <li>• Уровень громкости</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">🎛️ Управление</h4>
            <ul className="text-gray-400 text-sm space-y-1">
              <li>• 3 кнопки: Prev, Next, Vol</li>
              <li>• Опциональный энкодер</li>
              <li>• Debounce защита</li>
              <li>• Плавная регулировка</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">📻 Радиостанции</h4>
            <ul className="text-gray-400 text-sm space-y-1">
              <li>• До 20 станций по умолчанию</li>
              <li>• Radio Record потоки</li>
              <li>• Добавление своих станций</li>
              <li>• До 65535 в yoRadio</li>
            </ul>
          </div>
        </div>
      </div>

      {/* LCD Preview */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🖥️ Превью LCD 1602 (время всегда видно!)</h3>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <div className="text-center">
            <div className="bg-blue-900 border-4 border-gray-700 rounded-lg p-2 font-mono">
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider">
                <div>Record  14:35:22</div>
              </div>
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider mt-1">
                <div>Vol:12/21</div>
              </div>
            </div>
            <p className="text-gray-400 text-xs mt-2">Режим 1: Громкость</p>
          </div>
          <div className="text-center">
            <div className="bg-blue-900 border-4 border-gray-700 rounded-lg p-2 font-mono">
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider">
                <div>Record  14:35:22</div>
              </div>
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider mt-1">
                <div>Temp:-5.2C</div>
              </div>
            </div>
            <p className="text-gray-400 text-xs mt-2">Режим 2: Температура</p>
          </div>
          <div className="text-center">
            <div className="bg-blue-900 border-4 border-gray-700 rounded-lg p-2 font-mono">
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider">
                <div>Record  14:35:22</div>
              </div>
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider mt-1">
                <div>Date:15.01.24</div>
              </div>
            </div>
            <p className="text-gray-400 text-xs mt-2">Режим 3: Дата</p>
          </div>
          <div className="text-center">
            <div className="bg-blue-900 border-4 border-gray-700 rounded-lg p-2 font-mono">
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider">
                <div>Record  14:35:22</div>
              </div>
              <div className="bg-blue-500 text-blue-900 p-1 rounded font-bold text-sm tracking-wider mt-1">
                <div>IP:192.168.1</div>
              </div>
            </div>
            <p className="text-gray-400 text-xs mt-2">Режим 4: WiFi</p>
          </div>
        </div>
        <div className="mt-4 text-center text-gray-400 text-sm">
          <p className="text-cyan-400 font-bold">⏰ Время (HH:MM:SS) всегда видно в правом верхнем углу!</p>
          <p>Строка 1: Название станции + Время</p>
          <p>Строка 2: Переключается кнопкой MODE (Громкость → Температура → Дата → WiFi)</p>
        </div>
      </div>

      {/* Comparison */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">📊 Сравнение v.1 и v.2</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Параметр</th>
                <th className="text-left py-2 px-3 text-purple-400">NetRadio v.1</th>
                <th className="text-left py-2 px-3 text-emerald-400">NetRadio v.2</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Дисплей</td><td className="py-2 px-3">TFT 2.4" ILI9341</td><td className="py-2 px-3">LCD 1602 I2C</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Платформы</td><td className="py-2 px-3">ESP32</td><td className="py-2 px-3">Arduino/ESP8266/STM32</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Кнопки</td><td className="py-2 px-3">4 кнопки</td><td className="py-2 px-3">3 кнопки + энкодер</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Память</td><td className="py-2 px-3">~1.3MB Flash</td><td className="py-2 px-3">~500KB Flash</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Сложность</td><td className="py-2 px-3">Средняя</td><td className="py-2 px-3">Простая</td></tr>
              <tr><td className="py-2 px-3">Цена</td><td className="py-2 px-3">~$15</td><td className="py-2 px-3">~$8</td></tr>
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
          <h2 className="text-2xl font-bold text-white">💻 Arduino Скетч для ESP32</h2>
          <p className="text-gray-400 mt-1">Полный код прошивки для ESP32 + LCD 1602 I2C</p>
        </div>
        <div className="flex gap-3 flex-wrap">
          <a
            href="/NetRadio_v2_ESP32.ino"
            download="NetRadio_v2_ESP32.ino"
            className="px-6 py-3 bg-emerald-600 hover:bg-emerald-700 text-white rounded-lg transition-colors flex items-center gap-2 font-bold"
          >
            ⬇️ Скачать NetRadio_v2_ESP32.ino
          </a>
          <a
            href="/README_ESP32.md"
            download="README_ESP32.md"
            className="px-6 py-3 bg-blue-600 hover:bg-blue-700 text-white rounded-lg transition-colors flex items-center gap-2 font-bold"
          >
            📖 Скачать инструкцию
          </a>
        </div>
      </div>

      <div className="bg-emerald-900/20 border border-emerald-700/50 rounded-xl p-6">
        <h3 className="text-emerald-400 font-bold mb-3">✅ Как использовать скетч:</h3>
        <ol className="text-gray-300 space-y-2 text-sm">
          <li><span className="text-emerald-400 font-bold">1.</span> Нажмите "Скачать NetRadio_v2.ino"</li>
          <li><span className="text-emerald-400 font-bold">2.</span> Откройте Arduino IDE</li>
          <li><span className="text-emerald-400 font-bold">3.</span> Файл → Открыть → выберите скачанный файл</li>
          <li><span className="text-emerald-400 font-bold">4.</span> Установите библиотеки (см. ниже)</li>
          <li><span className="text-emerald-400 font-bold">5.</span> Выберите плату (NodeMCU рекомендуется)</li>
          <li><span className="text-emerald-400 font-bold">6.</span> Компилируйте и загружайте</li>
        </ol>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">📚 Необходимые библиотеки</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">Обязательные:</h4>
            <ul className="text-gray-400 text-sm space-y-2">
              <li><code className="text-emerald-400">LiquidCrystal_I2C</code> by Frank de Brabander</li>
              <li><code className="text-emerald-400">Wire</code> (встроена)</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">Для NodeMCU/ESP8266:</h4>
            <ul className="text-gray-400 text-sm space-y-2">
              <li><code className="text-emerald-400">ESP8266Audio</code> by Earle F. Philhower</li>
              <li><code className="text-emerald-400">ESP8266WiFi</code> (встроена)</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">Для ESP32:</h4>
            <ul className="text-gray-400 text-sm space-y-2">
              <li><code className="text-emerald-400">ESP32-audioI2S</code> by schreibfaul1</li>
              <li><code className="text-emerald-400">Preferences</code> (встроена)</li>
            </ul>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">Опциональные:</h4>
            <ul className="text-gray-400 text-sm space-y-2">
              <li><code className="text-emerald-400">ArduinoJson</code> by Benoit Blanchon</li>
              <li><code className="text-emerald-400">OneButton</code> by Matthias Hertel</li>
            </ul>
          </div>
        </div>
      </div>

      <div className="bg-blue-900/20 border border-blue-700/50 rounded-xl p-4">
        <h4 className="text-blue-400 font-bold mb-2">💡 Особенности кода:</h4>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>• <strong>Условная компиляция</strong> - один код для всех платформ</li>
          <li>• <strong>Автоматическое определение</strong> Arduino/ESP8266/ESP32/STM32</li>
          <li>• <strong>Минимальное потребление памяти</strong> - оптимизирован для Arduino</li>
          <li>• <strong>Прокрутка текста</strong> на LCD для длинных названий</li>
          <li>• <strong>Сохранение настроек</strong> в EEPROM/Preferences</li>
          <li>• <strong>Веб-интерфейс</strong> для управления станциями</li>
        </ul>
      </div>

      <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-xl p-4">
        <h4 className="text-yellow-400 font-bold mb-2">⚠️ Важно:</h4>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>1. Адрес I2C LCD может быть <code className="bg-gray-800 px-2 py-0.5 rounded">0x27</code> или <code className="bg-gray-800 px-2 py-0.5 rounded">0x3F</code></li>
          <li>2. Если LCD не работает - измените <code className="bg-gray-800 px-2 py-0.5 rounded">LCD_ADDRESS</code> в скетче</li>
          <li>3. Для Arduino нужен внешний WiFi модуль (ESP01 или ENC28J60)</li>
          <li>4. NodeMCU/Wemos - рекомендуемая платформа (всё встроено)</li>
          <li>5. Используйте блок питания 5V 2A для стабильной работы</li>
        </ul>
      </div>

      <div className="bg-blue-900/20 border border-blue-700/50 rounded-xl p-4">
        <h4 className="text-blue-400 font-bold mb-2">💡 Решение проблемы мерцания:</h4>
        <p className="text-gray-300 text-sm mb-2">
          Если нижний ряд дисплея мерцает - это не ошибка сборки! Проблема в частом обновлении экрана.
        </p>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>✅ <strong>Решение:</strong> Обновлённый скетч использует оптимизированное обновление</li>
          <li>✅ <strong>Что изменилось:</strong> Убран <code className="bg-gray-800 px-2 py-0.5 rounded">lcd.clear()</code></li>
          <li>✅ <strong>Результат:</strong> Дисплей обновляется только частично, без мерцания</li>
          <li>✅ <strong>Действие:</strong> Скачайте обновлённый скетч и загрузите заново</li>
        </ul>
      </div>
    </div>
  );
}

function WiringTab() {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">🔌 Схема подключения (ESP32)</h2>
        <p className="text-gray-400 mt-1">Подключение LCD 1602 I2C, кнопок и I2S DAC к ESP32</p>
      </div>

      {/* LCD Connection */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🖥️ LCD 1602 I2C → ESP32</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">LCD I2C</th>
                <th className="text-left py-2 px-3">ESP32</th>
                <th className="text-left py-2 px-3">Назначение</th>
                <th className="text-left py-2 px-3">Примечание</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">GND</td><td className="py-2 px-3">GND</td><td className="py-2 px-3">Земля</td><td className="py-2 px-3">-</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">VCC</td><td className="py-2 px-3">5V</td><td className="py-2 px-3">Питание</td><td className="py-2 px-3">5V, не 3.3V!</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">SDA</td><td className="py-2 px-3">GPIO21</td><td className="py-2 px-3">I2C Data</td><td className="py-2 px-3">Стандартный I2C</td></tr>
              <tr><td className="py-2 px-3 font-mono text-yellow-400">SCL</td><td className="py-2 px-3">GPIO22</td><td className="py-2 px-3">I2C Clock</td><td className="py-2 px-3">Стандартный I2C</td></tr>
            </tbody>
          </table>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Адрес I2C: 0x27 (обычно) или 0x3F. Используйте I2C Scanner для проверки.</p>
      </div>

      {/* Buttons */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🔘 Кнопки (5 шт.) → ESP32</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Кнопка</th>
                <th className="text-left py-2 px-3">ESP32</th>
                <th className="text-left py-2 px-3">Функция</th>
                <th className="text-left py-2 px-3">Подключение</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN_PREV</td><td className="py-2 px-3">GPIO32</td><td className="py-2 px-3">◀ Предыдущая станция</td><td className="py-2 px-3">GPIO32 → Кнопка → GND</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN_NEXT</td><td className="py-2 px-3">GPIO33</td><td className="py-2 px-3">▶ Следующая станция</td><td className="py-2 px-3">GPIO33 → Кнопка → GND</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN_VOL_UP</td><td className="py-2 px-3">GPIO34</td><td className="py-2 px-3">🔊 Громкость +</td><td className="py-2 px-3">GPIO34 → Кнопка → GND</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN_VOL_DOWN</td><td className="py-2 px-3">GPIO14</td><td className="py-2 px-3">🔉 Громкость -</td><td className="py-2 px-3">GPIO14 → Кнопка → GND</td></tr>
              <tr><td className="py-2 px-3 font-mono text-cyan-400">BTN_MODE</td><td className="py-2 px-3">GPIO15</td><td className="py-2 px-3">🔄 Смена режима дисплея</td><td className="py-2 px-3">GPIO15 → Кнопка → GND</td></tr>
            </tbody>
          </table>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Кнопки подключаются: Пин → Кнопка → GND. Используется INPUT_PULLUP, внешние резисторы не нужны.</p>
      </div>

      {/* Display Modes */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🖥️ Режимы дисплея (кнопка MODE)</h3>
        <div className="bg-cyan-900/20 border border-cyan-700/50 rounded-lg p-4 mb-4">
          <h4 className="text-cyan-400 font-bold mb-2">⏰ Время всегда видно!</h4>
          <p className="text-gray-300 text-sm">
            Текущее время отображается в верхнем правом углу экрана во всех режимах. 
            Синхронизация через NTP (часовой пояс Москва UTC+3).
          </p>
        </div>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="bg-blue-900/20 border border-blue-700/50 rounded-lg p-4">
            <h4 className="text-blue-400 font-bold mb-2">Режим 1: Громкость</h4>
            <div className="bg-blue-950 p-2 font-mono text-sm text-blue-300">
              <div>Record  14:35:22</div>
              <div>Vol:12/21       </div>
            </div>
          </div>
          <div className="bg-green-900/20 border border-green-700/50 rounded-lg p-4">
            <h4 className="text-green-400 font-bold mb-2">Режим 2: Температура</h4>
            <div className="bg-green-950 p-2 font-mono text-sm text-green-300">
              <div>Record  14:35:22</div>
              <div>Temp:-5.2C      </div>
            </div>
          </div>
          <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-lg p-4">
            <h4 className="text-yellow-400 font-bold mb-2">Режим 3: Дата</h4>
            <div className="bg-yellow-950 p-2 font-mono text-sm text-yellow-300">
              <div>Record  14:35:22</div>
              <div>Date:15.01.2024 </div>
            </div>
          </div>
          <div className="bg-purple-900/20 border border-purple-700/50 rounded-lg p-4">
            <h4 className="text-purple-400 font-bold mb-2">Режим 4: WiFi</h4>
            <div className="bg-purple-950 p-2 font-mono text-sm text-purple-300">
              <div>Record  14:35:22</div>
              <div>IP:192.168.1.100</div>
            </div>
          </div>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Нажимайте кнопку MODE для переключения между режимами. Режим сохраняется в памяти.</p>
      </div>

      {/* I2S DAC */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🔊 I2S DAC (MAX98357A / PCM5102) → ESP32</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">DAC Пин</th>
                <th className="text-left py-2 px-3">ESP32</th>
                <th className="text-left py-2 px-3">Назначение</th>
                <th className="text-left py-2 px-3">Примечание</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">VIN/VCC</td><td className="py-2 px-3">5V</td><td className="py-2 px-3">Питание</td><td className="py-2 px-3">5V, не 3.3V!</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">GND</td><td className="py-2 px-3">GND</td><td className="py-2 px-3">Земля</td><td className="py-2 px-3">-</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">BCLK</td><td className="py-2 px-3">GPIO26</td><td className="py-2 px-3">Bit Clock</td><td className="py-2 px-3">-</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">LRC/WS</td><td className="py-2 px-3">GPIO25</td><td className="py-2 px-3">Word Select</td><td className="py-2 px-3">Left/Right Clock</td></tr>
              <tr><td className="py-2 px-3 font-mono text-green-400">DIN</td><td className="py-2 px-3">GPIO27</td><td className="py-2 px-3">Serial Data</td><td className="py-2 px-3">Audio data</td></tr>
            </tbody>
          </table>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Подключите динамик к выходу DAC. Используйте короткий кабель для I2S.</p>
      </div>

      {/* Visual diagram */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">📐 Визуальная схема (ESP32 + 5 кнопок)</h3>
        <div className="bg-black rounded-lg p-6 font-mono text-xs text-gray-400 overflow-x-auto">
          <pre>{`
    ┌─────────────────────────────────────────────────────────┐
    │                    ESP32 DevKit V1                       │
    │                                                         │
    │  5V   ──────── LCD VCC, DAC VIN                         │
    │  GND  ──────── LCD GND, DAC GND, Buttons GND           │
    │                                                         │
    │  GPIO21 ────── LCD SDA                                  │
    │  GPIO22 ────── LCD SCL                                  │
    │                                                         │
    │  GPIO32 ────── BTN PREV ───┐                           │
    │  GPIO33 ────── BTN NEXT ───┤                           │
    │  GPIO34 ────── BTN VOL+ ───┤  Кнопки → GND            │
    │  GPIO14 ────── BTN VOL- ───┤  (INPUT_PULLUP)           │
    │  GPIO15 ────── BTN MODE ───┘                           │
    │                                                         │
    │  GPIO26 ────── DAC BCLK                                 │
    │  GPIO25 ────── DAC LRC                                  │
    │  GPIO27 ────── DAC DIN                                  │
    │                                                         │
    │  USB  ──────── Питание / Прошивка                       │
    └─────────────────────────────────────────────────────────┘
          │                    │
          ▼                    ▼
    ┌──────────┐        ┌──────────┐
    │ LCD 1602 │        │ I2S DAC  │──► 🔊 Speaker
    │ I2C      │        │MAX98357A │
    └──────────┘        └──────────┘
          `}</pre>
        </div>
      </div>

      {/* Weather API Info */}
      <div className="bg-emerald-900/20 border border-emerald-700/50 rounded-xl p-4">
        <h4 className="text-emerald-400 font-bold mb-2">🌤️ Погода (OpenWeatherMap)</h4>
        <p className="text-gray-300 text-sm mb-2">
          В скетч уже встроен ваш API ключ OpenWeatherMap для получения погоды в Москве.
        </p>
        <ul className="text-gray-400 text-sm space-y-1">
          <li>• <strong>API Key:</strong> cf0cd0d160ba580cef69e35dfe3064c8</li>
          <li>• <strong>Город:</strong> Moscow, Russia</li>
          <li>• <strong>Обновление:</strong> каждые 30 минут</li>
          <li>• <strong>Отображение:</strong> в режиме "Температура" (кнопка MODE)</li>
        </ul>
      </div>

      {/* I2C Scanner */}
      <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-xl p-4">
        <h4 className="text-yellow-400 font-bold mb-2">🔍 Определение адреса I2C LCD</h4>
        <p className="text-gray-300 text-sm mb-2">Если LCD не работает, запустите I2C Scanner для определения адреса:</p>
        <pre className="bg-black rounded-lg p-3 text-xs text-gray-300 overflow-x-auto">
{`#include <Wire.h>
void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}
void loop() {
  byte error, address;
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(address, HEX);
    }
  }
  delay(5000);
}`}
        </pre>
        <p className="text-gray-400 text-sm mt-2">Обычно адрес: <code className="bg-gray-800 px-2 py-0.5 rounded">0x27</code> или <code className="bg-gray-800 px-2 py-0.5 rounded">0x3F</code></p>
      </div>
    </div>
  );
}

function StationsTab() {
  const [search, setSearch] = useState('');
  
  const filteredStations = radioRecordStations.filter(s => 
    s.name.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">📡 Радиостанции</h2>
        <p className="text-gray-400 mt-1">Станции Radio Record для NetRadio v.2</p>
      </div>

      <div className="bg-emerald-900/20 border border-emerald-700/50 rounded-xl p-4">
        <h4 className="text-emerald-400 font-bold mb-2">ℹ️ О памяти</h4>
        <p className="text-gray-300 text-sm">
          NetRadio v.2 хранит до 20 станций в EEPROM/Preferences. 
          Этого достаточно для основных жанров. Станции сохраняются и не теряются при перезагрузке.
          В оригинальном yoRadio можно хранить до 65535 станций!
        </p>
      </div>

      <div className="relative">
        <input
          type="text"
          placeholder="🔍 Поиск станции..."
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          className="w-full px-4 py-3 bg-gray-900 border border-gray-700 rounded-xl text-white placeholder-gray-500 focus:border-emerald-500 focus:outline-none"
        />
      </div>

      <div className="bg-gray-900 rounded-xl border border-gray-800 overflow-hidden">
        <div className="bg-gray-800 px-4 py-3 border-b border-gray-700">
          <span className="text-gray-400 text-sm">Найдено станций: {filteredStations.length}</span>
        </div>
        <div className="divide-y divide-gray-800 max-h-[500px] overflow-y-auto">
          {filteredStations.map((station) => (
            <div key={station.id} className="flex items-center px-4 py-3 hover:bg-gray-800/50 transition-colors">
              <span className="text-gray-600 text-sm w-8 font-mono">{station.id}</span>
              <div className="flex-1">
                <p className="text-white font-medium">{station.name}</p>
                <p className="text-gray-500 text-xs font-mono truncate">{station.url}</p>
              </div>
              <span className="text-emerald-400 text-xs bg-emerald-900/30 px-2 py-1 rounded">320kbps</span>
            </div>
          ))}
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">📋 Станции по умолчанию (20 шт.)</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-2">
          {defaultStations.map((s, i) => (
            <div key={s.id} className="flex items-center gap-2 bg-gray-800/50 rounded-lg px-3 py-2">
              <span className="text-gray-600 text-xs w-5">{i + 1}.</span>
              <span className="text-gray-300 text-sm">{s.name}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}

function BuildTab() {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">⚙️ Сборка и прошивка</h2>
        <p className="text-gray-400 mt-1">Пошаговая инструкция для NetRadio v.2</p>
      </div>

      <div className="space-y-4">
        <StepCard
          num={1}
          title="Выбор платформы"
          content="Рекомендуется: NodeMCU (ESP8266) - всё встроено, простая прошивка.\nАльтернативы: ESP32 (больше памяти), Arduino + ESP01 (сложнее)."
        />
        <StepCard
          num={2}
          title="Установка Arduino IDE"
          content="Скачайте Arduino IDE 1.8.19 или 2.x.\nДля ESP8266: добавьте URL в настройки и установите пакет ESP8266.\nДля ESP32: установите пакет ESP32."
        />
        <StepCard
          num={3}
          title="Установка библиотек"
          content="Library Manager (Ctrl+Shift+I):\n• LiquidCrystal_I2C by Frank de Brabander\n• ESP8266Audio (для NodeMCU)\n• ESP32-audioI2S (для ESP32)\n• ArduinoJson (опционально)"
        />
        <StepCard
          num={4}
          title="Сборка схемы"
          content="Соберите по схеме из вкладки 'Схема':\n• LCD 1602 I2C → SDA, SCL, VCC, GND\n• 3 кнопки → GPIO пины → GND\n• I2S DAC → BCLK, LRC, DIN, VCC, GND"
        />
        <StepCard
          num={5}
          title="Определение адреса LCD"
          content="Если LCD не работает, запустите I2C Scanner (см. вкладка 'Схема').\nОбычно адрес 0x27 или 0x3F.\nИзмените LCD_ADDRESS в скетче если нужно."
        />
        <StepCard
          num={6}
          title="Загрузка прошивки"
          content="Откройте NetRadio_v2.ino в Arduino IDE.\nВыберите плату: NodeMCU 1.0 / ESP32 Dev Module.\nUpload Speed: 115200.\nНажмите Upload."
        />
        <StepCard
          num={7}
          title="Первый запуск"
          content="После прошивки LCD покажет 'NetRadio v.2'.\nЕсли WiFi не настроен, создастся AP 'NetRadio' с паролем 'netradio123'.\nПодключитесь и откройте http://192.168.4.1"
        />
        <StepCard
          num={8}
          title="Настройка WiFi"
          content="В веб-интерфейсе укажите SSID и пароль вашей сети.\nУстройство перезагрузится и подключится.\nПосле этого можно добавлять станции."
        />
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">🔧 Решение проблем</h3>
        <div className="space-y-4">
          <TroubleItem
            problem="LCD не показывает текст"
            solution="Проверьте адрес I2C (0x27 или 0x3F). Запустите I2C Scanner. Проверьте подключение SDA/SCL."
          />
          <TroubleItem
            problem="Нет звука"
            solution="Проверьте подключение I2S DAC. Убедитесь что DAC получает 5V. Проверьте динамик."
          />
          <TroubleItem
            problem="WiFi не подключается"
            solution="Проверьте SSID и пароль. ESP8266 работает только с 2.4GHz WiFi (не 5GHz)."
          />
          <TroubleItem
            problem="Кнопки не работают"
            solution="Проверьте что кнопки подключены к GND. Пины настроены как INPUT_PULLUP."
          />
          <TroubleItem
            problem="Ошибки компиляции"
            solution="Убедитесь что все библиотеки установлены. Выберите правильную плату в Arduino IDE."
          />
        </div>
      </div>

      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-emerald-400 mb-4">💰 Стоимость компонентов</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Компонент</th>
                <th className="text-left py-2 px-3">Цена (AliExpress)</th>
                <th className="text-left py-2 px-3">Примечание</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3">NodeMCU v3</td><td className="py-2 px-3">$3-4</td><td className="py-2 px-3">Рекомендуется</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">LCD 1602 I2C</td><td className="py-2 px-3">$2-3</td><td className="py-2 px-3">Синий фон</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">MAX98357A DAC</td><td className="py-2 px-3">$1-2</td><td className="py-2 px-3">I2S amplifier</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">3 кнопки</td><td className="py-2 px-3">$0.50</td><td className="py-2 px-3">6x6mm tact</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3">Динамик 3W</td><td className="py-2 px-3">$1-2</td><td className="py-2 px-3">4-8 Ohm</td></tr>
              <tr><td className="py-2 px-3 font-bold">ИТОГО</td><td className="py-2 px-3 font-bold text-emerald-400">$8-12</td><td className="py-2 px-3">Без доставки</td></tr>
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

function PlatformCard({ name, icon, features, recommended }: { name: string; icon: string; features: string[]; recommended: boolean }) {
  return (
    <div className={`rounded-xl p-4 border ${recommended ? 'bg-emerald-900/30 border-emerald-500/50' : 'bg-gray-800/50 border-gray-700'}`}>
      {recommended && <span className="text-xs text-emerald-400 font-bold">РЕКОМЕНДУЕТСЯ</span>}
      <div className="text-2xl mb-2">{icon}</div>
      <h4 className="text-white font-bold mb-2">{name}</h4>
      <ul className="text-gray-400 text-sm space-y-1">
        {features.map((f, i) => (
          <li key={i}>• {f}</li>
        ))}
      </ul>
    </div>
  );
}

function StepCard({ num, title, content }: { num: number; title: string; content: string }) {
  return (
    <div className="flex gap-4 bg-gray-900 rounded-xl p-5 border border-gray-800">
      <div className="flex-shrink-0 w-10 h-10 bg-emerald-600 rounded-full flex items-center justify-center text-white font-bold">
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
