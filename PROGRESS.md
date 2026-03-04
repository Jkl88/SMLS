# Прогресс проекта SMLS (Smart Motion Light System)

Соответствие пунктам из `.cursorrules`.

| № | Задача | Статус |
|---|--------|--------|
| 1 | Создать проект по ТЗ из README.md (PlatformIO, Arduino) | Сделано |
| 2 | Учесть использование библиотеки TKWifiManager | Сделано (lib_deps) |
| 3 | Таргеты master, LED_controller (основные), esp32dev_master, esp32dev_led_controller (конечные) | Сделано |
| 4 | Пины в основных таргетах, в конечных только переопределение | Сделано |
| 5 | Pre-скрипт: сбор дефайнов в config для линтера и Arduino IDE | Сделано (pre_build_config.py) |
| 6 | Структура проекта и аннотации на русском | Сделано |
| 7 | Пост-скрипт упаковки для прошивки (post_build_package.py) | Сделано, проверен |
| 8 | flash_auto.bat для прошивки | Проверен, используется как шаблон |
| 9 | Файл прогресса проекта (этот файл) | Ведётся |
| 10 | Создать и выгрузить проект в GitHub | Git инициализирован, первый коммит создан |

## Текущая стадия

- Прошивка Master и LED Controller реализованы.
- Сборка: `pio run -e esp32dev_master`, `pio run -e esp32dev_led_controller`.
- LittleFS (веб): `pio run -e esp32dev_master -t buildfs`, `-t uploadfs`.
- После настройки репозитория — выгрузка на GitHub.

### Выгрузка на GitHub

Репозиторий инициализирован, выполнен первый коммит. Чтобы выгрузить на GitHub:

1. Создайте новый репозиторий на GitHub (без README).
2. Выполните в каталоге проекта:
   ```
   git remote add origin https://github.com/<user>/<repo>.git
   git push -u origin master
   ```
