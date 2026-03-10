# Push-Cpp

Сервис пуш-рассылки (SPSC-style push server + клиенты)

Проект **push_broadcast сервис** — это сервис пуш-рассылки в стиле *SPSC (server → clients)*.

Проект состоит из:
- client_console 
- client_gui (Qt)
- client_lib
- database
- server
- tests 
---


## Зависимости

### Server
- SQLite3 — хранилище для push сообщений

### GUI
- Qt 6
- Qt test

### Tests
- GoogleTest

---

## Требования

### Обязательные
- **CMake ≥ 3.16**
- Conan ≥ 2.0
- **Компилятор с поддержкой C++20**
  - GCC ≥ 10
  - Clang ≥ 11
  - MSVC ≥ Visual Studio 2019
- **Boost**
  - header
  - filesystem
  - thread

### Для тестов
- **GoogleTest** (если `BUILD_TESTS=ON`)

### Для GUI
- **Qt 6**
  - Widgets
  - Test (только если включены Qt GUI тесты)

---

## Опции сборки

Проект использует следующие CMake-опции:

| Опция | Описание | По умолчанию |
|-----|--------|-------------|
| `BUILD_GUI` | Собирать Qt GUI клиент | `OFF` |
| `BUILD_TESTS` | Собирать unit-тесты (GoogleTest) | `ON` |
| `BUILD_QT_TESTS` | Собирать Qt GUI тесты | `OFF` |

### Автоматическая логика
Если включены `BUILD_GUI=ON` **и** `BUILD_TESTS=ON`, то: BUILD_QT_TESTS автоматически включается


## Конфигурация полной сборки

### 1 Установка Conan 2 (один раз)
Установить Conan 2 c сайта: https://conan.io/downloads

Нужно использовать conanfile из моей ветки.

Определение профиля:
conan profile detect --force

Узнать настройки для профиля default в Conan 2:
conan profile show -pr default

Если стоит в настройке compiler.cppstd=14 (тут стоит 14 стандарт CPP),
то нужно изменить на 20 стандарт CPP, читай далее.

Узнать путь к профилю : 
conan profile path default

Открой файл из пути и добавь / измени: 
[settings]
compiler.cppstd=14 на compiler.cppstd=20


### 2 Установка зависимостей и генерация toolchain
Собираться будет долго т.к. скачиваются библиотеки (для скачки пакета Qt нужен VPN)

conan install . \
  --output-folder=build \
  --build=missing \
  -pr default


### 3  Конфигурация CMake. Для полной сборки необходимо включить GUI и тесты:
В поле DCMAKE_TOOLCHAIN_FILE= нужно указать путь до conan_toolchain.cmake

cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=build/build/generators/conan_toolchain.cmake \
  -DBUILD_GUI=ON \
  -DBUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release

Опция BUILD_QT_TESTS включится автоматически,так как активны BUILD_GUI и BUILD_TESTS.


### 4 Сборка проекта
cmake --build build --config Release


### 5 Запуск тестов

После сборки выполните:

cd build
ctest --output-on-failure

Будут запущены: unit-тесты (GoogleTest) и GUI-тесты (Qt Test)

## Команды сборки для CI / GitHub Actions такие же как и для полной сборки 




