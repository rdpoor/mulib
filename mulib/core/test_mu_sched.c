// Unit tests for mu_sched


Please write unit tests for the mu_sched_get_current_task() function.  Assume the availability of an ASSERT macro:

#include <assert.h>

void test_mu_sched_init() {
  mu_sched_init();
  
  // Check that all queues are initialized correctly
  assert(s_sched.irq_tasks.buffer == s_irq_store);
  assert(s_sched.irq_tasks.max_items == MU_SCHED_MAX_IRQ_TASKS);
  assert(s_sched.irq_tasks.produce_index == 0);
  assert(s_sched.irq_tasks.consume_index == 0);
  
  assert(s_sched.now_tasks.buffer == s_now_store);
  assert(s_sched.now_tasks.max_items == MU_SCHED_MAX_NOW_TASKS);
  assert(s_sched.now_tasks.produce_index == 0);
  assert(s_sched.now_tasks.consume_index == 0);
  
  assert(s_sched.deferred_task_count == 0);
  assert(s_sched.curr_task == NULL);
  assert(s_sched.clock_fn == mu_time_now);
  assert(s_sched.idle_task == NULL);
}

int main() {
  test_mu_sched_init();
  return 0;
}

// Note that the peek_next_deferred_task() function is not provided, so it is
// assumed to be an internal function that is not directly testable. In this
// case, we can assume that mu_sched_reset() correctly resets the deferred task
// count and that fetch_runnable_deferred_task() properly handles an empty
// deferred task queue, since these are both functions that are called
// internally by mu_sched_step().

// Test that resetting the scheduler clears the deferred task count.
void test_mu_sched_reset_clears_deferred_task_count(void) {
  // Given a scheduler with a non-zero deferred task count
  s_sched.deferred_task_count = 3;
  
  // When mu_sched_reset() is called
  mu_sched_reset();
  
  // Then the deferred task count should be zero
  ASSERT(s_sched.deferred_task_count == 0);
}

// Test that resetting the scheduler clears the deferred task queue.
void test_mu_sched_reset_clears_deferred_task_queue(void) {
  // Given a scheduler with deferred tasks in the queue
  mu_sched_deferred_task_t task1 = {.task = NULL, .run_at = 0};
  mu_sched_deferred_task_t task2 = {.task = NULL, .run_at = 0};
  s_sched.deferred_task_count = 2;
  s_deferred_tasks[0] = task1;
  s_deferred_tasks[1] = task2;
  
  // When mu_sched_reset() is called
  mu_sched_reset();
  
  // Then the deferred task count should be zero
  ASSERT(s_sched.deferred_task_count == 0);
  // And all tasks in the deferred task queue should have been cleared
  ASSERT(peek_next_deferred_task() == NULL);
}

// Test that resetting the scheduler does not affect other scheduler state.
void test_mu_sched_reset_does_not_affect_other_state(void) {
  // Given a scheduler with non-default values for other state variables
  mu_spsc_put(&s_sched.irq_tasks, NULL);
  mu_mqueue_put(&s_sched.now_tasks, NULL);
  s_sched.curr_task = (mu_task_t*) 0xDEADBEEF;
  s_sched.clock_fn = NULL;
  s_sched.idle_task = (mu_task_t*) 0xBEEFDEAD;
  
  // When mu_sched_reset() is called
  mu_sched_reset();
  
  // Then the other state variables should not be affected
  ASSERT(mu_spsc_get(&s_sched.irq_tasks, NULL) == MU_SPSC_ERR_EMPTY);
  ASSERT(mu_mqueue_get(&s_sched.now_tasks, NULL) == false);
  ASSERT(s_sched.curr_task == NULL);
  ASSERT(s_sched.clock_fn != NULL);
  ASSERT(s_sched.idle_task == (mu_task_t*) 0xBEEFDEAD);
}

