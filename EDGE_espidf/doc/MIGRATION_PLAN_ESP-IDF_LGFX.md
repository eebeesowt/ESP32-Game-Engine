# План миграции на ESP-IDF и LovyanGFX

Этот документ описывает поэтапный план перевода текущего форка EDGE:
- с Arduino framework на ESP-IDF;
- с библиотеки U8g2 на LovyanGFX для отрисовки.

Цель — сохранить архитектуру движка (Scene/SceneManager/InputManager), но заменить нижележащие слои платформы и графики.

---

## Этап 0. Анализ текущих зависимостей

**Цель:** чётко понимать, что именно завязано на Arduino API и U8g2.

1. **Зависимости от Arduino:**
   - Макросы и заголовки `#include <Arduino.h>` (типы `String`, `millis()`, `uint8_t`, `Serial` и т.д.).
   - Внешний цикл `setup()/loop()` (ожидается скетч, а не `app_main`).
   - Библиотеки `ArduinoQueue`, использование `String` вместо `std::string`.
2. **Зависимость от U8g2:**
   - Заголовки `#include <U8g2lib.h>`.
   - Тип `U8G2*` внутри `EDGE`, `Renderer`, `DisplayConfig`.
   - Логика `firstPage()/nextPage()` и методы рисования (`drawStr`, `drawCircle`, `drawDisc`, `drawBox`, `drawFrame`, `drawLine`, `setFont`, `setContrast`).
3. **Зависимости от FreeRTOS/ESP-IDF уже есть:**
   - `freertos/FreeRTOS.h`, `freertos/queue.h` в `InputManager`.

**Результат:** список точек, которые нужно адаптировать:
- платформа/входная точка (Arduino → ESP-IDF `app_main()`);
- абстракции времени, логгирование, строки;
- графический backend `U8G2` → `LovyanGFX`;
- конфиг дисплея `DisplayConfig`.

---

## Этап 1. Подготовка конфигурации ESP-IDF

**Цель:** перевести проект на сборку под ESP-IDF без изменения логики движка.

1. **Создать отдельную ветку/копию под ESP-IDF** (рекомендуется):
   - Например ветка `esp-idf-port`.
2. **Структура проекта ESP-IDF:**
   - Создать минимальную структуру:
     - `CMakeLists.txt` в корне проекта;
     - `main/CMakeLists.txt`;
     - `main/main.cpp` (или `.c`, но лучше C++ для интеграции с существующими классами).
   - Подключить исходники движка из `src/` как часть компонента:
     - Либо перенести `src/` в `components/edge/src/`;
     - Либо добавить `src/*.cpp` в `main/CMakeLists.txt`.
3. **Настроить sdkconfig**:
   - Через `idf.py menuconfig` включить поддерживаемый дисплейный интерфейс (SPI/I2C) — понадобится для LovyanGFX, но уже сейчас можно зафиксировать пины.
4. **Сборка "пустого" приложения:**
   - В `main.cpp` сделать минимальный `app_main()` с логом через `ESP_LOGI`.
   - Убедиться, что базовая сборка ESP-IDF успешна.

**Результат:** каркас ESP-IDF проекта, способный собираться и запускаться без движка.

---

## Этап 2. Абстрагирование зависимостей от Arduino

**Цель:** минимизировать связку движка с Arduino API, чтобы затем проще собирать и под Arduino, и под ESP-IDF (если нужно).

1. **Время и типы:**
   - Ввести тонкую обёртку над временем:
     - например, `EDGE_Time::millis()` или функцию `unsigned long edge_millis()`;
     - в Arduino-конфигурации она будет вызывать `::millis()`;
     - в ESP-IDF будет использовать `esp_timer_get_time()` или `xTaskGetTickCount()`.
   - Заменить прямые вызовы `millis()` в `EDGE::update()` на обёртку.
2. **Логгирование:**
   - Логгер уже абстрактен (`EDGELogger` — `std::function<void(const char*)>`).
   - Для ESP-IDF реализовать логгер через `ESP_LOGI/ESP_LOGW/ESP_LOGE` внутри функции-адаптера.
3. **Строки `String`:**
   - В `SceneManager` активно используется `String` (Arduino).
   - Варианты:
     - а) На первом шаге оставить `String`, используя esp-idf-совместимую реализацию Arduino core (esp-idf as component with Arduino); или
     - б) Постепенно заменить `String` на `std::string` + свои typedef’ы.
   - План: **для ускорения начальной миграции** использовать Arduino-совместимый слой поверх ESP-IDF (вариант а), а затем отдельным этапом перейти на чистый `std::string`.
4. **ArduinoQueue:**
   - Определить, действительно ли она нужна (сейчас в `Scene` только поле, без логики).
   - Если возможно — удалить использование или заменить на `std::queue`/`std::deque`.

