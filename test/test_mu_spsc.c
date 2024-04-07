#include "../src/mu_spsc.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void test_spsc_init(void) {
    mu_spsc_t q;
    mu_spsc_item_t storage[4];

    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_init(&q, storage, 4));
    TEST_ASSERT_EQUAL_INT(3, mu_spsc_capacity(&q));

    // storage size must be a power of two
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_SIZE, mu_spsc_init(&q, storage, 5));
}

void test_spsc_put(void) {
    mu_spsc_t q;
    mu_spsc_item_t storage[4];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    mu_spsc_init(&q, storage, 4);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_a));
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_b));
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_c));
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_FULL, mu_spsc_put(&q, obj_d));
}

void test_spsc_get(void) {
    mu_spsc_t q;
    mu_spsc_item_t storage[4];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    mu_spsc_item_t obj;

    mu_spsc_init(&q, storage, 4);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_a)); // a
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_b)); // ab
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_c)); // abc

    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj)); // bc
    TEST_ASSERT_EQUAL_PTR(obj_a, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj)); // c
    TEST_ASSERT_EQUAL_PTR(obj_b, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj)); // empty
    TEST_ASSERT_EQUAL_PTR(obj_c, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_EMPTY, mu_spsc_get(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(NULL, obj);
}

void test_spsc_ordering(void) {
    // verify that put and get work after wrapping around
    mu_spsc_t q;
    mu_spsc_item_t storage[4];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";
    char *obj_e = "e";
    mu_spsc_item_t obj;

    mu_spsc_init(&q, storage, 4);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_a)); // a
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_b)); // ab
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_c)); // abc
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj));  // bc
    TEST_ASSERT_EQUAL_PTR(obj_a, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_d)); // bcd
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj));  // cd
    TEST_ASSERT_EQUAL_PTR(obj_b, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_put(&q, obj_e)); // cde
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj));  // de
    TEST_ASSERT_EQUAL_PTR(obj_c, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj)); // e
    TEST_ASSERT_EQUAL_PTR(obj_d, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_NONE, mu_spsc_get(&q, &obj)); // <empty>
    TEST_ASSERT_EQUAL_PTR(obj_e, obj);
    TEST_ASSERT_EQUAL_INT(MU_SPSC_ERR_EMPTY, mu_spsc_get(&q, &obj));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_spsc_init);
    RUN_TEST(test_spsc_put);
    RUN_TEST(test_spsc_get);
    RUN_TEST(test_spsc_ordering);

    return UNITY_END();
}
