#include "fff.h"
#include "../src/mu_json.h"
#include "../../mu_string/src/mu_str.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void test_json_parse_str(void) {
}

void test_json_parse_cstr(void) {
}

void test_json_emit(void) {
}

void test_json_emit_key(void) {
}

void test_json_emit_ckey(void) {
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_json_parse_str);
    RUN_TEST(test_json_parse_cstr);
    RUN_TEST(test_json_emit);
    RUN_TEST(test_json_emit_key);
    RUN_TEST(test_json_emit_ckey);

    return UNITY_END();
}
