#include "db.h"
#include "queries.h"

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

static void test_hours_and_resource(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    cmd.helicopter_number = -1;
    std::string err;
    std::string r = heli::q_hours_and_resource(g_db, cmd, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(r.find("101") != std::string::npos);
    CU_ASSERT_TRUE(r.find("500") != std::string::npos);
}

static void test_special_totals(void) {
    std::string err;
    std::string r = heli::q_special_totals(g_db, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(r.find("102") != std::string::npos);
}

static void test_regular_totals(void) {
    std::string err;
    std::string r = heli::q_regular_totals(g_db, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(r.find("101") != std::string::npos);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("queries", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "hours_and_resource", test_hours_and_resource)
        || !CU_add_test(suite, "special_totals", test_special_totals)
        || !CU_add_test(suite, "regular_totals", test_regular_totals)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
