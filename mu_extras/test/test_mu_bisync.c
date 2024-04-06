#include "../src/mu_bisync.h"
#include "unity.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void test_mu_bisync_encode_normal(void) {
    const uint8_t src[] = {1, 2, 3, 4, 5, 1}; // 1 is SOH, 4 is EOT, 16 is DLE
    uint8_t encoded[11];
    uint8_t expected[] = {1, 16, 1, 2, 3, 16, 4, 5, 16, 1, 4};

    memset(encoded, 0, sizeof(encoded));
    TEST_ASSERT_EQUAL_INT(sizeof(expected), mu_bisync_encode(encoded, sizeof(encoded), src, sizeof(src)));
    for (int i=0; i<sizeof(expected); i++) {
        TEST_ASSERT_EQUAL_INT(expected[i], encoded[i]);
    }
}

void test_mu_bisync_decode_normal(void) {
    const uint8_t encoded[] = {1, 2, 3, 16, 4, 5, 16, 1, 4};
    uint8_t dst[6];
    uint8_t expected[] = {1, 2, 3, 4, 5, 1};

    memset(dst, 0, sizeof(dst));
    TEST_ASSERT_EQUAL_INT(sizeof(expected), mu_bisync_decode(dst, sizeof(dst), encoded, sizeof(encoded)));
    for (int i=0; i<sizeof(expected); i++) {
        TEST_ASSERT_EQUAL_INT(expected[i], dst[i]);
    }
}

void test_mu_bisync_encode_oflow(void) {
    const uint8_t src[] = {1, 2, 3, 4, 5, 1}; // 1 is SOH, 4 is EOT, 16 is DLE
    uint8_t encoded[11];

    memset(encoded, 0, sizeof(encoded));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_OFLOW, mu_bisync_encode(encoded, 0, src, sizeof(src)));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_OFLOW, mu_bisync_encode(encoded, 1, src, sizeof(src)));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_OFLOW, mu_bisync_encode(encoded, 2, src, sizeof(src)));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_OFLOW, mu_bisync_encode(encoded, 10, src, sizeof(src)));
}

void test_mu_bisync_decode_oflow(void) {
    const uint8_t encoded[] = {1, 2, 3, 16, 4, 5, 16, 1, 4};
    uint8_t dst[6];

    memset(dst, 0, sizeof(dst));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_OFLOW, mu_bisync_decode(dst, 4, encoded, sizeof(encoded)));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_OFLOW, mu_bisync_decode(dst, 5, encoded, sizeof(encoded)));
}

void test_mu_bisync_decode_format(void) {
    const uint8_t encoded1[] = {2, 3, 16, 4, 5, 16, 1, 6}; // first != SOH
    const uint8_t encoded2[] = {1, 2, 3, 16, 4, 5, 16, 1, 6}; // last != EOT
    uint8_t dst[7];

    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_FORMAT, mu_bisync_decode(dst, sizeof(dst), encoded1, sizeof(encoded1)));
    TEST_ASSERT_EQUAL_INT(MU_BISYNC_ERR_FORMAT, mu_bisync_decode(dst, sizeof(dst), encoded2, sizeof(encoded2)));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_bisync_encode_normal);
    RUN_TEST(test_mu_bisync_decode_normal);
    RUN_TEST(test_mu_bisync_encode_oflow);
    RUN_TEST(test_mu_bisync_decode_oflow);
    RUN_TEST(test_mu_bisync_decode_format);

    return UNITY_END();
}
