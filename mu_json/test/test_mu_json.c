#include "mu_json.h"
#include "mu_str.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

#define MAX_TOKENS 12

static const char *s_json =
  "{\"a\":10, \"b\":11, \"c\":[3, 4], \"d\":[]}";

static mu_json_token_t s_tokens[MAX_TOKENS];

void setUp(void) {
    // Reset all faked functions
    // hand-build a set of parsed tokens.  In the future, use mu_json_parse_xxx
    // to create this.
    //   0000000000111111111122222222223333333
    //   0123456789012345678901234567890123456
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    mu_json_token_t *t;
    mu_str_t str;

    mu_str_init_cstr(&str, s_json);

    t = &s_tokens[0];
    mu_str_slice(&t->str, &str, 0, 36); // {"a":10, "b":11, "c":[3, 4], "d":[]}
    t->type = MU_JSON_TOKEN_TYPE_OBJECT;
    t->depth = 0;

    t = &s_tokens[1];
    mu_str_slice(&t->str, &str, 1, 4); // "a"
    t->type = MU_JSON_TOKEN_TYPE_STRING;
    t->depth = 1;

    t = &s_tokens[2];
    mu_str_slice(&t->str, &str, 5, 7); // 10
    t->type = MU_JSON_TOKEN_TYPE_INTEGER;
    t->depth = 1;

    t = &s_tokens[3];
    mu_str_slice(&t->str, &str, 9, 12); // "b"
    t->type = MU_JSON_TOKEN_TYPE_STRING;
    t->depth = 1;

    t = &s_tokens[4];
    mu_str_slice(&t->str, &str, 13, 15); // 11
    t->type = MU_JSON_TOKEN_TYPE_INTEGER;
    t->depth = 1;

    t = &s_tokens[5];
    mu_str_slice(&t->str, &str, 17, 20); // "c"
    t->type = MU_JSON_TOKEN_TYPE_STRING;
    t->depth = 1;

    t = &s_tokens[6];
    mu_str_slice(&t->str, &str, 21, 27); // [3, 4]
    t->type = MU_JSON_TOKEN_TYPE_ARRAY;
    t->depth = 1;

    t = &s_tokens[7];
    mu_str_slice(&t->str, &str, 22, 23); // 3
    t->type = MU_JSON_TOKEN_TYPE_INTEGER;
    t->depth = 2;

    t = &s_tokens[8];
    mu_str_slice(&t->str, &str, 25, 26); // 4
    t->type = MU_JSON_TOKEN_TYPE_INTEGER;
    t->depth = 2;

    t = &s_tokens[9];
    mu_str_slice(&t->str, &str, 29, 32); // "d"
    t->type = MU_JSON_TOKEN_TYPE_STRING;
    t->depth = 1;

    t = &s_tokens[10];
    mu_str_slice(&t->str, &str, 33, 35); // []
    t->type = MU_JSON_TOKEN_TYPE_ARRAY;
    t->depth = 1;

    t = &s_tokens[11];  // list end
    mu_str_slice(&t->str, &str, 0, 36); // {"a":10, "b":11, "c":[3, 4], "d":[]}
    t->type = MU_JSON_TOKEN_TYPE_EOL;
    t->depth = -1;   // marks end of expression
}

void tearDown(void) {
    // nothing yet
}

void test_json_parser_init(void) {
    mu_json_parser_t p;
    mu_json_token_t store[MAX_TOKENS];

    TEST_ASSERT_EQUAL_INT(&p, mu_json_parser_init(&p, store, MAX_TOKENS));
}

