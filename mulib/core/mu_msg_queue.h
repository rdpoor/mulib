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
 * @brief A msg_queue maintains a queue of one or more pointer-sized objects.
 *
 * When an element is added, msg_queue optionally notifies an on_put task, and
 * when an element is removed, msg_queue optionally notifies an on_get task.
 * In addition to inter-task message queues, you can use msg_queue to create
 * efficent semaphores, mutexes and other forms of locks.
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

/**
 * @brief Initialize a message queue.
 *
 * @prarm msg_queue A msg_queue struct to be used in subsequent operations.
 * @param storage User-supplied storage to hold pointer-sized elements.
 * @param capacity The number of elements in the user-supplied storage.
 * @param on_put A task to be invoked when an item is added to the queue.  May
 *        be NULL, in which case no notification is made.
 * @param on_get A task to be invoked when an item is removed from the queue.
 *        May be null, in which case no notification is made.
 * @return msg_queue
 */
mu_msg_queue_t *mu_msg_queue_init(mu_msg_queue_t *msg_queue,
                                  void **storage,
                                  size_t capacity,
                                  mu_task_t *on_put,
                                  mu_task_t *on_get);

/**
 * @brief Remove all items from the queue.
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @return msg_queue
 */
mu_msg_queue_t *mu_msg_queue_reset(mu_msg_queue_t *msg_queue);

/**
 * @brief Return the maximum number of elements the msg_queue can hold.
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @return The maximum number of element that the msg_queue can hold.
 */
size_t mu_msg_queue_capacity(mu_msg_queue_t *msg_queue);

/**
 * @brief Return the current number of elements in the msg_queue.
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @return The current number of elements in the msg_queue.
 */
size_t mu_msg_queue_count(mu_msg_queue_t *msg_queue);

/**
 * @brief Return true if the msg_queue has zero elements.
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @return true if the msg_queue has zero elements.
 */
bool mu_msg_queue_is_empty(mu_msg_queue_t *msg_queue);

/**
 * @brief Return true if the msg_queue is full.
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @return true if mu_msg_queue_count() equals capacity.
 */
bool mu_msg_queue_is_full(mu_msg_queue_t *msg_queue);

/**
 * @brief Insert an element into the queue.
 *
 * If the queue is not full before the call to mu_msg_queue_put():
 * - The element is added to the tail of the queue
 * - The on_put task is invoked if it is non-NULL
 * - The function returns true
 *
 * If the queue is full before the call to mu_msg_queue_put():
 * - The function returns false
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @param element The pointer-sized element to be added to the queue
 * @return True if the element was added, false otherwise.
 */
bool mu_msg_queue_put(mu_msg_queue_t *msg_queue, void *element);

/**
 * @brief Remove an element from the queue.
 *
 * If the queue is not empty before the call to mu_msg_queue_get():
 * - The element is removed from the head of the queue
 * - The on_get task is invoked if it is non-NULL
 * - The function returns true
 *
 * If the queue is empty before the call to mu_msg_queue_get():
 * - The function returns false
 *
 * @param msg_queue The queue set up by a previous call to Mu_msg_queue_init()
 * @param element A pointer to the location to receive the element
 * @return True if the element was fetched, false otherwise.
 */
bool mu_msg_queue_get(mu_msg_queue_t *msg_queue, void **element);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_MSG_H_ */