void test_mu_sched_step(void) {
  // Initialize the scheduler and create some tasks
  mu_sched_init();
  mu_task_t task1 = MU_TASK_INIT(1, 0, NULL);
  mu_task_t task2 = MU_TASK_INIT(2, 0, NULL);
  mu_task_t task3 = MU_TASK_INIT(3, 0, NULL);

  // Test case 1: Run a task from the "now" queue
  mu_sched_now(&task1);
  ASSERT(mu_sched_get_current_task() == NULL);
  mu_sched_step();
  ASSERT(mu_sched_get_current_task() == &task1);

  // Test case 2: Run a task from the deferred queue
  mu_sched_defer(&task2, 10);
  ASSERT(mu_sched_get_current_task() == NULL);
  mu_time_wait_ms(5);
  mu_sched_step();
  ASSERT(mu_sched_get_current_task() == NULL);
  mu_time_wait_ms(10);
  mu_sched_step();
  ASSERT(mu_sched_get_current_task() == &task2);

  // Test case 3: Run the idle task
  mu_sched_step();
  ASSERT(mu_sched_get_current_task() == mu_sched_get_idle_task());

  // Test case 4: Run a task from the IRQ task queue
  mu_sched_defer(&task3, 20);
  ASSERT(mu_sched_get_current_task() == NULL);
  mu_sched_from_isr(&task3);
  mu_sched_step();
  ASSERT(mu_sched_get_current_task() == &task3);
}



#include "mu_sched.h"
#include "mu_config.h"

// Mock task to test scheduling
mu_task_t g_task;

// Helper function to advance time by a specified duration
void advance_time(mu_time_rel_t duration) {
  mu_time_abs_t now = mu_sched_get_current_time();
  mu_sched_set_clock_source([=]() { return now + duration; });
}

void test_mu_sched_now_success() {
  mu_sched_init();
  mu_task_err_t err = mu_sched_now(&g_task);
  ASSERT(err == MU_TASK_ERR_NONE);
}

void test_mu_sched_now_full() {
  mu_sched_init();
  // Fill the "now" queue to capacity
  for (size_t i = 0; i < MU_SCHED_MAX_NOW_TASKS; ++i) {
    mu_task_t task;
    ASSERT(mu_sched_now(&task) == MU_TASK_ERR_NONE);
  }
  // Attempt to add one more task to the "now" queue
  mu_task_t task;
  ASSERT(mu_sched_now(&task) == MU_TASK_ERR_SCHED_FULL);
}

void test_mu_sched_now_calls_task() {
  mu_sched_init();
  // Schedule a task to run immediately
  ASSERT(mu_sched_now(&g_task) == MU_TASK_ERR_NONE);
  // Run the scheduler to execute the scheduled task
  mu_sched_step();
  // Check that the scheduled task was executed
  ASSERT(g_task.call_count == 1);
}

void test_mu_sched_now_multiple_tasks() {
  mu_sched_init();
  // Schedule multiple tasks to run immediately
  for (size_t i = 0; i < 10; ++i) {
    mu_task_t task;
    ASSERT(mu_sched_now(&task) == MU_TASK_ERR_NONE);
  }
  // Run the scheduler to execute the scheduled tasks
  for (size_t i = 0; i < 10; ++i) {
    mu_sched_step();
    // Check that the scheduled task was executed
    ASSERT(g_task.call_count == i + 1);
  }
}

void test_mu_sched_now_deferred_task() {
  mu_sched_init();
  // Schedule a task to run in the future
  mu_task_err_t err = mu_sched_defer_for(&g_task, 100);
  ASSERT(err == MU_TASK_ERR_NONE);
  // Check that the scheduled task has not been executed yet
  ASSERT(g_task.call_count == 0);
  // Advance time to when the task should run
  advance_time(100);
  // Run the scheduler to execute the scheduled task
  mu_sched_step();
  // Check that the scheduled task was executed
  ASSERT(g_task.call_count == 1);
}

void test_mu_sched_now_deferred_and_immediate_tasks() {
  mu_sched_init();
  // Schedule a task to run in the future
  mu_task_err_t err = mu_sched_defer_for(&g_task, 100);
  ASSERT(err == MU_TASK_ERR_NONE);
  // Schedule a task to run immediately
  mu_task_t immediate_task;
  err = mu_sched_now(&immediate_task);
  ASSERT(err == MU_TASK_ERR_NONE);
  // Check that the scheduled tasks have not been executed yet
  ASSERT(g_task.call_count == 0);
  ASSERT(immediate_task.call_count == 0);
  // Run the scheduler to execute the scheduled tasks
  mu_sched_step();
  // Check that the scheduled task was executed
  ASSERT(immediate_task.call_count == 1);
  // Check that the deferred task was not executed yet
  ASSERT(g_task.call_count == 0);
  // Advance time to when the deferred task should run
  // NOTE: incomplete
}

// *****************************************************************************
// Includes

#include "mu_sched.h"
#include "mu_test.h"

// *****************************************************************************
// Local (private) forward declarations

