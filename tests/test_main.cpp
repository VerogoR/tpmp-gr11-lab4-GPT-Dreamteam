#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>

static bool file_exists(const char* p) {
    struct stat st {};
    return stat(p, &st) == 0;
}

static std::string read_all(const char* p) {
    std::ifstream in(p);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

static void test_help_exit_zero(void) {
    int st = std::system("HELI_SQL_DIR=sql ./bin/helicopter_app --help > /tmp/heli_help.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
}

static void test_binary_exists(void) {
    CU_ASSERT_TRUE(file_exists("bin/helicopter_app"));
}

static void test_help_output_contains_name(void) {
    FILE* f = std::fopen("/tmp/heli_help.txt", "r");
    CU_ASSERT_PTR_NOT_NULL_FATAL(f);
    char buf[256];
    std::string out;
    while (fgets(buf, sizeof buf, f))
        out += buf;
    std::fclose(f);
    CU_ASSERT_TRUE(out.find("helicopter_app") != std::string::npos);
}

static void test_login_failure_exit_code_two(void) {
    int st = std::system(
        "printf 'commander\\nwrong\\n' | HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_main_fail.db > /tmp/heli_login_fail.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 2);
    std::string out = read_all("/tmp/heli_login_fail.txt");
    CU_ASSERT_TRUE(out.find("Неверный логин или пароль") != std::string::npos);
}

static void test_commander_menu_flow(void) {
    int st = std::system(
        "printf 'commander\\ncmd_secret\\n1\\n2\\n2025-01-01\\n2025-12-31\\n3\\n4\\n5\\n6\\n7\\n101\\n8\\n2025-01-01\\n2025-12-31\\n9\\n1001\\n2025-01-01\\n2025-12-31\\n10\\n1001\\n2025-01-01\\n2025-12-31\\n1\\n11\\n/tmp/heli_menu.csv\\n/tmp/heli_menu.json\\n12\\n101\\n42\\n0\\n' | HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_main_cmd.db > /tmp/heli_cmd_menu.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
    std::string out = read_all("/tmp/heli_cmd_menu.txt");
    CU_ASSERT_TRUE(out.find("Добро пожаловать, commander") != std::string::npos);
    CU_ASSERT_TRUE(out.find("неизвестная команда") != std::string::npos);
}

static void test_crew_restrictions_in_menu(void) {
    int st = std::system(
        "printf 'pilot_ivanov\\npilot1\\n3\\n7\\n0\\n' | HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_main_crew.db > /tmp/heli_crew_menu.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
    std::string out = read_all("/tmp/heli_crew_menu.txt");
    CU_ASSERT_TRUE(out.find("Добро пожаловать, pilot_ivanov") != std::string::npos);
    CU_ASSERT_TRUE(out.find("только командир") != std::string::npos);
}

static void test_invalid_sql_dir_exit_code_one(void) {
    int st = std::system(
        "printf 'x\\ny\\n' | HELI_SQL_DIR=no_such_sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_bad_sql_dir.db > /tmp/heli_bad_sql_dir.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 1);
}

static void test_non_numeric_menu_input_hits_default_branch(void) {
    int st = std::system(
        "printf 'commander\\ncmd_secret\\nabc\\n0\\n' | HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_menu_nonint.db > /tmp/heli_menu_nonint.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
    std::string out = read_all("/tmp/heli_menu_nonint.txt");
    CU_ASSERT_TRUE(out.find("неизвестная команда") != std::string::npos);
}

static void test_run_with_default_db_path(void) {
    int st = std::system(
        "printf 'commander\\ncmd_secret\\n0\\n' | HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app > /tmp/heli_default_db.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
    CU_ASSERT_TRUE(file_exists("build/helicopter.db"));
}

static void test_commander_full_crud_menu_paths(void) {
    int st = std::system(
        "printf "
        "'commander\\ncmd_secret\\n"
        "13\\n1\\n900\\nM900\\n2020-01-01\\n1000\\n2024-01-01\\n200\\n"
        "13\\n2\\n900\\nM900U\\n2020-01-02\\n1100\\n2024-01-02\\n220\\n"
        "13\\n3\\n900\\n"
        "14\\n1\\n9900\\nIvanov\\npilot\\n5\\naddr\\n1990-01-01\\n101\\n"
        "14\\n2\\n9900\\nIvanov2\\ncaptain\\n6\\naddr2\\n1990-01-02\\n101\\n"
        "14\\n3\\n9900\\n"
        "15\\n1\\n99001\\n2025-03-01\\n10\\n2\\n3\\n500\\n101\\n0\\n"
        "15\\n2\\n99001\\n2025-03-02\\n11\\n3\\n4\\n600\\n101\\n1\\n"
        "15\\n3\\n99001\\n"
        "16\\nmenu_user_1\\nmenu_pw\\n-1\\n"
        "0\\n' "
        "| HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_menu_crud.db > /tmp/heli_menu_crud.txt 2>&1");

    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
    std::string out = read_all("/tmp/heli_menu_crud.txt");
    CU_ASSERT_TRUE(out.find("Добро пожаловать, commander") != std::string::npos);
    CU_ASSERT_TRUE(out.find("ok") != std::string::npos);
}

static void test_commander_wrong_operation_submenu_paths(void) {
    int st = std::system(
        "printf 'commander\\ncmd_secret\\n13\\n9\\n901\\n14\\n9\\n9901\\n15\\n9\\n99101\\n0\\n' "
        "| HELI_SQL_DIR=sql HELI_SEED=1 ./bin/helicopter_app /tmp/heli_menu_subops.db > /tmp/heli_menu_subops.txt 2>&1");
    CU_ASSERT_TRUE(WIFEXITED(st));
    CU_ASSERT_EQUAL(WEXITSTATUS(st), 0);
    std::string out = read_all("/tmp/heli_menu_subops.txt");
    CU_ASSERT_TRUE(out.find("ok") != std::string::npos || out.find("неизвестная команда") != std::string::npos);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("app_main", nullptr, nullptr);
    if (!suite || !CU_add_test(suite, "help_exit_zero", test_help_exit_zero)
        || !CU_add_test(suite, "binary_exists", test_binary_exists)
        || !CU_add_test(suite, "help_output_contains_name", test_help_output_contains_name)
        || !CU_add_test(suite, "login_failure_exit_code_two", test_login_failure_exit_code_two)
        || !CU_add_test(suite, "commander_menu_flow", test_commander_menu_flow)
        || !CU_add_test(suite, "crew_restrictions_in_menu", test_crew_restrictions_in_menu)
        || !CU_add_test(suite, "invalid_sql_dir_exit_code_one", test_invalid_sql_dir_exit_code_one)
        || !CU_add_test(suite, "non_numeric_menu_input_hits_default_branch", test_non_numeric_menu_input_hits_default_branch)
        || !CU_add_test(suite, "run_with_default_db_path", test_run_with_default_db_path)
        || !CU_add_test(suite, "commander_full_crud_menu_paths", test_commander_full_crud_menu_paths)
        || !CU_add_test(suite, "commander_wrong_operation_submenu_paths", test_commander_wrong_operation_submenu_paths)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
