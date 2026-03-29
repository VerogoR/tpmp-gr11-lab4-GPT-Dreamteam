#pragma once

#include <string>

namespace heli {

enum class Role { Commander, Crew };

struct Session {
    Role role{};
    std::string login;
    /** Табельный номер члена экипажа; для командира не задан */
    int crew_number = -1;
    /** Вертолёт закреплённый за пользователем (для экипажа) */
    int helicopter_number = -1;
};

} // namespace heli
