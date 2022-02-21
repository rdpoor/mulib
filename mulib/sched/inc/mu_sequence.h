/**
 * @file mu_sequence.h
 *
 * MIT License
 *
 * Copyright (c) 2022 R. Dunbar Poor
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
 *
 */

#ifndef _MU_SEQUENCE_H_
#define _MU_SEQUENCE_H_

// *****************************************************************************
// Includes

#include "mu_task.h"
#include "mu_queue.h"

// =============================================================================
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

// At its heart, a mu_sequence is really just a queue of mu_task objects.
typedef mu_queue_t mu_sequence_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a sequence object with an empty sequence of deferables.
 */
mu_sequence_t *mu_sequence_init(mu_sequence_t *sequence);

/**
 * @brief Add a task to the end of the sequence (FIFO/queue order).
 */
mu_sequence_t *mu_sequence_append_task(mu_sequence_t *sequence,
                                             mu_task_t *task);

/**
 * @brief Add a task to the beginning of the sequence (LIFO/stack order).
 */
mu_sequence_t *mu_sequence_prepend_task(mu_sequence_t *sequence,
                                              mu_task_t *task);

/**
 * @brief Invoke the individual tasks in order.
 *
 * @param sequence the mu_sequence
 * @param arg The argument to pass to each task
 * @param retain If true, the sequence is unmodified.  If false, each
 *        task is removed from the sequence before calling it.
 */
void mu_sequence_call(mu_sequence_t *sequence, void *arg, bool retain);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_SEQUENCE_H_ */
