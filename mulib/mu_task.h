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

#ifndef _MU_TASK_H_
#define _MU_TASK_H_

// *****************************************************************************
// Includes

#include <stddef.h> // offsetof

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef unsigned int mu_task_state_t;

/**
 * A `mu_task` is a function that can be deferred.  It comprises a function
 * pointer (`mu_task_fn`) embedded within a context (`void *ctx`).  When called,
 * the task object itself is passed as an argument to the function.
 *
 * The MU_TASK_CTX macro returns a reference to the context surrounding a task
 * pointer.
 */

/**
 * @brief Given a pointer to a mu_task_t, return a pointer to the context
 * structure that contains it.
 */
#define MU_TASK_CTX(_task_pointer, _ctx_type, _task_slot)                      \
  ((_ctx_type *)((char *)(1 ? (_task_pointer)                                  \
                            : &((_ctx_type *)0)->_task_slot) -                 \
                 offsetof(_ctx_type, _task_slot)))

struct _mu_task; // forward declaration

// The signature of a mu_task function.
typedef void (*mu_task_fn)(struct _mu_task *task);

typedef struct _mu_task {
  mu_task_fn fn;         // the function to call
  mu_task_state_t state; // the current task state
  void *user_info;       // user-supplied info
} mu_task_t;

// The signature of a mu_task_set_state_hook() function
typedef void (*mu_task_set_state_hook)(mu_task_t *task, mu_task_state_t state);

// The signature of a mu_task_call_hook() function
typedef void (*mu_task_call_hook)(mu_task_t *task);

// The signature of a mu_task_transfer_hook() function
typedef void (*mu_task_transfer_hook)(mu_task_t *to_task);

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a task object with its function and context.
 */
mu_task_t *mu_task_init(mu_task_t *task, mu_task_fn fn,
                        mu_task_state_t initial_state,
                        void *user_info);

/**
 * @brief Install a user hook that gets called prior to setting the task state.
 */
void mu_task_install_set_state_hook(mu_task_set_state_hook fn);

/**
 * @brief Install a user hook that gets called prior to calling a task.
 */
void mu_task_install_call_hook(mu_task_call_hook fn);

/**
 * @brief Install a user hook that gets called when transferring to a new task.
 */
void mu_task_install_transfer_hook(mu_task_transfer_hook fn);

/**
 * @brief Invoke the task.
 * Note: Task may be NULL, in which case this is a no-op.
 * Note: If a mu_task_call_hook is installed, the user hook is called prior to
 * calling the task.
 */
void mu_task_call(mu_task_t *task);

/**
 * @brief Return the function of this task.
 */
mu_task_fn mu_task_get_fn(mu_task_t *task);

/**
 * @brief Return the state of the task.
 */
mu_task_state_t mu_task_get_state(mu_task_t *task);

/**
 * @brief Set the state of the task.
 * Note: The call has no effect if the task's current state equals the new
 * state.
 * Note: If a mu_task_set_state_hook is installed, the user hook is called prior
 * to setting the state.
 */
mu_task_t *mu_task_set_state(mu_task_t *task, mu_task_state_t state);

/**
 * @brief Get the user-supplied info.
 */
void *mu_task_get_user_info(mu_task_t *task);

/**
 * @brief Set the user-supplied info.
 */
void mu_task_set_user_info(mu_task_t *task, void *user_info);

/**
 * @brief invoke the transfer hook if set.
 */
void mu_task_call_transfer_hook(mu_task_t *task);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _MU_TASK_H_
