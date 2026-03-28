#include "db.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace heli {

static std::string read_file(const std::string& path, std::string& err) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        err = "cannot read: " + path;
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string resolve_sql_dir() {
    const char* e = std::getenv("HELI_SQL_DIR");
    if (e && e[0])
        return e;
    return "sql";
}

bool db_exec_sql_file(sqlite3* db, const std::string& path, std::string& err) {
    std::string sql = read_file(path, err);
    if (sql.empty() && !err.empty())
        return false;
    char* msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &msg);
    if (rc != SQLITE_OK) {
        err = msg ? msg : "sqlite3_exec failed";
        sqlite3_free(msg);
        return false;
    }
    return true;
}

static bool table_exists(sqlite3* db, const char* name) {
    sqlite3_stmt* st = nullptr;
    const char* q = "SELECT 1 FROM sqlite_master WHERE type='table' AND name=? LIMIT 1";
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_text(st, 1, name, -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(st) == SQLITE_ROW;
    sqlite3_finalize(st);
    return ok;
}

sqlite3* db_open_or_create(const std::string& path, const std::string& sql_dir, std::string& err) {
    sqlite3* db = nullptr;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        sqlite3_close(db);
        return nullptr;
    }
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    if (!table_exists(db, "AIR_HELICOPTERS")) {
        if (!db_exec_sql_file(db, sql_dir + "/HelicopterDelivery_create.sql", err)) {
            sqlite3_close(db);
            return nullptr;
        }
    }
    if (!db_exec_sql_file(db, sql_dir + "/extensions.sql", err)) {
        sqlite3_close(db);
        return nullptr;
    }

    const char* seed = std::getenv("HELI_SEED");
    if (seed && seed[0] == '1') {
        sqlite3_stmt* c = nullptr;
        int n = 0;
        if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM AIR_HELICOPTERS", -1, &c, nullptr) == SQLITE_OK) {
            if (sqlite3_step(c) == SQLITE_ROW)
                n = sqlite3_column_int(c, 0);
            sqlite3_finalize(c);
        }
        if (n == 0) {
            if (!db_exec_sql_file(db, sql_dir + "/seed.sql", err)) {
                sqlite3_close(db);
                return nullptr;
            }
        }
    }
    return db;
}

void db_close(sqlite3* db) {
    if (db)
        sqlite3_close(db);
}

} // namespace heli
