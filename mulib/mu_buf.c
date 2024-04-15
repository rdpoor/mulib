/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_buf.h"

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

mu_buf_t *mu_buf_init_ro(mu_buf_t *buf, const uint8_t *bytes, size_t capacity) {
    buf->ro_bytes = bytes;
    buf->capacity = capacity;
    return buf;
}

mu_buf_t *mu_buf_init_rw(mu_buf_t *buf, uint8_t *bytes, size_t capacity) {
    buf->ro_bytes = bytes;
    buf->capacity = capacity;
    return buf;
}

const uint8_t *mu_buf_bytes_ro(mu_buf_t *buf) {
    return buf->ro_bytes;
}

uint8_t *mu_buf_bytes_rw(mu_buf_t *buf) {
    return buf->wr_bytes;
}

int mu_buf_capacity(mu_buf_t *buf) {
    return buf->capacity;
}

const uint8_t *mu_buf_ref_ro(mu_buf_t *buf, size_t index) {
    if (index < buf->capacity) {
        return &buf->ro_bytes[index];
    } else {
        return NULL;
    }
}

uint8_t *mu_buf_ref_rw(mu_buf_t *buf, size_t index) {
    if (index < buf->capacity) {
        return &buf->wr_bytes[index];
    } else {
        return NULL;
    }
}

// *****************************************************************************
// Private (static) code

// *****************************************************************************
// End of file
