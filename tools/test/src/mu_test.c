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

#include "mu_ansi_term.h"
#include "mu_test_utils.h"
#include <stdbool.h>
#include <stdio.h>

// =============================================================================
// types and definitions

// =============================================================================
// private declarations

int mu_access_mgr_test();
int mu_bvec_test();
int mu_cirq_test();
int mu_dlist_test();
int mu_event_test();
int mu_list_test();
int mu_log_test();
int mu_periodic_test();
int mu_pstore_test();
int mu_queue_test();
int mu_sched_test();
int mu_sequence_test();
int mu_spsc_test();
int mu_str_test();
int mu_strbuf_test();
int mu_task_list_test();
int mu_task_test();
int mu_time_test();
int mu_timer_test();
int mu_vect_test();
// int mu_ansi_term_test();
// int mu_random_test();
// int mu_drunken_bishop_test();

// =============================================================================
// local storage

// =============================================================================
// public code

int main(void) {

  printf("\r\nstarting mu_test...\n");
  mu_test_init();

  mu_access_mgr_test();
  mu_bvec_test();
  mu_cirq_test();
  mu_dlist_test();
  mu_event_test();
  mu_list_test();
  mu_log_test();
  mu_periodic_test();
  mu_pstore_test();
  mu_queue_test();
  mu_sched_test();
  mu_spsc_test();
  mu_str_test();
  mu_strbuf_test();
  mu_task_list_test();
  mu_task_test();
  mu_time_test();
  // mu_timer_test();
  mu_vect_test();
  // extras
  // mu_ansi_term_test();
  // mu_random_test();
  // mu_drunken_bishop_test();

  bool hadErrors = mu_test_error_count() > 0;

  printf("completed mu_test.\n");
  mu_ansi_term_set_colors(hadErrors ? MU_ANSI_TERM_BRIGHT_RED
                                    : MU_ANSI_TERM_BRIGHT_GREEN,
                          MU_ANSI_TERM_DEFAULT_COLOR);
  printf("%d error%s in %d test%s\r\n",
         mu_test_error_count(),
         mu_test_error_count() == 1 ? "" : "s",
         mu_test_count(),
         mu_test_count() == 1 ? "" : "s");
  mu_ansi_term_reset();

  return mu_test_error_count(); // return error code 0 on success
}
