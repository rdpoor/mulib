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
 * @file: mu_buf.h
 *
 * @brief Safe access to an array of bytes.
 */

#ifndef _MU_BUF_H_
#define _MU_BUF_H_

// *****************************************************************************
// Includes

#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    union {
        const uint8_t *ro_bytes;  // a buffer of read-only bytes
        uint8_t *wr_bytes;        // a buffer of mutable bytes
    };
    size_t capacity;              // the size of the buffer.
} mu_buf_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a mu_buf with a readonly data buffer and a length.
 */
mu_buf_t *mu_buf_init_ro(mu_buf_t *buf, const uint8_t *bytes, size_t capacity);

/**
 * @brief Initialize a mu_buf with a mutable data buffer and a length.
 */
mu_buf_t *mu_buf_init_rw(mu_buf_t *buf, uint8_t *bytes, size_t capacity);

/**
 * @brief Return the mu_buf's readonly data buffer.
 */
const uint8_t *mu_buf_bytes_ro(mu_buf_t *buf);

/**
 * @brief Return the mu_buf's mutable data buffer.
 */
uint8_t *mu_buf_bytes_rw(mu_buf_t *buf);

/**
 * @brief Return the capacity of the mu_buf's data buffer in bytes
 */
int mu_buf_capacity(mu_buf_t *buf);

/**
 * @brief Return a pointer to the index'th readonly byte in buf, or NULL if
 * index is out of range.
 */
const uint8_t *mu_buf_ref_ro(mu_buf_t *buf, size_t index);

/**
 * @brief Return a pointer to the index'th mutable byte in buf, or NULL if index
 * is out of range.
 */
uint8_t *mu_buf_ref_rw(mu_buf_t *buf, size_t index);


// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_BUF_H_ */
