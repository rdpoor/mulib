/**
 * @file mu_sched.c
 *
 * MIT License
 *
 * Copyright (c) 2022 R. Dunbar Poor
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

// *****************************************************************************
// Includes

#include "mu_sched.h"

#include "mu_config.h"
#include "mu_pqueue.h"
#include "mu_spsc.h"
#include "mu_task.h"
#include "mu_time.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// #include <stdio.h> // debugging

// *****************************************************************************
// Local (private) types and definitions

typedef struct {
    mu_spsc_t irq_tasks;   // tasks queued from interrupt level.
    mu_pqueue_t now_tasks; // a queue of tasks to be run "now"
    size_t
        deferred_task_count; // index of next available slot in deferred_tasks
    mu_task_t *curr_task;    // the task currently being processed.
    mu_clock_fn clock_fn;    // the function to call to get the current time.
    mu_task_t *idle_task;    // the task to run when nothing else is runnable.
} mu_sched_t;

// *****************************************************************************
// Local (private, static) forward declarations

/**
 * @brief Fetch the next runnable task, if any, from the deferred task queue.
 */
static mu_task_t *fetch_runnable_deferred_task(void);

/**
 * @brief Schedule the given task at the given time.
 */
static mu_sched_err_t sched_aux(mu_task_t *task, mu_time_abs_t at);

// *****************************************************************************
// Local (private, static) storage

static void *s_irq_store[MU_SCHED_MAX_IRQ_TASKS];
static void *s_now_store[MU_SCHED_MAX_NOW_TASKS];
static mu_sched_deferred_task_t s_deferred_tasks[MU_SCHED_MAX_DEFERRED_TASKS];
static mu_sched_t s_sched;

// *****************************************************************************
// Public code

void mu_sched_init(void) {
    mu_spsc_init(&s_sched.irq_tasks, s_irq_store, MU_SCHED_MAX_IRQ_TASKS);
    mu_pqueue_init(&s_sched.now_tasks, s_now_store, MU_SCHED_MAX_NOW_TASKS);
    s_sched.deferred_task_count = 0;
    s_sched.curr_task = NULL;
    s_sched.clock_fn = mu_time_now;
    s_sched.idle_task = NULL;
}

void mu_sched_reset(void) { s_sched.deferred_task_count = 0; }

mu_sched_err_t mu_sched_step(void) {
    mu_spsc_item_t item = (mu_spsc_item_t)(&s_sched.curr_task);

    if (mu_spsc_get(&s_sched.irq_tasks, item) == MU_SPSC_ERR_NONE) {
        // pulled one task from the irq task queue
        asm("nop");

    } else if ((s_sched.curr_task = mu_pqueue_get(&s_sched.now_tasks)) !=
               NULL) {
        // pulled one task from the "now" task queue
        asm("nop");

    } else if ((s_sched.curr_task = fetch_runnable_deferred_task()) != NULL) {
        // pulled one runnable task from the deferred task queue
        asm("nop");

    } else {
        // no runnable tasks available -- use the idle task (may be NULL)
        s_sched.curr_task = s_sched.idle_task;
    }

    // invoke the task.
    mu_task_call(s_sched.curr_task, NULL);
    s_sched.curr_task = NULL;

    return MU_SCHED_ERR_NONE;
}

mu_clock_fn mu_sched_get_clock_source(void) { return s_sched.clock_fn; }

void mu_sched_set_clock_source(mu_clock_fn clock_fn) {
    s_sched.clock_fn = clock_fn;
}

mu_time_abs_t mu_sched_get_current_time(void) { return s_sched.clock_fn(); }

mu_task_t *mu_sched_get_idle_task(void) { return s_sched.idle_task; }

void mu_sched_set_idle_task(mu_task_t *task) { s_sched.idle_task = task; }

mu_task_t *mu_sched_get_current_task(void) { return s_sched.curr_task; }

mu_sched_deferred_task_t *mu_sched_peek_next_deferred_task(void) {
    mu_sched_deferred_task_t *deferred_task = NULL;
    if (s_sched.deferred_task_count > 0) {
        deferred_task = &s_deferred_tasks[s_sched.deferred_task_count - 1];
    }
    return deferred_task;
}

mu_sched_err_t mu_sched_remove_deferred_task(mu_task_t *task) {
    mu_sched_err_t err = MU_SCHED_ERR_NOT_FOUND;

    size_t i = s_sched.deferred_task_count;
    while (i > 0) {
        mu_sched_deferred_task_t *deferred_task = &s_deferred_tasks[i - 1];
        if (deferred_task->task == task) {
            // use memmove to close slot at i-1
            size_t to_move = s_sched.deferred_task_count - i;
            if (to_move > 0) {
                mu_sched_deferred_task_t *src = &s_deferred_tasks[i];
                memmove(deferred_task, src,
                        to_move * sizeof(mu_sched_deferred_task_t));
            }
            s_sched.deferred_task_count -= 1;
            err = MU_SCHED_ERR_NONE;
        }
        i -= 1;
    }
    return err;
}

