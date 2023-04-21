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

// *****************************************************************************
// Public code

mu_task_t *mu_task_init(mu_task_t *task, mu_task_fn fn,
                        mu_task_state_t initial_state
#ifdef MU_TASK_EXTENDED
                        ,
                        mu_task_name_fn task_name_fn,
                        mu_task_state_name_fn state_name_fn,
                        mu_task_called_fn called_fn,
                        mu_task_state_changed_fn state_changed_fn
#endif
) {
    task->fn = fn;
    task->state = initial_state;
#ifdef MU_TASK_EXTENDED
    task->task_name_fn = task_name_fn;
    task->task_state_name_fn = state_name_fn;
    task->called_fn = called_fn;
    task->state_changed_fn = state_changed_fn;
#endif
    return task;
}

void mu_task_call(mu_task_t *task, void *arg) {
    if (task != NULL) {
#ifdef MU_TASK_EXTENDED
        task->called_fn(task);
#endif
        task->fn(task, arg);
    }
}

mu_task_fn mu_task_get_fn(mu_task_t *task) { return task->fn; }

unsigned int mu_task_get_state(mu_task_t *task) { return task->state; }

void mu_task_set_state(mu_task_t *task, unsigned int state) {
#ifdef MU_TASK_EXTENDED
    task->state_changed_fn(task, state);
#endif
    task->state = state;
}

#ifdef MU_TASK_EXTENDED
const char *mu_task_name(mu_task_t *task) {
    if (task->task_name_fn) {
        return task->task_name_fn(task);
    } else {
        return NULL:
    }
}

const char *mu_task_state_name(mu_task_t *task, mu_task_state_t state) {
    if (task->state_name_fn) {
        return task->state_name_fn(task);
    } else {
        return NULL:
    }
}
#endif

mu_task_t *mu_task_get_current_task(void) {
    return mu_sched_get_current_task();
}

mu_task_err_t mu_task_wait(mu_task_t *task, mu_task_state_t next_state) {
    mu_task_set_state(task, next_state);
    return MU_TASK_ERR_NONE;
}

mu_task_err_t mu_task_yield(mu_task_t *task, mu_task_state_t next_state) {
    mu_task_set_state(task, next_state);
    return mu_sched_asap(task);
}

mu_task_err_t mu_task_sched_from_isr(mu_task_t *task) {
    return mu_task_sched_from_isr(task);
}

mu_task_err_t mu_task_defer_for(mu_task_t *task, mu_task_state_t next_state,
                                mu_time_rel_t in) {
    mu_task_set_state(task, next_state);
    return mu_sched_defer_for(task, in);
}

mu_task_err_t mu_task_defer_until(mu_task_t *task, mu_task_state_t next_state,
                                  mu_time_abs_t at) {
    mu_task_set_state(task, next_state);
    return mu_sched_defer_for(task, at);
}

mu_task_err_t mu_task_remove_deferred_task(mu_task_t *task) {
    return mu_sched_remove_deferred_task(task);
}

mu_task_err_t mu_task_transfer(mu_task_t *from_task, mu_task_state_t next_state,
                               mu_task_t *to_task) {
    mu_task_set_state(from_task, next_state);
    return mu_sched_asap(to_task);
}

// *****************************************************************************
// Private functions
