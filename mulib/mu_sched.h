/**
 * MIT License
 *
 * Copyright (c) 2020 R. Dunbar Poor <rdpoor@gmail.com>
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
Implmentation notes:

mu_sched implements a discrete time, run-to-completion scheduler.  A mu_task can
be scheduled to run at some point in the future through the following calls:

- mu_task_err_t mu_sched_from_isr(mu_task_t *task);

This safely schedules a task from interrupt level to be run upon returning to
the foreground.  The task is added to an interrupt-safe, non-blocking ISR queue.
The next time mu_sched_step() is called, the first task in the ISR queue will be
processed before any other tasks.

- mu_task_err_t mu_sched_defer_until(mu_task_t *task, MU_TIME_ABS at);
- mu_task_err_t mu_sched_defer_for(mu_task_t *task, MU_TIME_REL in);

Each of these functions add a task to the scheduler's deferred queue.  The next
time mu_sched_step() is called, if there are no tasks in the ISR queue and if
the task's scheduled time has arrived, it will be processed.

- mu_task_err_t mu_sched_immed(mu_task_t *task);

Insert the task in the "immediate" queue.  THe next time mu_sched_step() is
called, if there are no ISR tasks nor runnable deferred tasks, one task from
the immediate queue will be run.

The function

- mu_task_err_t mu_sched_step(void);

is where all the magic happens:

* If there is a task in the ISR queue, it is removed from the queue and called.
* Otherwise, if there is a task in the deferred queue whose time has arrived, it
  is removed from the queue and called.
* Otherwise if there is a task in the immediate queue, if is removed from the
  queue and called.
* Finally, if there are no tasks available to run, the idle task is invoked.

Other functions:

- mu_task_t *mu_sched_current_task(void)

* returns the task currently being processed, or NULL if none are being run.

- mu_task_t *mu_sched_next_deferred_task(void)

* returns the next task in the deferred queue, or NULL if there are none.

*/

#ifndef _MU_SCHED_H_
#define _MU_SCHED_H_

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Includes

#include "mu_platform.h"
#include "mu_task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#ifndef MU_SCHED_MAX_DEFERRED_TASKS
#define MU_SCHED_MAX_DEFERRED_TASKS 20
#endif

#ifndef MU_SCHED_MAX_IRQ_TASKS
#define MU_SCHED_MAX_IRQ_TASKS 8 // must be a power of two!
#endif

#ifndef MU_SCHED_MAX_IMMED_TASKS
#define MU_SCHED_MAX_IMMED_TASKS 20
#endif

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the schedule module.  Not interrupt safe.
 */
void mu_sched_init(void);

/**
 * @brief Remove all queued tasks from the schedule.  Not interrupt safe.
 */
void mu_sched_reset(void);

/**
 * @brief Process the next runnable task or -- if none are runnable -- the idle
 * task.
 */
void mu_sched_step(void);

/**
 * @brief Return the idle task.
 */
mu_task_t *mu_sched_get_idle_task(void);

/**
 * @brief Set the task to be invoked when there are no tasks to run.
 *
 * Setting this to NULL runs the default idle task.
 */
void mu_sched_set_idle_task(mu_task_t *task);

/**
 * @brief Return the task currently being run or NULL if none are active.
 */
mu_task_t *mu_sched_current_task(void);

/**
 * @brief Return the next deferred task to be processed, or NULL if none.
 *
 * Note: this consults only the deferred queue, not the irq nor immed queues.
 */
mu_task_t *mu_sched_next_task(void);

/**
 * @brief Do nothing.  This is syntactic sugar to show that a task will wait
 * for another task to invoke it.
 */
mu_task_err_t mu_sched_wait(mu_task_t *task);

/**
 * @brief Schedule a task to run as soon as possible.
 */
mu_task_err_t mu_sched_immed(mu_task_t *task);

/**
 * @brief Schedule a task to run as soon as possible from interrupt level.
 */
mu_task_err_t mu_sched_from_isr(mu_task_t *task);

/**
 * @brief Schedule a task to run at the specified time in the future.
 */
mu_task_err_t mu_sched_defer_until(mu_task_t *task, MU_TIME_ABS at);

/**
 * @brief Schedule a task to run after the specified delay.
 */
mu_task_err_t mu_sched_defer_for(mu_task_t *task, MU_TIME_REL in);

/**
 * @brief Remove a deferred task from the schedule.
 */
mu_task_err_t mu_sched_remove_deferred_task(mu_task_t *task);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_SCHED_H_ */
