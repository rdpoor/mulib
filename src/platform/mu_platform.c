/**
 * MIT License
 *
 * Copyright (c) 2021-2022 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_platform.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

// *****************************************************************************
// Public code

void mu_time_init(void) {}

mu_time_abs_t mu_time_now(void) { return 0; }

mu_time_abs_t mu_time_offset(mu_time_abs_t t, mu_time_rel_t dt) {
  (void)dt;
  return t;
}

mu_time_rel_t mu_time_difference(mu_time_abs_t t1, mu_time_abs_t t2) {
  (void)t1;
  (void)t2;
  return 0;
}

bool mu_time_precedes(mu_time_abs_t t1, mu_time_abs_t t2) {
  (void)t1;
  (void)t2;
  return false;
}

bool mu_time_equals(mu_time_abs_t t1, mu_time_abs_t t2) {
  (void)t1;
  (void)t2;
  return false;
}

bool mu_time_follows(mu_time_abs_t t1, mu_time_abs_t t2) {
  (void)t1;
  (void)t2;
  return false;
}

int mu_time_rel_to_ms(mu_time_rel_t dt) {
  (void)dt;
  return 0;
}

mu_time_rel_t mu_time_ms_to_rel(int ms) {
  (void)ms;
  return 0;
}

#ifdef MU_PLATFORM_HAS_FLOAT

MU_FLOAT mu_time_rel_to_s(mu_time_rel_t dt) {
  (void)dt;
  return 0.0;
}

mu_time_rel_t mu_time_s_to_rel(MU_FLOAT s) {
  (void s);
  return 0;
}

#endif

// *****************************************************************************
// Private (static) code