**Результат:** движок с минимальным количеством прямых зависимостей от Arduino, что позволяет выбирать: использовать Arduino core как компонент ESP-IDF или идти к "чистому" ESP-IDF.

---

## Этап 3. Интеграция движка с `app_main` (ESP-IDF entrypoint)

**Цель:** научиться создавать и запускать EDGE из `app_main` вместо `setup/loop`.

1. **Создать файл `main/main.cpp` для ESP-IDF:**
   - Объявить функцию логгера `void edgeLogger(const char* msg)` с использованием `ESP_LOGI`.
   - Инициализировать аппаратный дисплей (SPI/I2C) и объект `LovyanGFX` (пока заглушка, на этом этапе можно временно оставить U8g2 или мок).
   - Создать `DisplayConfig`.
   - Создать объект `EDGE edge(&gfx_or_u8g2, displayConfig, edgeLogger);`
   - Инициализировать сцены (регистрация фабрик) и вызвать `edge.init()`.
2. **Главный цикл:**
   - В `app_main()` запустить цикл, аналогичный Arduino `loop()`:
     - `while (true) { edge.update(); edge.draw(); vTaskDelay(pdMS_TO_TICKS(16)); }`.
   - Альтернатива: создать отдельную задачу FreeRTOS `xTaskCreate(gameTask, ...)` и там вызывать `update()/draw()`.

**Результат:** EDGE работает под ESP-IDF, но пока всё ещё может использовать U8g2 как backend (или заглушку).

---

## Этап 4. План замены U8g2 на LovyanGFX

**Цель:** спроектировать, как именно LovyanGFX впишется в текущую архитектуру (минимум изменений в `Scene`/`SceneManager`/`InputManager`).

1. **Определить целевой класс дисплея LovyanGFX:**
   - Например, `lgfx::LGFX_Device` или конкретный класс для вашего контроллера (SSD1306, SH1106 и т.п.).
   - Настроить его в отдельном файле `LGFX_Config.h`/`LGFX_Display.h`.
2. **Выбрать стратегию замены:**
   - Вариант A (прямой): заменить `U8G2*` на абстрактный интерфейс `IGraphics` и реализовать адаптер для LovyanGFX.
   - Вариант B (минимальные правки): заменить тип `U8G2*` на конкретный тип `LovyanGFX` и переписать методы `Renderer`.

**Рекомендуемый путь:**
- Ввести абстрактный интерфейс `IGraphics` с базовыми операциями (text, line, rect, circle, clear, display). Реализация для LovyanGFX (`LGFXGraphicsAdapter`).
- `Renderer` будет работать через `IGraphics*`, а внутри адаптера вызывать нужные методы LovyanGFX.

3. **Карта соответствий функций:**
   - `U8G2::clearBuffer()` → `LovyanGFX::fillScreen()` / буферный режим.
   - `U8G2::sendBuffer()` → `LovyanGFX::display()` (если используется sprite/Offscreen).
   - `U8G2::drawStr()` → `LovyanGFX::drawString()` или `drawFont`.
   - `U8G2::drawCircle()` → `LovyanGFX::drawCircle()`.
   - `U8G2::drawDisc()` → `LovyanGFX::fillCircle()`.
   - `U8G2::drawFrame()` → `LovyanGFX::drawRect()`.
   - `U8G2::drawBox()` → `LovyanGFX::fillRect()`.
   - `U8G2::drawLine()` → `LovyanGFX::drawLine()`.
   - `setFont()`/`setFontSize()` → семантически близкие методы LovyanGFX.

**Результат:** чёткое понимание, какие графические вызовы нужно адаптировать.

---

## Этап 5. Рефакторинг `Renderer` и `DisplayConfig` под LovyanGFX

**Цель:** сделать `Renderer` независимым от U8g2 и завязанным на абстракцию, которую можно реализовать через LovyanGFX.

1. **Ввести интерфейс `IGraphics`:**
   - В новом файле `src/IGraphics.h` объявить класс с виртуальными методами:
     - `virtual void clear() = 0;`
     - `virtual void present() = 0;`
     - `virtual void drawText(int x, int y, const char* text) = 0;`
     - `virtual void drawCircle(int x, int y, int r) = 0;`
     - `virtual void fillCircle(int x, int y, int r) = 0;`
     - `virtual void drawRect(int x, int y, int w, int h) = 0;`
     - `virtual void fillRect(int x, int y, int w, int h) = 0;`
     - `virtual void drawLine(int x1, int y1, int x2, int y2) = 0;`
     - и т.п.
