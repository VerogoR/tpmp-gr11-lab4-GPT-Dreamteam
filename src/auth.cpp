#include "auth.h"

namespace heli {

bool auth_login(sqlite3* db, const std::string& login, const std::string& password, Session& out, std::string& err) {
    const char* q = R"(
        SELECT u.password, u.AIR_CREW_number, cr.AIR_HELICOPTERS_number
        FROM AIR_USERS u
        LEFT JOIN AIR_CREW cr ON cr.number = u.AIR_CREW_number
        WHERE u.login = ?
    )";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, login.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(st);
        err = "Неверный логин или пароль";
        return false;
    }
    const char* pw = reinterpret_cast<const char*>(sqlite3_column_text(st, 0));
    if (!pw || password != pw) {
        sqlite3_finalize(st);
        err = "Неверный логин или пароль";
        return false;
    }
    int crew_col = sqlite3_column_type(st, 1);
    int crew_no = (crew_col == SQLITE_NULL) ? -1 : sqlite3_column_int(st, 1);
    int heli = (sqlite3_column_type(st, 2) == SQLITE_NULL) ? -1 : sqlite3_column_int(st, 2);
    sqlite3_finalize(st);

    out.login = login;
    out.crew_number = crew_no;
    out.helicopter_number = heli;
    out.role = (crew_no < 0) ? Role::Commander : Role::Crew;
    return true;
}

} // namespace heli
