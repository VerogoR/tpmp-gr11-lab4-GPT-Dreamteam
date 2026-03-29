#pragma once

#include "common.h"
#include <sqlite3.h>
#include <string>

namespace heli {

/** Все отчёты Select; для экипажа с role Crew фильтруются помеченные * */
std::string q_hours_and_resource(sqlite3* db, const Session& s, std::string& err);
std::string q_flights_in_period(sqlite3* db, const Session& s, const std::string& d1, const std::string& d2, std::string& err);
std::string q_special_totals(sqlite3* db, std::string& err);
std::string q_regular_totals(sqlite3* db, std::string& err);
std::string q_heli_max_flights_crew(sqlite3* db, std::string& err);
std::string q_crew_max_money_flights(sqlite3* db, std::string& err);
std::string q_crew_flights(sqlite3* db, const Session& s, std::string& err);
/** Для командира: рейсы по номеру вертолёта экипажа */
std::string q_crew_flights_by_helicopter(sqlite3* db, int helicopter_number, std::string& err);

} // namespace heli
