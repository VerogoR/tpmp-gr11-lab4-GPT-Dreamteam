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

static void test_update_and_delete_helicopter_and_crew(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    std::string err;

    CU_ASSERT_TRUE(heli::mut_update_helicopter(g_db, cmd, 501, "Upd", "2020-01-01", 1200, "2024-01-01", 20, err));
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(heli::mut_update_crew(g_db, cmd, 9001, "B", "штурман", 2, "addr2", "1991-01-01", 501, err));
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(heli::mut_delete_crew(g_db, cmd, 9001, err));
    CU_ASSERT_TRUE(err.empty());
    CU_ASSERT_TRUE(heli::mut_delete_helicopter(g_db, cmd, 501, err));
    CU_ASSERT_TRUE(err.empty());
}

static void test_crew_forbidden_for_admin_entities(void) {
    heli::Session crew{};
    crew.role = heli::Role::Crew;
    crew.helicopter_number = 101;
    std::string err;

    CU_ASSERT_FALSE(heli::mut_update_helicopter(g_db, crew, 101, "X", "2020-01-01", 1, "2024-01-01", 1, err));
    CU_ASSERT_FALSE(heli::mut_delete_helicopter(g_db, crew, 101, err));
    CU_ASSERT_FALSE(heli::mut_insert_crew(g_db, crew, 9991, "X", "X", 1, "x", "1990-01-01", 101, err));
    CU_ASSERT_FALSE(heli::mut_update_crew(g_db, crew, 1001, "X", "X", 1, "x", "1990-01-01", 101, err));
    CU_ASSERT_FALSE(heli::mut_delete_crew(g_db, crew, 1001, err));
    CU_ASSERT_FALSE(heli::mut_insert_user(g_db, crew, "u", "p", -1, err));
}

static void test_flight_update_delete_guards_and_user_insert(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    std::string err;

    CU_ASSERT_TRUE(heli::mut_insert_helicopter(g_db, cmd, 601, "T601", "2020-01-01", 1000, "2024-01-01", 100, err));
    CU_ASSERT_TRUE(heli::mut_insert_flight(g_db, cmd, 8001, "2025-02-01", 10, 2, 3, 500, 601, false, err));
    CU_ASSERT_TRUE(heli::mut_update_flight(g_db, cmd, 8001, "2025-02-02", 11, 3, 4, 600, 601, true, err));
    CU_ASSERT_TRUE(heli::mut_delete_flight(g_db, cmd, 8001, err));

    heli::Session crew{};
    crew.role = heli::Role::Crew;
    crew.helicopter_number = 102;
    CU_ASSERT_FALSE(heli::mut_insert_flight(g_db, crew, 8101, "2025-02-01", 1, 1, 1, 1, 101, false, err));
    CU_ASSERT_FALSE(heli::mut_update_flight(g_db, crew, 999999, "2025-02-01", 1, 1, 1, 1, 101, false, err));
    CU_ASSERT_FALSE(heli::mut_delete_flight(g_db, crew, 999999, err));

    CU_ASSERT_TRUE(heli::mut_insert_user(g_db, cmd, "new_commander", "pw", -1, err));
}

static void test_prepare_failure_branch_on_missing_table(void) {
    std::string err;
    unsetenv("HELI_SEED");
    sqlite3* db2 = heli::db_open_or_create(":memory:", "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db2);
    CU_ASSERT_EQUAL(sqlite3_exec(db2, "DROP TABLE AIR_HELICOPTERS", nullptr, nullptr, nullptr), SQLITE_OK);

    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    bool ok = heli::mut_insert_helicopter(db2, cmd, 1, "X", "2020-01-01", 1, "2024-01-01", 1, err);
    CU_ASSERT_FALSE(ok);
    CU_ASSERT_FALSE(err.empty());
    heli::db_close(db2);
}

static void test_insert_user_with_crew_id_and_fk_error_paths(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    std::string err;

    CU_ASSERT_TRUE(heli::mut_insert_helicopter(g_db, cmd, 701, "H701", "2020-01-01", 1000, "2024-01-01", 50, err));
    CU_ASSERT_TRUE(heli::mut_insert_crew(g_db, cmd, 9701, "Crew", "pilot", 5, "addr", "1990-01-01", 701, err));
    CU_ASSERT_TRUE(heli::mut_insert_user(g_db, cmd, "crew_user_9701", "pw", 9701, err));

    bool bad_crew_insert = heli::mut_insert_crew(g_db, cmd, 9702, "Bad", "pilot", 1, "addr", "1990-01-01", 99999, err);
    CU_ASSERT_FALSE(bad_crew_insert);
    CU_ASSERT_FALSE(err.empty());
}

static void test_update_delete_flight_permission_and_not_found(void) {
    heli::Session cmd{};
    cmd.role = heli::Role::Commander;
    std::string err;

    CU_ASSERT_TRUE(heli::mut_insert_helicopter(g_db, cmd, 801, "H801", "2020-01-01", 1000, "2024-01-01", 100, err));
    CU_ASSERT_TRUE(heli::mut_insert_helicopter(g_db, cmd, 802, "H802", "2020-01-01", 1000, "2024-01-01", 100, err));
    CU_ASSERT_TRUE(heli::mut_insert_flight(g_db, cmd, 8801, "2025-03-01", 1, 1, 1, 100, 801, false, err));

    heli::Session crew{};
    crew.role = heli::Role::Crew;
    crew.helicopter_number = 801;

    bool move_to_other = heli::mut_update_flight(g_db, crew, 8801, "2025-03-02", 2, 2, 2, 200, 802, false, err);
    CU_ASSERT_FALSE(move_to_other);
    CU_ASSERT_FALSE(err.empty());

    crew.helicopter_number = 802;
    bool delete_other = heli::mut_delete_flight(g_db, crew, 8801, err);
    CU_ASSERT_FALSE(delete_other);
    CU_ASSERT_FALSE(err.empty());

    bool upd_nf = heli::mut_update_flight(g_db, cmd, 999998, "2025-03-02", 1, 1, 1, 1, 801, false, err);
    CU_ASSERT_FALSE(upd_nf);
    CU_ASSERT_FALSE(err.empty());
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("mutations", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "commander_inserts_helicopter", test_commander_inserts_helicopter)
        || !CU_add_test(suite, "crew_cannot_insert_helicopter", test_crew_cannot_insert_helicopter)
        || !CU_add_test(suite, "trigger_blocks_over_resource", test_trigger_blocks_over_resource)
        || !CU_add_test(suite, "update_and_delete_helicopter_and_crew", test_update_and_delete_helicopter_and_crew)
        || !CU_add_test(suite, "crew_forbidden_for_admin_entities", test_crew_forbidden_for_admin_entities)
        || !CU_add_test(suite, "flight_update_delete_guards_and_user_insert", test_flight_update_delete_guards_and_user_insert)
        || !CU_add_test(suite, "prepare_failure_branch_on_missing_table", test_prepare_failure_branch_on_missing_table)
        || !CU_add_test(suite, "insert_user_with_crew_id_and_fk_error_paths", test_insert_user_with_crew_id_and_fk_error_paths)
        || !CU_add_test(suite, "update_delete_flight_permission_and_not_found", test_update_delete_flight_permission_and_not_found)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
