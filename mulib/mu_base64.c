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

#include "mu_base64.h"
#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define BASE64_PADDING '='

// *****************************************************************************
// Private (static) storage

static const char s_encode64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// *****************************************************************************
// Private (forward) declarations

/**
 * @brief Given a char c, return its index within s_encode64[], or -1 if not a
 * member.
 */
static int decode_index(char c);

// *****************************************************************************
// Public code

bool mu_base64_encode(char *dst, size_t dst_len, const char *src,
                      size_t src_len) {
    // validate input parameters
    if (dst == NULL) {
        return false;
    } else if (src == NULL) {
        return false;
    } else if (dst_len < ((src_len + 2) / 3 * 4 + 1)) {
        return false;
    }

    size_t i, j = 0;

    for (i = 0; i < src_len;) {
        // fetch three bytes from the source, padding with 0 as needed
        uint32_t octet_a = (unsigned char)src[i++];
        uint32_t octet_b = (i < src_len) ? (unsigned char)src[i++] : 0;
        uint32_t octet_c = (i < src_len) ? (unsigned char)src[i++] : 0;

        // Pack the three bytes into a 24 bit sequence
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        // Encode 24 bit binary into 4 ascii bytes
        for (int k = 3; k >= 0; k--) {
            dst[j++] = s_encode64[(triple >> k * 6) & 0x3F];
        }
    }

    // If src_len was not a multiple of 3, pad encoded result with '='
    int padding = (src_len % 3 == 0) ? 0 : 3 - (src_len % 3);
    while (padding-- > 0) {
        dst[j++] = BASE64_PADDING;
    }

    // Null terminate resulting string
    dst[j] = '\0';

    return true;
}

// *****************************************************************************
// Private (static) code

static int decode_index(char c) {
    if ('A' <= c && c <= 'Z') {
        return c - 'A';
    } else if ('a' <= c && c <= 'z') {
        return c - 'a' + 26;
    } else if ('0' <= c && c <= '9') {
        return c - '0' + 52;
    } else if (c == '+') {
        return 62;
    } else if (c == '/') {
        return 63;
    } else {
        return -1;
    }
}
