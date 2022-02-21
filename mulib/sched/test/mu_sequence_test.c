/**
 * @file mu_seuqnce_test.c
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

#include "mu_test_utils.h"
#include "mu_sequence.h"
#include <string.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) forward declarations

static void task1_fn(void *ctx, void *arg);

static void task2_fn(void *ctx, void *arg);

static void reset(void);

// *****************************************************************************
// Local (private, static) storage

static mu_sequence_t s_sequence;

static mu_task_t s_task1;
static mu_task_t s_task2;

static int s_task1_count;
static int s_task2_count;

// *****************************************************************************
// Public code

void mu_sequence_test() {

  // calling an empty sequence
  reset();
  mu_sequence_call(&s_sequence, NULL, true);
  ASSERT(s_task1_count == 0);
  ASSERT(s_task2_count == 0);

  // append one task
  reset();
  mu_sequence_append_task(&s_sequence, &s_task1);
  mu_sequence_call(&s_sequence, NULL, true);  // preserve the sequence
  ASSERT(s_task1_count == 1);
  ASSERT(s_task2_count == 0);
  mu_sequence_call(&s_sequence, NULL, false); // remove tasks when called
  ASSERT(s_task1_count == 2);
  ASSERT(s_task2_count == 0);
  mu_sequence_call(&s_sequence, NULL, false);
  ASSERT(s_task1_count == 2);
  ASSERT(s_task2_count == 0);

  // append two tasks
  reset();
  mu_sequence_append_task(&s_sequence, &s_task1);
  mu_sequence_append_task(&s_sequence, &s_task2);
  mu_sequence_call(&s_sequence, NULL, true);
  ASSERT(s_task1_count == 1);
  ASSERT(s_task2_count == 1);
  mu_sequence_call(&s_sequence, NULL, false);
  ASSERT(s_task1_count == 2);
  ASSERT(s_task2_count == 2);
  mu_sequence_call(&s_sequence, NULL, false);
  ASSERT(s_task1_count == 2);
  ASSERT(s_task2_count == 2);

  // prepend two tasks
  reset();
  mu_sequence_prepend_task(&s_sequence, &s_task2);
  mu_sequence_prepend_task(&s_sequence, &s_task1);
  mu_sequence_call(&s_sequence, NULL, true);
  ASSERT(s_task1_count == 1);
  ASSERT(s_task2_count == 1);
  mu_sequence_call(&s_sequence, NULL, false);
  ASSERT(s_task1_count == 2);
  ASSERT(s_task2_count == 2);
  mu_sequence_call(&s_sequence, NULL, false);
  ASSERT(s_task1_count == 2);
  ASSERT(s_task2_count == 2);

}

// *****************************************************************************
// Local (private, static) code

static void task1_fn(void *ctx, void *arg) {
  (void)ctx;
  (void)arg;
  s_task1_count += 1;
  // Guarantee that d1 is always called before d2
  ASSERT(s_task1_count > s_task2_count);
}

static void task2_fn(void *ctx, void *arg) {
  (void)ctx;
  (void)arg;
  s_task2_count += 1;
  // Guarantee that d2 is always called after d1
  ASSERT(s_task1_count == s_task2_count);
}

static void reset(void) {
  s_task1_count = 0;
  s_task2_count = 0;
  mu_sequence_init(&s_sequence);
  mu_task_init(&s_task1, task1_fn, NULL);
  mu_task_init(&s_task2, task2_fn, NULL);
}

/**
 * MIT License
 *
 * Copyright (c) 2020 R. Dunbar Poor <rdpoor@gmail.com>
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
// includes

// =============================================================================
// private types and definitions

// =============================================================================
// private declarations

// =============================================================================
// local storage

// =============================================================================
// public code

// =============================================================================
// private code
