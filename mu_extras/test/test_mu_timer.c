/**
 * @file test_mu_timer.c
 *
 * MIT License
 *
 * Copyright (c) 2022 - 2023 R. Dunbar Poor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "../src/mu_timer.h"
#include "../src/mu_sched.h"
#include "../src/mu_task.h"
#include "../src/mu_time.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(MU_SCHED_ABS_TIME, mu_time_offset, MU_SCHED_ABS_TIME, MU_SCHED_REL_TIME);
FAKE_VALUE_FUNC(MU_SCHED_REL_TIME, mu_time_ms_to_rel, int);
FAKE_VALUE_FUNC(mu_task_err_t, mu_task_cancel, mu_task_t *);
FAKE_VALUE_FUNC(mu_task_err_t, mu_task_defer_until, mu_task_t *, MU_SCHED_ABS_TIME);
FAKE_VALUE_FUNC(mu_task_err_t, mu_task_transfer, mu_task_t *);
FAKE_VALUE_FUNC(mu_task_state_t, mu_task_get_state, mu_task_t *);
FAKE_VALUE_FUNC(mu_task_t *, mu_task_init, mu_task_t *, mu_task_fn, mu_task_state_t, void *);
FAKE_VALUE_FUNC(mu_task_t *, mu_task_set_state, mu_task_t *, mu_task_state_t);

static MU_SCHED_ABS_TIME s_time;

// static void set_test_time(MU_SCHED_ABS_TIME time);

void setUp(void) {
    // Reset all faked functions
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

// ******************************************************************
// We could fake all the time functions used by mu_sched, but it's
// just as easy to provide basic implementations here.

MU_SCHED_ABS_TIME mu_time_now(void) {
    return s_time;
}

// MU_SCHED_ABS_TIME mu_time_offset(MU_SCHED_ABS_TIME t, MU_SCHED_REL_TIME dt) {
//   return t + dt;
// }

// MU_SCHED_REL_TIME mu_time_difference(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2) {
//   return t1 - t2;
// }

// bool mu_time_precedes(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2) {
//   return mu_time_difference(t1, t2) < 0;
// }

// bool mu_time_follows(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2) {
//   return mu_time_difference(t1, t2) > 0;
// }

