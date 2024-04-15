/**
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

#include "mu_task.h"

#include "mu_sched.h"
#include <stddef.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private declarations

// *****************************************************************************
// Local storage

static mu_task_set_state_hook s_set_state_hook = NULL;

static mu_task_call_hook s_call_hook = NULL;

static mu_task_transfer_hook s_transfer_hook = NULL;

// *****************************************************************************
// Public code

mu_task_t *mu_task_init(mu_task_t *task, mu_task_fn fn,
                        mu_task_state_t initial_state,
                        void *user_info) {
    task->fn = fn;
    task->state = initial_state;
    task->user_info = user_info;
    return task;
}

void mu_task_install_set_state_hook(mu_task_set_state_hook fn) {
    s_set_state_hook = fn;
}

void mu_task_install_call_hook(mu_task_call_hook fn) {
    s_call_hook = fn;
}

void mu_task_install_transfer_hook(mu_task_transfer_hook fn) {
    s_transfer_hook = fn;
}

void mu_task_call(mu_task_t *task) {
    // Ignore null tasks
    if (task != NULL) {
        // Call user hook if given
        if (s_call_hook != NULL) {
            s_call_hook(task);
        }
        // Invoke the task
        task->fn(task);
    }
}

mu_task_fn mu_task_get_fn(mu_task_t *task) { return task->fn; }

unsigned int mu_task_get_state(mu_task_t *task) { return task->state; }

mu_task_t *mu_task_set_state(mu_task_t *task, mu_task_state_t state) {
    if (task != NULL) {
        // Ignore null tasks
        if (s_set_state_hook != NULL) {
            // Call user hook if set
            s_set_state_hook(task, state);
        }
        // set the state
        task->state = state;
    }
    return task;
}

void *mu_task_get_user_info(mu_task_t *task) {
    return task->user_info;
}

void mu_task_set_user_info(mu_task_t *task, void *user_info) {
    task->user_info = user_info;
}

mu_task_err_t mu_task_transfer(mu_task_t *to_task) {
    if (s_transfer_hook != NULL) {
        // Call user hook if set
        s_transfer_hook(to_task);
    }
    return mu_sched_immed(to_task);
}

// *****************************************************************************
// Private functions
