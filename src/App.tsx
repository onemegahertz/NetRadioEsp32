import { useState } from 'react';
import { arduinoSketch, userSetupConfig } from './data/arduinoSketch';
import { radioRecordStations, defaultStations } from './data/stations';

type TabType = 'overview' | 'sketch' | 'webui' | 'wiring' | 'stations' | 'build';

function App() {
  const [activeTab, setActiveTab] = useState<TabType>('overview');
  const [copied, setCopied] = useState(false);

  const copyToClipboard = (text: string) => {
    navigator.clipboard.writeText(text);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const tabs: { id: TabType; label: string; icon: string }[] = [
    { id: 'overview', label: 'Обзор', icon: '📋' },
    { id: 'sketch', label: 'Скетч', icon: '💻' },
    { id: 'webui', label: 'Web UI', icon: '🌐' },
    { id: 'wiring', label: 'Схема', icon: '🔌' },
    { id: 'stations', label: 'Станции', icon: '📻' },
    { id: 'build', label: 'Сборка', icon: '⚙️' },
  ];

  return (
    <div className="min-h-screen bg-gray-950 text-gray-100">
      {/* Header */}
      <header className="bg-gradient-to-r from-indigo-900 via-purple-900 to-indigo-900 border-b border-purple-500/30">
        <div className="max-w-7xl mx-auto px-4 py-6">
          <div className="flex items-center justify-between">
            <div>
              <h1 className="text-3xl font-bold text-cyan-400 flex items-center gap-3">
                <span className="text-4xl">📻</span> NetRadio v.1
              </h1>
              <p className="text-gray-400 mt-1">ESP32 Internet Radio • TFT 2.4" ILI9341 • I2S Audio</p>
            </div>
            <div className="hidden md:flex items-center gap-4">
              <span className="px-3 py-1 bg-green-900/50 text-green-400 rounded-full text-sm border border-green-700">
                ✓ Код проверен
              </span>
              <span className="px-3 py-1 bg-blue-900/50 text-blue-400 rounded-full text-sm border border-blue-700">
                v1.0.0
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
                    ? 'bg-purple-600 text-white shadow-lg shadow-purple-500/20'
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
        {activeTab === 'sketch' && <SketchTab sketch={arduinoSketch} onCopy={copyToClipboard} copied={copied} />}
        {activeTab === 'webui' && <WebUITab />}
        {activeTab === 'wiring' && <WiringTab config={userSetupConfig} onCopy={copyToClipboard} copied={copied} />}
        {activeTab === 'stations' && <StationsTab />}
        {activeTab === 'build' && <BuildTab />}
      </main>

      {/* Footer */}
      <footer className="bg-gray-900 border-t border-gray-800 py-6 mt-12">
        <div className="max-w-7xl mx-auto px-4 text-center text-gray-500 text-sm">
          <p>NetRadio v.1 • ESP32 Internet Radio Project • 2024</p>
          <p className="mt-1">Arduino • TFT_eSPI • ESP32-audioI2S • ArduinoJson</p>
        </div>
      </footer>
    </div>
  );
}

function OverviewTab() {
  return (
    <div className="space-y-8">
      {/* Hero */}
      <div className="bg-gradient-to-br from-indigo-900/50 to-purple-900/50 rounded-2xl p-8 border border-purple-500/20">
        <h2 className="text-2xl font-bold text-white mb-4">🎵 Интернет-радио на ESP32</h2>
        <p className="text-gray-300 leading-relaxed mb-6">
          Полноценный проект интернет-радио на базе ESP32 DevKit с TFT дисплеем 2.4" (ILI9341, SPI, 320x240).
          Воспроизведение интернет-радиостанций через I2S DAC, управление 4 кнопками, 
          веб-интерфейс для добавления/редактирования станций. Отображение текущего времени, погоды (Москва), 
          названия станции и уровня громкости.
        </p>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          <FeatureCard icon="📺" title="TFT Дисплей" desc={'2.4" ILI9341 SPI 320x240. Отображает WiFi, IP, станцию, громкость'} />
          <FeatureCard icon="🔊" title="I2S Аудио" desc="Высококачественный звук через MAX98357A или PCM5102 DAC" />
          <FeatureCard icon="🌐" title="Web Интерфейс" desc="Управление станциями через браузер на любом устройстве" />
        </div>
      </div>

      {/* Features */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
          <h3 className="text-lg font-bold text-cyan-400 mb-4">🎛️ Функции устройства</h3>
          <ul className="space-y-3 text-gray-300">
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> Диагностика всех компонентов при включении</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> Отображение WiFi сети и IP адреса</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> Название текущей станции на дисплее</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> Уровень громкости с визуализацией</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> 4 кнопки: ◀ Назад | ▶ Вперёд | 🔊+ | 🔊-</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> До 20 радиостанций в памяти</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> Сохранение настроек (WiFi, громкость, станция)</li>
            <li className="flex items-start gap-2"><span className="text-green-400">✓</span> AP режим если WiFi не найден</li>
          </ul>
        </div>

        <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
          <h3 className="text-lg font-bold text-cyan-400 mb-4">📦 Необходимые компоненты</h3>
          <ul className="space-y-3 text-gray-300">
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> ESP32 DevKit V1 (или аналог)</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> TFT LCD 2.4" ILI9341 SPI 320x240</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> I2S DAC: MAX98357A или PCM5102</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> 4 тактовые кнопки (6x6mm)</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> Динамик 3W или усилитель</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> Блок питания 5V 2A</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> Макетная плата + провода</li>
            <li className="flex items-start gap-2"><span className="text-yellow-400">●</span> USB кабель для прошивки</li>
          </ul>
        </div>
      </div>

      {/* Libraries */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">📚 Необходимые библиотеки Arduino</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          <LibCard name="TFT_eSPI" version="2.5.43+" author="Bodmer" desc="Драйвер TFT дисплея" />
          <LibCard name="ESP32-audioI2S" version="3.0.7+" author="schreibfaul1" desc="Аудио плеер I2S" />
          <LibCard name="ArduinoJson" version="7.0.4+" author="B. Blanchon" desc="JSON парсер" />
          <LibCard name="Preferences" version="Built-in" author="Espressif" desc="NVS хранилище" />
        </div>
      </div>

      {/* Diagnostic info */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">🔍 Диагностика при запуске</h3>
        <p className="text-gray-400 mb-4">При каждом включении выполняется проверка всех компонентов:</p>
        <div className="bg-black rounded-lg p-4 font-mono text-sm">
          <p className="text-green-400">[DIAG] TFT ILI9341: OK - 240x320 SPI initialized</p>
          <p className="text-green-400">[DIAG] I2S Audio: OK - BCLK=26 LRC=25 DOUT=22</p>
          <p className="text-green-400">[DIAG] Buttons: OK - NEXT=32 PREV=33 VOL+=34 VOL-=35</p>
          <p className="text-yellow-400">[DIAG] WiFi Station: OK - SSID: MyNetwork IP: 192.168.1.100</p>
          <p className="text-gray-500 mt-2">========================================</p>
          <p className="text-gray-400">Diagnostics Summary: ALL PASS</p>
          <p className="text-gray-500">========================================</p>
        </div>
      </div>
    </div>
  );
}

function SketchTab({ sketch, onCopy, copied }: { sketch: string; onCopy: (t: string) => void; copied: boolean }) {
  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-2xl font-bold text-white">💻 Arduino Скетч</h2>
          <p className="text-gray-400 mt-1">Полный код прошивки для ESP32 (~800 строк)</p>
        </div>
        <button
          onClick={() => onCopy(sketch)}
          className="px-4 py-2 bg-purple-600 hover:bg-purple-700 text-white rounded-lg transition-colors flex items-center gap-2"
        >
          {copied ? '✓ Скопировано!' : '📋 Копировать'}
        </button>
      </div>

      <div className="bg-gray-900 rounded-xl border border-gray-800 overflow-hidden">
        <div className="bg-gray-800 px-4 py-2 flex items-center gap-2 border-b border-gray-700">
          <div className="w-3 h-3 rounded-full bg-red-500"></div>
          <div className="w-3 h-3 rounded-full bg-yellow-500"></div>
          <div className="w-3 h-3 rounded-full bg-green-500"></div>
          <span className="ml-3 text-gray-400 text-sm font-mono">NetRadio_v1.ino</span>
        </div>
        <pre className="p-4 overflow-x-auto text-sm font-mono text-gray-300 max-h-[600px] overflow-y-auto">
          <code>{sketch}</code>
        </pre>
      </div>

      <div className="bg-yellow-900/20 border border-yellow-700/50 rounded-xl p-4">
        <h4 className="text-yellow-400 font-bold mb-2">⚠️ Важно перед прошивкой:</h4>
        <ul className="text-gray-300 text-sm space-y-1">
          <li>1. Установите все библиотеки через Arduino Library Manager</li>
          <li>2. Настройте User_Setup.h для TFT_eSPI (см. вкладка "Схема")</li>
          <li>3. Выберите плату "ESP32 Dev Module" в Arduino IDE</li>
          <li>4. Установите скорость загрузки 921600 baud</li>
          <li>5. Partition Scheme: "Default 4MB with spiffs"</li>
        </ul>
      </div>
    </div>
  );
}

function WebUITab() {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">🌐 Веб-интерфейс ESP32</h2>
        <p className="text-gray-400 mt-1">Встроенный веб-сервер для управления радио через браузер</p>
      </div>

      {/* TFT Display Preview */}
      <div className="bg-gray-900 rounded-xl border border-gray-800 overflow-hidden">
        <div className="bg-gray-800 px-4 py-2 border-b border-gray-700">
          <span className="text-gray-400 text-sm">Превью TFT дисплея (как выглядит на экране 2.4")</span>
        </div>
        <div className="flex justify-center p-6 bg-black">
          <div className="w-[240px] h-[320px] bg-black border-2 border-gray-600 rounded-lg overflow-hidden font-mono text-[10px] relative">
            {/* Top bar - Time */}
            <div className="h-[25px] bg-gray-700 flex items-center px-1">
              <span className="text-cyan-400 text-[8px]">NetRadio v.1</span>
              <span className="text-white text-[14px] ml-2 font-bold">14:35:22</span>
              <span className="text-gray-500 text-[8px] ml-auto">15.01.2024</span>
            </div>
            {/* Weather */}
            <div className="h-[35px] bg-[#0a0a2e] px-1 py-1">
              <span className="text-yellow-400 text-[9px]">Moscow:</span>
              <span className="text-white text-[14px] ml-1 font-bold">-5°C</span>
              <span className="text-gray-500 text-[8px] ml-1">❄️</span>
              <div className="text-gray-500 text-[8px]">небольшой снег</div>
            </div>
            {/* Station */}
            <div className="px-2 mt-2">
              <div className="text-white text-[16px] font-bold">Record</div>
              <div className="text-gray-600 text-[9px]">Station 1/20</div>
            </div>
            {/* Volume */}
            <div className="px-2 mt-2">
              <div className="text-yellow-400 text-[9px]">Volume:</div>
              <div className="h-[12px] bg-gray-700 rounded mt-1 relative">
                <div className="h-full bg-green-500 rounded" style={{width: '57%'}}></div>
                <div className="absolute inset-0 border border-white rounded"></div>
              </div>
              <div className="text-white text-[9px] mt-1">12/21</div>
            </div>
            {/* WiFi */}
            <div className="px-2 mt-2">
              <div className="text-gray-600 text-[8px]">WiFi: MyHomeNetwork</div>
              <div className="text-gray-600 text-[8px]">IP: 192.168.1.100</div>
            </div>
            {/* Status */}
            <div className="px-2 mt-2 flex justify-between">
              <span className="text-gray-600 text-[8px]">BTN: Next/Prev/Vol+/-</span>
              <span className="text-green-400 text-[9px] font-bold">PLAY</span>
            </div>
            {/* URL */}
            <div className="px-2 mt-1">
              <div className="text-gray-600 text-[7px] truncate">https://radiorecord.hosting...</div>
            </div>
            {/* Web */}
            <div className="px-2 mt-1">
              <div className="text-cyan-400 text-[8px]">Web: http://192.168.1.100</div>
            </div>
          </div>
        </div>
      </div>

      {/* Web UI Preview */}
      <div className="bg-gray-900 rounded-xl border border-gray-800 overflow-hidden mt-6">
        <div className="bg-gray-800 px-4 py-2 border-b border-gray-700">
          <span className="text-gray-400 text-sm">Превью веб-интерфейса (так выглядит в браузере на ESP32)</span>
        </div>
        
        {/* Simulated Web UI */}
        <div className="bg-[#0a0a1a] p-6">
          <div className="max-w-md mx-auto">
            {/* Header */}
            <div className="bg-gradient-to-r from-indigo-900 to-purple-900 rounded-t-xl p-4 text-center border-b-2 border-purple-500">
              <h3 className="text-cyan-400 text-xl font-bold">📻 NetRadio v.1</h3>
              <p className="text-gray-500 text-xs">ESP32 Internet Radio Control Panel</p>
            </div>
            
            {/* Info */}
            <div className="bg-[#1a1a2e] p-4 border border-gray-700 rounded-b-xl">
              <div className="flex justify-between py-1 border-b border-gray-800">
                <span className="text-gray-500 text-sm">WiFi Network:</span>
                <span className="text-cyan-400 text-sm font-bold">MyHomeWiFi</span>
              </div>
              <div className="flex justify-between py-1 border-b border-gray-800">
                <span className="text-gray-500 text-sm">IP Address:</span>
                <span className="text-cyan-400 text-sm font-bold">192.168.1.100</span>
              </div>
              <div className="flex justify-between py-1 border-b border-gray-800">
                <span className="text-gray-500 text-sm">Current Station:</span>
                <span className="text-cyan-400 text-sm font-bold">Record</span>
              </div>
              <div className="flex justify-between py-1">
                <span className="text-gray-500 text-sm">Status:</span>
                <span className="text-green-400 text-sm font-bold">PLAYING</span>
              </div>
            </div>

            {/* Volume */}
            <div className="text-center text-cyan-400 text-lg my-4">Volume: <span className="font-bold">12</span>/21</div>

            {/* Controls */}
            <div className="grid grid-cols-2 gap-2 mb-4">
              <button className="bg-blue-600 text-white py-2 rounded-lg text-sm font-bold">◀ Prev</button>
              <button className="bg-green-600 text-white py-2 rounded-lg text-sm font-bold">Next ▶</button>
              <button className="bg-orange-600 text-white py-2 rounded-lg text-sm font-bold">🔉 Vol -</button>
              <button className="bg-red-600 text-white py-2 rounded-lg text-sm font-bold">🔊 Vol +</button>
            </div>

            {/* Station list */}
            <div className="bg-[#1a1a2e] p-4 rounded-xl border border-gray-700">
              <h4 className="text-cyan-400 font-bold mb-2">📻 Stations (20/20)</h4>
              <div className="space-y-1">
                {defaultStations.slice(0, 5).map((s, i) => (
                  <div key={s.id} className={`flex items-center p-2 rounded ${i === 0 ? 'bg-[#1a2a3e] border border-cyan-500' : 'bg-[#0d0d1a] border border-gray-800'}`}>
                    <span className="text-gray-600 text-xs w-6">{i + 1}</span>
                    <span className="flex-1 text-sm text-gray-300">{s.name}</span>
                    <div className="flex gap-1">
                      <span className="bg-green-600 text-white text-xs px-2 py-0.5 rounded">▶</span>
                      <span className="bg-blue-600 text-white text-xs px-2 py-0.5 rounded">✎</span>
                      <span className="bg-red-600 text-white text-xs px-2 py-0.5 rounded">✕</span>
                    </div>
                  </div>
                ))}
                <div className="text-gray-600 text-xs text-center py-1">... и ещё 15 станций</div>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Features */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        <div className="bg-gray-900 rounded-xl p-5 border border-gray-800">
          <h4 className="text-cyan-400 font-bold mb-3">🎵 Управление станциями</h4>
          <ul className="text-gray-400 text-sm space-y-2">
            <li>• Добавление новых станций (до 20)</li>
            <li>• Редактирование имени и URL</li>
            <li>• Удаление станций</li>
            <li>• Переключение воспроизведения</li>
            <li>• Сохранение в энергонезависимую память</li>
          </ul>
        </div>
        <div className="bg-gray-900 rounded-xl p-5 border border-gray-800">
          <h4 className="text-cyan-400 font-bold mb-3">📶 Настройка WiFi</h4>
          <ul className="text-gray-400 text-sm space-y-2">
            <li>• Ввод SSID и пароля через веб</li>
            <li>• Автоматическое переподключение</li>
            <li>• AP режим если WiFi не найден</li>
            <li>• AP SSID: NetRadio_Setup</li>
            <li>• AP пароль: netradio123</li>
          </ul>
        </div>
      </div>

      {/* API endpoints */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h4 className="text-cyan-400 font-bold mb-4">🔗 API Endpoints</h4>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Метод</th>
                <th className="text-left py-2 px-3">URL</th>
                <th className="text-left py-2 px-3">Описание</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/</td><td className="py-2 px-3">Веб-интерфейс</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/status</td><td className="py-2 px-3">Текущий статус</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/stations</td><td className="py-2 px-3">Список станций</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/next</td><td className="py-2 px-3">Следующая станция</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/prev</td><td className="py-2 px-3">Предыдущая станция</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/volup</td><td className="py-2 px-3">Громкость +</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/voldown</td><td className="py-2 px-3">Громкость -</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/play/N</td><td className="py-2 px-3">Играть станцию N (0-19)</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-blue-400">POST</td><td className="py-2 px-3 font-mono text-xs">/api/station</td><td className="py-2 px-3">Добавить/редактировать</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 text-green-400">GET</td><td className="py-2 px-3 font-mono text-xs">/api/delete/N</td><td className="py-2 px-3">Удалить станцию N</td></tr>
              <tr><td className="py-2 px-3 text-blue-400">POST</td><td className="py-2 px-3 font-mono text-xs">/api/wifi</td><td className="py-2 px-3">Сохранить WiFi настройки</td></tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}

function WiringTab({ config, onCopy, copied }: { config: string; onCopy: (t: string) => void; copied: boolean }) {
  return (
    <div className="space-y-6">
      <div>
        <h2 className="text-2xl font-bold text-white">🔌 Схема подключения</h2>
        <p className="text-gray-400 mt-1">Распиновка и подключение всех компонентов</p>
      </div>

      {/* White screen fix notice */}
      <div className="bg-red-900/20 border border-red-700/50 rounded-xl p-4">
        <h4 className="text-red-400 font-bold mb-2">⚠️ ВАЖНО: Исправление белого экрана!</h4>
        <p className="text-gray-300 text-sm mb-2">
          Если экран светится белым — <strong>измените пины в User_Setup.h</strong> библиотеки TFT_eSPI:
        </p>
        <div className="bg-black rounded-lg p-3 font-mono text-sm text-green-400">
          <p>#define TFT_CS     5   // было 15</p>
          <p>#define TFT_DC     4   // было 2</p>
          <p>#define TFT_RST   15   // было 4</p>
        </div>
        <p className="text-gray-400 text-sm mt-2">
          Также убедитесь что закомментированы ВСЕ другие драйверы в User_Setup.h, 
          кроме <code className="text-yellow-400">#define ILI9341_DRIVER</code>
        </p>
      </div>

      {/* Wiring Diagram */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">ESP32 DevKit → TFT ILI9341 (SPI)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">TFT Пин</th>
                <th className="text-left py-2 px-3">ESP32 Пин</th>
                <th className="text-left py-2 px-3">Назначение</th>
                <th className="text-left py-2 px-3">Цвет провода</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">VCC</td><td className="py-2 px-3">3.3V</td><td className="py-2 px-3">Питание</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-red-500 rounded mr-1"></span>Красный</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">GND</td><td className="py-2 px-3">GND</td><td className="py-2 px-3">Земля</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-gray-800 rounded mr-1 border border-gray-600"></span>Чёрный</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">CS</td><td className="py-2 px-3 font-bold text-green-400">GPIO 5</td><td className="py-2 px-3">Chip Select</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-orange-500 rounded mr-1"></span>Оранжевый</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">RESET</td><td className="py-2 px-3 font-bold text-green-400">GPIO 15</td><td className="py-2 px-3">Reset</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-yellow-500 rounded mr-1"></span>Жёлтый</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">DC</td><td className="py-2 px-3 font-bold text-green-400">GPIO 4</td><td className="py-2 px-3">Data/Command</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-white rounded mr-1"></span>Белый</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">MOSI</td><td className="py-2 px-3">GPIO 23</td><td className="py-2 px-3">SPI Data</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-blue-500 rounded mr-1"></span>Синий</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-yellow-400">SCK</td><td className="py-2 px-3">GPIO 18</td><td className="py-2 px-3">SPI Clock</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-purple-500 rounded mr-1"></span>Фиолетовый</td></tr>
              <tr><td className="py-2 px-3 font-mono text-yellow-400">LED</td><td className="py-2 px-3">3.3V</td><td className="py-2 px-3">Backlight</td><td className="py-2 px-3"><span className="inline-block w-3 h-3 bg-pink-500 rounded mr-1"></span>Розовый</td></tr>
            </tbody>
          </table>
        </div>
      </div>

      {/* I2S DAC */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">ESP32 → I2S DAC (MAX98357A / PCM5102)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">DAC Пин</th>
                <th className="text-left py-2 px-3">ESP32 Пин</th>
                <th className="text-left py-2 px-3">Назначение</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">VIN/VCC</td><td className="py-2 px-3">5V</td><td className="py-2 px-3">Питание</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">GND</td><td className="py-2 px-3">GND</td><td className="py-2 px-3">Земля</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">BCLK</td><td className="py-2 px-3">GPIO 26</td><td className="py-2 px-3">Bit Clock</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">LRC/WS</td><td className="py-2 px-3">GPIO 25</td><td className="py-2 px-3">Word Select (L/R Clock)</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-green-400">DIN</td><td className="py-2 px-3">GPIO 22</td><td className="py-2 px-3">Serial Data</td></tr>
              <tr><td className="py-2 px-3 font-mono text-green-400">SD/GAIN</td><td className="py-2 px-3">не подкл.</td><td className="py-2 px-3">Gain select (open=9dB)</td></tr>
            </tbody>
          </table>
        </div>
      </div>

      {/* Buttons */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">ESP32 → Кнопки (4 шт.)</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-gray-500 border-b border-gray-700">
                <th className="text-left py-2 px-3">Кнопка</th>
                <th className="text-left py-2 px-3">ESP32 Пин</th>
                <th className="text-left py-2 px-3">Функция</th>
                <th className="text-left py-2 px-3">Подключение</th>
              </tr>
            </thead>
            <tbody className="text-gray-300">
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN 1</td><td className="py-2 px-3">GPIO 32</td><td className="py-2 px-3">▶ Следующая станция</td><td className="py-2 px-3">Пин → Кнопка → GND</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN 2</td><td className="py-2 px-3">GPIO 33</td><td className="py-2 px-3">◀ Предыдущая станция</td><td className="py-2 px-3">Пин → Кнопка → GND</td></tr>
              <tr className="border-b border-gray-800"><td className="py-2 px-3 font-mono text-cyan-400">BTN 3</td><td className="py-2 px-3">GPIO 34</td><td className="py-2 px-3">🔊 Громкость +</td><td className="py-2 px-3">Пин → Кнопка → GND</td></tr>
              <tr><td className="py-2 px-3 font-mono text-cyan-400">BTN 4</td><td className="py-2 px-3">GPIO 35</td><td className="py-2 px-3">🔉 Громкость -</td><td className="py-2 px-3">Пин → Кнопка → GND</td></tr>
            </tbody>
          </table>
        </div>
        <p className="text-gray-500 text-sm mt-3">* Кнопки подключаются с внутренним pull-up (INPUT_PULLUP). Второй контакт кнопки → GND.</p>
      </div>

      {/* TFT_eSPI Config */}
      <div className="bg-gray-900 rounded-xl border border-gray-800 overflow-hidden">
        <div className="flex items-center justify-between bg-gray-800 px-4 py-2 border-b border-gray-700">
          <span className="text-gray-400 text-sm font-mono">TFT_eSPI/User_Setup.h (фрагмент)</span>
          <button
            onClick={() => onCopy(config)}
            className="px-3 py-1 bg-purple-600 hover:bg-purple-700 text-white rounded text-xs transition-colors"
          >
            {copied ? '✓ OK' : '📋 Copy'}
          </button>
        </div>
        <pre className="p-4 overflow-x-auto text-sm font-mono text-gray-300">
          <code>{config}</code>
        </pre>
      </div>

      {/* Visual diagram */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">📐 Визуальная схема (ОБНОВЛЁННАЯ)</h3>
        <div className="bg-black rounded-lg p-6 font-mono text-xs text-gray-400 overflow-x-auto">
          <pre>{`
    ┌─────────────────────────────────────────────────────────┐
    │                    ESP32 DevKit V1                       │
    │                                                         │
    │  3.3V ──────── TFT VCC, TFT LED                         │
    │  5V   ──────── DAC VCC                                  │
    │  GND  ──────── TFT GND, DAC GND, Buttons GND           │
    │                                                         │
    │  GPIO 4 ────── TFT DC        (ИЗМЕНЕНО!)                │
    │  GPIO 5 ────── TFT CS        (ИЗМЕНЕНО!)                │
    │  GPIO 15 ───── TFT RESET     (ИЗМЕНЕНО!)                │
    │  GPIO 18 ───── TFT SCK                                  │
    │  GPIO 23 ───── TFT MOSI                                 │
    │                                                         │
    │  GPIO 22 ───── DAC DIN (Data)                           │
    │  GPIO 25 ───── DAC LRC (Word Select)                    │
    │  GPIO 26 ───── DAC BCLK (Bit Clock)                     │
    │                                                         │
    │  GPIO 32 ───── BTN NEXT  ──┐                           │
    │  GPIO 33 ───── BTN PREV  ──┤  Кнопки → GND            │
    │  GPIO 34 ───── BTN VOL+  ──┤  (INPUT_PULLUP)           │
    │  GPIO 35 ───── BTN VOL-  ──┘                           │
    │                                                         │
    │  USB  ──────── Питание / Прошивка                       │
    └─────────────────────────────────────────────────────────┘
          │                    │
          ▼                    ▼
    ┌──────────┐        ┌──────────┐
    │ TFT 2.4" │        │ I2S DAC  │──► 🔊 Speaker
    │ ILI9341  │        │MAX98357A │
    └──────────┘        └──────────┘
          `}</pre>
        </div>
      </div>

      {/* New features */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">🆕 Новые функции на дисплее</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">⏰ Время и дата</h4>
            <p className="text-gray-400 text-sm">Автоматическая синхронизация через NTP (pool.ntp.org). Часовой пояс: Москва UTC+3</p>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">🌤️ Погода (Москва)</h4>
            <p className="text-gray-400 text-sm">Обновление каждые 30 минут. Требуется бесплатный API ключ от openweathermap.org</p>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">📻 Название станции</h4>
            <p className="text-gray-400 text-sm">Крупным шрифтом отображается текущая станция и её номер в списке</p>
          </div>
          <div className="bg-gray-800/50 rounded-lg p-4">
            <h4 className="text-white font-bold mb-2">🔊 Уровень громкости</h4>
            <p className="text-gray-400 text-sm">Визуальная шкала + числовое значение (0-21)</p>
          </div>
        </div>
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
        <h2 className="text-2xl font-bold text-white">📻 Радиостанции Radio Record</h2>
        <p className="text-gray-400 mt-1">Все доступные станции с radiorecord.ru (можно добавить до 20 в память ESP32)</p>
      </div>

      {/* Info */}
      <div className="bg-blue-900/20 border border-blue-700/50 rounded-xl p-4">
        <h4 className="text-blue-400 font-bold mb-2">ℹ️ О памяти ESP32</h4>
        <p className="text-gray-300 text-sm">
          ESP32 имеет достаточно NVS памяти для хранения 20 станций (имя + URL). 
          Этого вполне достаточно для основных жанров. Станции сохраняются в Preferences (NVS) 
          и не теряются при перезагрузке. Для большего количества потребуется SD карта, 
          но 20 станций — оптимальный выбор для удобной навигации кнопками.
        </p>
      </div>

      {/* Search */}
      <div className="relative">
        <input
          type="text"
          placeholder="🔍 Поиск станции..."
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          className="w-full px-4 py-3 bg-gray-900 border border-gray-700 rounded-xl text-white placeholder-gray-500 focus:border-purple-500 focus:outline-none"
        />
      </div>

      {/* Station list */}
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
              <span className="text-green-400 text-xs bg-green-900/30 px-2 py-1 rounded">320kbps</span>
            </div>
          ))}
        </div>
      </div>

      {/* Default stations */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">📋 Станции по умолчанию (20 шт.)</h3>
        <p className="text-gray-400 text-sm mb-4">Эти станции загружаются при первом запуске:</p>
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
        <p className="text-gray-400 mt-1">Пошаговая инструкция по сборке проекта</p>
      </div>

      {/* Steps */}
      <div className="space-y-4">
        <StepCard
          num={1}
          title="Установка Arduino IDE"
          content="Скачайте Arduino IDE 2.x с arduino.cc. Установите пакет ESP32 через Boards Manager: добавьте URL https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json в настройки, затем установите 'esp32 by Espressif Systems'."
        />
        <StepCard
          num={2}
          title="Установка библиотек"
          content="Откройте Library Manager (Ctrl+Shift+I) и установите:\n• TFT_eSPI by Bodmer (v2.5.43+)\n• ESP32-audioI2S by schreibfaul1 (v3.0.7+)\n• ArduinoJson by Benoit Blanchon (v7.0.4+)"
        />
        <StepCard
          num={3}
          title="Настройка TFT_eSPI"
          content="Откройте файл User_Setup.h в папке библиотеки TFT_eSPI. Закомментируйте все определения драйверов и добавьте конфигурацию для ILI9341 (см. вкладка 'Схема'). Это критически важно!"
        />
        <StepCard
          num={4}
          title="Сборка компонентов"
          content="Соберите схему по распиновке из вкладки 'Схема'. Убедитесь что все соединения надёжны. TFT питается от 3.3V, DAC от 5V. Кнопки подключаются к GND."
        />
        <StepCard
          num={5}
          title="Загрузка прошивки"
          content="В Arduino IDE: Tools → Board → ESP32 Dev Module. Partition Scheme: Default 4MB with spiffs. Upload Speed: 921600. Вставьте скетч и нажмите Upload."
        />
        <StepCard
          num={6}
          title="Первый запуск"
          content="После прошивки откройте Serial Monitor (115200 baud). Устройство проведёт диагностику. Если WiFi не настроен, создастся AP 'NetRadio_Setup' с паролем 'netradio123'. Подключитесь к нему и откройте http://192.168.4.1"
        />
        <StepCard
          num={7}
          title="Настройка через веб"
          content="В веб-интерфейсе перейдите в 'WiFi Settings' и укажите вашу домашнюю сеть. Устройство перезагрузится и подключится к WiFi. После этого можно добавлять/редактировать станции."
        />
        <StepCard
          num={8}
          title="Настройка погоды (опционально)"
          content={'Для отображения погоды на дисплее:\n1. Зарегистрируйтесь на openweathermap.org (бесплатно)\n2. Получите API ключ в разделе My API keys\n3. В скетче найдите строку: #define WEATHER_API_KEY "YOUR_API_KEY_HERE"\n4. Замените YOUR_API_KEY_HERE на ваш ключ\n5. Перекомпилируйте и загрузите прошивку'}
        />
      </div>

      {/* Troubleshooting */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">🔧 Решение проблем</h3>
        <div className="space-y-4">
          <TroubleItem
            problem="TFT белый экран"
            solution="Проверьте User_Setup.h. Убедитесь что выбран ILI9341_DRIVER и правильные пины. Проверьте питание 3.3V."
          />
          <TroubleItem
            problem="Нет звука"
            solution="Проверьте подключение I2S пинов (22, 25, 26). Убедитесь что DAC получает 5V. Проверьте динамик/усилитель."
          />
          <TroubleItem
            problem="WiFi не подключается"
            solution="Проверьте SSID и пароль. ESP32 работает только с 2.4GHz WiFi (не 5GHz). Убедитесь что сеть в зоне покрытия."
          />
          <TroubleItem
            problem="Кнопки не реагируют"
            solution="Проверьте что кнопки подключены к GND. Пины 32-35 имеют INPUT_PULLUP. Проверьте целостность кнопок мультиметром."
          />
          <TroubleItem
            problem="Ошибки компиляции"
            solution="Убедитесь что все библиотеки установлены актуальных версий. Проверьте что выбрана плата ESP32 Dev Module."
          />
        </div>
      </div>

      {/* Memory usage */}
      <div className="bg-gray-900 rounded-xl p-6 border border-gray-800">
        <h3 className="text-lg font-bold text-cyan-400 mb-4">💾 Использование памяти</h3>
        <div className="space-y-3">
          <MemoryBar label="Flash (прошивка)" used={78} total={100} color="blue" />
          <MemoryBar label="RAM (стек + данные)" used={65} total={100} color="green" />
          <MemoryBar label="NVS (станции)" used={40} total={100} color="purple" />
        </div>
        <p className="text-gray-500 text-sm mt-4">
          * Примерные значения. 20 станций занимают ~2KB в NVS. Прошивка ~780KB из доступных 4MB Flash.
        </p>
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

function LibCard({ name, version, author, desc }: { name: string; version: string; author: string; desc: string }) {
  return (
    <div className="bg-gray-800/50 rounded-lg p-3 border border-gray-700">
      <p className="text-white font-bold text-sm">{name}</p>
      <p className="text-purple-400 text-xs">v{version}</p>
      <p className="text-gray-500 text-xs mt-1">{author}</p>
      <p className="text-gray-400 text-xs mt-1">{desc}</p>
    </div>
  );
}

function StepCard({ num, title, content }: { num: number; title: string; content: string }) {
  return (
    <div className="flex gap-4 bg-gray-900 rounded-xl p-5 border border-gray-800">
      <div className="flex-shrink-0 w-10 h-10 bg-purple-600 rounded-full flex items-center justify-center text-white font-bold">
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

function MemoryBar({ label, used, total, color }: { label: string; used: number; total: number; color: string }) {
  const colorMap: Record<string, string> = {
    blue: 'bg-blue-500',
    green: 'bg-green-500',
    purple: 'bg-purple-500',
  };
  
  return (
    <div>
      <div className="flex justify-between text-sm mb-1">
        <span className="text-gray-400">{label}</span>
        <span className="text-gray-500">{used}%</span>
      </div>
      <div className="w-full h-3 bg-gray-800 rounded-full overflow-hidden">
        <div
          className={`h-full rounded-full ${colorMap[color] || 'bg-blue-500'} transition-all`}
          style={{ width: `${(used / total) * 100}%` }}
        />
      </div>
    </div>
  );
}

export default App;
