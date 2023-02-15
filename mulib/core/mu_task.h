/**
 * MIT License
 *
 * Copyright (c) 2020-2022 R. D. Poor <rdpoor@gmail.com>
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

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

/**
 * A `mu_task` is a function that can be called later.  It comprises a function
 * pointer (`mu_task_fn`) embedded within a context (`void *ctx`).  When called,
 * function is passed the task object itself.
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
typedef void (*mu_task_fn)(struct _mu_task *task, void *arg);

// Signature of a function that maps a state to a state string.
typedef const char *(*mu_task_state_name_fn)(struct _mu_task *task,
                                             unsigned int state);

typedef struct _mu_task {
  mu_task_fn fn;                       // the function to call
  unsigned int state;                  // the current task state
  mu_task_state_name_fn state_name_fn; // fn to map state to state name
} mu_task_t;

typedef void (*mu_task_state_change_hook_fn)(mu_task_t *task,
                                             unsigned int prev_state,
                                             unsigned int state);

// *****************************************************************************
// Public declarations

void mu_task_register_state_change_hook(mu_task_state_change_hook_fn fn);

/**
 * @brief Initialize a task object with its function and context.
 */
mu_task_t *mu_task_init(mu_task_t *task,
                        mu_task_fn fn,
                        unsigned int initial_state,
                        mu_task_state_name_fn state_name_fn);

/**
 * @brief Invoke the deferred function.
 * Note: Task may be NULL, in which case this is a no-op.
 */
void mu_task_call(mu_task_t *task, void *arg);

/**
 * @brief Return the function of this task.
 */
mu_task_fn mu_task_get_fn(mu_task_t *task);

/**
 * @brief Return the state variable of this task.
 */
unsigned int mu_task_get_state(mu_task_t *task);

/**
 * @brief Set the state variable of this task.
 */
void mu_task_set_state(mu_task_t *task, unsigned int state);

/**
 * @brief Return a string naming the given state.  Returns NULL if not known.
 */
const char *mu_task_state_name(mu_task_t *task, unsigned int state);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _MU_TASK_H_
