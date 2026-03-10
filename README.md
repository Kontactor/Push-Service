# Push-Cpp --- C++ Push Broadcast Server

## 📚 Оглавление

- [📌 Описание проекта](#about)
- [🧑‍💼 Архитектура проекта](#architect)
- [⚙️ Основной функционал](#functionality)
  - [🧠 Push Server](#push-server)
  - [🤖 Клиенты](#clients)
    - [🧑‍💼 Console Client](#console-client)
    - [🧑‍💼 GUI Client](#gui-client)
  - [🔄 Push Generator](#push-generator)
  - [💾 Database Module](#database)
  - [🧪 Tests](#tests)
- [⚙️ Технологии](#technology)
- [🧬 Зависимости](#dependency)
- [📦 Опции сборки](#options)
- [▶️ Сборка проекта](#build)
- [🚀 Запуск](#launch)
- [⚙️ Работа с Console Client](#console-client-work) 
- [🖥️ Архитектура взаимодействия](#interaction)
---

## Описание проекта <a name="about"></a>

**Push-Cpp** --- это сервис пуш-рассылки в стиле SPSC (Server →
Clients), реализованный на C++ с использованием Boost.Asio, SQLite и Qt.

Проект предназначен для отправки и приёма push сообщений. В проекте есть реализация : 
- клиент-серверной архитектуры
- сериализации данных
- работы с базой данных
- многопоточности
- архитектуры масштабируемого push-сервиса

------------------------------------------------------------------------

## Архитектура проекта <a name="architect"></a>

```text
    Push-Cpp
    │
    ├── server/                # Push сервер
    ├── client_console/        # Консольный клиент
    ├── client_gui/            # GUI клиент (Qt 6)
    ├── client_lib/            # Общая клиентская библиотека
    ├── push_generator/        # Генератор push-сообщений
    ├── database/              # SQLite слой хранения
    ├── common/                # Общие утилиты, сериализация, протокол
    ├── network/               # Сетевые протоколы и transport слой
    ├── tests/                 # Unit и интеграционные тесты
    ├── CMakeLists.txt
    ├── conanfile.txt
    └── README.md
```

------------------------------------------------------------------------

## Основной функционал <a name="functionality"></a>

### Push Server <a name="push-server"></a>

-   Асинхронный TCP сервер (Boost.Asio)
-   Регистрация клиентов
-   Отправка push-сообщений всем подписчикам
-   Административный push-канал
-   Хранение событий в SQLite
-   Поддержка JSON / Binary сериализации
-   Graceful shutdown

---
### Клиенты <a name="clients"></a>
---
#### Console Client <a name="console-client"></a>
-   Подключение к серверу
-   Получение push-уведомлений
-   Просмотр истории push-уведомлений

#### Особенности реализации:

работа клиента разделена на три потока:
- ввод команд пользователем через std::cin;
- вывод в std::cout push-уведомлений и системных сообщений от сервера из соответствующих очередей;
- сетевое взаимодействие и функции обратного вызова.
---
#### GUI Client (Qt 6) <a name="gui-client"></a>

-   Подписка на push-уведомления
-   UI отображение сообщений
-   История push-событий

---
### Push Generator <a name="push-generator"></a>

-   Симуляция push-событий
-   Admin режим (ручной ввод push)
-   Автогенерация событий

---
### Database Module <a name="database"></a>

-   SQLite3 storage
-   Таблицы ServerEvents
-   CRUD API
-   Тестируемый слой БД
#### Структура базы данных:

<img width="460" height="375" alt="QuickDBD-Push-server database scheme (3)" src="https://github.com/user-attachments/assets/931f0297-6ea6-423a-a8fa-197b58c56e28" />

---
### Tests <a name="tests"></a>

-   GoogleTest\
-   Unit тесты сериализации\
-   Тесты БД\
-   Интеграционные тесты клиента и сервера

------------------------------------------------------------------------

## Технологии <a name="technology"></a>

```text
  Технология   Назначение
  ------------ --------------------
  C++20        Основной язык
  Boost.Asio   Асинхронная сеть
  SQLite3      Хранение событий
  Qt 6         GUI клиент
  GoogleTest   Unit-тесты
  Conan        Dependency manager
  CMake        Сборка проекта
```
------------------------------------------------------------------------

## Зависимости <a name="dependency"></a>

### Обязательные

-   CMake ≥ 3.22
-   C++20 compiler (MSVC / GCC / Clang)
-   Conan 1.x или 2.x
-   Boost
-   SQLite3

### Опциональные

-   Qt 6 (для GUI клиента)
-   GoogleTest (для тестов)

------------------------------------------------------------------------

## Опции сборки <a name="options"></a>

Проект использует следующие CMake-опции:

| Опция | Описание | По умолчанию |
|-----|--------|-------------|
| `BUILD_GUI` | Собирать Qt GUI клиент | `OFF` |
| `BUILD_TESTS` | Собирать unit-тесты (GoogleTest) | `ON` |
| `BUILD_QT_TESTS` | Собирать Qt GUI тесты | `OFF` |
| `BUILD_DNETWORK_BUILD_EXAMPLES` | Собрать Demo network | `OFF` |
| `BUILD_DSERVER_BUILD_EXAMPLES` | Собрать Demo server | `OFF` |

### Автоматическая логика
Если включены `BUILD_GUI=ON` **и** `BUILD_TESTS=ON`, то: BUILD_QT_TESTS автоматически включается

------------------------------------------------------------------------

## Сборка проекта <a name="build"></a>

### 1 Установка Conan 2 (один раз)
Установить Conan 2 c сайта:

```text
https://conan.io/downloads
```

Определение профиля Conan:

```bash
conan profile detect --force
```

Узнать настройки для профиля default в Conan 2:

```bash
conan profile show -pr default
```

Если стоит в настройке compiler.cppstd=14 (тут стоит 14 стандарт CPP),
то нужно изменить на 20 стандарт CPP, читай далее.

Узнать путь к профилю : 

```bash
conan profile path default
```

Открой файл из пути и добавь / измени: 

```text
[settings]
compiler.cppstd=14 на compiler.cppstd=20
```

### 2 Установка зависимостей и генерация toolchain
Собираться будет долго (примерно 40 минут) т.к. скачиваются библиотеки (для скачки пакета Qt нужен VPN)

```bash
conan install . \
  --output-folder=build \
  --build=missing \
  -pr default
```

### 3  Конфигурация CMake. Для полной сборки необходимо включить все флаги сборки(см. далее):

В поле DCMAKE_TOOLCHAIN_FILE= (см. далее) нужно указать путь до conan_toolchain.cmake

```bash
cmake -S . -B build
-DCMAKE_TOOLCHAIN_FILE=build/build/generators/conan_toolchain.cmake
-DBUILD_GUI=ON
-DBUILD_TESTS=ON
-DNETWORK_BUILD_EXAMPLES=ON
-DSERVER_BUILD_EXAMPLES=ON
-DCMAKE_BUILD_TYPE=Release
```

### 4 Сборка проекта

```bash
cmake --build build --config Release
```

### 5 Запуск тестов

После сборки выполните:

```bash
cd build
ctest --output-on-failure
```

------------------------------------------------------------------------


## Запуск <a name="launch"></a>

#### Запуск сервера

``` bash
./push_server
```

#### Запуск push генератора

``` bash
./push_generator
```

#### Запуск консольного клиента

``` bash
./client_console
```

#### Запуск GUI клиента

``` bash
./client_gui
```

------------------------------------------------------------------------

## База данных

Используется SQLite файл:

```text
    push_storage.db
```

------------------------------------------------------------------------

## Работа с Console Client <a name="console-client-work"></a>

После запуске программы в терминале будут выведены доступные команды:

<img width="1015" height="146" alt="start" src="https://github.com/user-attachments/assets/73f3de37-50cf-48ae-ba4d-24581d97ebe8" />

- connect для подключения к серверу (затем нужно ввести хост и порт)
  После успешного подключения к серверу мы получаем пуши (они сохраняются во внутреннюю очередь)
- show для вывода на экран получаемых push-уведомлений
- status для показа текущего состояния подключения
- history для вывода на экран истории push-уведомлений
- clear для очистки истории push-уведомлений
- disconnect для отключения от сервера
- exit для выхода из программы
- help для вывода доступных команд

Для соединения с сервером нужно ввести команду `connect` и нажать `ENTER`, затем ввести хост и нажать `ENTER`, ввести порт и нажать `ENTER`.
Программа выведет сообщение об успешном или неуспешном подключении к серверу.

<img width="1014" height="73" alt="connect" src="https://github.com/user-attachments/assets/0cd82764-0649-4fd8-b1a6-b0471c50f41e" />

После успешного подключения к серверу программа начинает получать push-уведомления, которые сохраняются во внутреннюю очередь.
Чтобы вывести на экран полученные и вновь получаемые push-уведомления нужно ввести команду `show` и нажать `ENTER`.

<img width="1014" height="115" alt="show" src="https://github.com/user-attachments/assets/f00785f3-1488-42af-a77a-7b7de58853a9" />

Чтобы прекратить вывод на экран получаемых push-уведомлений, нужно нажать `ENTER`.
Чтобы вывести все полученные push-уведомления, нужно ввести команду `history` и нажать `ENTER`:

<img width="1013" height="154" alt="history" src="https://github.com/user-attachments/assets/2069574a-1723-4bce-ad9c-8f75d6cb1a27" />

Для выхода из программы нужно ввести команду `exit` и нажать `ENTER`. Программа автоматически закроет соединение с сервером и завершится.

<img width="1006" height="43" alt="exit" src="https://github.com/user-attachments/assets/a0f5be10-d021-4072-ab81-961d39398161" />

------------------------------------------------------------------------

## Архитектура взаимодействия <a name="interaction"></a>

```text
    Push Generator → Server → Clients (GUI / Console)
                             ↓
                         SQLite Storage
```

------------------------------------------------------------------------


