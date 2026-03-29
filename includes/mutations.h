#pragma once

#include "common.h"
#include <sqlite3.h>
#include <string>

namespace heli {

bool mut_insert_helicopter(sqlite3* db, const Session& s, int number, const std::string& mark,
    const std::string& date_creation, int lifting, const std::string& date_last_fix, int flight_resource, std::string& err);
bool mut_update_helicopter(sqlite3* db, const Session& s, int number, const std::string& mark,
    const std::string& date_creation, int lifting, const std::string& date_last_fix, int flight_resource, std::string& err);
bool mut_delete_helicopter(sqlite3* db, const Session& s, int number, std::string& err);

bool mut_insert_crew(sqlite3* db, const Session& s, int number, const std::string& surname, const std::string& grade,
    int experience, const std::string& address, const std::string& dob, int heli_num, std::string& err);
bool mut_update_crew(sqlite3* db, const Session& s, int number, const std::string& surname, const std::string& grade,
    int experience, const std::string& address, const std::string& dob, int heli_num, std::string& err);
bool mut_delete_crew(sqlite3* db, const Session& s, int number, std::string& err);

bool mut_insert_flight(sqlite3* db, const Session& s, int flight_code, const std::string& date, int cargo, int people,
    int duration, int cost, int heli_num, bool is_special, std::string& err);
bool mut_update_flight(sqlite3* db, const Session& s, int flight_code, const std::string& date, int cargo, int people,
    int duration, int cost, int heli_num, bool is_special, std::string& err);
bool mut_delete_flight(sqlite3* db, const Session& s, int flight_code, std::string& err);

bool mut_insert_user(sqlite3* db, const Session& s, const std::string& login, const std::string& password,
    int crew_number_or_neg1, std::string& err);

} // namespace heli
