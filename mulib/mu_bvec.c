/**
 * MIT License
 *
 * Copyright (c) 2021-2024 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_bvec.h"
#include "mu_bbuf.h"

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

mu_bvec_t *mu_bvec_init(mu_bvec_t *bvec, mu_bbuf_t *bbuf) {
    mu_bbuf_copy(&bvec->bbuf, bbuf);
    return mu_bvec_reset(bvec);
}

mu_bvec_t *mu_bvec_init_ro(mu_bvec_t *bvec, const uint8_t *bytes, size_t capacity) {
    mu_bbuf_init_ro(&bvec->bbuf, bytes, capacity);
    return mu_bvec_reset(bvec);
}

mu_bvec_t *mu_bvec_init_rw(mu_bvec_t *bvec, uint8_t *bytes, size_t capacity) {
    mu_bbuf_init_rw(&bvec->bbuf, bytes, capacity);
    return mu_bvec_reset(bvec);
}

mu_bvec_t *mu_bvec_reset(mu_bvec_t *bvec) {
    bvec->count = 0;
    return bvec;
}

mu_bbuf_t *mu_bvec_get_bbuf(mu_bvec_t *bvec) {
    return &bvec->bbuf;
}

size_t mu_bvec_get_capacity(mu_bvec_t *bvec) {
    return mu_bbuf_capacity(&bvec->bbuf);
}

size_t mu_bvec_get_count(mu_bvec_t *bvec) {
    return bvec->count;
}

void mu_bvec_set_count(mu_bvec_t *bvec, size_t count) {
    bvec->count = count;
}

size_t mu_bvec_get_available(mu_bvec_t *bvec) {
    size_t capacity = mu_bbuf_capacity(&bvec->bbuf);
    if (bvec->count > capacity) {
        return 0;
    } else {
        return capacity - bvec->count;
    }
}

const uint8_t *mu_bvec_ref_ro(mu_bvec_t *bvec, size_t index) {
    return mu_bbuf_ref_ro(&bvec->bbuf, index);
}

uint8_t *mu_bvec_ref_rw(mu_bvec_t *bvec, size_t index) {
    return mu_bbuf_ref_rw(&bvec->bbuf, index);
}

bool mu_bvec_read_byte(mu_bvec_t *bvec, uint8_t *byte) {
    const uint8_t *p = mu_bvec_ref_ro(bvec, bvec->count++);
    if (p == NULL) {
        return false;
    } else {
        *byte = *p;
        return true;
    }
}

bool mu_bvec_write_byte(mu_bvec_t *bvec, uint8_t byte) {
    uint8_t *p = mu_bvec_ref_rw(bvec, bvec->count++);
    if (p == NULL) {
        return false;
    } else {
        *p = byte;
        return true;
    }
}

// *****************************************************************************
// Private (static) code
