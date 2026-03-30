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

static void test_flights_in_period_for_crew_is_filtered(void) {
    heli::Session crew{};
    crew.role = heli::Role::Crew;
    crew.helicopter_number = 101;
    std::string err;
    std::string r = heli::q_flights_in_period(g_db, crew, "2025-01-01", "2025-12-31", err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(r.find("101") != std::string::npos);
    CU_ASSERT_TRUE(r.find("102") == std::string::npos);
}

static void test_max_reports_have_content(void) {
    std::string err;
    std::string a = heli::q_heli_max_flights_crew(g_db, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(a.find("макс. число рейсов") != std::string::npos);

    std::string b = heli::q_crew_max_money_flights(g_db, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(b.find("экипаж с макс. начислениями") != std::string::npos);
}

static void test_crew_queries_and_commander_guard(void) {
    std::string err;

    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    cmd.helicopter_number = -1;
    std::string empty = heli::q_crew_flights(g_db, cmd, err);
    CU_ASSERT_TRUE(empty.empty());
    CU_ASSERT_FALSE(err.empty());

    heli::Session crew{};
    crew.role = heli::Role::Crew;
    crew.helicopter_number = 101;
    err.clear();
    std::string own = heli::q_crew_flights(g_db, crew, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(own.find("вашего вертолёта 101") != std::string::npos);

    std::string by_h = heli::q_crew_flights_by_helicopter(g_db, 102, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(by_h.find("вертолёта 102") != std::string::npos);
}

static void test_no_data_branch_for_crew_max_money(void) {
    std::string err;
    unsetenv("HELI_SEED");
    sqlite3* db2 = heli::db_open_or_create(":memory:", "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db2);

    std::string r = heli::q_crew_max_money_flights(db2, err);
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(r.find("нет данных") != std::string::npos);
    heli::db_close(db2);
}

static void test_prepare_failure_branch(void) {
    std::string err;
    unsetenv("HELI_SEED");
    sqlite3* db2 = heli::db_open_or_create(":memory:", "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db2);
    CU_ASSERT_EQUAL(sqlite3_exec(db2, "DROP TABLE AIR_FLIGHTS", nullptr, nullptr, nullptr), SQLITE_OK);

    std::string r = heli::q_regular_totals(db2, err);
    CU_ASSERT_TRUE(r.empty());
    CU_ASSERT_FALSE(err.empty());
    heli::db_close(db2);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("queries", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "hours_and_resource", test_hours_and_resource)
        || !CU_add_test(suite, "special_totals", test_special_totals)
        || !CU_add_test(suite, "regular_totals", test_regular_totals)
        || !CU_add_test(suite, "flights_in_period_for_crew_is_filtered", test_flights_in_period_for_crew_is_filtered)
        || !CU_add_test(suite, "max_reports_have_content", test_max_reports_have_content)
        || !CU_add_test(suite, "crew_queries_and_commander_guard", test_crew_queries_and_commander_guard)
        || !CU_add_test(suite, "no_data_branch_for_crew_max_money", test_no_data_branch_for_crew_max_money)
        || !CU_add_test(suite, "prepare_failure_branch", test_prepare_failure_branch)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
