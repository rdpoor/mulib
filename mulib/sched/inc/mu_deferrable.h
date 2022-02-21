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

#ifndef _MU_DEFERRABLE_H_
#define _MU_DEFERRABLE_H_

// *****************************************************************************
// includes

#include "mu_list.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// types and definitions

/**
 * A `mu_deferrable` is a function that can be called later.  It comprises a
 * function pointer (`mu_deferrable_fn`) and a context (`void *ctx`).  When
 * called, the function is passed the ctx argument and a caller-supplied `void
 * *` argument.
 *
 * mu_deferrable objects may be linked together into a mu_seqquece.  See
 * mu_sequence.h for more information.
 */

// The signature of a mu_deferrable function.
typedef void (*mu_deferrable_fn)(void *ctx, void *arg);

typedef struct {
  mu_deferrable_fn fn;     // function to call
  void *ctx;               // context to pass when called
  mu_list_t sequence_link; // link field for mu_sequence
} mu_deferrable_t;

// *****************************************************************************
// Declarations

/**
 * @brief Initialize a deferrable object with its function and context.
 */
mu_deferrable_t *mu_deferrable_init(mu_deferrable_t *deferrable,
                                    mu_deferrable_fn fn,
                                    void *ctx);

mu_deferrable_fn mu_deferrable_get_fn(mu_deferrable_t *deferrable);

void *mu_deferrable_get_ctx(mu_deferrable_t *deferrable);

void mu_deferrable_call(mu_deferrable_t *deferrable, void *arg);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _MU_DEFERRABLE_H_
