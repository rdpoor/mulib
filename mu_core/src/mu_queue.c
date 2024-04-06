/**
 * MIT License
 *
 * Copyright (c) 2021-2024 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_queue.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

/**
 * @brief get or peek at the element at the head of the queue.
 *
 * @param queue A previously initialized queue.
 * @param element Pointer to location to receive the result (unless empty).
 * @param fetch If true remove the element, else leave the element in the queue.
 * @return false if the queue is empty.
 */
static bool access_queue(mu_queue_t *queue, void **element, bool fetch);

// *****************************************************************************
// Public code

mu_queue_t *mu_queue_init(mu_queue_t *queue, void **storage,
                            size_t capacity) {
    queue->storage = storage;
    queue->capacity = capacity;
    return mu_queue_reset(queue);
}

mu_queue_t *mu_queue_reset(mu_queue_t *queue) {
    queue->count = 0;
    queue->index = 0;
    return queue;
}

size_t mu_queue_capacity(mu_queue_t *queue) { return queue->capacity; }

size_t mu_queue_count(mu_queue_t *queue) { return queue->count; }

bool mu_queue_is_empty(mu_queue_t *queue) { return queue->count == 0; }

bool mu_queue_is_full(mu_queue_t *queue) {
    return queue->count == queue->capacity;
}

bool mu_queue_put(mu_queue_t *queue, void *element) {
    if (!mu_queue_is_full(queue)) {
        queue->storage[queue->index] = element;
        queue->index = (queue->index + 1) % queue->capacity;
        queue->count += 1;
        return true;
    } else {
        return false;
    }
}

bool mu_queue_get(mu_queue_t *queue, void **element) {
    return access_queue(queue, element, true);
}

bool mu_queue_peek(mu_queue_t *queue, void **element) {
    return access_queue(queue, element, false);
}

// *****************************************************************************
// Private (static) code

static bool access_queue(mu_queue_t *queue, void **element, bool fetch) {
    if (!mu_queue_is_empty(queue)) {
        // idx = (index - count) MOD capacity, using unsigned arithmetic
        size_t idx = queue->capacity + queue->index - queue->count;
        if (idx >= queue->capacity) {
            idx -= queue->capacity;
        }
        *element = queue->storage[idx];
        if (fetch) {
            queue->count -= 1;
        }
        return true;
    } else {
        *element = NULL;
        return false;
    }
}
