# ESP32-Game-Engine (EDGE) — архитектура и взаимодействия

Этот документ описывает текущую архитектуру движка EDGE в этом форке: какие есть основные компоненты, как они связаны друг с другом и какой поток данных/управления во время работы игры.

## 1. Верхнеуровневая структура проекта

Основные файлы находятся в папке `src/`:

- `EDGE.h / EDGE.cpp` — "ядро" движка, которое соединяет все подсистемы: отрисовку, сцены и ввод.
- `Scene.h / Scene.cpp` — базовый класс для всех игровых сцен.
- `SceneManager.h / SceneManager.cpp` — менеджер сцен, стек сцен, фабрики и переходы между сценами.
- `Renderer.h / Renderer.cpp` — обёртка над `U8G2`, отвечает за рисование на дисплее.
- `InputManager.h / InputManager.cpp` — абстракция ввода, события кнопок, отложенные действия.
- `DisplayConfig.h` — конфигурация дисплея и его геометрии.

Дополнительно в корне:

- `platformio.ini` — конфигурация PlatformIO (цель: `dfrobot_beetle_esp32c3`).
- `library.json` / `library.properties` — метаданные Arduino/PlatformIO библиотеки.
- `README.md` — краткое описание движка и основных идей рефакторинга.

## 2. Главный класс движка: `EDGE`

**Файлы:** `src/EDGE.h`, `src/EDGE.cpp`

### 2.1. Роль

`EDGE` — фасад движка. Его задача:

- принять извне уже инициализированный объект `U8G2` и конфиг `DisplayConfig`;
- создать и связать `Renderer`, `SceneManager`, `InputManager`;
- пробрасывать логгер (`EDGELogger`) во все подсистемы;
- предоставлять методы `init()`, `update()`, `draw()`, которые необходимо вызывать из `loop()` в Arduino‑скетче.

### 2.2. Важные поля

```cpp
EDGELogger _logger;      // callback логгера, задаётся пользователем
SceneManager sceneManager;
Renderer     renderer;
InputManager inputManager;

unsigned long previousMillis;
unsigned long deltaTime;
```

`deltaTime` вычисляется каждый кадр и может использоваться сценами для анимации.

### 2.3. Жизненный цикл

- `EDGE::EDGE(U8G2* u8g2_ptr, const DisplayConfig& displayConf, EDGELogger logger)`
  - Сохраняет логгер и создаёт `renderer`, `sceneManager`, `inputManager`.
- `init()`
  - Передаёт логгер в `renderer`, `inputManager`, `sceneManager`.
  - Вызывает `renderer.init()`.
  - Вызывает `inputManager.init()`.
  - Связывает `SceneManager` и `InputManager`:
    - `sceneManager.setInputManager(&inputManager);`
    - `inputManager.setSceneManager(&sceneManager);`
- `update()`
  - Считает `deltaTime` на основе `millis()`.
  - `sceneManager.processSceneChanges()` — обрабатывает отложенные запросы смены сцены.
  - `inputManager.update(deltaTime)` — обрабатывает отложенные действия по вводу.
  - `sceneManager.update(deltaTime)` — обновляет активную сцену.
- `draw()`
  - Берёт текущую сцену из `SceneManager`.
  - Если сцена сама управляет страницами дисплея (`doesManageOwnDrawing()` возвращает `true`), просто делегирует `sceneManager.draw(renderer)`.
  - Иначе использует стандартный цикл `u8g2->firstPage()/nextPage()` и внутри него вызывает `sceneManager.draw(renderer)`.

Таким образом, `EDGE` — главный цикл: **время → ввод → логика сцен → отрисовка**.

## 3. Базовый класс сцены: `Scene`

**Файлы:** `src/Scene.h`, `src/Scene.cpp`

### 3.1. Назначение

`Scene` — абстрактная базовая сущность игрового экрана/состояния:

- Меню, игровой уровень, экран паузы и т.д.
- Содержит базовый интерфейс для жизненного цикла и рисования.

### 3.2. Интерфейс

Ключевые методы:

- `virtual ~Scene()` — виртуальный деструктор.
- `virtual void init()` — инициализация сцены (по умолчанию включает стандартное поведение по отрисовке).
- `virtual void onEnter()` — вызывается при входе на сцену (после добавления в стек).
- `virtual void onExit()` — вызывается при выходе со сцены (перед удалением/скрытием).
- `virtual void update(unsigned long deltaTime)` — обновление логики сцены каждый кадр.
- `virtual void draw(Renderer& renderer)` — отрисовка сцены.
- `virtual bool usesKeyQueue() const` — сообщает, использует ли сцена аппаратную очередь клавиш (для GEM / меню).
- `virtual void processKeyPress(uint8_t keyCode)` — обработка "сырых" кодов клавиш, если `usesKeyQueue() == true`.
- `virtual class DialogBox* getDialogBox()` — хук для сцен, которые могут содержать диалоговое окно (по умолчанию `nullptr`).