static void test_mu_sched_now_full(void);
static void test_mu_sched_now_empty(void);
static void test_mu_sched_now_single_task(void);
static void test_mu_sched_now_multiple_tasks(void);
static void test_mu_sched_now_task_queue_overflow(void);

// *****************************************************************************
// Test Runner

int main(void) {
    MU_RUN_TEST(test_mu_sched_now_full);
    MU_RUN_TEST(test_mu_sched_now_empty);
    MU_RUN_TEST(test_mu_sched_now_single_task);
    MU_RUN_TEST(test_mu_sched_now_multiple_tasks);
    MU_RUN_TEST(test_mu_sched_now_task_queue_overflow);

    MU_REPORT();
    return 0;
}

// *****************************************************************************
// Test cases

static void test_mu_sched_now_full(void) {
    mu_sched_init();
    mu_task_t task;

    for (size_t i = 0; i < MU_SCHED_MAX_NOW_TASKS; ++i) {
        ASSERT_EQ(mu_sched_now(&task), MU_TASK_ERR_NONE);
    }

    ASSERT_EQ(mu_sched_now(&task), MU_TASK_ERR_SCHED_FULL);
}

static void test_mu_sched_now_empty(void) {
    mu_sched_init();
    mu_task_t *initial_task = s_sched.curr_task;

    mu_sched_step();

    ASSERT_EQ(s_sched.curr_task, initial_task);
}

static void dummy_task_fn(void *p) {
    *((bool *)p) = true;
}

static void test_mu_sched_now_single_task(void) {
    mu_sched_init();
    bool task_executed = false;
    mu_task_t task = {.fn = dummy_task_fn, .p = &task_executed};

    ASSERT_EQ(mu_sched_now(&task), MU_TASK_ERR_NONE);
    mu_sched_step();
    ASSERT_TRUE(task_executed);
    ASSERT_NULL(s_sched.curr_task);
}

static void test_mu_sched_now_multiple_tasks(void) {
    mu_sched_init();
    bool task1_executed = false;
    bool task2_executed = false;
    mu_task_t task1 = {.fn = dummy_task_fn, .p = &task1_executed};
    mu_task_t task2 = {.fn = dummy_task_fn, .p = &task2_executed};

    ASSERT_EQ(mu_sched_now(&task1), MU_TASK_ERR_NONE);
    ASSERT_EQ(mu_sched_now(&task2), MU_TASK_ERR_NONE);

    mu_sched_step();
    ASSERT_TRUE(task1_executed);
    ASSERT_FALSE(task2_executed);
    ASSERT_NULL(s_sched.curr_task);

    mu_sched_step();
    ASSERT_TRUE(task2_executed);
    ASSERT_NULL(s_sched.curr_task);
}

static void test_mu_sched_now_task_queue_overflow(void) {
    mu_sched_init();
    mu_task_t task;

    for (size_t i = 0; i < MU_SCHED_MAX_NOW_TASKS + 1; ++i) {
        if (i < MU_SCHED_MAX_NOW_TASKS) {
            ASSERT_EQ(mu_sched_now(&task), MU_TASK_ERR_NONE);
        } else {
            ASSERT_EQ(mu_sched_now(&task), MU_TASK_ERR_SCHED_FULL);
        }
    }
}

// *****************************************************************************
// Test code

#include "test_utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Test functions forward declarations
static void test_mu_sched_from_isr_normal_usage(void);
static void test_mu_sched_from_isr_full_queue(void);

void run_mu_sched_from_isr_tests(void) {
RUN_TEST(test_mu_sched_from_isr_normal_usage);
RUN_TEST(test_mu_sched_from_isr_full_queue);
}

// *****************************************************************************
// Test functions

static void test_mu_sched_from_isr_normal_usage(void) {
mu_sched_init();
mu_task_t task1 = {.fn = NULL, .arg = NULL};
mu_task_t task2 = {.fn = NULL, .arg = NULL};

ASSERT(mu_sched_from_isr(&task1) == MU_TASK_ERR_NONE);
ASSERT(mu_sched_from_isr(&task2) == MU_TASK_ERR_NONE);

mu_task_t *current_task;
mu_spsc_item_t item = (mu_spsc_item_t)(&current_task);

ASSERT(mu_spsc_get(&s_sched.irq_tasks, item) == MU_SPSC_ERR_NONE);
ASSERT(current_task == &task1);

ASSERT(mu_spsc_get(&s_sched.irq_tasks, item) == MU_SPSC_ERR_NONE);
ASSERT(current_task == &task2);

ASSERT(mu_spsc_get(&s_sched.irq_tasks, item) == MU_SPSC_ERR_EMPTY);
}

