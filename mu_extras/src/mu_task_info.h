/**
 * @file mu_mu_task_info.h
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
 * @brief Debugging aid: Log task transitions and state transitions.
 */

#ifndef _MU_TASK_INFO_H_
#define _MU_TASK_INFO_H_

// *****************************************************************************
// Includes

#include "mulib/core/mu_task.h"
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
    const char *task_name;    // string name of this task
    const char **state_names; // array of state names for this task
    size_t n_states;          // number of states
} mu_task_info_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Install mu_task_info hooks to log task and state transitions.
 *
 * Call once at startup.
 *
 * If MU_LOG_LEVEL is set to MU_LOG_DEBUG or lower, all transitions from
 * one state to another will be logged.  In addition, all transitions from
 * one task to another will be logged.
 */
void mu_task_info_init(void);

/**
 * @brief Return the task name associated with a task.
 */
const char *mu_task_info_task_name(mu_task_t *task);

/**
 * @brief Return the state name associated with a task.
 */
const char *mu_task_info_state_name(mu_task_t *task, mu_task_state_t state);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_TASK_INFO_H_ */
