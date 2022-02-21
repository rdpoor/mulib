/**
 * MIT License
 *
 * Copyright (c) 2020 R. D. Poor <rdpoor@gmail.com>
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

// =============================================================================
// Includes

#include "mu_deferrable.h"
#include <stddef.h>

// =============================================================================
// Private types and definitions

// =============================================================================
// Private declarations

// =============================================================================
// Local storage

// =============================================================================
// Public code

mu_deferrable_t *mu_deferrable_init(mu_deferrable_t *deferrable, mu_deferrable_fn fn, void *ctx) {
  deferrable->fn = fn;
  deferrable->ctx = ctx;
  return deferrable;
}

mu_deferrable_fn mu_deferrable_get_fn(mu_deferrable_t *deferrable) { return deferrable->fn; }

void *mu_deferrable_get_ctx(mu_deferrable_t *deferrable) { return deferrable->ctx; }

void mu_deferrable_call(mu_deferrable_t *deferrable, void *arg) {
  if (deferrable == NULL) {
    // allow null deferrable arg => no-op
    return;
  }
  deferrable->fn(deferrable->ctx, arg);
}

// =============================================================================
// Private functions
