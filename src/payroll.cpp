#include "payroll.h"

#include <ctime>
#include <sstream>

namespace heli {

static std::string utc_now() {
    std::time_t t = std::time(nullptr);
    std::tm* g = std::gmtime(&t);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", g);
    return buf;
}

bool payroll_compute_squad_period(sqlite3* db, const std::string& period_start, const std::string& period_end, std::string& err) {
    char* emsg = nullptr;
    sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr);

    const char* del = "DELETE FROM AIR_PAYROLL_PERIOD WHERE period_start = ? AND period_end = ?";
    sqlite3_stmt* dst = nullptr;
    if (sqlite3_prepare_v2(db, del, -1, &dst, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(dst, 1, period_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(dst, 2, period_end.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(dst);
    sqlite3_finalize(dst);

    const char* ins = R"(
        INSERT INTO AIR_PAYROLL_PERIOD (period_start, period_end, computed_at, crew_number, amount_cents)
        SELECT ?, ?, ?, cr.number,
               COALESCE(SUM(
                 CASE WHEN f.is_special THEN f.flight_cost * cfg.SPECIAL_FLIGHT_PCT / 100
                      ELSE f.flight_cost * cfg.REGULAR_FLIGHT_PCT / 100 END
               ), 0)
        FROM AIR_CREW cr
        JOIN AIR_CONFIG cfg ON cfg.id = 1
        LEFT JOIN AIR_FLIGHTS f ON f.AIR_HELICOPTERS_number = cr.AIR_HELICOPTERS_number
            AND f.date >= ? AND f.date <= ?
        GROUP BY cr.number
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, ins, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    std::string now = utc_now();
    sqlite3_bind_text(st, 1, period_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, period_end.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 3, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 4, period_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 5, period_end.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_exec(db, "COMMIT", nullptr, &emsg, nullptr);
    if (emsg) {
        err = emsg;
        sqlite3_free(emsg);
        return false;
    }
    return true;
}

bool payroll_pilot_period_amount(sqlite3* db, int crew_number, const std::string& period_start,
    const std::string& period_end, long long& amount_out, std::string& err) {
    const char* q = R"(
        SELECT COALESCE(SUM(
            CASE WHEN f.is_special THEN f.flight_cost * cfg.SPECIAL_FLIGHT_PCT / 100
                 ELSE f.flight_cost * cfg.REGULAR_FLIGHT_PCT / 100 END
        ), 0)
        FROM AIR_CREW cr
        JOIN AIR_CONFIG cfg ON cfg.id = 1
        JOIN AIR_FLIGHTS f ON f.AIR_HELICOPTERS_number = cr.AIR_HELICOPTERS_number
            AND f.date >= ? AND f.date <= ?
        WHERE cr.number = ?
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, period_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, period_end.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 3, crew_number);
    amount_out = 0;
    if (sqlite3_step(st) == SQLITE_ROW)
        amount_out = sqlite3_column_int64(st, 0);
    sqlite3_finalize(st);
    return true;
}

bool payroll_pilot_period_by_type(sqlite3* db, int crew_number, const std::string& period_start,
    const std::string& period_end, bool only_special, long long& amount_out, std::string& err) {
    std::ostringstream q;
    q << R"(
        SELECT COALESCE(SUM(
            CASE WHEN f.is_special THEN f.flight_cost * cfg.SPECIAL_FLIGHT_PCT / 100
                 ELSE f.flight_cost * cfg.REGULAR_FLIGHT_PCT / 100 END
        ), 0)
        FROM AIR_CREW cr
        JOIN AIR_CONFIG cfg ON cfg.id = 1
        JOIN AIR_FLIGHTS f ON f.AIR_HELICOPTERS_number = cr.AIR_HELICOPTERS_number
            AND f.date >= ? AND f.date <= ?
        WHERE cr.number = ?
    )";
    if (only_special)
        q << " AND f.is_special = 1";
    else
        q << " AND f.is_special = 0";

    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q.str().c_str(), -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, period_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, period_end.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 3, crew_number);
    amount_out = 0;
    if (sqlite3_step(st) == SQLITE_ROW)
        amount_out = sqlite3_column_int64(st, 0);
    sqlite3_finalize(st);
    return true;
}

} // namespace heli
