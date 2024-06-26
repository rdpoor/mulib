#include "../mulib/mu_queue.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

#define QUEUE_CAPACITY 3

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void test_queue_init(void) {
    mu_queue_t q;
    void *storage[QUEUE_CAPACITY];

    TEST_ASSERT_EQUAL_PTR(mu_queue_init(&q, storage, QUEUE_CAPACITY), &q);
    TEST_ASSERT_EQUAL_INT(QUEUE_CAPACITY, mu_queue_capacity(&q));
}

void test_queue_put(void) {
    mu_queue_t q;
    void *storage[QUEUE_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    mu_queue_init(&q, storage, QUEUE_CAPACITY);
    TEST_ASSERT_TRUE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 0);

    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_a));
    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 1);

    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_b));
    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 2);

    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_c));
    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_TRUE(mu_queue_is_full(&q));
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 3);

    TEST_ASSERT_FALSE(mu_queue_put(&q, obj_d));  // is now full
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 3);
}

void test_queue_get(void) {
    mu_queue_t q;
    void *storage[QUEUE_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    void *obj;

    mu_queue_init(&q, storage, QUEUE_CAPACITY);
    mu_queue_put(&q, obj_a);
    mu_queue_put(&q, obj_b);
    mu_queue_put(&q, obj_c);

    // get returns items in FIFO order
    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_TRUE(mu_queue_is_full(&q));

    TEST_ASSERT_TRUE(mu_queue_get(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);
    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));

    TEST_ASSERT_TRUE(mu_queue_get(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);
    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));

    TEST_ASSERT_TRUE(mu_queue_get(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);
    TEST_ASSERT_TRUE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));

    TEST_ASSERT_FALSE(mu_queue_get(&q, &obj));
}

void test_queue_peek(void) {
    mu_queue_t q;
    void *storage[QUEUE_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    void *obj;

    mu_queue_init(&q, storage, QUEUE_CAPACITY);
    mu_queue_put(&q, obj_a);
    mu_queue_put(&q, obj_b);
    mu_queue_put(&q, obj_c);

    TEST_ASSERT_TRUE(mu_queue_peek(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);
    mu_queue_get(&q, &obj);

    TEST_ASSERT_TRUE(mu_queue_peek(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);
    mu_queue_get(&q, &obj);

    TEST_ASSERT_TRUE(mu_queue_peek(&q, &obj));
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);
    mu_queue_get(&q, &obj);

    TEST_ASSERT_FALSE(mu_queue_peek(&q, &obj));
}

void test_queue_wraparaound(void) {
    mu_queue_t q;
    void *storage[QUEUE_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";
    void *obj;

    // fill the queue
    mu_queue_init(&q, storage, QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL_INT(q.index, 0);         // (circumventing the API)
    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_a)); // [- - -] => [a - -]
    TEST_ASSERT_EQUAL_INT(q.index, 1);         // (circumventing the API)

    // fetch one, put another...
    TEST_ASSERT_TRUE(mu_queue_get(&q, &obj));  // [a - -] => [- - -]
    TEST_ASSERT_EQUAL_INT(q.index, 1);         // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(obj, obj_a);
    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_b)); // [- - -] => [- b -]
    TEST_ASSERT_EQUAL_INT(q.index, 2);         // (circumventing the API)

    TEST_ASSERT_TRUE(mu_queue_get(&q, &obj));  // [- b -] => [- - -]
    TEST_ASSERT_EQUAL_INT(q.index, 2);         // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(obj, obj_b);
    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_c)); // [- - -] => [- - c]
    TEST_ASSERT_EQUAL_INT(q.index, 0);         // (circumventing the API)

    TEST_ASSERT_TRUE(mu_queue_get(&q, &obj));  // [c - -] => [- - -]
    TEST_ASSERT_EQUAL_INT(q.index, 0);         // (circumventing the API)
    TEST_ASSERT_EQUAL_PTR(obj, obj_c);
    TEST_ASSERT_TRUE(mu_queue_put(&q, obj_d)); // [- - -] => [d - -]
    TEST_ASSERT_EQUAL_INT(q.index, 1);         // (circumventing the API)
}


void test_queue_reset(void) {
    mu_queue_t q;
    void *storage[QUEUE_CAPACITY];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";

    mu_queue_init(&q, storage, QUEUE_CAPACITY);
    mu_queue_put(&q, obj_a);
    mu_queue_put(&q, obj_b);
    mu_queue_put(&q, obj_c);

    TEST_ASSERT_FALSE(mu_queue_is_empty(&q));
    TEST_ASSERT_TRUE(mu_queue_is_full(&q));
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 3);

    TEST_ASSERT_EQUAL_PTR(mu_queue_reset(&q), &q);

    TEST_ASSERT_TRUE(mu_queue_is_empty(&q));
    TEST_ASSERT_FALSE(mu_queue_is_full(&q));
    TEST_ASSERT_EQUAL_INT(mu_queue_count(&q), 0);
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_queue_init);
    RUN_TEST(test_queue_put);
    RUN_TEST(test_queue_get);
    RUN_TEST(test_queue_peek);
    RUN_TEST(test_queue_wraparaound);
    RUN_TEST(test_queue_reset);

    return UNITY_END();
}
