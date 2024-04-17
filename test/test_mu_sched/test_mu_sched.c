/**
 * @file test_mu_sched.c
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

#include "../mulib/mu_mqueue.h"
#include "../mulib/mu_sched.h"
#include "../mulib/mu_spsc.h"
#include "../mulib/mu_task.h"
#include "fff.h"
#include "mu_platform.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

// *****************************************************************************
// Includes

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) storage

FAKE_VOID_FUNC(idle_task_fn, mu_task_t *);
FAKE_VOID_FUNC(task1_fn, mu_task_t *);
FAKE_VOID_FUNC(task2_fn, mu_task_t *);
FAKE_VOID_FUNC(task3_fn, mu_task_t *);
// Since we compile and link the original mu_task.c (for mu_task_init() and
// mu_task_call()), any attempt to fake other mu_task functions result in a
// duplicate symbol error from the linker.  Not sure the best way to test for
// calling mu_task_call_transfer_hook(), so that test gets skipped.
// FAKE_VOID_FUNC(mu_task_call_transfer_hook, mu_task_t *);

static mu_task_t s_idle_task;
static mu_task_t s_task1;
static mu_task_t s_task2;
static mu_task_t s_task3;
static mu_task_t s_basic_task;
static MU_TIME_ABS s_time;

// *****************************************************************************
// Local (private, static) forward declarations

static void set_test_time(MU_TIME_ABS time);
static void basic_task_fn(mu_task_t *task);

// *****************************************************************************
// Public code

void idle_task_runs_if_sched_is_empty(void) {
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
}

void idle_task_is_accessible(void) {
    TEST_ASSERT_EQUAL_PTR(&s_idle_task, mu_sched_get_idle_task());
}

void priorities_favor_isr_over_deferred_over_immed(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task1));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_from_isr(&s_task2));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_defer_for(&s_task3, 0));
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task3_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count); // isr runs first
    TEST_ASSERT_EQUAL_INT(0, task3_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count); // isr runs first
    TEST_ASSERT_EQUAL_INT(1, task3_fn_fake.call_count); // deferred second
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count); // immed third
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count); // isr runs first
    TEST_ASSERT_EQUAL_INT(1, task3_fn_fake.call_count); // deferred second
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);     // immed third
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);     // isr runs first
    TEST_ASSERT_EQUAL_INT(1, task3_fn_fake.call_count);     // deferred second
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count); // idle last
}

void reset_empties_all_queues(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task1));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_from_isr(&s_task2));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_defer_for(&s_task3, 0));
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task3_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_reset();
    mu_sched_step();
    // no tasks run...
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task3_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
}

void fill_to_overflowing_from_isr(void) {
    // irq queue only accepts MU_SCHED_MAX_IRQ_TASKS-1 tasks
    for (int i = 0; i < MU_SCHED_MAX_IRQ_TASKS - 1; i++) {
        TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_from_isr(&s_task1));
    }
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_FULL, mu_sched_from_isr(&s_task1));
}

void fill_to_overflowing_immed(void) {
    for (int i = 0; i < MU_SCHED_MAX_IMMED_TASKS; i++) {
        TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task1));
    }
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_FULL, mu_sched_immed(&s_task1));
}

void fill_to_overflowing_deferred(void) {
    for (int i = 0; i < MU_SCHED_MAX_DEFERRED_TASKS; i++) {
        TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE,
                              mu_sched_defer_for(&s_task1, 1));
    }
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_FULL, mu_sched_defer_for(&s_task1, 1));
}

void immediate_task_runs_first_then_idle_task(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task1));
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    mu_sched_step(); // task1 is available to run
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    mu_sched_step(); // task1 already ran - run idle task instead
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
}

void mu_sched_immediate_preserves_order(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task1));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task2));
    mu_sched_step(); // task1 runs first
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    mu_sched_step(); // followed by task2
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
    mu_sched_step(); // followed by idle task
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
}

void mu_sched_from_isr_preserves_order(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_from_isr(&s_task1));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_from_isr(&s_task2));
    mu_sched_step(); // task1 runs first
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    mu_sched_step(); // followed by task2
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
    mu_sched_step(); // followed by idle task
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
}

void isr_tasks_run_before_regular_tasks(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_immed(&s_task1));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_from_isr(&s_task2));
    mu_sched_step(); // task2 runs first
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
    mu_sched_step(); // followed by task1
    TEST_ASSERT_EQUAL_INT(0, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
    mu_sched_step(); // followed by idle task
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
}

void mu_sched_wait_returns_no_error(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_wait(&s_task1));
}

void mu_sched_transfer_calls_transfer_hook(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_transfer(&s_task1));
    // See comment with FAKE_VOID_FUNC(mu_task_call_transfer_hook) above.
    // TEST_ASSERT_EQUAL_INT(1, mu_task_call_transfer_hook_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
}

void defer_until_tasks_run_when_their_times_arrive(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE,
                          mu_sched_defer_until(&s_task1, 10));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE,
                          mu_sched_defer_until(&s_task2, 12));
    set_test_time(8);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    set_test_time(10);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    set_test_time(12);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
}

void defer_for_tasks_run_when_their_times_arrive(void) {
    set_test_time(8);
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_defer_for(&s_task1, 2));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE, mu_sched_defer_for(&s_task2, 4));
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    set_test_time(10);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    set_test_time(12);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task2_fn_fake.call_count);
}

void deferred_tasks_can_be_removed(void) {
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE,
                          mu_sched_defer_until(&s_task1, 10));
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE,
                          mu_sched_defer_until(&s_task2, 12));
    set_test_time(8);
    // first remove_deferred_task removes the task
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NONE,
                          mu_sched_remove_deferred_task(&s_task2));
    // second call fails (since it's been removed)
    TEST_ASSERT_EQUAL_INT(MU_SCHED_ERR_NOT_FOUND,
                          mu_sched_remove_deferred_task(&s_task2));
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    // task1 will not get called because it's been removed.  idle get called.
    set_test_time(10);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(1, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
    set_test_time(12);
    mu_sched_step();
    TEST_ASSERT_EQUAL_INT(2, idle_task_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(1, task1_fn_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, task2_fn_fake.call_count);
}

void current_task_is_bound_inside_mu_sched_step(void) {
    mu_sched_immed(&s_basic_task);
    // verify that mu_sched_current_task() == &s_basic_task
    // see body of basic_task_fn
    mu_sched_step();
    // check that mu_sched_current_task() is null outside of a step() call
    TEST_ASSERT_EQUAL_PTR(NULL, mu_sched_current_task());
}

void next_deferred_task_is_accessible(void) {
    TEST_ASSERT_EQUAL_PTR(NULL, mu_sched_next_task());
    mu_sched_defer_for(&s_task1, 1);
    TEST_ASSERT_EQUAL_PTR(&s_task1, mu_sched_next_task());
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(idle_task_runs_if_sched_is_empty);
    RUN_TEST(idle_task_is_accessible);
    RUN_TEST(priorities_favor_isr_over_deferred_over_immed);
    RUN_TEST(reset_empties_all_queues);
    RUN_TEST(fill_to_overflowing_from_isr);
    RUN_TEST(fill_to_overflowing_immed);
    RUN_TEST(fill_to_overflowing_deferred);
    RUN_TEST(immediate_task_runs_first_then_idle_task);
    RUN_TEST(mu_sched_immediate_preserves_order);
    RUN_TEST(mu_sched_from_isr_preserves_order);
    RUN_TEST(isr_tasks_run_before_regular_tasks);
    RUN_TEST(mu_sched_wait_returns_no_error);
    RUN_TEST(defer_until_tasks_run_when_their_times_arrive);
    RUN_TEST(defer_for_tasks_run_when_their_times_arrive);
    RUN_TEST(deferred_tasks_can_be_removed);
    RUN_TEST(current_task_is_bound_inside_mu_sched_step);
    RUN_TEST(next_deferred_task_is_accessible);

    return UNITY_END();
}

// *****************************************************************************
// Local (private, static) code

void setUp(void) {

    RESET_FAKE(idle_task_fn);
    RESET_FAKE(task1_fn);
    RESET_FAKE(task2_fn);
    RESET_FAKE(task3_fn);
    // RESET_FAKE(mu_task_call_transfer_hook);

    mu_sched_init();
    mu_task_init(&s_idle_task, idle_task_fn, 0, NULL);
    mu_task_init(&s_task1, task1_fn, 0, NULL);
    mu_task_init(&s_task2, task2_fn, 0, NULL);
    mu_task_init(&s_task3, task3_fn, 0, NULL);
    mu_task_init(&s_basic_task, basic_task_fn, (mu_task_state_t)9, NULL);

    mu_sched_set_idle_task(&s_idle_task);
    set_test_time(0);
}

void tearDown(void) {}

static void set_test_time(MU_TIME_ABS time) { s_time = time; }

static void basic_task_fn(mu_task_t *task) {
    TEST_ASSERT_EQUAL_PTR(task, mu_sched_current_task());
}

// ******************************************************************
// We could fake all the time functions used by mu_sched, but it's
// just as easy to provide basic implementations here.

MU_TIME_ABS mu_time_now(void) { return s_time; }

MU_TIME_ABS mu_time_offset(MU_TIME_ABS t, MU_TIME_REL dt) { return t + dt; }

MU_TIME_REL mu_time_difference(MU_TIME_ABS t1, MU_TIME_ABS t2) {
    return t1 - t2;
}

bool mu_time_precedes(MU_TIME_ABS t1, MU_TIME_ABS t2) {
    return mu_time_difference(t1, t2) < 0;
}
