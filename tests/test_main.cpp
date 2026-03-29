#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>

static bool file_exists(const char* p) {
    struct stat st {};
    return stat(p, &st) == 0;
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

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("app_main", nullptr, nullptr);
    if (!suite || !CU_add_test(suite, "help_exit_zero", test_help_exit_zero)
        || !CU_add_test(suite, "binary_exists", test_binary_exists)
        || !CU_add_test(suite, "help_output_contains_name", test_help_output_contains_name)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
