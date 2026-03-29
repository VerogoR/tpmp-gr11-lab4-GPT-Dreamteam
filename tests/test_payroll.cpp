#include "db.h"
#include "payroll.h"

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

static void test_pilot_period_amount_positive(void) {
    long long amt = 0;
    std::string err;
    CU_ASSERT_TRUE(heli::payroll_pilot_period_amount(g_db, 1001, "2025-01-01", "2025-12-31", amt, err));
    CU_ASSERT_TRUE(amt > 0);
}

static void test_compute_squad_period_rows(void) {
    std::string err;
    CU_ASSERT_TRUE(heli::payroll_compute_squad_period(g_db, "2025-01-01", "2025-12-31", err));
    sqlite3_stmt* st = nullptr;
    CU_ASSERT_EQUAL(sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM AIR_PAYROLL_PERIOD", -1, &st, nullptr), SQLITE_OK);
    CU_ASSERT_EQUAL(sqlite3_step(st), SQLITE_ROW);
    CU_ASSERT_TRUE(sqlite3_column_int(st, 0) >= 6);
    sqlite3_finalize(st);
}

static void test_pilot_by_type_special_and_regular(void) {
    long long sp = 0, rg = 0;
    std::string err;
    CU_ASSERT_TRUE(heli::payroll_pilot_period_by_type(g_db, 1001, "2025-01-01", "2025-12-31", true, sp, err));
    CU_ASSERT_TRUE(heli::payroll_pilot_period_by_type(g_db, 1001, "2025-01-01", "2025-12-31", false, rg, err));
    CU_ASSERT_TRUE(sp > 0);
    CU_ASSERT_TRUE(rg > 0);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("payroll", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "pilot_period_amount_positive", test_pilot_period_amount_positive)
        || !CU_add_test(suite, "compute_squad_period_rows", test_compute_squad_period_rows)
        || !CU_add_test(suite, "pilot_by_type_special_and_regular", test_pilot_by_type_special_and_regular)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
