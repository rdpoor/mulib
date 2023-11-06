/**
 * @file timer.h
 *
 * MIT License
 *
 * Copyright (c) 2021-2023 R. D. Poor <rdpoor@gmail.com>
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

/**
 * @brief A general purpose timer for one-shot or periodic activation of a task.
 */

#ifndef _MU_TIMER_H_
#define _MU_TIMER_H_

// *****************************************************************************
// Includes

#include "mu_task.h"
#include "mu_time.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    mu_task_t task;              // the timer task object
    mu_task_t *on_expiration;    // task to invoke when the timer expires
    MU_SCHED_ABS_TIME expire_at; // time at which the timer will expire
    MU_SCHED_REL_TIME period;    // repeat interval.
    bool is_periodic;            // true if periodic, false if one-shot
} mu_timer_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the timer.  Returns timer.
 */
mu_timer_t *mu_timer_init(mu_timer_t *timer, void *user_info);

/**
 * @brief Start (or restart) the timer in one-shot mode
 *
 * @param timer Previously initialized by timer_init().
 * @param expire_in_tics The number of tics before the timer expires.
 * @param on_expiration The task to call upon expiration.  May be NULL.
 *
 * Note: if the timer is already running, it will be stopped and restarted
 * without notifying on_completion.
 */
void mu_timer_one_shot(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                       mu_task_t *on_expiration);

/**
 * @brief Identical to mu_timer_one_shot, but takes time arguments in
 * milliseconds.
 */
void mu_timer_one_shot_ms(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_ms,
                          mu_task_t *on_expiration);

/**
 * @brief Start (or restart) the timer in periodic mode
 *
 * @param timer Previously initialized by timer_init().
 * @param expire_in_tics The number of tics before the timer expires.
 * @param period_tics The repeat interval.
 * @param on_expiration The task to call upon expiration.  May be NULL.
 *
 * Note: if the timer is already running, it will be stopped and restarted
 * without notifying on_completion.
 */
void mu_timer_periodic(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                       MU_SCHED_REL_TIME period_tics, mu_task_t *on_expiration);

/**
 * @brief Identical to mu_timer_periodic, but takes time arguments in
 * milliseconds.
 */
void mu_timer_periodic_ms(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_ms,
                          MU_SCHED_REL_TIME period_ms,
                          mu_task_t *on_expiration);

/**
 * @brief Start (or restart) the timer.
 *
 * @param timer Previously initialized by timer_init().
 * @param expire_in The number of tics before the timer expires.
 * @param period If non-zero, the timer repeats at this interval.
 * @param on_expiration The task to call upon expiration.  May be NULL.
 *
 * Note: if the timer is already running, it will be stopped and restarted
 * without notifying on_completion.
 */
void mu_timer_start(mu_timer_t *timer, MU_SCHED_REL_TIME expire_in_tics,
                    MU_SCHED_REL_TIME period_tics, mu_task_t *on_expiration);

/**
 * @brief Start (or restart) the timer, using millisecond units.
 *
 * @param timer Previously initialized by timer_init().
 * @param expire_in_ms The number of milliseconds before the timer expires.
 * @param period_ms If non-zero, the timer repeats at this interval.
 * @param on_expiration The task to call upon expiration.  May be NULL.
 *
 * Note: if the timer is already running, it will be stopped and restarted
 * without notifying on_completion.
 */
void mu_timer_start_ms(mu_timer_t *timer, int expire_in_ms, int period_ms,
                       mu_task_t *on_expiration);

/**
 * @brief Stop / cancel the timer, return to idle state.
 *
 * Note: The on_completion task will not be notified.
 * Note: If the timer is already stopped, this has no effect.
 *
 * @param timer The timer, previously initialized by timer_init().
 */
void mu_timer_stop(mu_timer_t *timer);

/**
 * @brief Return true if the timer is idle.
 */
bool mu_timer_is_idle(mu_timer_t *timer);

/**
 * @brief Return true if the timer is running.
 */
bool mu_timer_is_running(mu_timer_t *timer);

/**
 * @brief Return true if the timer is idle or expired ("not running")
 */
bool mu_timer_is_stopped(mu_timer_t *timer);

/**
 * @brief Return true if the timer has expired.
 *
 * Note: a periodic timer will never expire: it's either idle or running.
 */
bool mu_timer_is_expired(mu_timer_t *timer);

/**
 * @brief Return true if the timer has expired, else cancel it and return false.
 *
 * This function is optimized for a timeout function along these line:
 *
 *     case MY_TASK_START_ASYNC_TASK_STATE: {
 *        // configure my_timer as a one-shot timer with a 500 mSec timeout
 *         mu_timer_start_ms(&my_timer, 500, 0, task);
 *         async_task_start(task);
 *         mu_task_wait(MY_TASK_AWAIT_ASYNC_TASK_STATE);
 *     } break;
 *     case MY_TASK_AWAIT_ASYNC_TASK_STATE: {
 *         // Arrive here if my_async_task completed OR the timer timed out
 *         if (mu_timer_check(&my_timer)) {
 *             MU_LOG_ERROR("timed out waiting for my_async_task");
 *             mu_task_enqueue(task, MY_TASK_ERROR_STATE);
 *         } else {
 *             MU_LOG_INFO("async_task completed normally");
 *         }
 *     } break;
 *
 */
bool mu_timer_check(mu_timer_t *timer);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_TIMER_H_ */
