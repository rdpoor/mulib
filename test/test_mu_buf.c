#include "../src/mu_buf.h"
#include "unity.h"
#include <stdbool.h>

#define RO_BYTES_CAPACITY 8
#define RW_BYTES_CAPACITY 10

static const uint8_t s_ro_bytes[RO_BYTES_CAPACITY] = {0, 1, 2, 3, 4, 5, 6, 7};
static uint8_t s_rw_bytes[RW_BYTES_CAPACITY];
static mu_buf_t s_buf;

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void test_mu_buf_init_ro(void) {
    TEST_ASSERT_EQUAL_PTR(&s_buf, mu_buf_init_ro(&s_buf, &s_ro_bytes, RO_BYTES_CAPACITY));
    TEST_ASSERT_EQUAL_PTR(&s_ro_bytes, mu_buf_bytes_ro(&s_buf));
    TEST_ASSERT_EQUAL_INT(RO_BYTES_CAPACITY, mu_buf_capacity(&s_buf));
}

void test_mu_buf_init_rw(void) {
    TEST_ASSERT_EQUAL_PTR(&s_buf, mu_buf_init_rw(&s_buf, &s_rw_bytes, RW_BYTES_CAPACITY));
    TEST_ASSERT_EQUAL_PTR(&s_rw_bytes, mu_buf_bytes_rw(&s_buf));
    TEST_ASSERT_EQUAL_INT(RW_BYTES_CAPACITY, mu_buf_capacity(&s_buf));
}

void test_mu_buf_ref_ro(void) {
    mu_buf_init_ro(&s_buf, &s_ro_bytes, RO_BYTES_CAPACITY);
    TEST_ASSERT_EQUAL_PTR(&s_ro_bytes[0], mu_buf_ref_ro(&s_buf, 0));
    TEST_ASSERT_EQUAL_PTR(&s_ro_bytes[RO_BYTES_CAPACITY-1], mu_buf_ref_ro(&s_buf, RO_BYTES_CAPACITY-1));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_buf_ref_ro(&s_buf, RO_BYTES_CAPACITY));
}

void test_mu_buf_ref_rw(void) {
    mu_buf_init_rw(&s_buf, &s_rw_bytes, RW_BYTES_CAPACITY);
    TEST_ASSERT_EQUAL_PTR(&s_rw_bytes[0], mu_buf_ref_rw(&s_buf, 0));
    TEST_ASSERT_EQUAL_PTR(&s_rw_bytes[RW_BYTES_CAPACITY-1], mu_buf_ref_rw(&s_buf, RW_BYTES_CAPACITY-1));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_buf_ref_rw(&s_buf, RW_BYTES_CAPACITY));
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_buf_init_ro);
    RUN_TEST(test_mu_buf_init_rw);
    RUN_TEST(test_mu_buf_ref_ro);
    RUN_TEST(test_mu_buf_ref_rw);

    return UNITY_END();
}
