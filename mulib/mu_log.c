/**
 * MIT License
 *
 * Copyright (c) 2020-2023 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_log.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) storage

// define s_level_names[], an array that maps a logging level to a string
#define EXPAND_LEVEL_NAMES(_enum_id, _name) _name,
static const char *s_level_names[] = {MU_LOG_LEVELS(EXPAND_LEVEL_NAMES)};
#define N_LOG_LEVELS (sizeof(s_level_names)/sizeof(s_level_names[0]))

// the current reporting threshold.  May be changed dynamically.
static mu_log_level_t s_reporting_threshold;

// the current reporting level, valid only within a call to mu_log
static mu_log_level_t s_reporting_level;

static mu_log_logging_fn s_logging_fn;

// *****************************************************************************
// Local (private, static) forward declarations

// *****************************************************************************
// Public code

void mu_log_init(mu_log_level_t reporting_threshold, mu_log_logging_fn logging_fn) {
    mu_log_set_reporting_threshold(reporting_threshold);
    mu_log_set_logging_function(logging_fn);
}

void mu_log_set_reporting_threshold(mu_log_level_t reporting_threshold) {
    s_reporting_threshold = reporting_threshold;
}

mu_log_level_t mu_log_get_reporting_threshold(void) {
    return s_reporting_threshold;
}

void mu_log_set_logging_function(mu_log_logging_fn logging_fn) {
    s_logging_fn = logging_fn;
}

mu_log_logging_fn mu_log_get_logging_function(void) {
    return s_logging_fn;
}

mu_log_level_t mu_log_get_reporting_level(void) {
    return s_reporting_level;
}

bool mu_log_is_reporting(mu_log_level_t reporting_level) {
    return reporting_level >= s_reporting_threshold;
}

const char *mu_log_level_name(mu_log_level_t level) {
    if (level < N_LOG_LEVELS) {
        return s_level_names[level];
    } else {
        return "UNKNOWN";
    }
}

void mu_log(mu_log_level_t level, const char *fmt, ...) {
    if (mu_log_is_reporting(level) && (s_logging_fn != NULL)) {
        s_reporting_level = level;  // make available to get_logging_level()
        va_list ap;
        va_start(ap, fmt);
        s_logging_fn(fmt, ap);
        va_end(ap);
    }
}

// *****************************************************************************
// Local (private, static) code

