---
layout: page
---

# Схема базы данных

## Описание

База **SQLite**. Основная схема смоделирована под предметную область «Воздушный извозчик»: вертолёты, экипаж, рейсы, пользователи, конфигурация процентов. Дополнительно под ЛР4 добавлены таблица **начислений за период**, **медиа (BLOB)** и **триггер** контроля летного ресурса.

## Основные таблицы

| Таблица | Назначение |
|---------|-------------|
| `AIR_CONFIG` | Проценты от стоимости: обычный / спецрейс |
| `AIR_HELICOPTERS` | Вертолёты |
| `AIR_CREW` | Экипаж, FK на вертолёт |
| `AIR_FLIGHTS` | Рейсы, FK на вертолёт, флаг `is_special` |
| `AIR_USERS` | Логин, пароль, опциональная связь с экипажем (`NULL` = командир) |

## Расширения (`sql/extensions.sql`)

| Объект | Назначение |
|--------|-------------|
| `AIR_PAYROLL_PERIOD` | Результаты массового расчёта выплат за период |
| `AIR_IMAGES` | Хранение изображений (формат + BLOB), ссылка на сущность |
| `trg_air_flights_resource_check` | Запрет INSERT, если суммарная длительность после `date_last_fix` превысит `flight_resource` |

## SQL-файлы в репозитории

| Файл | Содержимое |
|------|------------|
| [HelicopterDelivery_create.sql]({{ site.github.repository_url }}/blob/main/sql/HelicopterDelivery_create.sql) | CREATE базовых таблиц |
| [extensions.sql]({{ site.github.repository_url }}/blob/main/sql/extensions.sql) | Таблицы начислений и изображений, триггер |
| [seed.sql]({{ site.github.repository_url }}/blob/main/sql/seed.sql) | Тестовые данные и пользователи |

## Визуальная схема

<img width="627" height="580" alt="HelicopterDelivery-2026-03-28_10-27" src="https://github.com/user-attachments/assets/b47e224f-1719-4997-b1ac-d9b649ba1099" />

