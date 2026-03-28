#pragma once

#include "common.h"
#include <sqlite3.h>
#include <string>

namespace heli {

/** Аутентификация по AIR_USERS. Командир: AIR_CREW_number IS NULL */
bool auth_login(sqlite3* db, const std::string& login, const std::string& password, Session& out, std::string& err);

} // namespace heli
