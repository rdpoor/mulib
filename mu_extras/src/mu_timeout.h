/**
 * @file mu_timeout.h
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

/**
 * @brief Notify a task after timeout period expires.
 */

#ifndef _MU_TIMEOUT_H_
#define _MU_TIMEOUT_H_

// *****************************************************************************
// Includes

#include "mu_time.h" // platform specific
#include "mulib/core/mu_task.h"
#include <stdbool.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    mu_task_t task;
    mu_time_abs_t until;      // time at which time keeper expires
    mu_task_t *on_timeout;    // task to invoke on timeout
} mu_timeout_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a mu_timeout.
 */
void mu_timeout_init(mu_timeout_t *mu_timeout, const char *timeout_name);

/**
 * @brief Return true if the timer is idle.
 */
bool mu_timeout_is_idle(mu_timeout_t *mu_timeout);

/**
 * @brief Return true if the timer is armed.
 */
bool mu_timeout_is_armed(mu_timeout_t *mu_timeout);

/**
 * @brief Return true if the timer has expired.
 */
bool mu_timeout_is_expired(mu_timeout_t *mu_timeout);

/**
 * @brief Arm (or re-arm) mu_timeout to notify a task on timeout.
 *
 * @param mu_timeout Reference to mu_timeout structure
 * @param ms Number of milliseconds before timing out
 * @param on_timeout Task to invoke upon timeout.
 */
void mu_timeout_arm(mu_timeout_t *mu_timeout, uint16_t ms,
                    mu_task_t *on_timeout);

/**
 * @brief Cancel a mu_timeout, return to idle state.
 */
void mu_timeout_cancel(mu_timeout_t *mu_timeout);

/**
 * @brief Return true if mu_timeout expired, otherwise cancel it and return
 * false.
 */
bool mu_timeout_check(mu_timeout_t *mu_timeout);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_TIMEOUT_H_ */