Флаг `managesOwnDrawing` влияет на то, кто отвечает за `firstPage()/nextPage()`:

- `false` (по умолчанию) — движок сам выполняет цикл страниц, сцена просто рисует своё содержимое.
- `true` — сцена полностью контролирует процесс отрисовки (возможно, для сложных/особых режимов).

### 3.3. Логгер

- Статическое поле `static EDGELogger _masterLogger;` — общий логгер для всех сцен.
- Статический метод `Scene::setMasterLogger(EDGELogger logger)` вызывается из `SceneManager::setLogger()`.
- Конкретные сцены могут использовать `_masterLogger` для отладочного вывода.

## 4. Менеджер сцен: `SceneManager`

**Файлы:** `src/SceneManager.h`, `src/SceneManager.cpp`

### 4.1. Роль

`SceneManager` управляет стеком сцен и их жизненным циклом:

- Поддерживает стек до `MAX_SCENES` сцен.
- Умеет:
  - регистрировать фабрики сцен по имени (`registerScene`);
  - переключать текущую сцену (`setCurrentScene`);
  - накладывать новую сцену поверх (`pushScene`);
  - убирать верхнюю сцену (`popScene`);
  - обрабатывать отложенные запросы на смену сцены (`requestSetCurrentScene`, `requestPushScene` + `processSceneChanges`).

### 4.2. Важные поля

- `Scene* sceneStack[MAX_SCENES];` — стек указателей на сцены.
- `String _sceneNameStack[MAX_SCENES];` — имена сцен в стеке.
- `int sceneCount;` — текущий размер стека.
- `InputManager* inputManager;` — ссылка на `InputManager` для автоматического снятия подписок.
- `EDGELogger _logger;` — логгер.
- `std::map<String, SceneFactoryFunction> _sceneFactories;` — регистр фабрик сцен.
- Поля для отложенных переключений:
  - `_pendingNextSceneName` / `_pendingConfigData` / `_pendingReplaceStack` / `_pendingSceneChange`.

`SceneFactoryFunction` — это `std::function<Scene*(void* configData)>`. Фабрика создаёт сцену, опционально используя конфиг.

### 4.3. Жизненный цикл сцен

- `registerScene(name, factory)` — вы регистрируете тип сцены по строковому имени.
- `requestSetCurrentScene(name, config)` или `requestPushScene(name, config)`
  - только ставят флаг `_pendingSceneChange` и заполняют поля `_pending*`.
  - это важно: **смена сцены не происходит сразу**, а откладывается до начала следующего кадра.
- `processSceneChanges()` (вызывается в `EDGE::update()`)
  - проверяет `_pendingSceneChange`;
  - в зависимости от `_pendingReplaceStack` вызывает `setCurrentScene` или `pushScene`.

### 4.4. `setCurrentScene`

- Проверяет, что есть `inputManager`.
- Сохраняет имя предыдущей сцены (`_previousSceneName`).
- Очищает весь стек (`clearStack()`), при этом:
  - для каждой сцены вызывает `inputManager->unregisterAllListenersForScene(scene)`;
  - `inputManager->clearDeferredActionsForScene(scene)`;
  - удаляет сцену и очищает имя.
- Создаёт новую сцену через `createSceneByName(sceneName, configData)`.
- Если успешно — кладёт в стек и вызывает `onEnter()` для новой сцены.

### 4.5. `pushScene`

- Проверяет `inputManager` и что стек не переполнен.
- Если есть сцена на вершине стека, вызывает у неё `onExit()` и сохраняет имя как `_previousSceneName`.
- Создаёт новую сцену через фабрику, кладёт в стек, вызывает `onEnter()`.

### 4.6. `popScene`

- Проверяет `inputManager` и что стек не пуст.
- Берёт верхнюю сцену, вызывает `onExit()`.
- Снимает все слушатели ввода и отложенные действия, связанные с этой сценой, через `InputManager`.
- Удаляет сцену и её имя из стека.
- Если в стеке ещё есть сцены, у новой верхней сцены вызывается `onEnter()`.

