#include "../src/mu_array.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

#define ARRAY_CAPACITY 3

void setUp(void) {
    // Code to run before each test
}

void tearDown(void) {
    // Code to run after each test
}

static int one = 1;
static int two = 2;
static int three = 3;
static int four = 4;

static int int_compare(void *a, void *b) {
    int int_a = *(int *)a;
    int int_b = *(int *)b;

    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

void test_mu_array_init(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    TEST_ASSERT_EQUAL_PTR(mu_array_init(&a, storage, ARRAY_CAPACITY), &a);
    TEST_ASSERT_EQUAL_INT(ARRAY_CAPACITY, mu_array_capacity(&a));

}

void test_mu_array_push(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    mu_array_init(&a, storage, ARRAY_CAPACITY);
    TEST_ASSERT_TRUE(mu_array_is_empty(&a));
    TEST_ASSERT_FALSE(mu_array_is_full(&a));
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 0);

    TEST_ASSERT_TRUE(mu_array_push(&a, obj_a));
    TEST_ASSERT_FALSE(mu_array_is_empty(&a));
    TEST_ASSERT_FALSE(mu_array_is_full(&a));
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 1);

    TEST_ASSERT_TRUE(mu_array_push(&a, obj_b));
    TEST_ASSERT_FALSE(mu_array_is_empty(&a));
    TEST_ASSERT_FALSE(mu_array_is_full(&a));
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 2);

    TEST_ASSERT_TRUE(mu_array_push(&a, obj_c));
    TEST_ASSERT_FALSE(mu_array_is_empty(&a));
    TEST_ASSERT_TRUE(mu_array_is_full(&a));
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 3);

    TEST_ASSERT_FALSE(mu_array_push(&a, obj_d));  // is now full
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 3);
}

void test_mu_array_pop(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    void *obj;

    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);

    // get returns items in LIFO (stack) order
    TEST_ASSERT_FALSE(mu_array_is_empty(&a));
    TEST_ASSERT_TRUE(mu_array_is_full(&a));

    TEST_ASSERT_TRUE(mu_array_pop(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);
    TEST_ASSERT_FALSE(mu_array_is_empty(&a));
    TEST_ASSERT_FALSE(mu_array_is_full(&a));

    TEST_ASSERT_TRUE(mu_array_pop(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);
    TEST_ASSERT_FALSE(mu_array_is_empty(&a));
    TEST_ASSERT_FALSE(mu_array_is_full(&a));

    TEST_ASSERT_TRUE(mu_array_pop(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);
    TEST_ASSERT_TRUE(mu_array_is_empty(&a));
    TEST_ASSERT_FALSE(mu_array_is_full(&a));

    TEST_ASSERT_FALSE(mu_array_pop(&a, &obj));
}

void test_mu_array_peek(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    void *obj;

    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);

    // get item that will be returned by pop
    TEST_ASSERT_TRUE(mu_array_peek(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);
    TEST_ASSERT_TRUE(mu_array_pop(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);

    TEST_ASSERT_TRUE(mu_array_peek(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);
    TEST_ASSERT_TRUE(mu_array_pop(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);

    TEST_ASSERT_TRUE(mu_array_peek(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);
    TEST_ASSERT_TRUE(mu_array_pop(&a, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);

    TEST_ASSERT_FALSE(mu_array_peek(&a, &obj));
}

void test_mu_array_ref(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    void *obj;

    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);

    TEST_ASSERT_TRUE(mu_array_ref(&a, &obj, 0));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);

    TEST_ASSERT_TRUE(mu_array_ref(&a, &obj, 1));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);

    TEST_ASSERT_FALSE(mu_array_ref(&a, &obj, 2));

    TEST_ASSERT_FALSE(mu_array_ref(&a, &obj, 3));
}

void test_mu_array_insert(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    // insert into empty array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    memset(storage, 0, sizeof(storage));
    TEST_ASSERT_TRUE(mu_array_insert(&a, obj_a, 0));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 1);

    // insert at head of non-empty array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    memset(storage, 0, sizeof(storage));
    mu_array_push(&a, obj_a);
    TEST_ASSERT_TRUE(mu_array_insert(&a, obj_b, 0));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_b);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 2);

    // insert at tail of non-empty array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    memset(storage, 0, sizeof(storage));
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    TEST_ASSERT_TRUE(mu_array_insert(&a, obj_c, 2));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_b);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[2], obj_c);  // (circumventing the API)
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 3);

    // insert at tail of full array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    memset(storage, 0, sizeof(storage));
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    TEST_ASSERT_FALSE(mu_array_insert(&a, obj_d, 3));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_b);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[2], obj_c);  // (circumventing the API)
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 3);

    // insert at head of full array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    memset(storage, 0, sizeof(storage));
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    TEST_ASSERT_FALSE(mu_array_insert(&a, obj_d, 0));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_b);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[2], obj_c);  // (circumventing the API)
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 3);

    // insert beyond tail of empty array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    memset(storage, 0, sizeof(storage));
    TEST_ASSERT_FALSE(mu_array_insert(&a, obj_d, 1));
    TEST_ASSERT_EQUAL_INT(mu_array_count(&a), 0);
}

