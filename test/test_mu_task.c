#include "fff.h"
#include "../src/mu_task.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

// Fake mu_sched's functions (only those that mu_task calls)
FAKE_VALUE_FUNC(mu_task_err_t, mu_sched_immed, mu_task_t *);

// Fake task's function and user-defined hooks
FAKE_VOID_FUNC(test_task_fn, mu_task_t *);
FAKE_VOID_FUNC(test_task_set_state_hook, mu_task_t *, mu_task_state_t);
FAKE_VOID_FUNC(test_task_call_hook, mu_task_t *);
FAKE_VOID_FUNC(test_task_transfer_hook, mu_task_t *);

void setUp(void) {
    // Reset all faked functions
    RESET_FAKE(mu_sched_immed);

    RESET_FAKE(test_task_fn);
    RESET_FAKE(test_task_set_state_hook);
    RESET_FAKE(test_task_call_hook);
    RESET_FAKE(test_task_transfer_hook);
}

void tearDown(void) {
    // nothing yet
}

void test_task_init(void) {
    mu_task_t task;
    int user_obj;

    // mu_task_init returns task arg.
    TEST_ASSERT_EQUAL_PTR(&task, mu_task_init(&task, test_task_fn, 22, &user_obj));
    // fields are properly set up
    TEST_ASSERT_EQUAL_PTR(test_task_fn, mu_task_get_fn(&task));
    TEST_ASSERT_EQUAL_INT(22, mu_task_get_state(&task));
    TEST_ASSERT_EQUAL_PTR(&user_obj, mu_task_get_user_info(&task));
}

void test_task_call(void) {
    mu_task_t task;
    mu_task_init(&task, test_task_fn, 22, NULL);
    TEST_ASSERT_EQUAL_INT(0, test_task_fn_fake.call_count);

    // mu_task_call invokes test_task_fn
    mu_task_call(&task);
    TEST_ASSERT_EQUAL_INT(1, test_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, test_task_fn_fake.arg0_val);
    // mu_task_call on NULL task is a no-op
    mu_task_call(NULL);
    TEST_ASSERT_EQUAL_INT(1, test_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, test_task_fn_fake.arg0_val);

    // install user hook that gets called when state is set
    mu_task_install_call_hook(test_task_call_hook);
    TEST_ASSERT_EQUAL_INT(0, test_task_call_hook_fake.call_count);
    mu_task_call(&task);
    // test_task_call_hook was called with task arg
    TEST_ASSERT_EQUAL_INT(1, test_task_call_hook_fake.call_count);
    TEST_ASSERT_EQUAL_INT(&task, test_task_call_hook_fake.arg0_val);
    // test_task_fn was called as usual
    TEST_ASSERT_EQUAL_INT(2, test_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, test_task_fn_fake.arg0_val);
}

void user_info_can_be_updated(void) {
    mu_task_t task;
    int info;
    mu_task_init(&task, test_task_fn, 22, NULL);
    TEST_ASSERT_EQUAL_PTR(NULL, mu_task_get_user_info(&task));
    mu_task_set_user_info(&task, (void *)&info);
    TEST_ASSERT_EQUAL_PTR((void *)&info, mu_task_get_user_info(&task));
}

void test_task_set_state(void) {
    mu_task_t task;

    // Install user hook that gets called when state is set
    mu_task_install_set_state_hook(test_task_set_state_hook);
    mu_task_init(&task, test_task_fn, 22, NULL);
    TEST_ASSERT_EQUAL_INT(22, mu_task_get_state(&task));
    // set_state_hook is not invoked on init
    TEST_ASSERT_EQUAL_INT(0, test_task_set_state_hook_fake.call_count);

    mu_task_set_state(&task, 33);
    TEST_ASSERT_EQUAL_INT(33, mu_task_get_state(&task));
    // set_state_hook was called with the correct args.
    TEST_ASSERT_EQUAL_INT(1, test_task_set_state_hook_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, test_task_set_state_hook_fake.arg0_val);
    TEST_ASSERT_EQUAL_INT(33, test_task_set_state_hook_fake.arg1_val);

    // setting hook to NULL prevents further calls to user's hook function
    mu_task_install_set_state_hook(NULL);
    mu_task_set_state(&task, 44);
    TEST_ASSERT_EQUAL_INT(1, test_task_set_state_hook_fake.call_count);

    // Setting state returns task
    TEST_ASSERT_EQUAL_PTR(&task, mu_task_set_state(&task, 11));
    // Passing NULL task returns NULL
    TEST_ASSERT_EQUAL_PTR(NULL, mu_task_set_state(NULL, 11));
}

void test_task_user_info(void) {
    mu_task_t task;
    int user_info;

    mu_task_init(&task, test_task_fn, 22, NULL);
    TEST_ASSERT_EQUAL_PTR(NULL, mu_task_get_user_info(&task));
    mu_task_set_user_info(&task, &user_info);
    TEST_ASSERT_EQUAL_PTR(&user_info, mu_task_get_user_info(&task));
}

void test_scheduler_functions(void) {
    // lump all of the functions delegated to the scheduler in one test...
    mu_task_t task;
    mu_task_err_t ret = MU_TASK_ERR_NONE;
    mu_task_init(&task, test_task_fn, 0, NULL);

    // mu_task_transfer() with NULL transfer_hook is like mu_task_enqueue
    mu_task_install_transfer_hook(NULL);
    TEST_ASSERT_EQUAL_INT(ret, mu_task_transfer(&task));
    TEST_ASSERT_EQUAL_INT(1, mu_sched_immed_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, mu_sched_immed_fake.arg0_val);
    TEST_ASSERT_EQUAL_INT(0, test_task_transfer_hook_fake.call_count);

    // mu_task_transfer() with transfer_hook calls hook function
    mu_task_install_transfer_hook(test_task_transfer_hook);
    TEST_ASSERT_EQUAL_INT(ret, mu_task_transfer(&task));
    TEST_ASSERT_EQUAL_INT(2, mu_sched_immed_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, mu_sched_immed_fake.arg0_val);
    TEST_ASSERT_EQUAL_INT(1, test_task_transfer_hook_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&task, test_task_transfer_hook_fake.arg0_val);

}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_task_init);
    RUN_TEST(test_task_call);
    RUN_TEST(user_info_can_be_updated);
    RUN_TEST(test_task_set_state);
    RUN_TEST(test_scheduler_functions);

    return UNITY_END();
}
