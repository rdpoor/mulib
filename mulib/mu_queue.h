/**
 * MIT License
 *
 * Copyright (c) 2021-2024 R. Dunbar Poor <rdpoor@gmail.com>
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
 * @file: mu_queue.h
 *
 * @brief Maintain a queue of one or more pointer-sized objects.
 */

#ifndef _MU_QUEUE_H_
#define _MU_QUEUE_H_

// *****************************************************************************
// Includes

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
    void **storage;    // user-supplied storage for queued items
    size_t capacity;   // maximum number of items that can be stored
    size_t count;      // number of items currently in the queue
    size_t index;      // index of next item to be stored
} mu_queue_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a message queue.
 *
 * @prarm queue A queue struct to be used in subsequent operations.
 * @param storage User-supplied storage to hold pointer-sized elements.
 * @param capacity The number of elements in the user-supplied storage.
 * @return queue
 */
mu_queue_t *mu_queue_init(mu_queue_t *queue, void **storage,
                            size_t capacity);

/**
 * @brief Remove all items from the queue.
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @return queue
 */
mu_queue_t *mu_queue_reset(mu_queue_t *queue);

/**
 * @brief Return the maximum number of elements the queue can hold.
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @return The maximum number of element that the queue can hold.
 */
size_t mu_queue_capacity(mu_queue_t *queue);

/**
 * @brief Return the current number of elements in the queue.
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @return The current number of elements in the queue.
 */
size_t mu_queue_count(mu_queue_t *queue);

/**
 * @brief Return true if the queue has zero elements.
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @return true if the queue has zero elements.
 */
bool mu_queue_is_empty(mu_queue_t *queue);

/**
 * @brief Return true if the queue is full.
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @return true if mu_queue_count() equals capacity.
 */
bool mu_queue_is_full(mu_queue_t *queue);

/**
 * @brief Insert an element into the queue.
 *
 * If the queue is not full before the call to mu_queue_put():
 * - The element is added to the tail of the queue
 * - The function returns true
 *
 * If the queue is full before the call to mu_queue_put():
 * - The function returns false
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @param element The pointer-sized element to be added to the queue
 * @return True if the element was added, false otherwise.
 */
bool mu_queue_put(mu_queue_t *queue, void *element);

/**
 * @brief Remove and return the element at the head of the queue.
 *
 * If the queue is not empty before the call to mu_queue_get():
 * - The element is removed from the head of the queue
 * - The function returns true
 *
 * If the queue is empty before the call to mu_queue_get():
 * - The function returns false
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @param element A pointer to the location to receive the element
 * @return True if the element was fetched, false otherwise.
 */
bool mu_queue_get(mu_queue_t *queue, void **element);

/**
 * @brief Return the element at the head of the queue without removing it.
 *
 * If the queue is not empty before the call to mu_queue_peek():
 * - The element at the head of the queue is returned by reference
 * - The function returns true
 *
 * If the queue is empty before the call to mu_queue_peek():
 * - The function returns false
 *
 * @param queue The queue set up by a previous call to mu_queue_init()
 * @param element A pointer to the location to receive the element
 * @return True if the element was present, false otherwise.
 */
bool mu_queue_peek(mu_queue_t *queue, void **element);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_QUEUE_H_ */
