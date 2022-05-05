/**
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
 */

/**
 * @file mu_sync.h
 *
 * @brief Invoke a task when one or more other tasks have completed.
 */

#ifndef _MU_SYNC_H_
#define _MU_SYNC_H_

// *****************************************************************************
// Includes

#include "mu_pstore.h"
#include "mu_task.h"
#include "mu_time.h"
#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    mu_pstore_t *waiting_on;  // The tasks on which this mu_sync is waiting
    mu_task_t *on_completion; // The task to invoke on completion.
    mu_task_t timeout_task;   // A timeout task (active iff timeout specified)
    mu_time_rel_t timeout_in; // The duration after which sync will time out.
} mu_sync_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a mu_sync object.
 *
 * @param sync The mu_sync object.
 * @param store Storage for waited-upon tasks
 * @param capacity The maximum number of waited-upon tasks
 * @return The mu_sync object.
 */
mu_sync_t *mu_sync_init(mu_sync_t *sync, void *store, size_t capacity);


/**
 * @brief Get the capacity of a mu_sync object.
 */
size_t mu_sync_capacity(mu_sync_t *sync);

/**
 * @brief Remove all the tasks from the mu_sync.
 */
mu_sync_t *mu_sync_reset(mu_sync_t *sync);

/**
 * @brief Add a task on which to wait for completion.
 *
 * @param sync The mu_sync object
 * @param task The task on which to wait.
 */
mu_sync_err_t mu_sync_wait_add_task(mu_sync_t *sync, mu_task_t *task);

/**
 * @brief Record the fact that a task has completed.
 *
 * If this was the last task to report in, the on_completion task is called.
 *
 * @param sync The mu_sync object.
 * @param task The task that completed.
 */
mu_sync_err_t mu_sync_remove_task(mu_sync_t *sync, mu_task_t *task);

/**
 * @brief Start waiting with optional timeout.
 *
 * @param sync The mu_sync object
 * @param on_completion The task to invoke when all tasks complete (or timeout)
 * @param timeout_in If non-zero, time out after the given interval.
 */
mu_sync_err_t mu_sync_wait(mu_sync_t *sync,
                           mu_task_t *on_completion,
                           mu_time_rel_t timeout_in);


// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_SYNC_H_ */
