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

#include "mu_button_io.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// Local types and definitions

// =============================================================================
// Local storage

static mu_button_io_callback_t s_button_cb;

// =============================================================================
// Local (forward) declarations

// =============================================================================
// Public code

void mu_button_io_init(void) {
  s_button_cb = NULL;
}

void mu_button_io_set_callback(mu_button_io_callback_t cb) {
  s_button_cb = cb;
}

bool mu_button_io_get_button(uint8_t button_id) {
  (void)button_id;
  //return !USER_BUTTON_get_level();
  return false;
}

void mu_button_io_on_change(void) {
  if (s_button_cb) {
    s_button_cb(MU_BUTTON_0, mu_button_io_get_button(MU_BUTTON_0));
  }
}
// =============================================================================
// Local (static) code