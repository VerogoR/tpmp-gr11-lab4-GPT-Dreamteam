#pragma once

#include <sqlite3.h>
#include <string>

namespace heli {

/** Открыть БД, при необходимости применить sql/HelicopterDelivery_create.sql и extensions.sql */
sqlite3* db_open_or_create(const std::string& path, const std::string& sql_dir, std::string& err);

/** Выполнить SQL-файл построчно (простой split по ';') */
bool db_exec_sql_file(sqlite3* db, const std::string& path, std::string& err);

void db_close(sqlite3* db);

/** Путь к каталогу sql относительно cwd или переменной HELI_SQL_DIR */
std::string resolve_sql_dir();

} // namespace heli
