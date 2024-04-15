/**
 * @file bisync.c
 *
 * MIT License
 *
 * Copyright (c) 2023-2024 R. Dunbar Poor
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

#include "mu_bisync.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

/**
 * @brief Support safe buffer read and write operations.
 */
typedef struct {
    union {
        uint8_t *wbuf;
        const uint8_t *rbuf;
    };
    int capacity;
    int index;
} buffer_t;

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Safely write a byte to a buffer
 *
 * @param writer buffer object
 * @param byte byte to be written into the buffer.
 * @return true on success, false on overflow.
 */
static bool buffer_put(buffer_t *writer, uint8_t byte);

// *****************************************************************************
// Public code

int mu_bisync_encode(uint8_t *encoded, size_t encoded_capacity,
                     const uint8_t *src, size_t src_len) {
    buffer_t writer = {
        .wbuf = encoded, .capacity = encoded_capacity, .index = 0};

    if (!buffer_put(&writer, BISYNC_SOH)) {
        return MU_BISYNC_ERR_OFLOW;
    }
    for (int i = 0; i < src_len; i++) {
        uint8_t b = src[i];
        if ((b == BISYNC_EOT) || (b == BISYNC_SOH) || (b == BISYNC_DLE)) {
            // need to insert DLE
            if (!buffer_put(&writer, BISYNC_DLE)) {
                return MU_BISYNC_ERR_OFLOW;
            }
        }
        if (!buffer_put(&writer, b)) {
            return MU_BISYNC_ERR_OFLOW;
        }
    }
    if (!buffer_put(&writer, BISYNC_EOT)) {
        return MU_BISYNC_ERR_OFLOW;
    }
    return writer.index;
}

int mu_bisync_decode(uint8_t *dst, size_t dst_capacity, const uint8_t *encoded,
                     size_t encoded_len) {
    buffer_t writer = {.wbuf = dst, .capacity = dst_capacity, .index = 0};
    uint8_t b;
    bool escaping = false;

    if ((encoded_len < 1) || (encoded[0] != BISYNC_SOH)) {
        return MU_BISYNC_ERR_FORMAT; // bisync must start wtih SOH
    }

    for (int i = 0; i < encoded_len; i++) {
        b = encoded[i];
        if (escaping) {
            if (!buffer_put(&writer, b)) {
                return MU_BISYNC_ERR_OFLOW;
            }
            escaping = false;
        } else if (b == BISYNC_DLE) {
            escaping = true;
        } else if (b == BISYNC_EOT) {
            break;
        } else {
            if (!buffer_put(&writer, b)) {
                return MU_BISYNC_ERR_OFLOW;
            }
        }
    }
    // Error if final byte was not EOT
    if (b != BISYNC_EOT) {
        return MU_BISYNC_ERR_FORMAT;
    }
    // success.  Return number of bytes written into dst.
    return writer.index;
}

// *****************************************************************************
// Private (static) code

static bool buffer_put(buffer_t *writer, uint8_t byte) {
    if (writer->index >= writer->capacity) {
        return false;
    } else {
        writer->wbuf[writer->index++] = byte;
        return true;
    }
}

// *****************************************************************************
// End of file
