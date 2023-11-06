#include "../src/mu_timer.h"
#include "../src/mu_sched.h"
#include "../src/mu_task.h"
#include "../src/mu_time.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(mu_clock_fn, mu_sched_get_clock_source);
FAKE_VALUE_FUNC(MU_SCHED_ABS_TIME, mu_time_offset, MU_SCHED_ABS_TIME, MU_SCHED_REL_TIME);
FAKE_VALUE_FUNC(MU_SCHED_REL_TIME, mu_time_ms_to_rel, int);
FAKE_VALUE_FUNC(mu_task_err_t, mu_task_cancel, mu_task_t *);
FAKE_VALUE_FUNC(mu_task_err_t, mu_task_defer_until, mu_task_t *, MU_SCHED_ABS_TIME);
FAKE_VALUE_FUNC(mu_task_err_t, mu_task_transfer, mu_task_t *);
FAKE_VALUE_FUNC(mu_task_state_t, mu_task_get_state, mu_task_t *);
FAKE_VALUE_FUNC(mu_task_t *, mu_task_init, mu_task_t *, mu_task_fn, mu_task_state_t, void *);
FAKE_VALUE_FUNC(mu_task_t *, mu_task_set_state, mu_task_t *, mu_task_state_t);


void setUp(void) {
    // Reset all faked functions
    RESET_FAKE(mu_sched_get_clock_source);
    RESET_FAKE(mu_time_offset);
    RESET_FAKE(mu_time_ms_to_rel);
    RESET_FAKE(mu_task_cancel);
    RESET_FAKE(mu_task_defer_until);
    RESET_FAKE(mu_task_transfer);
    RESET_FAKE(mu_task_get_state);
    RESET_FAKE(mu_task_init);
    RESET_FAKE(mu_task_set_state);
}

void tearDown(void) {
    // nothing yet
}

void test_mu_timer_init(void) {
    mu_timer_t timer;
    int user_info;

    TEST_ASSERT_EQUAL_PTR(&timer, mu_timer_init(&timer, &user_info));
    TEST_ASSERT_EQUAL_INT(1, mu_task_init_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&timer.task, mu_task_init_fake.arg0_val);
    // TEST_ASSERT_EQUAL_INT(MU_TIMER_STATE_IDLE, mu_task_init_fake.arg2_val);
    TEST_ASSERT_EQUAL_PTR(&user_info, mu_task_init_fake.arg3_val);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_timer_init);

    return UNITY_END();
}
