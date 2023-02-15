/**
 * MIT License
 *
 * Copyright (c) 2020 R. D. Poor <rdpoor@gmail.com>
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
#include <stddef.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private declarations

// *****************************************************************************
// Local storage

mu_task_state_change_hook_fn s_state_change_hook_fn = NULL;

// *****************************************************************************
// Public code

void mu_task_register_state_change_hook(mu_task_state_change_hook_fn fn) {
  s_state_change_hook_fn = fn;
}

mu_task_t *mu_task_init(mu_task_t *task,
                        mu_task_fn fn,
                        unsigned int initial_state,
                        mu_task_state_name_fn state_name_fn) {
  task->fn = fn;
  task->state = initial_state;
  task->state_name_fn = state_name_fn;
  return task;
}

void mu_task_call(mu_task_t *task, void *arg) {
  if (task != NULL) {
    task->fn(task, arg);
  }
}

mu_task_fn mu_task_get_fn(mu_task_t *task) { return task->fn; }

unsigned int mu_task_get_state(mu_task_t *task) { return task->state; }

void mu_task_set_state(mu_task_t *task, unsigned int state) {
  if (s_state_change_hook_fn) {
    s_state_change_hook_fn(task, task->state, state);
  }
  task->state = state;
}

const char *mu_task_state_name(mu_task_t *task, unsigned int state) {
  if (task->state_name_fn) {
    return task->state_name_fn(task, state);
  } else {
    return NULL;
  }
}

// *****************************************************************************
// Private functions
