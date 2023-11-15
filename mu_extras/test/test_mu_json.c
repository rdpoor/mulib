#include "fff.h"
#include "../src/mu_json.h"
#include "../../mu_string/src/mu_str.h"
#include "unity.h"
#include <stdio.h>

DEFINE_FFF_GLOBALS;

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void parse_valid_json_sequentially(void) {
    mu_json_token_t tokens[20];
    mu_json_token_t *tok;
    size_t n_tokens = sizeof(tokens)/sizeof(tokens[0]);
    const char *json;

    // idx: 01 2  3   4  5     6  78  9 10   11  12  13  14
    // lvl: 01 2  2   2  2     2  23  3  3    2  2    2  2
    // str: [{"a":1, "b":2.0, "c":[4, 5, 6], "d":{}, "e":null}]
    json = "[{\"a\":1, \"b\":2.0, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}]";
    TEST_ASSERT_EQUAL_INT(15, mu_json_parse_cstr(tokens, n_tokens, json));

    tok = &tokens[0];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_ARRAY, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(0, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN(
        "[{\"a\":1, \"b\":2, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}]",
        mu_json_token_string(tok), mu_json_token_string_length(tok));

    tok = &tokens[1];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_OBJECT, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN(
        "{\"a\":1, \"b\":2.0, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}",
        mu_json_token_string(tok), mu_json_token_string_length(tok));

    tok = &tokens[2];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("\"a\"", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[3];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("1", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[4];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("\"b\"", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[5];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_NUMBER, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("2.0", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));
    tok = &tokens[6];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("\"c\"", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[7];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_ARRAY, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("[4, 5, 6]", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[8];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(3, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("4", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[9];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(3, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("5", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[10];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(3, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("6", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[11];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("\"d\"", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[12];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_OBJECT, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("{}", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[13];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("\"e\"", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

    tok = &tokens[14];
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_NULL, mu_json_token_type(tok));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_level(tok));
    TEST_ASSERT_EQUAL_STRING_LEN("null", mu_json_token_string(tok),
                             mu_json_token_string_length(tok));

}

// See test_json_checker
#define JSON_TEST_SUITE_DIR "/mnt/c/Users/r/Projects/JSONTestSuite/test_parsing/"

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_json_parse_cstr);

    return UNITY_END();
}
