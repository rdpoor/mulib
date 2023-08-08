/**
 * @file mu_timeout.c
 *
 * MIT License
 *
 * Copyright (c) 2020-2023 R. D. Poor <rdpoor@gmail.com>
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
 */

// *****************************************************************************
// Includes

#include "mu_timeout.h"

#include "mulib/core/mu_sched.h"
#include "mulib/core/mu_task.h"
#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

typedef enum {
    MU_TIMEOUT_STATE_IDLE,
    MU_TIMEOUT_STATE_ARMED,
    MU_TIMEOUT_STATE_EXPIRED
} mu_timeout_state_t;

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Return a reference to mu_timeout's task.
 */
static inline mu_task_t *mu_timeout_task(mu_timeout_t *mu_timeout) {
    return &mu_timeout->task;
}

/**
 * @brief The primary state machine for the retry timer task.
 */
static void mu_timeout_fn(mu_task_t *task, void *arg);

// *****************************************************************************
// Public code

void mu_timeout_init(mu_timeout_t *mu_timeout, const char *timeout_name) {
    mu_task_init(mu_timeout_task(mu_timeout), mu_timeout_fn,
                 MU_TIMEOUT_STATE_IDLE, NULL);
}

bool mu_timeout_is_idle(mu_timeout_t *mu_timeout) {
    return mu_task_get_state(mu_timeout_task(mu_timeout)) ==
           MU_TIMEOUT_STATE_IDLE;
}

bool mu_timeout_is_armed(mu_timeout_t *mu_timeout) {
    return mu_task_get_state(mu_timeout_task(mu_timeout)) ==
           MU_TIMEOUT_STATE_ARMED;
}

bool mu_timeout_is_expired(mu_timeout_t *mu_timeout) {
    return mu_task_get_state(mu_timeout_task(mu_timeout)) ==
           MU_TIMEOUT_STATE_EXPIRED;
}

void mu_timeout_arm(mu_timeout_t *mu_timeout, uint16_t ms,
                     mu_task_t *on_timeout) {
    mu_timeout_cancel(mu_timeout);
    if (ms != 0) {
        mu_timeout->on_timeout = on_timeout;
        mu_time_rel_t tics = mu_time_ms_to_rel(ms);
        mu_task_defer_for(mu_timeout_task(mu_timeout),
                          MU_TIMEOUT_STATE_ARMED, tics);
    }
}

void mu_timeout_cancel(mu_timeout_t *mu_timeout) {
    // Remove timeout task from scheduler, revert to idle state.
    if (mu_timeout_is_armed(mu_timeout)) {
        mu_task_remove_deferred_task(mu_timeout_task(mu_timeout));
        mu_timeout->on_timeout = NULL;  // prevent false triggers
        mu_task_wait(mu_timeout_task(mu_timeout), MU_TIMEOUT_STATE_IDLE);
    }
}

bool mu_timeout_check(mu_timeout_t *mu_timeout) {
    if (mu_timeout_is_expired(mu_timeout)) {
        return true;
    } else {
        mu_timeout_cancel(mu_timeout);
        return false;
    }
}

// *****************************************************************************
// Private (static) code

static void mu_timeout_fn(mu_task_t *task, void *arg) {
    mu_timeout_t *self = MU_TASK_CTX(task, mu_timeout_t, task);
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case MU_TIMEOUT_STATE_IDLE: {
        // Wait here for a call to mu_timeout_arm()
    } break;

    case MU_TIMEOUT_STATE_ARMED: {
        // Here from scheduler when mu_timeout expires.  Invoke on_timeout.
        mu_task_transfer(task, MU_TIMEOUT_STATE_EXPIRED, self->on_timeout);
    } break;

    case MU_TIMEOUT_STATE_EXPIRED: {
        // Terminal state: here when the mu_timeout has expired.
    } break;

    } // switch
}

// *****************************************************************************
// End of file