### 4.7. Обновление и отрисовка

- `update(dt)`
  - Если стек не пуст, вызывает `update(dt)` у верхней сцены.
- `draw(Renderer& renderer)`
  - Если стек не пуст, вызывает `draw(renderer)` у верхней сцены.

Таким образом, **в каждый момент времени "активна" ровно одна сцена** — верхняя в стеке.

## 5. Подсистема отрисовки: `Renderer`

**Файлы:** `src/Renderer.h`, `src/Renderer.cpp`

### 5.1. Роль

`Renderer` — тонкая обёртка над `U8G2`, которая:

- хранит указатель на внешний `U8G2` и конфиг `DisplayConfig`;
- знает размеры дисплея и смещения `xOffset`, `yOffset`;
- предоставляет удобные методы рисования (текст, линии, прямоугольники, круги и т.п.);
- умеет логировать ошибки/события (через `EDGELogger`).

### 5.2. Конфигурация дисплея

`DisplayConfig` (см. `DisplayConfig.h`) описывает:

- тип дисплея (`SSD1306` / `SH1106`);
- пины `clockPin`, `dataPin`, `resetPin`;
- указатель на структуру поворота `rotation` (например, `U8G2_R0`);
- реальные размеры дисплея `width` и `height`;
- флаг `useHardwareI2C`;
- смещения `xOffset`, `yOffset`.

Если сцена/игровая логика использует, например, виртуальное разрешение меньше физического, смещения позволяют центрировать изображение.

### 5.3. Важные методы `Renderer`

- `init()`
  - Настраивает `u8g2`: частоту шины, контраст, шрифт.
- `beginFrame()` / `endFrame()`
  - Обёртка над `clearBuffer()` и `sendBuffer()` для режима буферной отрисовки.
- Методы рисования:
  - `drawText`, `drawTextSafe` (форматированная строка через `vsnprintf`).
  - `drawCircle`, `drawFilledCircle`.
  - `drawRectangle`, `drawFilledRectangle`.
  - `drawLine`.
- `setFont`, `setFontSize` — управление шрифтами.
- `setContrast` — управление яркостью.

Все координаты проходят через `xOffset`/`yOffset`, чтобы учитывать возможное смещение области игры относительно физического экрана.

## 6. Подсистема ввода: `InputManager`

**Файлы:** `src/InputManager.h`, `src/InputManager.cpp`

### 6.1. Абстрактные типы ввода

`InputManager` оперирует абстрактными типами, не завязанных на конкретные кнопки железа:

- `enum class EDGE_Button { UP, DOWN, LEFT, RIGHT, OK, CANCEL }` — логические кнопки.
- `enum class EDGE_Event { PRESS, RELEASE, CLICK, LONG_PRESS }` — тип события.

Это позволяет:

- на уровне аппаратной обвязки преобразовать реальные GPIO/кнопки в `EDGE_Button`/`EDGE_Event`;
- на уровне сцен работать с абстрактным вводом.

### 6.2. Слушатели событий

`InputManager` хранит вектор `listeners`, где каждый элемент — `ListenerInfo`:

- `EDGE_Button button;`
- `EDGE_Event eventType;`
- `Scene* scene;` — сцена-владелец обработчика.
- `DeferredAction callback;` — функция, которую нужно выполнить при совпадении.

Регистрация и снятие слушателей:

- `registerButtonListener(button, eventType, scene, callback)` — добавить слушатель.
- `unregisterButtonListener(button, eventType, scene)` — удалить конкретный.
- `unregisterAllListenersForScene(scene)` — удалить все слушатели, связанные со сценой (вызывается из `SceneManager` при смене/удалении сцены).

### 6.3. Отложенные действия

Все колбэки ввода **не выполняются сразу**, а ставятся в очередь `_deferredActionsQueue`:

- Каждая запись — `DeferredActionEntry { Scene* ownerScene; DeferredAction action; }`.
- `processButtonEvent()` при нахождении подходящего слушателя вызывает `deferAction(scene, callback)`.
- `update(dt)` каждый кадр достаёт ограниченное число действий из очереди (по умолчанию до 3) и выполняет их.

Плюсы отложенных действий:

- исключают риск рекурсивных вызовов и сложных взаимозависимостей при обработке ввода;
- выравнивают нагрузку по кадрам.

### 6.4. Связь с `SceneManager`

`InputManager` хранит указатель `sceneManager` и (опционально) `keyQueue` (FreeRTOS очередь):

