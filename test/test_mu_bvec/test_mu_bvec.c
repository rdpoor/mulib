#include "../mulib/mu_bvec.h"
#include "unity.h"
#include <stdbool.h>

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

#define CAPACITY 5
#define ABSURDLY_LARGE_COUNT 99999999

static const uint8_t s_ro_bytes[CAPACITY] = {'a', 'b', 'c', 'd', 'e'};
static uint8_t s_rw_bytes[CAPACITY];
static mu_bbuf_t s_bbuf;
static mu_bvec_t s_bvec;

// mu_bvec_t *mu_bvec_init(mu_bvec_t *bvec, mu_bbuf_t *bbuf)
void test_mu_bvec_init(void) {
    mu_bbuf_init_ro(&s_bbuf, s_ro_bytes, CAPACITY);
    TEST_ASSERT_EQUAL_PTR(&s_bvec, mu_bvec_init(&s_bvec, &s_bbuf));
}

// mu_bvec_t *mu_bvec_init_ro(mu_bvec_t *bvec, const uint8_t *bytes, size_t capacity);
// mu_bbuf_t *mu_bvec_get_bbuf(mu_bvec_t *bvec);
void test_mu_bvec_init_ro(void) {
    TEST_ASSERT_EQUAL_PTR(&s_bvec, mu_bvec_init_ro(&s_bvec, s_ro_bytes, CAPACITY));
    TEST_ASSERT_EQUAL_PTR(&s_bvec.bbuf, mu_bvec_get_bbuf(&s_bvec));
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_count(&s_bvec));
}

// mu_bvec_t *mu_bvec_init_rw(mu_bvec_t *bvec, uint8_t *bytes, size_t capacity);
// mu_bbuf_t *mu_bvec_get_bbuf(mu_bvec_t *bvec);
void test_mu_bvec_init_rw(void) {
    TEST_ASSERT_EQUAL_PTR(&s_bvec, mu_bvec_init_rw(&s_bvec, s_rw_bytes, CAPACITY));
    TEST_ASSERT_EQUAL_PTR(&s_bvec.bbuf, mu_bvec_get_bbuf(&s_bvec));
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_count(&s_bvec));
}

// mu_bvec_t *mu_bvec_reset(mu_bvec_t *bvec);
// size_t mu_get_count(mu_bvec_t *bvec);
// void mu_set_count(mu_bvec_t *bvec, size_t count);
void test_mu_bvec_reset(void) {
    mu_bvec_init_rw(&s_bvec, s_rw_bytes, CAPACITY);
    mu_bvec_set_count(&s_bvec, CAPACITY/2);
    TEST_ASSERT_EQUAL_INT(CAPACITY/2, mu_bvec_get_count(&s_bvec));
    mu_bvec_reset(&s_bvec);
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_count(&s_bvec));
}

// mu_bbuf_t *mu_bvec_get_bbuf(mu_bvec_t *bvec);
void test_mu_bvec_get_bbuf(void) {
    mu_bbuf_init_ro(&s_bbuf, s_ro_bytes, CAPACITY);
    mu_bvec_init(&s_bvec, &s_bbuf);
    TEST_ASSERT_EQUAL_PTR(&s_bvec.bbuf, mu_bvec_get_bbuf(&s_bvec));
    // Make sure they reference the same data
    TEST_ASSERT_EQUAL_PTR(mu_bbuf_bytes_ro(mu_bvec_get_bbuf(&s_bvec)),
                          mu_bbuf_bytes_ro(&s_bbuf));
}

// size_t mu_bvec_get_capacity(mu_bvec_t *bvec)
void test_mu_bvec_get_capacity(void) {
    mu_bvec_init_ro(&s_bvec, s_ro_bytes, CAPACITY);
    TEST_ASSERT_EQUAL_INT(CAPACITY, mu_bvec_get_capacity(&s_bvec));
}

// void mu_bvec_set_count(mu_bvec_t *bvec, size_t count);
void test_mu_bvec_set_count(void) {
    mu_bvec_init_rw(&s_bvec, s_rw_bytes, CAPACITY);
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_count(&s_bvec));
    mu_bvec_set_count(&s_bvec, CAPACITY);
    TEST_ASSERT_EQUAL_INT(CAPACITY, mu_bvec_get_count(&s_bvec));
    mu_bvec_set_count(&s_bvec, -1);
    TEST_ASSERT_EQUAL_INT(-1, mu_bvec_get_count(&s_bvec));
    mu_bvec_set_count(&s_bvec, ABSURDLY_LARGE_COUNT);
    TEST_ASSERT_EQUAL_INT(ABSURDLY_LARGE_COUNT, mu_bvec_get_count(&s_bvec));
}

