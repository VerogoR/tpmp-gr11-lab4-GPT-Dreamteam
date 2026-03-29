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
    if (!g_db || !err.empty())
        return -1;
    return 0;
}

static int suite_clean(void) {
    if (g_db) {
        heli::db_close(g_db);
        g_db = nullptr;
    }
    return 0;
}

static void test_open_and_seed_helicopters(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(g_db);
    sqlite3_stmt* st = nullptr;
    CU_ASSERT_EQUAL(sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM AIR_HELICOPTERS", -1, &st, nullptr), SQLITE_OK);
    CU_ASSERT_EQUAL(sqlite3_step(st), SQLITE_ROW);
    CU_ASSERT_EQUAL(sqlite3_column_int(st, 0), 2);
    sqlite3_finalize(st);
}

static void test_extensions_payroll_table(void) {
    sqlite3_stmt* st = nullptr;
    CU_ASSERT_EQUAL(
        sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='AIR_PAYROLL_PERIOD'", -1, &st, nullptr),
        SQLITE_OK);
    CU_ASSERT_EQUAL(sqlite3_step(st), SQLITE_ROW);
    CU_ASSERT_EQUAL(sqlite3_column_int(st, 0), 1);
    sqlite3_finalize(st);
}

static void test_trigger_resource_check(void) {
    sqlite3_stmt* st = nullptr;
    CU_ASSERT_EQUAL(
        sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM sqlite_master WHERE type='trigger' AND name='trg_air_flights_resource_check'", -1, &st, nullptr),
        SQLITE_OK);
    CU_ASSERT_EQUAL(sqlite3_step(st), SQLITE_ROW);
    CU_ASSERT_EQUAL(sqlite3_column_int(st, 0), 1);
    sqlite3_finalize(st);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("db", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "open_and_seed_helicopters", test_open_and_seed_helicopters)
        || !CU_add_test(suite, "extensions_payroll_table", test_extensions_payroll_table)
        || !CU_add_test(suite, "trigger_resource_check", test_trigger_resource_check)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
