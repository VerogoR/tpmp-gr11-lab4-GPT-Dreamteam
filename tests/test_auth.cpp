#include "auth.h"
#include "db.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <cstdlib>
#include <string>

static sqlite3* g_db = nullptr;

static int suite_init(void) {
    setenv("HELI_SEED", "1", 1);
    setenv("HELI_SQL_DIR", "sql", 1);
    std::string err;
    g_db = heli::db_open_or_create(":memory:", "sql", err);
    return g_db ? 0 : -1;
}

static int suite_clean(void) {
    if (g_db) {
        heli::db_close(g_db);
        g_db = nullptr;
    }
    return 0;
}

static void test_wrong_password(void) {
    heli::Session s{};
    std::string e;
    bool ok = heli::auth_login(g_db, "commander", "wrong", s, e);
    CU_ASSERT_FALSE(ok);
    CU_ASSERT_FALSE(e.empty());
}

static void test_unknown_login(void) {
    heli::Session s{};
    std::string e;
    bool ok = heli::auth_login(g_db, "no_such_user", "x", s, e);
    CU_ASSERT_FALSE(ok);
    CU_ASSERT_FALSE(e.empty());
}

static void test_commander_login(void) {
    heli::Session s{};
    std::string err;
    CU_ASSERT_TRUE(heli::auth_login(g_db, "commander", "cmd_secret", s, err));
    CU_ASSERT_EQUAL(static_cast<int>(s.role), static_cast<int>(heli::Role::Commander));
    CU_ASSERT_TRUE(s.crew_number < 0);
}

static void test_pilot_login(void) {
    heli::Session s{};
    std::string err;
    CU_ASSERT_TRUE(heli::auth_login(g_db, "pilot_ivanov", "pilot1", s, err));
    CU_ASSERT_EQUAL(static_cast<int>(s.role), static_cast<int>(heli::Role::Crew));
    CU_ASSERT_EQUAL(s.crew_number, 1001);
    CU_ASSERT_EQUAL(s.helicopter_number, 101);
}

static void test_commander_has_no_bound_helicopter(void) {
    heli::Session s{};
    std::string err;
    CU_ASSERT_TRUE(heli::auth_login(g_db, "commander", "cmd_secret", s, err));
    CU_ASSERT_TRUE(s.helicopter_number < 0);
}

static void test_prepare_failure_when_users_table_missing(void) {
    std::string err;
    unsetenv("HELI_SEED");
    sqlite3* db2 = heli::db_open_or_create(":memory:", "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db2);
    CU_ASSERT_EQUAL(sqlite3_exec(db2, "DROP TABLE AIR_USERS", nullptr, nullptr, nullptr), SQLITE_OK);

    heli::Session s{};
    bool ok = heli::auth_login(db2, "commander", "cmd_secret", s, err);
    CU_ASSERT_FALSE(ok);
    CU_ASSERT_FALSE(err.empty());
    heli::db_close(db2);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("auth", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "wrong_password", test_wrong_password)
        || !CU_add_test(suite, "unknown_login", test_unknown_login)
        || !CU_add_test(suite, "commander_login", test_commander_login)
        || !CU_add_test(suite, "pilot_login", test_pilot_login)
        || !CU_add_test(suite, "commander_has_no_bound_helicopter", test_commander_has_no_bound_helicopter)
        || !CU_add_test(suite, "prepare_failure_when_users_table_missing", test_prepare_failure_when_users_table_missing)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
