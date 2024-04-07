#include "../src/mu_jems.h"
#include "unity.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// *****************************************************************************
// Local test support

#define MAX_LEVEL 10
#define TEST_STRING_LENGTH 120

#define PI_100                                                                 \
  "3.1415926535"                                                               \
  "8979323846"                                                                 \
  "2643383279"                                                                 \
  "5028841971"                                                                 \
  "6939937510"                                                                 \
  "5820974944"                                                                 \
  "5923078164"                                                                 \
  "0628620899"                                                                 \
  "8628034825"                                                                 \
  "3421170679"

static mu_jems_t s_jems;

static mu_jems_level_t s_levels[MAX_LEVEL];

static char s_test_string[TEST_STRING_LENGTH];

static int s_test_idx; // index into next char of s_test_string[]

void setUp(void) {
}

void tearDown(void) {
    // nothing yet
}

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Write one character to the test string.
 */
static void test_writer(char c, uintptr_t arg);

// *****************************************************************************
// Public code

void test_mu_jems_init(void) {
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0));
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_reset(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_item_count(&s_jems));
}

void test_mu_jems_object(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_object_open(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_object_close(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("{}", s_test_string, s_test_idx);
}

void test_mu_jems_array(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_array_open(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_array_close(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("[]", s_test_string, s_test_idx);
}

void test_mu_jems_number(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_number(&s_jems, 1.5));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("1.500000", s_test_string, s_test_idx);
}

void test_mu_jems_number_representable_as_integer(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_number(&s_jems, 2.0));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("2", s_test_string, s_test_idx);
}

void test_mu_jems_negative_integer(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_integer(&s_jems, -2));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("-2", s_test_string, s_test_idx);
}

void test_mu_jems_string(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_string(&s_jems, "woof"));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("\"woof\"", s_test_string, s_test_idx);
}

void test_mu_jems_bool_true(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_bool(&s_jems, true));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("true", s_test_string, s_test_idx);
}

void test_mu_jems_bool_false(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_bool(&s_jems, false));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("false", s_test_string, s_test_idx);
}

void test_mu_jems_true(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_true(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("true", s_test_string, s_test_idx);
}

void test_mu_jems_false(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_false(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("false", s_test_string, s_test_idx);
}

void test_mu_jems_null(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    TEST_ASSERT_EQUAL_PTR(&s_jems, mu_jems_null(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("null", s_test_string, s_test_idx);
}

void test_mu_jems_literal(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_literal(&s_jems, PI_100, strlen(PI_100));
    TEST_ASSERT_EQUAL_STRING_LEN(PI_100, s_test_string, s_test_idx);
}

void test_mu_jems_escaped_string(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_string(&s_jems, "say \"hey\"!");
    TEST_ASSERT_EQUAL_STRING_LEN("\"say \\\"hey\\\"!\"", s_test_string, s_test_idx);
}

void test_mu_jems_slashed_string(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_string(&s_jems, "forward / and back \\ slash");
    TEST_ASSERT_EQUAL_STRING_LEN("\"forward / and back \\\\ slash\"", s_test_string, s_test_idx);
}

void test_mu_jems_string_with_newline_and_return(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    mu_jems_string(&s_jems, "newline \n and return \r oh my");
    TEST_ASSERT_EQUAL_STRING_LEN("\"newline \\u000a and return \\u000d oh my\"", s_test_string, s_test_idx);
}

void test_mu_jems_uencoded_string(void) {
    // test \uxxxx style encoding
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    const char str[] = {0x01, 0x20, 0x7e, 0x7f, 0x80, 0x00};
    mu_jems_string(&s_jems, str);
    TEST_ASSERT_EQUAL_STRING_LEN("\"\\u0001 ~\\u007f\\u0080\"", s_test_string, s_test_idx);
}

void test_mu_jems_key_object(void) {
    // key:value pairs
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    mu_jems_object_open(&s_jems);
    mu_jems_key_object_open(&s_jems, "key");
    mu_jems_object_close(&s_jems);
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":{}}", s_test_string, s_test_idx);
}

void test_mu_jems_key_array(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    mu_jems_object_open(&s_jems);
    mu_jems_key_array_open(&s_jems, "key");
    mu_jems_array_close(&s_jems);
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":[]}", s_test_string, s_test_idx);
}

void test_mu_jems_key_number(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    mu_jems_object_open(&s_jems);
    mu_jems_key_number(&s_jems, "key", 1.234);
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":1.234000}", s_test_string, s_test_idx);
}

void test_mu_jems_key_number_representable_as_integer(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    mu_jems_object_open(&s_jems);
    mu_jems_key_integer(&s_jems, "key", 1234);
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":1234}", s_test_string, s_test_idx);
}