2. **Сделать `Renderer` работать через `IGraphics*`:**
   - Заменить `U8G2* u8g2;` на `IGraphics* gfx;`.
   - В конструктор `Renderer` передавать `IGraphics*` + `DisplayConfig`.
   - Переписать методы `init()`, `beginFrame()`, `endFrame()`, `drawText()` и т.д. на вызовы `gfx->...`.
3. **Адаптер для LovyanGFX:**
   - Создать `src/LGFXGraphicsAdapter.h/.cpp`:
     - внутри хранить `LovyanGFX` (конкретный тип: например, `lgfx::LGFX_Device display;`).
     - реализовать интерфейс `IGraphics` через методы LovyanGFX (см. карту соответствий).
   - Инициализация конкретного дисплея (пины, размеры) будет внутри адаптера или отдельного файла конфигурации.
4. **Переписать `EDGE` конструктор:**
   - Вместо `EDGE(U8G2* u8g2_ptr, const DisplayConfig& displayConf, EDGELogger logger)` → опция:
     - `EDGE(IGraphics* gfx, const DisplayConfig& displayConf, EDGELogger logger)`.
   - Соответственно, обновить создание `Renderer renderer(gfx, displayConf);`.
5. **Актуализировать `DisplayConfig`:**
   - Оставить общие поля (`width`, `height`, offsets), они полезны и для LovyanGFX.
   - Специфичные для U8g2 поля (rotation, useHardwareI2C) либо удалить, либо переосмыслить под LovyanGFX.

**Результат:** все сцены и менеджеры продолжают работать с `Renderer`, но под капотом используется LovyanGFX через адаптер.

---

## Этап 6. Интеграция LovyanGFX в ESP-IDF проект

**Цель:** физически подключить и настроить LovyanGFX под нужный дисплей.

1. **Подключение LovyanGFX как компонента:**
   - Добавить LovyanGFX в `components/` (либо через `git submodule`, либо вручную).
   - Настроить `CMakeLists.txt` для компонента LovyanGFX.
2. **Создать файл конфигурации дисплея LovyanGFX:**
   - Например, `components/lgfx_display/LGFX_Display.h`.
   - Описать пины, интерфейс (SPI/I2C), параметры контроллера.
3. **Инициализация в `app_main`:**
   - Создать экземпляр `LGFX_Display`.
   - Вызвать `init()`/`begin()` LovyanGFX.
   - Обернуть его в `LGFXGraphicsAdapter` и передать в `EDGE`.

**Результат:** EDGE крутится под ESP-IDF, рисуя через LovyanGFX на реальном дисплее.

---

## Этап 7. Очистка и выравнивание API

**Цель:** навести порядок после миграции, чтобы API был единообразным и лёгким для дальнейших улучшений.

1. **Удалить или изолировать Arduino-зависимый код:**
   - Удалить ненужные `#include <Arduino.h>` из файлов, где он больше не нужен.
   - Либо полностью отказаться от Arduino core, либо оставить его только в отдельной конфигурации (если важно).
2. **Уточнить типы строк:**
   - Определить единый подход (либо `std::string`, либо свой typedef поверх неё) для новых участков кода.
3. **Актуализировать документацию:**
   - Обновить `README.md` и `ENGINE_STRUCTURE.md`, описав:
     - переход на ESP-IDF;
     - использование LovyanGFX;
     - пример интеграции сцены в ESP-IDF.
4. **Добавить минимальные примеры:**
   - Пример простой сцены "Hello, EDGE".
   - Пример сцены, использующей keyQueue и кнопки.

---

## Этап 8. Тестирование и оптимизация

**Цель:** убедиться, что движок стабильно работает на ESP-IDF + LovyanGFX и не деградировал по производительности.

1. **Функциональные тесты:**
   - Проверка переключения сцен (push/pop/setCurrentScene).
   - Проверка ввода (EDGE_Button/EDGE_Event → делегирование в сцену и keyQueue).
   - Проверка отрисовки (текст, геометрия, разные шрифты).
2. **Стресс-тест:**
   - Много быстрых переключений сцен.
   - Много событий ввода подряд.
3. **Оптимизация:**
   - Проверка частоты кадров (FPS), нагрузки на CPU/память.
   - При необходимости оптимизировать количество аллокаций, форматирование строк, логгирование.

---

## Резюме

План разделён на независимые, но логически связанные шаги:
- сначала — каркас ESP-IDF и минимальное отвязывание от Arduino;
- затем — перенос главной петли `setup/loop` в `app_main`;
- после — абстрагирование графического backend’а и переключение на LovyanGFX;
- в конце — зачистка, документация и тестирование.

Дальше можно детализировать любой из этапов (например, сразу расписать конкретный `CMakeLists.txt` и пример `app_main`), когда ты решишь, с какого шага начинать миграцию на практике.
