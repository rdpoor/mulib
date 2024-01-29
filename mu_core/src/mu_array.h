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
 * @file: mu_array.h
 *
 * @brief Maintain a dense array of pointer-sized objects.
 */

#ifndef _MU_ARRAY_H_
#define _MU_ARRAY_H_

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

#define MU_ARRAY_NOT_FOUND SIZE_MAX

typedef struct {
    void **storage;  // user-supplied storage for items
    size_t capacity; // maximum number of items that can be stored
    size_t count;    // number of items currently in the array
} mu_array_t;

/**
 * @brief Signature for comparison function.
 *
 * A comparison function should return a negative, zero, or positive value if
 * the item referred to by item1 is less than, equal to , or greater than the
 * item referred to by item2.
 */
typedef int (*mu_array_compare_fn)(void *item1, void *item2);

/**
 * @brief Signature for filter function.
 *
 * A filter function should return true if the indicated item matches a user-
 * specified criterion.
 */
typedef bool (*mu_array_filter_fn)(void *item, uintptr_t user_arg);

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a array.
 *
 * @prarm array A array struct to be used in subsequent operations.
 * @param storage User-supplied storage to hold pointer-sized items.
 * @param capacity The number of items in the user-supplied storage.
 * @return array
 */
mu_array_t *mu_array_init(mu_array_t *array, void **storage, size_t capacity);

/**
 * @brief Remove all items from the array.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @return array
 */
mu_array_t *mu_array_reset(mu_array_t *array);

/**
 * @brief Return the maximum number of items the array can hold.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @return The maximum number of item that the array can hold.
 */
size_t mu_array_capacity(mu_array_t *array);

/**
 * @brief Return the current number of items in the array.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @return The current number of items in the array.
 */
size_t mu_array_count(mu_array_t *array);

/**
 * @brief Return true if the array has zero items.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @return true if the array has zero items.
 */
bool mu_array_is_empty(mu_array_t *array);

/**
 * @brief Return true if the array is full.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @return true if mu_array_count() equals capacity.
 */
bool mu_array_is_full(mu_array_t *array);

/**
 * @brief Insert an item at the end of the array.
 *
 * If the array is not full before the call to mu_array_push():
 * - The item is added to the tail of the array
 * - The function returns true
 *
 * If the array is full before the call to mu_array_push():
 * - The function returns false
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param item The pointer-sized item to be added to the array
 * @return True if the item was added, false otherwise.
 */
bool mu_array_push(mu_array_t *array, void *item);

/**
 * @brief Remove and return the item at the tail of the array.
 *
 * If the array is not empty before the call to mu_array_pop():
 * - The item is removed from the tail of the array
 * - The function returns true
 *
 * If the array is empty before the call to mu_array_pop():
 * - The function returns false
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param item A pointer to the location to receive the item
 * @return True if the item was fetched, false otherwise.
 */
bool mu_array_pop(mu_array_t *array, void **item);

/**
 * @brief Return the item at the tail of the array.
 *
 * If the array is not empty, return the item at the tail of the array and
 * return true, else return false.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param item A pointer to the location to receive the item
 * @return True if the item was fetched, false otherwise.
 */
bool mu_array_peek(mu_array_t *array, void **item);

/**
 * @brief Reference an item by index.
 *
 * If index is within range, item is set to the referenced item and the
 * function returns true, else returns false.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param index The index of the item to reference.
 * @param item A pointer to the location to receive the item
 * @return True if the item was available, false otherwise.
 */
bool mu_array_ref(mu_array_t *array, void **item, size_t index);

/**
 * @brief Insert an item.
 *
 * If index is within range and the array is not full, insert the item at
 * index and move all higher-indexed items up by one.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param index The insertion index for the item.
 * @param item The item to insert
 * @return True if the item inserted, false otherwise.
 */
bool mu_array_insert(mu_array_t *array, void *item, size_t index);

/**
 * @brief Remove an item from the array.
 *
 * If index is within range, item is set to the referenced, all
 * higher-indexed items are moved down by one and the function returns true,
 * else returns false.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param index The index of the item to delete.
 * @param item A pointer to the location to receive the item
 * @return True if the item was available, false otherwise.
 */
bool mu_array_delete(mu_array_t *array, void **item, size_t index);

/**
 * @brief Find an item.
 *
 * If item is present in the array, return its index, else return
 * MU_ARRAY_NOT_FOUND.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param item The item being search for
 * @return Index of the item, or MU_ARRAY_NOT_FOUND if not present.
 */
size_t mu_array_index(mu_array_t *array, void *item);

/**
 * @brief Find an item searching backwards.
 *
 * If item is present in the array, return its index, else return
 * MU_ARRAY_NOT_FOUND.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param item The item being search for
 * @return Index of the item, or MU_ARRAY_NOT_FOUND if not present.
 */
size_t mu_array_rindex(mu_array_t *array, void *item);

/**
 * @brief Sort an array in place.
 *
 * mu_array_sort() performs an in-place heapsort on its items according to
 * a user-supplied comparison function.  The heapsort operation is O(log N)
 * with very low overhead.
 *
 * The user-supplied comparison function is called with two void * arguments,
 * and should return an integer less than, equal to, or greater than zero if
 * the first argument is less than, equal to, or greater than the second
 * argument.
 *
 * @param array The array structure.
 * @param cmp The user supplied comparison function.
 * @return array
 */
mu_array_t *mu_array_sort(mu_array_t *array, mu_array_compare_fn cmp);

/**
 * @brief Insert an item into a sorted list according to a user-supplied
 * comparison function.
 *
 * Note: the list must sorted prior to the call to mu_array_insert_sorted(),
 * else the results are undefined.  See mu_array_sort() if you need to sort
 * the array first.
 *
 * The user-supplied comparison function is called with two void * arguments,
 * and should return an integer less than, equal to, or greater than zero if
 * the first argument is less than, equal to, or greater than the second
 * argument.
 *
 * @param array The array set up by a previous call to mu_array_init()
 * @param item The item to be inserted into the array.
 * @param cmp The user supplied comparison function.
 * @return true if the item was inserted, false if the array was full prior
 *         to the call to mu_array_insert_sorted()
 */
bool mu_array_insert_sorted(mu_array_t *array, void *item,
                            mu_array_compare_fn cmp);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_ARRAY_H_ */
