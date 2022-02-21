/**
 * @file mu_sequence.c
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

// *****************************************************************************
// Includes

#include "mu_sequence.h"

#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) forward declarations

static void *call_deferrable(mu_list_t *list, void *arg);

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

mu_sequence_t *mu_sequence_init(mu_sequence_t *sequence) {
  return mu_queue_init(sequence); // this works b/c mu_squence_t == mu_queue_t
}

mu_sequence_t *mu_sequence_append_deferrable(mu_sequence_t *sequence,
                                            mu_deferrable_t *deferrable) {
  return mu_queue_append(sequence, MU_LIST_REF(deferrable, sequence_link));
}

/**
 * @brief Add a deferrable to the beginning of the sequence (FIFO order).
 */
mu_sequence_t *mu_sequence_prepend_deferrable(mu_sequence_t *sequence,
                                              mu_deferrable_t *deferrable) {
  return mu_queue_prepend(sequence, MU_LIST_REF(deferrable, sequence_link));
}

/**
 * @brief Invoke the individual deferrables in order.
 *
 * @param sequence the mu_sequence
 * @param arg The argument to pass to each deferrable
 * @param retain If true, the sequence is unmodified.  If false, each
 *        deferrable is removed from the sequence before calling it.
 */
void mu_sequence_call(mu_sequence_t *sequence, void *arg, bool retain) {
  mu_list_t *list;
  if (retain) {
    list = mu_queue_list(sequence);
    mu_list_traverse(list, call_deferrable, arg);
  } else {
    while ((list = mu_queue_remove(sequence)) != NULL) {
      call_deferrable(list, arg);
    }
  }
}

// *****************************************************************************
// Local (private, static) code

static void *call_deferrable(mu_list_t *list, void *arg) {
  mu_deferrable_t *deferrable = MU_LIST_CONTAINER(list,
                                                  mu_deferrable_t,
                                                  sequence_link);
  mu_deferrable_call(deferrable, arg);
  return NULL;
}
