/**
 * MIT License
 *
 * Copyright (c) 2021 R. D. Poor <rdpoor@gmail.com>
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
 * @file mu_jemit.h
 *
 * @brief Emit structured JSON
 */

#ifndef _MU_JEMIT_H_
#define _MU_JEMIT_H_

// *****************************************************************************
// Includes

#include "mu_str.h"
#include <stddef.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    size_t item_count; // # of items emitted at this level
    bool is_object;    // if true, use ':' separator else use ',' separator
} mu_jemit_level_t;

/**
 * @brief Signature for a user-supplied function to write a single JSON byte
 *
 * @param ch A byte to be written
 * @param arg User-supplied argument passed in from mu_jemit_init()
 * @return true if the character was successfully written
 */
typedef bool (*mu_jemit_writer_fn)(uint8_t ch, uintptr_t arg);

typedef struct {
    mu_jemit_level_t *levels;  // array of mu_jemit_level_t objects
    size_t max_level;          // number of mu_jemit_level_t objects
    size_t curr_level;         // level currently being processed
    mu_jemit_writer_fn writer; // user-supplied "write one byte" function
    uintptr_t arg;             // user argument passed to writer_fn
} mu_jemit_t;

// *****************************************************************************
// Public declarations

mu_jemit_t *mu_jemit_init(mu_jemit_t *emitter, mu_jemit_level_t *levels,
                          size_t max_levels, mu_jemit_writer_fn writer,
                          uintptr_t arg);

/**
 * @brief Return the current expression depth.
 */
size_t mu_jemit_curr_level(mu_jemit_t *emitter);

/**
 * @brief Return the number of items emitted.
 */
size_t mu_jemit_item_count(mu_jemit_t *emitter);

/**
 * @brief Return the number of items emitted at this level.
 */
size_t mu_jemit_sibling_count(mu_jemit_t *emitter);

bool mu_jemit_array_open(mu_jemit_t *emitter);
bool mu_jemit_array_close(mu_jemit_t *emitter);
bool mu_jemit_object_open(mu_jemit_t *emitter);
bool mu_jemit_object_close(mu_jemit_t *emitter);

bool mu_jemit_c_str(mu_jemit_t *emitter, const char *c_str);
bool mu_jemit_mu_str(mu_jemit_t *emitter, mu_str_t *mu_str);
bool mu_jemit_number(mu_jemit_t *emitter, double value);
bool mu_jemit_integer(mu_jemit_t *emitter, int64_t value);
bool mu_jemit_bool(mu_jemit_t *emitter, bool boolean);
bool mu_jemit_true(mu_jemit_t *emitter);
bool mu_jemit_false(mu_jemit_t *emitter);
bool mu_jemit_null(mu_jemit_t *emitter);

bool mu_jemit_literal_buf(mu_jemit_t *emitter, uint8_t *buf, size_t n_bytes);
bool mu_jemit_literal_byte(mu_jemit_t *emitter, uint8_t byte);
bool mu_jemit_literal_c_str(mu_jemit_t *emitter, const char *c_str);
bool mu_jemit_literal_mu_str(mu_jemit_t *emitter, mu_str_t *mu_str);

bool mu_jemit_key_c_str(mu_jemit_t *emitter, const char *key,
                        const char *c_str);
bool mu_jemit_key_mu_str(mu_jemit_t *emitter, const char *key,
                         mu_str_t *mu_str);
bool mu_jemit_key_number(mu_jemit_t *emitter, const char *key, double value);
bool mu_jemit_key_integer(mu_jemit_t *emitter, const char *key, int64_t value);
bool mu_jemit_key_bool(mu_jemit_t *emitter, const char *key, bool boolean);
bool mu_jemit_key_true(mu_jemit_t *emitter, const char *key);
bool mu_jemit_key_false(mu_jemit_t *emitter, const char *key);
bool mu_jemit_key_null(mu_jemit_t *emitter, const char *key);

bool mu_jemit_mu_key_c_str(mu_jemit_t *emitter, mu_str_t *mu_key,
                           const char *c_str);
bool mu_jemit_mu_key_mu_str(mu_jemit_t *emitter, mu_str_t *mu_key,
                            mu_str_t *mu_str);
bool mu_jemit_mu_key_number(mu_jemit_t *emitter, mu_str_t *mu_key,
                            double value);
bool mu_jemit_mu_key_integer(mu_jemit_t *emitter, mu_str_t *mu_key,
                             int64_t value);
bool mu_jemit_mu_key_bool(mu_jemit_t *emitter, mu_str_t *mu_key, bool boolean);
bool mu_jemit_mu_key_true(mu_jemit_t *emitter, mu_str_t *mu_key);
bool mu_jemit_mu_key_false(mu_jemit_t *emitter, mu_str_t *mu_key);
bool mu_jemit_mu_key_null(mu_jemit_t *emitter, mu_str_t *mu_key);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JEMIT_H_ */
