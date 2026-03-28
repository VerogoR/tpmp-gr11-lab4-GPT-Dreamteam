#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>

namespace heli {

/** Сохранить изображение (форматы: png, jpeg по префиксу magic bytes) */
bool storage_save_image(sqlite3* db, const std::string& ref_table, int ref_id, const std::string& format,
    const std::vector<unsigned char>& bytes, std::string& err);

/** Экспорт рейсов в CSV (разный формат хранения) */
bool storage_export_flights_csv(sqlite3* db, const std::string& path, std::string& err);

/** Простой JSON-дамп таблицы вертолётов */
bool storage_export_helicopters_json(sqlite3* db, const std::string& path, std::string& err);

} // namespace heli
