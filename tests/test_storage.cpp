#include "db.h"
#include "storage.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

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

static void test_save_image_blob(void) {
    std::string err;
    std::vector<unsigned char> png = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    CU_ASSERT_TRUE(heli::storage_save_image(g_db, "AIR_HELICOPTERS", 101, "png", png, err));
}

static void test_export_csv_header(void) {
    std::string err;
    CU_ASSERT_TRUE(heli::storage_export_flights_csv(g_db, "/tmp/heli_flights_test.csv", err));
    std::ifstream in("/tmp/heli_flights_test.csv");
    std::string line;
    CU_ASSERT_TRUE(static_cast<bool>(std::getline(in, line)));
    CU_ASSERT_TRUE(line.find("flight_code") != std::string::npos);
}

static void test_export_json_contains_heli(void) {
    std::string err;
    CU_ASSERT_TRUE(heli::storage_export_helicopters_json(g_db, "/tmp/heli_heli.json", err));
    std::ifstream in("/tmp/heli_heli.json");
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CU_ASSERT_TRUE(s.find("\"number\":101") != std::string::npos);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("storage", suite_init, suite_clean);
    if (!suite || !CU_add_test(suite, "save_image_blob", test_save_image_blob)
        || !CU_add_test(suite, "export_csv_header", test_export_csv_header)
        || !CU_add_test(suite, "export_json_contains_heli", test_export_json_contains_heli)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int nfail = CU_get_number_of_failures();
    CU_cleanup_registry();
    return nfail ? 1 : 0;
}