- `setSceneManager(SceneManager* sm)` — устанавливается в `EDGE::init()`.
- `setKeyQueue(QueueHandle_t queue)` — для интеграции с GEM/menu‑системой.

`processButtonEvent(button, eventType)` выполняет:

1. Обновляет время последней активности (`updateLastActivityTime()` — внешняя функция).
2. Получает текущую сцену и её имя из `sceneManager`.
3. Если сцена использует keyQueue (`usesKeyQueue()`):
   - переводит `EDGE_Button`/`EDGE_Event` в конкретный код GEM (`GEM_KEY_*` из `GEM_u8g2.h`);
   - отправляет его во FreeRTOS‑очередь `keyQueue`;
   - сцена позже заберёт этот код в `processQueuedKeys()` и передаст в `Scene::processKeyPress()`.
4. Если сцена **не** использует keyQueue:
   - ищет подходящие `ListenerInfo` (по кнопке, событию и текущей сцене);
   - для каждого находит `callback` и добавляет его в `_deferredActionsQueue`.

Отдельно есть метод `processQueuedKeys()`, который читает сырые коды из `keyQueue` и передаёт их сцене, если она хочет обрабатывать их напрямую.

### 6.5. Очистка при смене сцен

`SceneManager` при удалении сцены гарантирует вызов:

- `inputManager->unregisterAllListenersForScene(scene);`
- `inputManager->clearDeferredActionsForScene(scene);`

Так ввод не "утекает" на удалённые сцены.

## 7. Конфигурация и зависимости

### 7.1. Платформа и зависимости

Файл `platformio.ini` задаёт:

- Платформу: `espressif32`, плата `dfrobot_beetle_esp32c3`.
- Фреймворк: `arduino`.
- Библиотеки:
  - `U8g2` — графика/дисплей.
  - `ArduinoQueue` — очереди (используется в `Scene`, но фактическое применение сейчас минимально).

`library.json` дополнительно объявляет зависимости `U8g2` и `ArduinoQueue` для режима библиотеки.

### 7.2. DisplayConfig

`DisplayConfig` — часть абстракции hardware‑конфига. Внешний код:

- сам создаёт объект `DisplayConfig` с нужными параметрами дисплея;
- сам инициализирует `U8G2` (с конкретными пинами и интерфейсом I2C/SPI);
- передаёт и то, и другое в конструктор `EDGE`.

Это и есть ключевое отличие этого форка: **движок не занимается прямой инициализацией железа**, а принимает готовые объекты.

## 8. Поток управления за один кадр

С точки зрения Arduino‑скетча (упрощённо):

1. В `setup()`:
   - инициализируется `Serial`, железо, `U8G2`, `DisplayConfig`;
   - создаётся объект `EDGE edge(&u8g2, displayConfig, logger);`
   - регистрируются сцены в `edge.getSceneManager().registerScene(...)`;
   - выбирается стартовая сцена через `requestSetCurrentScene` и *один раз* вызывается `edge.init()`.

2. В `loop()` каждый кадр:

   - вызывается `edge.update();`
     - `sceneManager.processSceneChanges();`
     - `inputManager.update(deltaTime);` — выполняет отложенные действия ввода;
     - `sceneManager.update(deltaTime);` — обновляет текущую сцену;
   - вызывается `edge.draw();`
     - при необходимости запускает `firstPage()/nextPage()`;
     - делегирует рисование текущей сцене через `SceneManager::draw(renderer)`.

Внешний код (например, обработчик реальных кнопок) должен вызывать `edge.getInputManager().processButtonEvent(...)` или класть события в очередь `keyQueue`.

## 9. Как создавать свои сцены и интегрировать игру

Высокоуровнево процесс такой:

1. Создать свой класс сцены:
   - унаследоваться от `Scene`;
   - переопределить минимум `init()`, `update(dt)`, `draw(renderer)`;
   - при необходимости в `onEnter()` зарегистрировать слушатели ввода через `InputManager`;
   - в `onExit()` — освободить ресурсы/состояния, если нужно.
2. Определить фабрику:
   - функция или лямбда `Scene* mySceneFactory(void* config)`, которая создаёт сцену и вызывает её `init()`.
3. Зарегистрировать сцену в `SceneManager` по строковому имени.
4. В нужный момент вызвать `requestSetCurrentScene("MyScene")` или `requestPushScene("MyScene")`.

Если хочешь, могу следующим шагом:

- показать пример простейшей пользовательской сцены (меню или экран "Hello World");
- нарисовать схему (текстовую) вызовов для конкретного кадра или для сценария смены сцен.
