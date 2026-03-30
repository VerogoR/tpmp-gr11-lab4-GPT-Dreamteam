#include "db.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <cstdlib>
#include <fstream>
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

static void test_resolve_sql_dir_default_and_env(void) {
    unsetenv("HELI_SQL_DIR");
    CU_ASSERT_STRING_EQUAL(heli::resolve_sql_dir().c_str(), "sql");

    setenv("HELI_SQL_DIR", "custom_sql", 1);
    CU_ASSERT_STRING_EQUAL(heli::resolve_sql_dir().c_str(), "custom_sql");
}

static void test_exec_sql_file_missing_file_fails(void) {
    std::string err;
    bool ok = heli::db_exec_sql_file(g_db, "sql/does-not-exist.sql", err);
    CU_ASSERT_FALSE(ok);
    CU_ASSERT_TRUE(err.find("cannot read") != std::string::npos);
}

static void test_open_with_bad_sql_dir_fails(void) {
    unsetenv("HELI_SEED");
    std::string err;
    sqlite3* db = heli::db_open_or_create(":memory:", "missing_sql_dir", err);
    CU_ASSERT_PTR_NULL(db);
    CU_ASSERT_FALSE(err.empty());
}

static void test_exec_sql_file_bad_sql_fails(void) {
    const char* path = "/tmp/heli_bad_sql.sql";
    {
        std::ofstream out(path);
        out << "THIS IS NOT SQL;";
    }
    std::string err;
    bool ok = heli::db_exec_sql_file(g_db, path, err);
    CU_ASSERT_FALSE(ok);
    CU_ASSERT_FALSE(err.empty());
}

static void test_exec_sql_file_empty_succeeds(void) {
    const char* path = "/tmp/heli_empty_sql.sql";
    {
        std::ofstream out(path);
        out << "";
    }
    std::string err;
    bool ok = heli::db_exec_sql_file(g_db, path, err);
    CU_ASSERT_TRUE(ok);
    CU_ASSERT_TRUE(err.empty());
}

static void test_open_without_seed_has_empty_data(void) {
    unsetenv("HELI_SEED");
    std::string err;
    sqlite3* db = heli::db_open_or_create(":memory:", "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db);
    sqlite3_stmt* st = nullptr;
    CU_ASSERT_EQUAL(sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM AIR_HELICOPTERS", -1, &st, nullptr), SQLITE_OK);
    CU_ASSERT_EQUAL(sqlite3_step(st), SQLITE_ROW);
    CU_ASSERT_EQUAL(sqlite3_column_int(st, 0), 0);
    sqlite3_finalize(st);
    heli::db_close(db);
}

static void test_db_close_nullptr_is_safe(void) {
    heli::db_close(nullptr);
    CU_ASSERT_TRUE(true);
}

static void test_open_with_invalid_path_fails(void) {
    std::string err;
    sqlite3* db = heli::db_open_or_create("/no/such/dir/heli.db", "sql", err);
    CU_ASSERT_PTR_NULL(db);
    CU_ASSERT_FALSE(err.empty());
}

static void test_seed_not_duplicated_on_reopen(void) {
    setenv("HELI_SEED", "1", 1);
    std::string err;
    const char* p = "/tmp/heli_seed_reopen.db";
    sqlite3* db1 = heli::db_open_or_create(p, "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db1);
    heli::db_close(db1);

    sqlite3* db2 = heli::db_open_or_create(p, "sql", err);
    CU_ASSERT_PTR_NOT_NULL_FATAL(db2);

    sqlite3_stmt* st = nullptr;
    CU_ASSERT_EQUAL(sqlite3_prepare_v2(db2, "SELECT COUNT(*) FROM AIR_HELICOPTERS", -1, &st, nullptr), SQLITE_OK);
    CU_ASSERT_EQUAL(sqlite3_step(st), SQLITE_ROW);
    CU_ASSERT_EQUAL(sqlite3_column_int(st, 0), 2);
    sqlite3_finalize(st);
    heli::db_close(db2);
}

static void test_exec_sql_file_valid_sql_succeeds(void) {
    const char* p = "/tmp/heli_valid_sql.sql";
    {
        std::ofstream out(p);
        out << "CREATE TABLE IF NOT EXISTS T_DB_TEST(ID INTEGER);";
    }
    std::string err;
    bool ok = heli::db_exec_sql_file(g_db, p, err);
    CU_ASSERT_TRUE(ok);
    CU_ASSERT_TRUE(err.empty());
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("db", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "open_and_seed_helicopters", test_open_and_seed_helicopters)
        || !CU_add_test(suite, "extensions_payroll_table", test_extensions_payroll_table)
        || !CU_add_test(suite, "trigger_resource_check", test_trigger_resource_check)
        || !CU_add_test(suite, "resolve_sql_dir_default_and_env", test_resolve_sql_dir_default_and_env)
        || !CU_add_test(suite, "exec_sql_file_missing_file_fails", test_exec_sql_file_missing_file_fails)
        || !CU_add_test(suite, "open_with_bad_sql_dir_fails", test_open_with_bad_sql_dir_fails)
        || !CU_add_test(suite, "exec_sql_file_bad_sql_fails", test_exec_sql_file_bad_sql_fails)
        || !CU_add_test(suite, "exec_sql_file_empty_succeeds", test_exec_sql_file_empty_succeeds)
        || !CU_add_test(suite, "open_without_seed_has_empty_data", test_open_without_seed_has_empty_data)
        || !CU_add_test(suite, "db_close_nullptr_is_safe", test_db_close_nullptr_is_safe)
        || !CU_add_test(suite, "open_with_invalid_path_fails", test_open_with_invalid_path_fails)
        || !CU_add_test(suite, "seed_not_duplicated_on_reopen", test_seed_not_duplicated_on_reopen)
        || !CU_add_test(suite, "exec_sql_file_valid_sql_succeeds", test_exec_sql_file_valid_sql_succeeds)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