void test_mu_jems_key_string(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;

    mu_jems_object_open(&s_jems);
    mu_jems_key_string(&s_jems, "key", "value");
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":\"value\"}", s_test_string, s_test_idx);
}

void test_mu_jems_key_bytes(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_object_open(&s_jems);
    mu_jems_key_bytes(&s_jems, "key", (uint8_t *)"value", strlen("value"));
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":\"value\"}", s_test_string, s_test_idx);
}

void test_mu_jems_key_bool(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_object_open(&s_jems);
    mu_jems_key_bool(&s_jems, "key", true);
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":true}", s_test_string, s_test_idx);
}

void test_mu_jems_key_true(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_object_open(&s_jems);
    mu_jems_key_true(&s_jems, "key");
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":true}", s_test_string, s_test_idx);
}

void test_mu_jems_key_false(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_object_open(&s_jems);
    mu_jems_key_false(&s_jems, "key");
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":false}", s_test_string, s_test_idx);
}

void test_mu_jems_key_null(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_object_open(&s_jems);
    mu_jems_key_null(&s_jems, "key");
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"key\":null}", s_test_string, s_test_idx);
}

void test_mu_jems_key_literal(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    mu_jems_object_open(&s_jems);
    mu_jems_key_literal(&s_jems, "pi", PI_100, strlen(PI_100));
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_STRING_LEN("{\"pi\":" PI_100 "}", s_test_string, s_test_idx);
}

void test_mu_jems_composite(void) {
    mu_jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_item_count(&s_jems));
    mu_jems_object_open(&s_jems);
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_item_count(&s_jems));
    mu_jems_string(&s_jems, "colors");
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    mu_jems_array_open(&s_jems);
    TEST_ASSERT_EQUAL_INT(2, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(0, mu_jems_item_count(&s_jems));
    mu_jems_integer(&s_jems, 1);
    TEST_ASSERT_EQUAL_INT(2, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    mu_jems_integer(&s_jems, 2);
    TEST_ASSERT_EQUAL_INT(2, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(2, mu_jems_item_count(&s_jems));
    mu_jems_integer(&s_jems, 3);
    TEST_ASSERT_EQUAL_INT(2, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(3, mu_jems_item_count(&s_jems));
    mu_jems_array_close(&s_jems);
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(2, mu_jems_item_count(&s_jems));
    mu_jems_string(&s_jems, "valid");
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(3, mu_jems_item_count(&s_jems));
    mu_jems_true(&s_jems);
    TEST_ASSERT_EQUAL_INT(1, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(4, mu_jems_item_count(&s_jems));
    mu_jems_object_close(&s_jems);
    TEST_ASSERT_EQUAL_INT(0, mu_jems_curr_level(&s_jems));
    TEST_ASSERT_EQUAL_INT(1, mu_jems_item_count(&s_jems));
    TEST_ASSERT_EQUAL_STRING_LEN("{\"colors\":[1,2,3],\"valid\":true}", s_test_string, s_test_idx);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_jems_init);
    RUN_TEST(test_mu_jems_object);
    RUN_TEST(test_mu_jems_array);
    RUN_TEST(test_mu_jems_number);
    RUN_TEST(test_mu_jems_number_representable_as_integer);
    RUN_TEST(test_mu_jems_negative_integer);
    RUN_TEST(test_mu_jems_string);
    RUN_TEST(test_mu_jems_bool_true);
    RUN_TEST(test_mu_jems_bool_false);
    RUN_TEST(test_mu_jems_true);
    RUN_TEST(test_mu_jems_false);
    RUN_TEST(test_mu_jems_null);
    RUN_TEST(test_mu_jems_literal);
    RUN_TEST(test_mu_jems_escaped_string);
    RUN_TEST(test_mu_jems_slashed_string);
    RUN_TEST(test_mu_jems_string_with_newline_and_return);
    RUN_TEST(test_mu_jems_uencoded_string);
    RUN_TEST(test_mu_jems_key_object);
    RUN_TEST(test_mu_jems_key_array);
    RUN_TEST(test_mu_jems_key_number);
    RUN_TEST(test_mu_jems_key_number_representable_as_integer);
    RUN_TEST(test_mu_jems_key_string);
    RUN_TEST(test_mu_jems_key_bytes);
    RUN_TEST(test_mu_jems_key_bool);
    RUN_TEST(test_mu_jems_key_true);
    RUN_TEST(test_mu_jems_key_false);
    RUN_TEST(test_mu_jems_key_null);
    RUN_TEST(test_mu_jems_key_literal);
    RUN_TEST(test_mu_jems_composite);
    return UNITY_END();
}

// *****************************************************************************
// Private (static) code

static void test_writer(char c, uintptr_t arg) {
    (void)arg;
    if (s_test_idx < sizeof(s_test_string)) {
        s_test_string[s_test_idx++] = c;
    }
}

// *****************************************************************************
// End of file