void test_mu_array_delete(void) {
    mu_array_t a;
    void *storage[ARRAY_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    void *obj;

    // delete from head of array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    TEST_ASSERT_TRUE(mu_array_delete(&a, &obj, 0));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_b);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_c);  // (circumventing the API)

    // delete from middle of array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    TEST_ASSERT_TRUE(mu_array_delete(&a, &obj, 1));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_c);  // (circumventing the API)

    // delete from tail of array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    TEST_ASSERT_TRUE(mu_array_delete(&a, &obj, 2));
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_b);  // (circumventing the API)

    // delete beyond capacity of full array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    TEST_ASSERT_FALSE(mu_array_delete(&a, &obj, 3));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_b);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[2], obj_c);  // (circumventing the API)

    // delete beyond tail of non-full array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    TEST_ASSERT_FALSE(mu_array_delete(&a, &obj, 2));
    TEST_ASSERT_EQUAL_PTR(a.storage[0], obj_a);  // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(a.storage[1], obj_b);  // (circumventing the API)

    // delete from empty array
    mu_array_init(&a, storage, ARRAY_CAPACITY);
    TEST_ASSERT_FALSE(mu_array_delete(&a, &obj, 0));
}

void test_mu_array_index(void) {
    mu_array_t a;
    void *storage[5];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    mu_array_init(&a, storage, sizeof(storage)/sizeof(storage[0]));
    memset(storage, 0, sizeof(storage));

    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_a);

    TEST_ASSERT_EQUAL_INT(mu_array_index(&a, obj_a), 0);
    TEST_ASSERT_EQUAL_INT(mu_array_index(&a, obj_b), 1);
    TEST_ASSERT_EQUAL_INT(mu_array_index(&a, obj_c), 2);
    TEST_ASSERT_EQUAL_INT(mu_array_index(&a, obj_d), MU_ARRAY_NOT_FOUND);
}

void test_mu_array_rindex(void) {
    mu_array_t a;
    void *storage[5];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    mu_array_init(&a, storage, sizeof(storage)/sizeof(storage[0]));
    memset(storage, 0, sizeof(storage));

    mu_array_push(&a, obj_a);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_c);
    mu_array_push(&a, obj_b);
    mu_array_push(&a, obj_a);

    TEST_ASSERT_EQUAL_INT(mu_array_rindex(&a, obj_a), 4);
    TEST_ASSERT_EQUAL_INT(mu_array_rindex(&a, obj_b), 3);
    TEST_ASSERT_EQUAL_INT(mu_array_rindex(&a, obj_c), 2);
    TEST_ASSERT_EQUAL_INT(mu_array_rindex(&a, obj_d), MU_ARRAY_NOT_FOUND);
}

