#pragma once

#include <sqlite3.h>
#include <string>

namespace heli {

/** П.5: за период — начисления экипажам, запись в AIR_PAYROLL_PERIOD */
bool payroll_compute_squad_period(sqlite3* db, const std::string& period_start, const std::string& period_end, std::string& err);

/** П.6: сумма начислений летчику за период (копейки) */
bool payroll_pilot_period_amount(sqlite3* db, int crew_number, const std::string& period_start,
    const std::string& period_end, long long& amount_out, std::string& err);

/** П.7: начисления летчику за период по типу рейса: only_special true = только спецрейсы */
bool payroll_pilot_period_by_type(sqlite3* db, int crew_number, const std::string& period_start,
    const std::string& period_end, bool only_special, long long& amount_out, std::string& err);

} // namespace heli
