# Push Generator

`push_generator` — отдельный бинарник в проекте **Push-Cpp**, предназначенный для
асинхронной генерации и отправки push-сообщений на сервер по TCP.

Поддерживает:
- обычные пользовательские push
- административные (critical / error) push с авторизацией
- передачу **сырых байтов** без дополнительного протокола
- формат сообщений: **BinaryPushSerializer**

---

## Структура

```text
push_generator/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── push_generator.h
│   ├── admin_push_generator.h
│   └── random_push_factory.h
└── src/
    ├── push_generator.cpp
    ├── admin_push_generator.cpp
    └── main.cpp
```
## Возможности

- Асинхронная генерация (std::thread)
- User generator — случайные push-сообщения
- Admin generator — Critical / Error push
- Авторизация администратора (login / password)
- Использует реальный TcpClient из network/
- Сериализация через BinaryPushSerializer
- Отдельный бинарник (не библиотека)

## Типы генераторов

Обычный генератор
- случайный приоритет / категория / источник
- случайный текст сообщения

Административный генератор
- PushPriority::Critical
- PushCategory::Error
- перед первым push отправляет сообщение авторизации:

```php-template
AUTH|<login>|<password>
```

## Авторизация администратора

Авторизация реализована на уровне push-сообщения:
- выполняется один раз при первом GeneratePush
- используется тот же бинарный формат
- сервер сам решает, как обрабатывать AUTH|...

Пример текста push:
```pgsql
AUTH|admin|qwerty123
```

## Сборка
push_generator собирается как подпроект корневого Push-Cpp.

В результате будет собран бинарник:

```text
build/push_generator/push_generator
```
(путь может отличаться в зависимости от генератора)

## Запуск

```bash
./push_generator
```

При запуске будет предложено выбрать режим:
```text
Start as admin [y/n]:
```
- y — запуск admin-генератора (запрос пароля)
- n — запуск обычного генератора

## Сервер должен быть запущен заранее и слушать TCP-порт 8080.
