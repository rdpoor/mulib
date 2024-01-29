#include "../src/mu_array.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

void setUp(void) {
    // Code to run before each test
}

void tearDown(void) {
    // Code to run after each test
}

static int one = 1;
static int two = 2;
static int three = 3;

static int uint32_compare(void *a, void *b) {
    int int_a = *(int *)a;
    int int_b = *(int *)b;

    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}


void test_mu_array_sort(void) {
    mu_array_t array;
    void *storage[3];

    // test sorting an empty array: count remains zero
    mu_array_init(&array, storage, sizeof(storage)/sizeof(storage[0]));
    TEST_ASSERT_EQUAL_PTR(&array, mu_array_sort(&array, uint32_compare));
    TEST_ASSERT_EQUAL_UINT(0, mu_array_count(&array));

    // test sorting a non-empty array
    mu_array_reset(&array);
    mu_array_push(&array, &three);
    mu_array_push(&array, &one);
    mu_array_push(&array, &two);
    TEST_ASSERT_EQUAL_PTR(&array, mu_array_sort(&array, uint32_compare));
    TEST_ASSERT_EQUAL_PTR(&one, storage[0]);
    TEST_ASSERT_EQUAL_INT(&two, storage[1]);
    TEST_ASSERT_EQUAL_INT(&three, storage[2]);

    // test sorting an already sorted array
    mu_array_reset(&array);
    mu_array_push(&array, &one);
    mu_array_push(&array, &two);
    mu_array_push(&array, &three);
    TEST_ASSERT_EQUAL_PTR(&array, mu_array_sort(&array, uint32_compare));
    TEST_ASSERT_EQUAL_PTR(&one, storage[0]);
    TEST_ASSERT_EQUAL_INT(&two, storage[1]);
    TEST_ASSERT_EQUAL_INT(&three, storage[2]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mu_array_sort);
    return UNITY_END();
}