// size_t mu_bvec_available(mu_bvec_t *bvec);
void test_mu_bvec_get_available(void) {
    mu_bvec_init_rw(&s_bvec, s_rw_bytes, CAPACITY);
    TEST_ASSERT_EQUAL_INT(CAPACITY, mu_bvec_get_available(&s_bvec));
    mu_bvec_set_count(&s_bvec, CAPACITY);
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_available(&s_bvec));
    mu_bvec_set_count(&s_bvec, CAPACITY+1);
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_available(&s_bvec));
    mu_bvec_set_count(&s_bvec, -1);
    TEST_ASSERT_EQUAL_INT(0, mu_bvec_get_available(&s_bvec));
}

void test_mu_bvec_ref_ro(void) {
    mu_bvec_init_ro(&s_bvec, s_ro_bytes, CAPACITY);
    TEST_ASSERT_EQUAL_PTR(&s_ro_bytes[2], mu_bvec_ref_ro(&s_bvec, 2));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_bvec_ref_ro(&s_bvec, CAPACITY));
}

void test_mu_bvec_ref_rw(void) {
    mu_bvec_init_rw(&s_bvec, s_rw_bytes, CAPACITY);
    TEST_ASSERT_EQUAL_PTR(&s_rw_bytes[2], mu_bvec_ref_rw(&s_bvec, 2));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_bvec_ref_rw(&s_bvec, CAPACITY));
}

// bool mu_bvec_read_byte(mu_bvec_t *bvec, uint8_t *byte);
void test_mu_bvec_read_byte(void) {
    uint8_t b;
    mu_bvec_init_ro(&s_bvec, s_ro_bytes, CAPACITY);
    TEST_ASSERT_TRUE(mu_bvec_read_byte(&s_bvec, &b));
    TEST_ASSERT_EQUAL_INT('a', b);
    TEST_ASSERT_TRUE(mu_bvec_read_byte(&s_bvec, &b));
    TEST_ASSERT_EQUAL_INT('b', b);
    TEST_ASSERT_TRUE(mu_bvec_read_byte(&s_bvec, &b));
    TEST_ASSERT_EQUAL_INT('c', b);
    TEST_ASSERT_TRUE(mu_bvec_read_byte(&s_bvec, &b));
    TEST_ASSERT_EQUAL_INT('d', b);
    TEST_ASSERT_TRUE(mu_bvec_read_byte(&s_bvec, &b));
    TEST_ASSERT_EQUAL_INT('e', b);
    TEST_ASSERT_FALSE(mu_bvec_read_byte(&s_bvec, &b));
}

// bool mu_bvec_write_byte(mu_bvec_t *bvec, uint8_t byte);
void test_mu_bvec_write_byte(void) {
    mu_bvec_init_rw(&s_bvec, s_rw_bytes, CAPACITY);
    mu_bbuf_t *buf = mu_bvec_get_bbuf(&s_bvec);
    TEST_ASSERT_TRUE(mu_bvec_write_byte(&s_bvec, 'f'));
    TEST_ASSERT_EQUAL_INT('f', *mu_bbuf_ref_rw(buf, 0));
    TEST_ASSERT_TRUE(mu_bvec_write_byte(&s_bvec, 'g'));
    TEST_ASSERT_EQUAL_INT('g', *mu_bbuf_ref_rw(buf, 1));
    TEST_ASSERT_TRUE(mu_bvec_write_byte(&s_bvec, 'h'));
    TEST_ASSERT_EQUAL_INT('h', *mu_bbuf_ref_rw(buf, 2));
    TEST_ASSERT_TRUE(mu_bvec_write_byte(&s_bvec, 'i'));
    TEST_ASSERT_EQUAL_INT('i', *mu_bbuf_ref_rw(buf, 3));
    TEST_ASSERT_TRUE(mu_bvec_write_byte(&s_bvec, 'j'));
    TEST_ASSERT_EQUAL_INT('j', *mu_bbuf_ref_rw(buf, 4));
    TEST_ASSERT_FALSE(mu_bvec_write_byte(&s_bvec, 'k'));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mu_bvec_init);
    RUN_TEST(test_mu_bvec_init_ro);
    RUN_TEST(test_mu_bvec_init_rw);
    RUN_TEST(test_mu_bvec_reset);
    RUN_TEST(test_mu_bvec_get_bbuf);
    RUN_TEST(test_mu_bvec_get_capacity);
    RUN_TEST(test_mu_bvec_set_count);
    RUN_TEST(test_mu_bvec_get_available);
    RUN_TEST(test_mu_bvec_ref_ro);
    RUN_TEST(test_mu_bvec_ref_rw);
    RUN_TEST(test_mu_bvec_read_byte);
    RUN_TEST(test_mu_bvec_write_byte);
    return UNITY_END();
}