void test_mu_array_sort(void) {
    mu_array_t a;
    void *storage[3];

    // test sorting an empty array: count remains zero
    mu_array_init(&a, storage, sizeof(storage)/sizeof(storage[0]));
    TEST_ASSERT_EQUAL_PTR(&a, mu_array_sort(&a, int_compare));
    TEST_ASSERT_EQUAL_UINT(0, mu_array_count(&a));

    // test sorting a non-empty array
    mu_array_reset(&a);
    mu_array_push(&a, &three);
    mu_array_push(&a, &one);
    mu_array_push(&a, &two);
    TEST_ASSERT_EQUAL_PTR(&a, mu_array_sort(&a, int_compare));
    TEST_ASSERT_EQUAL_PTR(&one, storage[0]);
    TEST_ASSERT_EQUAL_PTR(&two, storage[1]);
    TEST_ASSERT_EQUAL_PTR(&three, storage[2]);

    // test sorting an already sorted array
    mu_array_reset(&a);
    mu_array_push(&a, &one);
    mu_array_push(&a, &two);
    mu_array_push(&a, &three);
    TEST_ASSERT_EQUAL_PTR(&a, mu_array_sort(&a, int_compare));
    TEST_ASSERT_EQUAL_PTR(&one, storage[0]);
    TEST_ASSERT_EQUAL_PTR(&two, storage[1]);
    TEST_ASSERT_EQUAL_PTR(&three, storage[2]);
}

void test_mu_array_insert_sorted(void) {
    mu_array_t a;
    void *storage[3];

    mu_array_init(&a, storage, sizeof(storage)/sizeof(storage[0]));
    memset(storage, 0, sizeof(storage));

    // insert into empty array
    TEST_ASSERT_TRUE(mu_array_insert_sorted(&a, &two, int_compare));
    TEST_ASSERT_EQUAL_PTR(&two, storage[0]);
    TEST_ASSERT_EQUAL_UINT(mu_array_count(&a), 1);

    // insert as last item
    TEST_ASSERT_TRUE(mu_array_insert_sorted(&a, &three, int_compare));
    TEST_ASSERT_EQUAL_PTR(&two, storage[0]);
    TEST_ASSERT_EQUAL_PTR(&three, storage[1]);
    TEST_ASSERT_EQUAL_UINT(mu_array_count(&a), 2);

    // insert as first item
    TEST_ASSERT_TRUE(mu_array_insert_sorted(&a, &one, int_compare));
    TEST_ASSERT_EQUAL_PTR(&one, storage[0]);
    TEST_ASSERT_EQUAL_PTR(&two, storage[1]);
    TEST_ASSERT_EQUAL_PTR(&three, storage[2]);
    TEST_ASSERT_EQUAL_UINT(mu_array_count(&a), 3);

    // insert into full array
    TEST_ASSERT_FALSE(mu_array_insert_sorted(&a, &four, int_compare));

    // insert between two items
    mu_array_init(&a, storage, sizeof(storage)/sizeof(storage[0]));
    memset(storage, 0, sizeof(storage));
    mu_array_push(&a, &one);
    mu_array_push(&a, &three);
    TEST_ASSERT_TRUE(mu_array_insert_sorted(&a, &two, int_compare));
    TEST_ASSERT_EQUAL_PTR(&one, storage[0]);
    TEST_ASSERT_EQUAL_PTR(&two, storage[1]);
    TEST_ASSERT_EQUAL_PTR(&three, storage[2]);
    TEST_ASSERT_EQUAL_UINT(mu_array_count(&a), 3);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mu_array_init);
    RUN_TEST(test_mu_array_push);
    RUN_TEST(test_mu_array_pop);
    RUN_TEST(test_mu_array_peek);
    RUN_TEST(test_mu_array_ref);
    RUN_TEST(test_mu_array_insert);
    RUN_TEST(test_mu_array_delete);
    RUN_TEST(test_mu_array_index);
    RUN_TEST(test_mu_array_rindex);
    RUN_TEST(test_mu_array_sort);
    RUN_TEST(test_mu_array_insert_sorted);
    return UNITY_END();
}
