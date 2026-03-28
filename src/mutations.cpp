#include "mutations.h"

namespace heli {

static bool is_commander(const Session& s) {
    return s.role == Role::Commander;
}

static bool crew_heli_ok(const Session& s, int heli) {
    if (is_commander(s))
        return true;
    return s.helicopter_number == heli;
}

bool mut_insert_helicopter(sqlite3* db, const Session& s, int number, const std::string& mark,
    const std::string& date_creation, int lifting, const std::string& date_last_fix, int flight_resource, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "INSERT INTO AIR_HELICOPTERS (number, mark, date_creation, lifting_capacity, date_last_fix, flight_resource) VALUES (?,?,?,?,?,?)";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_int(st, 1, number);
    sqlite3_bind_text(st, 2, mark.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 3, date_creation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 4, lifting);
    sqlite3_bind_text(st, 5, date_last_fix.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 6, flight_resource);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool mut_update_helicopter(sqlite3* db, const Session& s, int number, const std::string& mark,
    const std::string& date_creation, int lifting, const std::string& date_last_fix, int flight_resource, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "UPDATE AIR_HELICOPTERS SET mark=?, date_creation=?, lifting_capacity=?, date_last_fix=?, flight_resource=? WHERE number=?";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, mark.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, date_creation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 3, lifting);
    sqlite3_bind_text(st, 4, date_last_fix.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 5, flight_resource);
    sqlite3_bind_int(st, 6, number);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool mut_delete_helicopter(sqlite3* db, const Session& s, int number, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "DELETE FROM AIR_HELICOPTERS WHERE number=?";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_int(st, 1, number);
    sqlite3_step(st);
    sqlite3_finalize(st);
    return true;
}

bool mut_insert_crew(sqlite3* db, const Session& s, int number, const std::string& surname, const std::string& grade,
    int experience, const std::string& address, const std::string& dob, int heli_num, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "INSERT INTO AIR_CREW (number, surname, grade, experience, address, date_of_birth, AIR_HELICOPTERS_number) VALUES (?,?,?,?,?,?,?)";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_int(st, 1, number);
    sqlite3_bind_text(st, 2, surname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 3, grade.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 4, experience);
    sqlite3_bind_text(st, 5, address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 6, dob.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 7, heli_num);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool mut_update_crew(sqlite3* db, const Session& s, int number, const std::string& surname, const std::string& grade,
    int experience, const std::string& address, const std::string& dob, int heli_num, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "UPDATE AIR_CREW SET surname=?, grade=?, experience=?, address=?, date_of_birth=?, AIR_HELICOPTERS_number=? WHERE number=?";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, surname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, grade.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 3, experience);
    sqlite3_bind_text(st, 4, address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 5, dob.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 6, heli_num);
    sqlite3_bind_int(st, 7, number);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool mut_delete_crew(sqlite3* db, const Session& s, int number, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "DELETE FROM AIR_CREW WHERE number=?";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_int(st, 1, number);
    sqlite3_step(st);
    sqlite3_finalize(st);
    return true;
}

bool mut_insert_flight(sqlite3* db, const Session& s, int flight_code, const std::string& date, int cargo, int people,
    int duration, int cost, int heli_num, bool is_special, std::string& err) {
    if (!crew_heli_ok(s, heli_num)) {
        err = "нет прав на этот вертолёт";
        return false;
    }
    const char* q = "INSERT INTO AIR_FLIGHTS (flight_code, date, cargo_weight, amount_people, flight_duration, flight_cost, AIR_HELICOPTERS_number, is_special) VALUES (?,?,?,?,?,?,?,?)";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_int(st, 1, flight_code);
    sqlite3_bind_text(st, 2, date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 3, cargo);
    sqlite3_bind_int(st, 4, people);
    sqlite3_bind_int(st, 5, duration);
    sqlite3_bind_int(st, 6, cost);
    sqlite3_bind_int(st, 7, heli_num);
    sqlite3_bind_int(st, 8, is_special ? 1 : 0);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool mut_update_flight(sqlite3* db, const Session& s, int flight_code, const std::string& date, int cargo, int people,
    int duration, int cost, int heli_num, bool is_special, std::string& err) {
    {
        sqlite3_stmt* st = nullptr;
        const char* q0 = "SELECT AIR_HELICOPTERS_number FROM AIR_FLIGHTS WHERE flight_code=?";
        if (sqlite3_prepare_v2(db, q0, -1, &st, nullptr) != SQLITE_OK) {
            err = sqlite3_errmsg(db);
            return false;
        }
        sqlite3_bind_int(st, 1, flight_code);
        if (sqlite3_step(st) != SQLITE_ROW) {
            sqlite3_finalize(st);
            err = "рейс не найден";
            return false;
        }
        int old_heli = sqlite3_column_int(st, 0);
        sqlite3_finalize(st);
        if (!crew_heli_ok(s, old_heli) || !crew_heli_ok(s, heli_num)) {
            err = "нет прав на этот вертолёт";
            return false;
        }
    }
    const char* q = "UPDATE AIR_FLIGHTS SET date=?, cargo_weight=?, amount_people=?, flight_duration=?, flight_cost=?, AIR_HELICOPTERS_number=?, is_special=? WHERE flight_code=?";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 2, cargo);
    sqlite3_bind_int(st, 3, people);
    sqlite3_bind_int(st, 4, duration);
    sqlite3_bind_int(st, 5, cost);
    sqlite3_bind_int(st, 6, heli_num);
    sqlite3_bind_int(st, 7, is_special ? 1 : 0);
    sqlite3_bind_int(st, 8, flight_code);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool mut_delete_flight(sqlite3* db, const Session& s, int flight_code, std::string& err) {
    int heli = -1;
    {
        sqlite3_stmt* st = nullptr;
        const char* q = "SELECT AIR_HELICOPTERS_number FROM AIR_FLIGHTS WHERE flight_code=?";
        if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
            err = sqlite3_errmsg(db);
            return false;
        }
        sqlite3_bind_int(st, 1, flight_code);
        if (sqlite3_step(st) != SQLITE_ROW) {
            sqlite3_finalize(st);
            err = "рейс не найден";
            return false;
        }
        heli = sqlite3_column_int(st, 0);
        sqlite3_finalize(st);
    }
    if (!crew_heli_ok(s, heli)) {
        err = "нет прав на этот вертолёт";
        return false;
    }
    sqlite3_stmt* st = nullptr;
    const char* q = "DELETE FROM AIR_FLIGHTS WHERE flight_code=?";
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_int(st, 1, flight_code);
    sqlite3_step(st);
    sqlite3_finalize(st);
    return true;
}

bool mut_insert_user(sqlite3* db, const Session& s, const std::string& login, const std::string& password,
    int crew_number_or_neg1, std::string& err) {
    if (!is_commander(s)) {
        err = "только командир";
        return false;
    }
    const char* q = "INSERT INTO AIR_USERS (login, password, AIR_CREW_number) VALUES (?,?,?)";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, login.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(st, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    if (crew_number_or_neg1 < 0)
        sqlite3_bind_null(st, 3);
    else
        sqlite3_bind_int(st, 3, crew_number_or_neg1);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

} // namespace heli