mu_sched_err_t mu_sched_now(mu_task_t *task) {
    // push task onto the "now" queue
    if (mu_pqueue_put(&s_sched.now_tasks, task) == NULL) {
        return MU_SCHED_ERR_FULL;
    } else {
        return MU_SCHED_ERR_NONE;
    }
}

mu_sched_err_t mu_sched_from_isr(mu_task_t *task) {
    if (mu_spsc_put(&s_sched.irq_tasks, task) == MU_SPSC_ERR_FULL) {
        return MU_SCHED_ERR_FULL;
    } else {
        return MU_SCHED_ERR_NONE;
    }
}

mu_sched_err_t mu_sched_defer_until(mu_task_t *task, mu_time_abs_t at) {
    return sched_aux(task, at);
}

mu_sched_err_t mu_sched_defer_for(mu_task_t *task, mu_time_rel_t in) {
    mu_time_abs_t at = mu_time_offset(mu_sched_get_current_time(), in);
    return sched_aux(task, at);
}

void mu_sched_yield(mu_task_t *task, unsigned int state) {
    mu_task_set_state(task, state);
    mu_sched_now(task);
}

void mu_sched_deferred_yield(mu_task_t *task, unsigned int state,
                             mu_time_rel_t tics) {
    mu_task_set_state(task, state);
    mu_sched_defer_for(task, tics);
}

void mu_sched_transfer(mu_task_t *from, unsigned int state, mu_task_t *to) {
    mu_task_set_state(from, state);
    mu_sched_now(to);
}

void mu_sched_deferred_transfer(mu_task_t *task, unsigned int state,
                                mu_task_t *to, mu_time_rel_t tics) {
    mu_task_set_state(from, state);
    mu_sched_defer_for(to, tics);
}

// void *mu_sched_visit_deferred_deferred_tasks(
//     mu_sched_visit_deferred_task_fn user_fn,
//     void *arg) {
//   size_t i = s_sched.deferred_task_count;
//
//   while (i > 0) {
//     mu_sched_deferred_task_t *deferred_task = &s_deferred_tasks[i - 1];
//     void *r = user_fn(deferred_task, arg);
//     if (r) {
//       return r;
//     }
//   }
//   return NULL;
// }
//
// void *mu_sched_visit_immediate_tasks(mu_sched_visit_task_fn user_fn,
//                                      void *arg) {
//   // stub for now
//   return NULL;
// }

// *****************************************************************************
// Local (private, static) code

static mu_task_t *fetch_runnable_deferred_task(void) {
    mu_sched_deferred_task_t *deferred_task;
    mu_time_abs_t now = mu_sched_get_current_time();

    deferred_task = mu_sched_peek_next_deferred_task();
    if (deferred_task && !mu_time_follows(deferred_task->at, now)) {
        // A deferred_task's time has arrived.
        // NOTE: normally it would be an error to decrement the
        // deferred_task_count before the task is consumed, but this is
        // operating in a single-threaded environment, so this is safe.
        s_sched.deferred_task_count -= 1;
        return deferred_task->task;
    } else {
        return NULL;
    }
}

static mu_sched_err_t sched_aux(mu_task_t *task, mu_time_abs_t at) {
    mu_sched_deferred_task_t *deferred_task;

    if (s_sched.deferred_task_count == MU_SCHED_MAX_DEFERRED_TASKS) {
        return MU_SCHED_ERR_FULL;
    }

    // perform a linear search to find the insertion point
    size_t i = s_sched.deferred_task_count;
    while (i > 0) {
        deferred_task = &s_deferred_tasks[i - 1];
        // Strict ordering: if a task already is scheduled for 'at', schedule
        // this new deferred_task to follow it.
        if (mu_time_follows(deferred_task->at, at)) {
            break;
        }
        i -= 1;
    }

    // Here, i is the index for the new deferred_task.  If needed, open a slot
    // at i.
    int to_move = s_sched.deferred_task_count - i;
    if (to_move > 0) {
        mu_sched_deferred_task_t *src = &s_deferred_tasks[i];
        mu_sched_deferred_task_t *dst = &s_deferred_tasks[i + 1];
        memmove(dst, src, to_move * sizeof(mu_sched_deferred_task_t));
    }

    // Write the time and task into the deferred_task, bump the deferred_task
    // count.
    deferred_task = &s_deferred_tasks[i];
    deferred_task->at = at;
    deferred_task->task = task;
    s_sched.deferred_task_count += 1;
    return MU_SCHED_ERR_NONE;
}
