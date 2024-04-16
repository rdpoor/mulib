#include "fff.h"
#include "../mulib/mu_mqueue.h"
#include "../mulib/mu_task.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(mu_task_call, mu_task_t *);

void setUp(void) {
    // Reset all faked functions
    RESET_FAKE(mu_task_call);
}

void tearDown(void) {
    // nothing yet
}

void test_mqueue_init(void) {
    mu_mqueue_t mqueue;
    void *storage[3];

    // mu_mqueue_init returns mqueue
    TEST_ASSERT_EQUAL_PTR(&mqueue, mu_mqueue_init(&mqueue, storage, 3, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(3, mu_mqueue_capacity(&mqueue));
    TEST_ASSERT_EQUAL_INT(0, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_is_empty(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_full(&mqueue));
}

void test_mqueue_count(void) {
    mu_mqueue_t mqueue;
    void *storage[3];

    mu_mqueue_init(&mqueue, storage, 3, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_is_empty(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_full(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, "a"));
    TEST_ASSERT_EQUAL_INT(1, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_empty(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_full(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, "b"));
    TEST_ASSERT_EQUAL_INT(2, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_empty(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_full(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, "c"));
    TEST_ASSERT_EQUAL_INT(3, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_empty(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_is_full(&mqueue));

    // put after full
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_put(&mqueue, "d"));
    TEST_ASSERT_EQUAL_INT(3, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_is_empty(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_is_full(&mqueue));
}

void test_mqueue_ordering(void) {
    mu_mqueue_t mqueue;
    void *storage[3];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";
    char *obj_e = "e";
    char *obj_f = "f";
    void *obj;

    mu_mqueue_init(&mqueue, storage, 3, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_a));  // a
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_b));  // a b
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_c));  // a b c
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_put(&mqueue, obj_d)); // a b c
    TEST_ASSERT_EQUAL_INT(3, mu_mqueue_count(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));   // b c
    TEST_ASSERT_EQUAL_PTR(obj_a, obj);
    TEST_ASSERT_EQUAL_INT(2, mu_mqueue_count(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));   // c
    TEST_ASSERT_EQUAL_PTR(obj_b, obj);
    TEST_ASSERT_EQUAL_INT(1, mu_mqueue_count(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_e));  // c e
    TEST_ASSERT_EQUAL_INT(2, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_f));  // c e f
    TEST_ASSERT_EQUAL_INT(3, mu_mqueue_count(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));   // e f
    TEST_ASSERT_EQUAL_PTR(obj_c, obj);
    TEST_ASSERT_EQUAL_INT(2, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));   // f
    TEST_ASSERT_EQUAL_PTR(obj_e, obj);
    TEST_ASSERT_EQUAL_INT(1, mu_mqueue_count(&mqueue));
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));   // <empty>
    TEST_ASSERT_EQUAL_PTR(obj_f, obj);
    TEST_ASSERT_EQUAL_INT(0, mu_mqueue_count(&mqueue));

    // mu_mqueue_get sets obj to NULL on empty queue
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_get(&mqueue, &obj));   // <empty>
    TEST_ASSERT_EQUAL_PTR(NULL, obj);
    TEST_ASSERT_EQUAL_INT(0, mu_mqueue_count(&mqueue));
}

void test_mqueue_peek(void) {
    mu_mqueue_t mqueue;
    void *storage[3];
    char *obj_a = "a";
    void *obj;

    mu_mqueue_init(&mqueue, storage, 3, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_peek(&mqueue, &obj));
    TEST_ASSERT_EQUAL_PTR(NULL, obj);
    TEST_ASSERT_EQUAL_INT(0, mu_mqueue_count(&mqueue));

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_a));  // a
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_peek(&mqueue, &obj));  // still a
    TEST_ASSERT_EQUAL_PTR(obj_a, obj);
    TEST_ASSERT_EQUAL_INT(1, mu_mqueue_count(&mqueue));
}

void test_mqueue_on_put(void) {
    mu_mqueue_t mqueue;
    mu_task_t on_put;
    void *storage[3];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";

    mu_mqueue_init(&mqueue, storage, 3, &on_put, NULL);
    TEST_ASSERT_EQUAL_INT(0, mu_task_call_fake.call_count);

    // on_put is invoked following mu_mqueue_put()
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_a));  // a
    TEST_ASSERT_EQUAL_INT(1, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_put, mu_task_call_fake.arg0_val);

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_b));  // a b
    TEST_ASSERT_EQUAL_INT(2, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_put, mu_task_call_fake.arg0_val);

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_c));  // a b c
    TEST_ASSERT_EQUAL_INT(3, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_put, mu_task_call_fake.arg0_val);

    // on_put is not invoked following mu_mqueue_put() if queue is full
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_put(&mqueue, obj_d));  // a b c
    TEST_ASSERT_EQUAL_INT(3, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_put, mu_task_call_fake.arg0_val);
}

void test_mqueue_on_get(void) {
    mu_mqueue_t mqueue;
    mu_task_t on_get;
    void *storage[3];
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    void *obj;

    mu_mqueue_init(&mqueue, storage, 3, NULL, &on_get);
    TEST_ASSERT_EQUAL_INT(0, mu_task_call_fake.call_count);

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_a));  // a
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_b));  // a b
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_put(&mqueue, obj_c));  // a b c

    // on_get is invoked following a call to mu_queue_get on non-empty queue
    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));  // b c
    TEST_ASSERT_EQUAL_INT(1, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_get, mu_task_call_fake.arg0_val);

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));  // c
    TEST_ASSERT_EQUAL_INT(2, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_get, mu_task_call_fake.arg0_val);

    TEST_ASSERT_EQUAL_INT(true, mu_mqueue_get(&mqueue, &obj));  // <empty>
    TEST_ASSERT_EQUAL_INT(3, mu_task_call_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&on_get, mu_task_call_fake.arg0_val);

    // on_get is not invoked following a call to mu_queue_get on an empty queue
    TEST_ASSERT_EQUAL_INT(false, mu_mqueue_get(&mqueue, &obj)); // <empty>
    TEST_ASSERT_EQUAL_INT(3, mu_task_call_fake.call_count);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mqueue_init);
    RUN_TEST(test_mqueue_count);
    RUN_TEST(test_mqueue_ordering);
    RUN_TEST(test_mqueue_peek);
    RUN_TEST(test_mqueue_on_put);
    RUN_TEST(test_mqueue_on_get);

    return UNITY_END();
}