static void test_mu_sched_from_isr_full_queue(void) {
mu_sched_init();
mu_task_t tasks[MU_SCHED_MAX_IRQ_TASKS + 1];

for (size_t i = 0; i < MU_SCHED_MAX_IRQ_TASKS; ++i) {
tasks[i] = (mu_task_t){.fn = NULL, .arg = NULL};
ASSERT(mu_sched_from_isr(&tasks[i]) == MU_TASK_ERR_NONE);
}

tasks[MU_SCHED_MAX_IRQ_TASKS] = (mu_task_t){.fn = NULL, .arg = NULL};
ASSERT(mu_sched_from_isr(&tasks[MU_SCHED_MAX_IRQ_TASKS]) == MU_TASK_ERR_SCHED_FULL);
}






#include "mu_sched.h"
#include "mu_test.h"

static mu_task_t task1;
static mu_task_t task2;
static mu_task_t task3;

static void task_fn_noop(void *arg) {}

// Test the sched_aux() function when the deferred task queue is empty
MU_TEST(test_sched_aux_empty) {
    mu_sched_init();
    mu_sched_reset();

    mu_task_err_t err = sched_aux(&task1, 10);
    ASSERT_EQ(MU_TASK_ERR_NONE, err);
    ASSERT_EQ(1, s_sched.deferred_task_count);
    ASSERT_EQ(task1, s_deferred_tasks[0].task);
    ASSERT_EQ(10, s_deferred_tasks[0].at);
}

// Test the sched_aux() function when adding a task to the beginning of the deferred task queue
MU_TEST(test_sched_aux_insert_beginning) {
    mu_sched_init();
    mu_sched_reset();

    sched_aux(&task1, 20);
    mu_task_err_t err = sched_aux(&task2, 10);
    ASSERT_EQ(MU_TASK_ERR_NONE, err);
    ASSERT_EQ(2, s_sched.deferred_task_count);
    ASSERT_EQ(task2, s_deferred_tasks[0].task);
    ASSERT_EQ(10, s_deferred_tasks[0].at);
    ASSERT_EQ(task1, s_deferred_tasks[1].task);
    ASSERT_EQ(20, s_deferred_tasks[1].at);
}

// Test the sched_aux() function when adding a task to the end of the deferred task queue
MU_TEST(test_sched_aux_insert_end) {
    mu_sched_init();
    mu_sched_reset();

    sched_aux(&task1, 10);
    mu_task_err_t err = sched_aux(&task2, 20);
    ASSERT_EQ(MU_TASK_ERR_NONE, err);
    ASSERT_EQ(2, s_sched.deferred_task_count);
    ASSERT_EQ(task1, s_deferred_tasks[0].task);
    ASSERT_EQ(10, s_deferred_tasks[0].at);
    ASSERT_EQ(task2, s_deferred_tasks[1].task);
    ASSERT_EQ(20, s_deferred_tasks[1].at);
}

// Test the sched_aux() function when adding a task in the middle of the deferred task queue
MU_TEST(test_sched_aux_insert_middle) {
    mu_sched_init();
    mu_sched_reset();

    sched_aux(&task1, 10);
    sched_aux(&task2, 30);
    mu_task_err_t err = sched_aux(&task3, 20);
    ASSERT_EQ(MU_TASK_ERR_NONE, err);
    ASSERT_EQ(3, s_sched.deferred_task_count);
    ASSERT_EQ(task1, s_deferred_tasks[0].task);
    ASSERT_EQ(10, s_deferred_tasks[0].at);
    ASSERT_EQ(task3, s_deferred_tasks[1].task);
    ASSERT_EQ(20, s_deferred_tasks[1].at);
    ASSERT_EQ(task2, s_deferred_tasks[2].task);
    ASSERT_EQ(30, s_deferred_tasks[2].at);
}

// Test the sched_aux() function when the deferred task queue is full
MU_TEST(test_sched_aux_full) {
    mu_sched_init();
    mu_sched_reset();

    for (size_t i = 0; i < MU_SCHED_MAX_DEFERRED_TASKS; i++) {
        mu_task_t task;
        sched_aux(&task, i);
    }
    mu_task_t new_task;
    mu_task_err_t err = sched_aux(&new_task, 50);
    ASSERT_EQ(MU_TASK_ERR_SCHED_FULL, err);
}


