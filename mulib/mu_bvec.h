/**
 * MIT License
 *
 * Copyright (c) 2021-2024 R. Dunbar Poor <rdpoor@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without rebufiction, including without limitation the rights
 * to use, copy, modify, merge, publish, dibufibute, sublicense, and/or sell
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
 * @file: mu_bvec.h
 *
 * @brief Indexable buffer of bytes.
 */

#ifndef _MU_BVEC_H_
#define _MU_BVEC_H_

// *****************************************************************************
// Includes

#include "mu_bbuf.h"
#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    mu_bbuf_t bbuf; // byte storage and capacity
    size_t count;   // number of bytes written or read.
} mu_bvec_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a mu_bvec from a mu_bbuf
 */
mu_bvec_t *mu_bvec_init(mu_bvec_t *bvec, mu_bbuf_t *bbuf);

/**
 * @brief Initialize a mu_bvec with a readonly data buffer and a length.
 */
mu_bvec_t *mu_bvec_init_ro(mu_bvec_t *bvec, const uint8_t *bytes,
                           size_t capacity);

/**
 * @brief Initialize a mu_bvec with a mutable data buffer and a length.
 */
mu_bvec_t *mu_bvec_init_rw(mu_bvec_t *bvec, uint8_t *bytes, size_t capacity);

mu_bvec_t *mu_bvec_reset(mu_bvec_t *bvec);

mu_bbuf_t *mu_bvec_get_bbuf(mu_bvec_t *bvec);

size_t mu_bvec_get_capacity(mu_bvec_t *bvec);

size_t mu_bvec_get_count(mu_bvec_t *bvec);

void mu_bvec_set_count(mu_bvec_t *bvec, size_t count);

size_t mu_bvec_get_available(mu_bvec_t *bvec);

const uint8_t *mu_bvec_ref_ro(mu_bvec_t *bvec, size_t index);

uint8_t *mu_bvec_ref_rw(mu_bvec_t *bvec, size_t index);

bool mu_bvec_read_byte(mu_bvec_t *bvec, uint8_t *byte);

bool mu_bvec_write_byte(mu_bvec_t *bvec, uint8_t byte);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_BVEC_H_ */
