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

#include "mu_array.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

static size_t find_insertion_index(mu_array_t *array, void *item,
                                   mu_array_compare_fn cmp);

static void heapify(void **items, size_t count, mu_array_compare_fn cmp);

static void sift_down(void **items, mu_array_compare_fn cmp, int start,
                      int end);

static void swap(void **items, int a, int b);

// *****************************************************************************
// Public code

mu_array_t *mu_array_init(mu_array_t *array, void **storage, size_t capacity) {
    array->storage = storage;
    array->capacity = capacity;
    return mu_array_reset(array);
}

mu_array_t *mu_array_reset(mu_array_t *array) {
    array->count = 0;
    return array;
}

size_t mu_array_capacity(mu_array_t *array) { return array->capacity; }

size_t mu_array_count(mu_array_t *array) { return array->count; }

bool mu_array_is_empty(mu_array_t *array) { return array->count == 0; }

bool mu_array_is_full(mu_array_t *array) {
    return array->count == array->capacity;
}

bool mu_array_push(mu_array_t *array, void *item) {
    if (!mu_array_is_full(array)) {
        array->storage[array->count++] = item;
        return true;
    } else {
        return false;
    }
}

bool mu_array_pop(mu_array_t *array, void **item) {
    if (!mu_array_is_empty(array)) {
        *item = array->storage[--array->count];
        return true;
    } else {
        return false;
    }
}

bool mu_array_peek(mu_array_t *array, void **item) {
    if (!mu_array_is_empty(array)) {
        *item = array->storage[array->count - 1];
        return true;
    } else {
        return false;
    }
}

bool mu_array_ref(mu_array_t *array, void **item, size_t index) {
    if (index < array->count) {
        *item = array->storage[index];
        return true;
    } else {
        return false;
    }
}

bool mu_array_insert(mu_array_t *array, void *item, size_t index) {
    if (mu_array_is_full(array) || index > array->count) {
        return false;
    } else if (index == array->count) {
        // optimize inserting at end (identical to mu_array_push())
        array->storage[array->count++] = item;
        return true;
    } else {
        // open a slot at index
        int to_move = array->count - index;
        void *src = &array->storage[index];
        void *dst = &array->storage[index + 1];
        memmove(dst, src, to_move * sizeof(void *));
        // Store the item
        array->storage[index] = item;
        array->count += 1;
        return true;
    }
}

bool mu_array_delete(mu_array_t *array, void **item, size_t index) {
    if (mu_array_is_empty(array) || index >= array->count) {
        return false;
    }

    if (index == array->count - 1) {
        // optimize deleting at end (identical to mu_array_pop())
        *item = array->storage[--array->count];
    } else {
        *item = array->storage[index];
        // close the hole at index
        int to_move = array->count - index;
        void *src = &array->storage[index + 1];
        void *dst = &array->storage[index];
        memmove(dst, src, to_move * sizeof(void *));
    }
    return true;
}

size_t mu_array_index(mu_array_t *array, void *item) {
    for (size_t index = 0; index < array->count; index++) {
        if (array->storage[index] == item) {
            return index;
        }
    }
    return MU_ARRAY_NOT_FOUND;
}

size_t mu_array_rindex(mu_array_t *array, void *item) {
    for (size_t index = array->count - 1; index != MU_ARRAY_NOT_FOUND; index--) {
        if (array->storage[index] == item) {
            return index;
        }
    }
    return MU_ARRAY_NOT_FOUND;
}

mu_array_t *mu_array_sort(mu_array_t *array, mu_array_compare_fn cmp) {
    void *items = array->storage;
    size_t count = array->count;

    if (count > 0) {
        size_t end = count - 1;

        heapify(items, count, cmp);

        while (end > 0) {
            swap(items, end, 0);
            end -= 1;
            sift_down(items, cmp, 0, end);
        }
    }
    return array;
}

bool mu_array_insert_sorted(mu_array_t *array, void *item,
                            mu_array_compare_fn cmp) {
    if (mu_array_count(array) == 0) {
        return mu_array_push(array, item);
    } else {
        int index = find_insertion_index(array, item, cmp);
        return mu_array_insert(array, item, index);
    }
}

// *****************************************************************************
// Private (static) code

// Find "leftmost" insertion point
// ref: http://rosettacode.org/wiki/Binary_search
static size_t find_insertion_index(mu_array_t *array, void *item,
                                   mu_array_compare_fn cmp) {
    int low = 0;
    int high = array->count - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (cmp(array->storage[mid], item) > 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return low;
}

static void heapify(void **items, size_t count, mu_array_compare_fn cmp) {
    int start = (count - 2) / 2; // index of last parent node

    while (start >= 0) {
        sift_down(items, cmp, start, count - 1);
        start -= 1;
    }
}

static void sift_down(void **items, mu_array_compare_fn cmp, int start,
                      int end) {
    int root = start;
    while (root * 2 + 1 <= end) {
        // root has at least one child...
        int child = root * 2 + 1; // left child
        if ((child + 1 <= end) && cmp(items[child], items[child + 1]) < 0) {
            // child has a sibling and its value is less than the sibling's...
            child += 1; // then act on right child instead
        }
        if (cmp(items[root], items[child]) < 0) {
            // not in heap order...
            swap(items, root, child);
            root = child; // continue sifting down the child
        } else {
            return;
        }
    }
}

static void swap(void **items, int a, int b) {
    void *temp = items[a];
    items[a] = items[b];
    items[b] = temp;
}
