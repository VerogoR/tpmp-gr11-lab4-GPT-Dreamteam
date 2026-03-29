#include "storage.h"

#include <fstream>
#include <sstream>

namespace heli {

bool storage_save_image(sqlite3* db, const std::string& ref_table, int ref_id, const std::string& format,
    const std::vector<unsigned char>& bytes, std::string& err) {
    const char* q = "INSERT INTO AIR_IMAGES (ref_table, ref_id, format, image_data) VALUES (?,?,?,?)";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(st, 1, ref_table.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(st, 2, ref_id);
    sqlite3_bind_text(st, 3, format.c_str(), -1, SQLITE_TRANSIENT);
    if (bytes.empty())
        sqlite3_bind_blob(st, 4, "", 0, SQLITE_STATIC);
    else
        sqlite3_bind_blob(st, 4, bytes.data(), static_cast<int>(bytes.size()), SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) {
        err = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool storage_export_flights_csv(sqlite3* db, const std::string& path, std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }
    out << "flight_code,date,cargo_weight,amount_people,flight_duration,flight_cost,helicopter,is_special\n";
    const char* q = "SELECT flight_code, date, cargo_weight, amount_people, flight_duration, flight_cost, AIR_HELICOPTERS_number, is_special FROM AIR_FLIGHTS ORDER BY date";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    while (sqlite3_step(st) == SQLITE_ROW) {
        out << sqlite3_column_int(st, 0) << ","
            << sqlite3_column_text(st, 1) << ","
            << sqlite3_column_int(st, 2) << ","
            << sqlite3_column_int(st, 3) << ","
            << sqlite3_column_int(st, 4) << ","
            << sqlite3_column_int(st, 5) << ","
            << sqlite3_column_int(st, 6) << ","
            << sqlite3_column_int(st, 7) << "\n";
    }
    sqlite3_finalize(st);
    return true;
}

bool storage_export_helicopters_json(sqlite3* db, const std::string& path, std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }
    out << "[\n";
    const char* q = "SELECT number, mark, date_creation, lifting_capacity, date_last_fix, flight_resource FROM AIR_HELICOPTERS ORDER BY number";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK) {
        err = sqlite3_errmsg(db);
        return false;
    }
    bool first = true;
    while (sqlite3_step(st) == SQLITE_ROW) {
        if (!first)
            out << ",\n";
        first = false;
        out << "  {\"number\":" << sqlite3_column_int(st, 0)
            << ",\"mark\":\"" << sqlite3_column_text(st, 1) << "\""
            << ",\"date_creation\":\"" << sqlite3_column_text(st, 2) << "\""
            << ",\"lifting_capacity\":" << sqlite3_column_int(st, 3)
            << ",\"date_last_fix\":\"" << sqlite3_column_text(st, 4) << "\""
            << ",\"flight_resource\":" << sqlite3_column_int(st, 5) << "}";
    }
    sqlite3_finalize(st);
    out << "\n]\n";
    return true;
}

} // namespace heli