void test_json_token_type(void) {
    //   0000000000111111111122222222223333333
    //   0123456789012345678901234567890123456
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_OBJECT, mu_json_token_type(&s_tokens[0]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(&s_tokens[1]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(&s_tokens[2]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(&s_tokens[3]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(&s_tokens[4]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(&s_tokens[5]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_ARRAY, mu_json_token_type(&s_tokens[6]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(&s_tokens[7]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_INTEGER, mu_json_token_type(&s_tokens[8]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_STRING, mu_json_token_type(&s_tokens[9]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_ARRAY, mu_json_token_type(&s_tokens[10]));
    TEST_ASSERT_EQUAL_INT(MU_JSON_TOKEN_TYPE_EOL, mu_json_token_type(&s_tokens[11]));
}

void test_json_token_depth(void) {
    //   0000000000111111111122222222223333333
    //   0123456789012345678901234567890123456
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    TEST_ASSERT_EQUAL_INT(0, mu_json_token_depth(&s_tokens[0]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[1]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[2]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[3]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[4]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[5]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[6]));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_depth(&s_tokens[7]));
    TEST_ASSERT_EQUAL_INT(2, mu_json_token_depth(&s_tokens[8]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[9]));
    TEST_ASSERT_EQUAL_INT(1, mu_json_token_depth(&s_tokens[10]));
    TEST_ASSERT_EQUAL_INT(-1, mu_json_token_depth(&s_tokens[11]));
}

void test_json_token_prev(void) {
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_prev(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_prev(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[1], mu_json_token_prev(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[2], mu_json_token_prev(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[3], mu_json_token_prev(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[4], mu_json_token_prev(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[5], mu_json_token_prev(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[6], mu_json_token_prev(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[7], mu_json_token_prev(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[8], mu_json_token_prev(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[9], mu_json_token_prev(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[10], mu_json_token_prev(&s_tokens[11]));
}

void test_json_token_next(void) {
    TEST_ASSERT_EQUAL_PTR(&s_tokens[1], mu_json_token_next(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[2], mu_json_token_next(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[3], mu_json_token_next(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[4], mu_json_token_next(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[5], mu_json_token_next(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[6], mu_json_token_next(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[7], mu_json_token_next(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[8], mu_json_token_next(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[9], mu_json_token_next(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[10], mu_json_token_next(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[11], mu_json_token_next(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_next(&s_tokens[11]));
}

void test_json_token_root(void) {
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_root(&s_tokens[11]));
}

void test_json_token_parent(void) {
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_parent(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[6], mu_json_token_parent(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[6], mu_json_token_parent(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[0], mu_json_token_parent(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_parent(&s_tokens[11]));
}

void test_json_token_child(void) {
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    TEST_ASSERT_EQUAL_PTR(&s_tokens[1], mu_json_token_child(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[7], mu_json_token_child(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_child(&s_tokens[11]));
}

void test_json_token_prev_sibling(void) {
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_prev_sibling(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_prev_sibling(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[1], mu_json_token_prev_sibling(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[2], mu_json_token_prev_sibling(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[3], mu_json_token_prev_sibling(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[4], mu_json_token_prev_sibling(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[5], mu_json_token_prev_sibling(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_prev_sibling(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[7], mu_json_token_prev_sibling(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[6], mu_json_token_prev_sibling(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[9], mu_json_token_prev_sibling(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[10], mu_json_token_prev_sibling(&s_tokens[11]));
}

void test_json_token_next_sibling(void) {
    //   {"a":10, "b":11, "c":[3, 4], "d":[]}
    //   01   2   3   4   5   67  8   9   0  1
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_next_sibling(&s_tokens[0]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[2], mu_json_token_next_sibling(&s_tokens[1]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[3], mu_json_token_next_sibling(&s_tokens[2]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[4], mu_json_token_next_sibling(&s_tokens[3]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[5], mu_json_token_next_sibling(&s_tokens[4]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[6], mu_json_token_next_sibling(&s_tokens[5]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[9], mu_json_token_next_sibling(&s_tokens[6]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[8], mu_json_token_next_sibling(&s_tokens[7]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_next_sibling(&s_tokens[8]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[10], mu_json_token_next_sibling(&s_tokens[9]));
    TEST_ASSERT_EQUAL_PTR(&s_tokens[11], mu_json_token_next_sibling(&s_tokens[10]));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_json_token_next_sibling(&s_tokens[11]));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_json_parser_init);
    RUN_TEST(test_json_token_type);
    RUN_TEST(test_json_token_depth);
    RUN_TEST(test_json_token_prev);
    RUN_TEST(test_json_token_next);
    RUN_TEST(test_json_token_root);
    RUN_TEST(test_json_token_parent);
    RUN_TEST(test_json_token_child);
    RUN_TEST(test_json_token_prev_sibling);
    RUN_TEST(test_json_token_next_sibling);

    return UNITY_END();
}
