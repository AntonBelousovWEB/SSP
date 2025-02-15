# SuperPuper Protocol (SPP)

SuperPuper Protocol (SPP) — это кастомный интернет-протокол для работы с `.spp` файлами и управления сайтами на основе SPP.

## 🚀 Возможности
- Открытие и просмотр `.spp` файлов
- Список доступных SPP-сайтов
- Просмотр статистики соединения
- Поиск по всем `.spp` файлам
- Загрузка новых сайтов
- Поддержка авторизации

## 🛠 Установка и запуск
1. Соберите проект:
   ```sh
   make
   ```
2. Запустите сервер:
   ```sh
   ./spp_server
   ```
3. Подключитесь к серверу с помощью `netcat`:
   ```sh
   nc localhost 8080
   ```

## 🔑 Авторизация
При подключении потребуется ввести:
```
Username: Admin
Password: password
```

## 📜 Доступные команды
```
open <superpuper://site-salt/file.spp>  - Открыть и отобразить .spp файл
list                                    - Показать список всех доступных SPP-сайтов
stats                                   - Вывести статистику соединения
search <keyword>                        - Искать ключевое слово во всех .spp файлах
upload <path>                           - Загрузить новый сайт
help                                    - Показать справку по командам
quit                                    - Отключиться от сервера
```

## 📝 Пример работы
```sh
$ nc localhost 8080  
Username: Admin  
Password: password  

[INFO] Available commands:
  open <superpuper://site-salt/file.spp>
  list
  stats
  search <keyword>
  upload <path>
  help
  quit
```

## 📌 Лицензия
MIT License