/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. Dunbar Poor <rdpoor@gmail.com>
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
 * @file: mu_msg_queue.h
 *
 * @brief Single level messaging queue
 */

#ifndef _MU_MSG_H_
#define _MU_MSG_H_

// *****************************************************************************
// Includes

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

typedef struct {
  void **storage;     // user-supplied storage for queued items
  size_t capacity;    // maximum number of items that can be stored
  size_t count;       // number of items currently in the queue
  size_t index;       // index of next item to be stored
  mu_task_t *on_put;  // task to invoke when an item is stored
  mu_task_t *on_get;  // task to invoke when an item is fetched
} mu_msg_queue_t;

// *****************************************************************************
// Public declarations

mu_msg_queue_t *mu_msg_queue_init(mu_msg_queue_t *msg_queue,
                                  void **storage,
                                  size_t capacity,
                                  mu_task_t *on_put,
                                  mu_task_t *on_get);
mu_msg_queue_t *mu_msg_queue_reset(mu_msg_queue_t *msg_queue);
size_t mu_msg_queue_capacity(mu_msg_queue_t *msg_queue);
size_t mu_msg_queue_count(mu_msg_queue_t *msg_queue);
bool mu_msg_queue_is_empty(mu_msg_queue_t *msg_queue);
bool mu_msg_queue_is_full(mu_msg_queue_t *msg_queue);
bool mu_msg_queue_put(mu_msg_queue_t *msg_queue, void *obj);
bool mu_msg_queue_get(mu_msg_queue_t *msg_queue, void **obj);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_MSG_H_ */
