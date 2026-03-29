#include "queries.h"

#include <sstream>

namespace heli {

static bool crew_ok_heli(const Session& s, int heli_num) {
    if (s.role == Role::Commander)
        return true;
    return s.helicopter_number == heli_num;
}

std::string q_hours_and_resource(sqlite3* db, const Session& s, std::string& err) {
    (void)err;
    std::ostringstream os;
    const char* q = R"(
        SELECT h.number, h.mark, h.date_last_fix, h.flight_resource AS resource_hours,
               COALESCE(SUM(CASE WHEN f.date >= h.date_last_fix THEN f.flight_duration ELSE 0 END), 0) AS hours_after_repair
        FROM AIR_HELICOPTERS h
        LEFT JOIN AIR_FLIGHTS f ON f.AIR_HELICOPTERS_number = h.number
        WHERE (? = 1 OR h.number = ?)
        GROUP BY h.number
        ORDER BY h.number
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    int all = (s.role == Role::Commander) ? 1 : 0;
    sqlite3_bind_int(st, 1, all);
    sqlite3_bind_int(st, 2, s.helicopter_number);
    os << "вертолёт | марка | дата капремонта | ресурс ч | налёт после капремонта\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << reinterpret_cast<const char*>(sqlite3_column_text(st, 1)) << " | "
           << reinterpret_cast<const char*>(sqlite3_column_text(st, 2)) << " | "
           << sqlite3_column_int(st, 3) << " | "
           << sqlite3_column_int64(st, 4) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

std::string q_flights_in_period(sqlite3* db, const Session& s, const std::string& d1, const std::string& d2, std::string& err) {
    (void)err;
    std::ostringstream os;
    const char* q = R"(
        SELECT f.flight_code, f.date, f.AIR_HELICOPTERS_number, f.cargo_weight, f.amount_people, f.is_special,
               SUM(f.cargo_weight) OVER (PARTITION BY f.AIR_HELICOPTERS_number) AS sum_cargo,
               SUM(f.amount_people) OVER (PARTITION BY f.AIR_HELICOPTERS_number) AS sum_people
        FROM AIR_FLIGHTS f
        WHERE f.date >= ? AND f.date <= ? AND (? = 1 OR f.AIR_HELICOPTERS_number = ?)
        ORDER BY f.AIR_HELICOPTERS_number, f.date, f.flight_code
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    sqlite3_bind_text(st, 1, d1.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, d2.c_str(), -1, SQLITE_TRANSIENT);
    int all = (s.role == Role::Commander) ? 1 : 0;
    sqlite3_bind_int(st, 3, all);
    sqlite3_bind_int(st, 4, s.helicopter_number);
    os << "рейс | дата | вертолёт | груз | люди | спец | сумм_груз | сумм_люди (по вертолёту за период)\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << reinterpret_cast<const char*>(sqlite3_column_text(st, 1)) << " | "
           << sqlite3_column_int(st, 2) << " | "
           << sqlite3_column_int(st, 3) << " | "
           << sqlite3_column_int(st, 4) << " | "
           << sqlite3_column_int(st, 5) << " | "
           << sqlite3_column_int64(st, 6) << " | "
           << sqlite3_column_int64(st, 7) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

std::string q_special_totals(sqlite3* db, std::string& err) {
    (void)err;
    std::ostringstream os;
    const char* q = R"(
        SELECT f.AIR_HELICOPTERS_number,
               COUNT(*) AS flights,
               SUM(f.cargo_weight) AS total_cargo,
               SUM(f.flight_cost * cfg.SPECIAL_FLIGHT_PCT / 100) AS money_crew_total
        FROM AIR_FLIGHTS f
        JOIN AIR_CONFIG cfg ON cfg.id = 1
        WHERE f.is_special = 1
        GROUP BY f.AIR_HELICOPTERS_number
        ORDER BY f.AIR_HELICOPTERS_number
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    os << "спецрейсы: вертолёт | рейсов | масса | сумма начислений (всем летчикам по 10%%)\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << sqlite3_column_int64(st, 1) << " | "
           << sqlite3_column_int64(st, 2) << " | "
           << sqlite3_column_int64(st, 3) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

std::string q_regular_totals(sqlite3* db, std::string& err) {
    (void)err;
    std::ostringstream os;
    const char* q = R"(
        SELECT f.AIR_HELICOPTERS_number,
               COUNT(*) AS flights,
               SUM(f.cargo_weight) AS total_cargo,
               SUM(f.flight_cost * cfg.REGULAR_FLIGHT_PCT / 100) AS money_crew_total
        FROM AIR_FLIGHTS f
        JOIN AIR_CONFIG cfg ON cfg.id = 1
        WHERE f.is_special = 0
        GROUP BY f.AIR_HELICOPTERS_number
        ORDER BY f.AIR_HELICOPTERS_number
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    os << "обычные рейсы: вертолёт | рейсов | масса | сумма начислений (всем летчикам по 5%%)\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << sqlite3_column_int64(st, 1) << " | "
           << sqlite3_column_int64(st, 2) << " | "
           << sqlite3_column_int64(st, 3) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

std::string q_heli_max_flights_crew(sqlite3* db, std::string& err) {
    (void)err;
    std::ostringstream os;
    const char* q = R"(
        WITH cnt AS (
            SELECT AIR_HELICOPTERS_number AS hn, COUNT(*) AS c
            FROM AIR_FLIGHTS GROUP BY AIR_HELICOPTERS_number
        ), mx AS (SELECT MAX(c) AS mc FROM cnt)
        SELECT h.number, h.mark, cr.number, cr.surname, cr.grade,
               (SELECT COALESCE(SUM(
                  CASE WHEN f.is_special THEN f.flight_cost * cfg.SPECIAL_FLIGHT_PCT / 100
                       ELSE f.flight_cost * cfg.REGULAR_FLIGHT_PCT / 100 END
                ), 0)
                FROM AIR_FLIGHTS f
                JOIN AIR_CONFIG cfg ON cfg.id = 1
                WHERE f.AIR_HELICOPTERS_number = h.number) AS earned
        FROM AIR_HELICOPTERS h
        JOIN cnt ON cnt.hn = h.number AND cnt.c = (SELECT mc FROM mx)
        JOIN AIR_CREW cr ON cr.AIR_HELICOPTERS_number = h.number
        ORDER BY cr.number
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    os << "макс. число рейсов: экипаж и суммарные начисления экипажу по рейсам\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << "heli " << sqlite3_column_int(st, 0) << " " << reinterpret_cast<const char*>(sqlite3_column_text(st, 1))
           << " | таб.№" << sqlite3_column_int(st, 2) << " " << reinterpret_cast<const char*>(sqlite3_column_text(st, 3))
           << " " << reinterpret_cast<const char*>(sqlite3_column_text(st, 4))
           << " | начисления_всего " << sqlite3_column_int64(st, 5) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

static long long crew_total_money(sqlite3* db, int heli) {
    const char* q = R"(
        SELECT COALESCE(SUM(
            CASE WHEN f.is_special THEN f.flight_cost * cfg.SPECIAL_FLIGHT_PCT / 100
                 ELSE f.flight_cost * cfg.REGULAR_FLIGHT_PCT / 100 END
        ), 0)
        FROM AIR_FLIGHTS f
        JOIN AIR_CONFIG cfg ON cfg.id = 1
        WHERE f.AIR_HELICOPTERS_number = ?
    )";
    sqlite3_stmt* st = nullptr;
    long long v = 0;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK)
        return 0;
    sqlite3_bind_int(st, 1, heli);
    if (sqlite3_step(st) == SQLITE_ROW)
        v = sqlite3_column_int64(st, 0);
    sqlite3_finalize(st);
    return v;
}

std::string q_crew_max_money_flights(sqlite3* db, std::string& err) {
    (void)err;
    std::ostringstream os;
    sqlite3_stmt* st = nullptr;
    const char* q1 = "SELECT DISTINCT AIR_HELICOPTERS_number FROM AIR_CREW";
    if (sqlite3_prepare_v2(db, q1, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    int best_heli = -1;
    long long best_m = -1;
    while (sqlite3_step(st) == SQLITE_ROW) {
        int h = sqlite3_column_int(st, 0);
        long long m = crew_total_money(db, h);
        if (m > best_m) {
            best_m = m;
            best_heli = h;
        }
    }
    sqlite3_finalize(st);
    if (best_heli < 0) {
        os << "нет данных\n";
        return os.str();
    }
    const char* q2 = R"(
        SELECT f.flight_code, f.date, f.cargo_weight, f.amount_people, f.flight_duration, f.flight_cost, f.is_special
        FROM AIR_FLIGHTS f
        WHERE f.AIR_HELICOPTERS_number = ?
        ORDER BY f.date, f.flight_code
    )";
    if (sqlite3_prepare_v2(db, q2, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    sqlite3_bind_int(st, 1, best_heli);
    os << "экипаж с макс. начислениями (вертолёт " << best_heli << "), рейсы:\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << reinterpret_cast<const char*>(sqlite3_column_text(st, 1)) << " | "
           << sqlite3_column_int(st, 2) << " | "
           << sqlite3_column_int(st, 3) << " | "
           << sqlite3_column_int(st, 4) << " | "
           << sqlite3_column_int(st, 5) << " | "
           << sqlite3_column_int(st, 6) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

std::string q_crew_flights(sqlite3* db, const Session& s, std::string& err) {
    (void)err;
    std::ostringstream os;
    int heli = s.helicopter_number;
    if (s.role == Role::Commander) {
        err = "укажите режим экипажа";
        return {};
    }
    if (!crew_ok_heli(s, heli)) {
        err = "нет доступа";
        return {};
    }
    const char* q = R"(
        SELECT f.flight_code, f.date, f.cargo_weight, f.amount_people, f.flight_duration, f.flight_cost, f.is_special
        FROM AIR_FLIGHTS f
        WHERE f.AIR_HELICOPTERS_number = ?
        ORDER BY f.date, f.flight_code
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    sqlite3_bind_int(st, 1, heli);
    os << "рейсы вашего вертолёта " << heli << ":\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << reinterpret_cast<const char*>(sqlite3_column_text(st, 1)) << " | "
           << sqlite3_column_int(st, 2) << " | "
           << sqlite3_column_int(st, 3) << " | "
           << sqlite3_column_int(st, 4) << " | "
           << sqlite3_column_int(st, 5) << " | "
           << sqlite3_column_int(st, 6) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

std::string q_crew_flights_by_helicopter(sqlite3* db, int helicopter_number, std::string& err) {
    (void)err;
    std::ostringstream os;
    const char* q = R"(
        SELECT f.flight_code, f.date, f.cargo_weight, f.amount_people, f.flight_duration, f.flight_cost, f.is_special
        FROM AIR_FLIGHTS f
        WHERE f.AIR_HELICOPTERS_number = ?
        ORDER BY f.date, f.flight_code
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return {};
    }
    sqlite3_bind_int(st, 1, helicopter_number);
    os << "рейсы вертолёта " << helicopter_number << ":\n";
    while (sqlite3_step(st) == SQLITE_ROW) {
        os << sqlite3_column_int(st, 0) << " | "
           << reinterpret_cast<const char*>(sqlite3_column_text(st, 1)) << " | "
           << sqlite3_column_int(st, 2) << " | "
           << sqlite3_column_int(st, 3) << " | "
           << sqlite3_column_int(st, 4) << " | "
           << sqlite3_column_int(st, 5) << " | "
           << sqlite3_column_int(st, 6) << "\n";
    }
    sqlite3_finalize(st);
    return os.str();
}

} // namespace heli
