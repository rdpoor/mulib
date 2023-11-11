/**
 * @file mu_timer.c
 *
 * MIT License
 *
 * Copyright (c) 2021-2022 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_timer.h"
#include "mu_sched.h"
#include "mu_task.h"
#include "mu_time.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

typedef enum {
    MU_TIMER_STATE_IDLE,
    MU_TIMER_STATE_RUNNING,
    MU_TIMER_STATE_EXPIRED
} mu_timeer_state_t;

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

static inline mu_task_t *timer_task(mu_timer_t *timer) { return &timer->task; }

static inline mu_task_state_t timer_state(mu_timer_t *timer) {
    return mu_task_get_state(timer_task(timer));
}

/**
 * @brief Common entry point for starting the timer.
 */
static void start_aux(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                      MU_SCHED_REL_TIME period_tics, mu_task_t *on_expiration,
                      bool is_periodic);

/**
 * @brief The primary state machine for the timer
 */
static void timer_fn(mu_task_t *task);

// *****************************************************************************
// Public code

mu_timer_t *mu_timer_init(mu_timer_t *timer, void *user_info) {
    mu_task_init(&timer->task, timer_fn, MU_TIMER_STATE_IDLE, user_info);
    return timer;
}

void mu_timer_one_shot(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                       mu_task_t *on_expiration) {
    start_aux(timer, expire_in_tics, (MU_SCHED_REL_TIME)0, on_expiration,
              false);
}

void mu_timer_one_shot_ms(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_ms,
                          mu_task_t *on_expiration) {
    start_aux(timer, mu_time_ms_to_rel(expire_in_ms), (MU_SCHED_REL_TIME)0,
              on_expiration, false);
}

void mu_timer_periodic(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                       MU_SCHED_REL_TIME period_tics,
                       mu_task_t *on_expiration) {
    start_aux(timer, expire_in_tics, period_tics, on_expiration, true);
}

void mu_timer_periodic_ms(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_ms,
                          MU_SCHED_REL_TIME period_ms,
                          mu_task_t *on_expiration) {
    start_aux(timer, mu_time_ms_to_rel(expire_in_ms),
              mu_time_ms_to_rel(period_ms), on_expiration, true);
}

void mu_timer_stop(mu_timer_t *timer) {
    mu_task_t *task = timer_task(timer);

    if (mu_task_get_state(task) == MU_TIMER_STATE_RUNNING) {
        // cancel any exiting timer
        mu_task_cancel(&timer->task);
    }
    mu_task_set_state(task, MU_TIMER_STATE_IDLE);
}

bool mu_timer_is_idle(mu_timer_t *timer) {
    return timer_state(timer) == MU_TIMER_STATE_IDLE;
}

bool mu_timer_is_running(mu_timer_t *timer) {
    return timer_state(timer) == MU_TIMER_STATE_RUNNING;
}

bool mu_timer_is_stopped(mu_timer_t *timer) {
    return timer_state(timer) != MU_TIMER_STATE_RUNNING;
}

bool mu_timer_is_expired(mu_timer_t *timer) {
    return timer_state(timer) == MU_TIMER_STATE_EXPIRED;
}

bool mu_timer_check(mu_timer_t *timer) {
    if (mu_timer_is_expired(timer)) {
        return true;
    } else {
        mu_timer_stop(timer);
        return false;
    }
}

// *****************************************************************************
// Private (static) code

static void start_aux(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                      MU_SCHED_REL_TIME period_tics, mu_task_t *on_expiration,
                      bool is_periodic) {
    mu_timer_stop(timer);

    MU_SCHED_ABS_TIME now = mu_time_now();
    timer->expire_at = mu_time_offset(now, expire_in_tics);
    timer->period = period_tics;
    timer->on_expiration = on_expiration;
    timer->is_periodic = is_periodic;
    mu_task_set_state(timer_task(timer), MU_TIMER_STATE_RUNNING);
    mu_task_defer_until(&timer->task, timer->expire_at);
}

static void timer_fn(mu_task_t *task) {
    mu_timer_t *self = MU_TASK_CTX(task, mu_timer_t, task);

    switch (mu_task_get_state(task)) {

    case MU_TIMER_STATE_IDLE: {
        // wait here for a call to mu_timer_start_xxx();
    } break;

    case MU_TIMER_STATE_RUNNING: {
        // arrive here when timer has expired
        if (self->is_periodic) {
            // Schedule next wakeup (and prevent time slippage...)
            self->expire_at = mu_time_offset(self->expire_at, self->period);
            mu_task_defer_until(task, self->expire_at);
        } else {
            // Stop timer.
            mu_task_set_state(task, MU_TIMER_STATE_EXPIRED);
        }
        // Invoke on-expiration task.
        mu_task_transfer(self->on_expiration);
    } break;

    case MU_TIMER_STATE_EXPIRED: {
        asm("nop");
    } break;

    } // switch
}

// *****************************************************************************
// Standalone tests

// Run this command to run the standalone tests.
/**

gcc -g -Wall -DMU_TIMER_STANDALONE_TESTS -I../posix_platform -o
mu_timer_test \ mu_timer.c mu_sched.c mu_spsc.c mu_task.c
../posix_platform/mu_time.c && \
  ./mu_timer_test && rm -rf ./mu_timer_test ./mu_timer_test.dSYM

 */

#ifdef MU_TIMER_STANDALONE_TESTS

#include <stdio.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)

static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

int s_called_count;

// The timer under test.
static mu_timer_t s_timer;

// A task that simply prints out the current time
static mu_task_t s_dummy_task;

static mu_time_abs_t get_now(void) { return mu_time_now(); }

static void dummy_task_fn(void *ctx, void *arg) {
    printf("\ntimer call count = %d, called at %ld", s_called_count, get_now());
    if (s_called_count > 5) {
        printf("\nstopping.");
        mu_timer_stop(&s_timer);
    } else {
        s_called_count += 1;
    }
}

static void setup(void) {
    mu_sched_init();
    mu_timer_init(&s_timer);
    s_called_count = 0;
    mu_task_init(&s_dummy_task, dummy_task_fn, NULL, "Dummy Task");
}

int main(void) {
    printf("\nStarting mu_timer tests...");
    ASSERT(1 == 1);

    setup();
    mu_timer_start(&s_timer, 500, true, &s_dummy_task);
    while (s_called_count < 5) {
        mu_sched_step();
    }
    // pretty weak assertion...
    ASSERT(s_called_count == 5);

    setup();
    mu_timer_start(&s_timer, 500, false, &s_dummy_task);
    while (s_called_count < 1) {
        mu_sched_step();
    }
    // pretty weak assertion...
    ASSERT(s_called_count == 1);

    printf("\n...tests complete\n");
    return 0;
}

#endif // #ifdef MU_TIMER_STANDALONE_TESTS

// *****************************************************************************
// End of file
