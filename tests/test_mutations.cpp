#include "db.h"
#include "mutations.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <cstdlib>
#include <string>

static sqlite3* g_db = nullptr;

static int suite_init(void) {
    setenv("HELI_SQL_DIR", "sql", 1);
    unsetenv("HELI_SEED");
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

static void test_commander_inserts_helicopter(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    std::string err;
    CU_ASSERT_TRUE(heli::mut_insert_helicopter(g_db, cmd, 501, "Test", "2020-01-01", 1000, "2024-01-01", 10, err));
    CU_ASSERT_TRUE(err.empty());
}

static void test_crew_cannot_insert_helicopter(void) {
    heli::Session cr{};
    cr.role = heli::Role::Crew;
    cr.helicopter_number = 501;
    std::string err;
    CU_ASSERT_FALSE(heli::mut_insert_helicopter(g_db, cr, 502, "X", "2020-01-01", 1, "2024-01-01", 1, err));
}

static void test_trigger_blocks_over_resource(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    std::string err;
    CU_ASSERT_TRUE(heli::mut_insert_crew(g_db, cmd, 9001, "A", "пилот", 1, "addr", "1990-01-01", 501, err));
    CU_ASSERT_TRUE(heli::mut_insert_flight(g_db, cmd, 7001, "2025-01-01", 1, 0, 5, 1000, 501, false, err));
    CU_ASSERT_TRUE(heli::mut_insert_flight(g_db, cmd, 7002, "2025-01-02", 1, 0, 5, 1000, 501, false, err));
    bool third = heli::mut_insert_flight(g_db, cmd, 7003, "2025-01-03", 1, 0, 1, 1000, 501, false, err);
    CU_ASSERT_FALSE(third);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("mutations", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "commander_inserts_helicopter", test_commander_inserts_helicopter)
        || !CU_add_test(suite, "crew_cannot_insert_helicopter", test_crew_cannot_insert_helicopter)
        || !CU_add_test(suite, "trigger_blocks_over_resource", test_trigger_blocks_over_resource)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
