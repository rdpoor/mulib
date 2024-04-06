/**
 * @file mu_bisync.h
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
 */

 /**
  * @brief Support for bisync encoding and decoding.
  *
  * A raw binary buffer is bisync encoded as:
  *   [SOH <binary_data> EOT]
  * where within <binary_data>, each instance of SOH, EOT or DLE is preceded
  * with a single DLE byte.
  */

#ifndef _MU_BISYNC_H_
#define _MU_BISYNC_H_

// *****************************************************************************
// Includes

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#define BISYNC_SOH 1
#define BISYNC_EOT 4
#define BISYNC_DLE 16

typedef enum {
    MU_BISYNC_ERR_OFLOW = -1,  // Ran out of room writing to buffer
    MU_BISYNC_ERR_FORMAT = -2  // BiSync format error
} mu_bisync_err_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Encapsulate binary data using bisync encoding.
 *
 * @param encoded Buffer to hold the bisync encoded data
 * @param encoded_capacity Size of the encoded buffer
 * @param src The raw binary source
 * @param src_len The number of bytes in the source data
 * @return number of bytes written to encoded on success, error code on failure
 */
int mu_bisync_encode(uint8_t *encoded, size_t encoded_capacity, const uint8_t *src, size_t src_len);

/**
 * @brief Decode bisync encoded data
 *
 * @param dst Buffer to hold the decoded bisync data
 * @param dst_caapcity Size of the dst buffer
 * @param encoded The bisync encoded data
 * @param encoded_len Size of the encoded buffer
 * @return number of bytes written to dst on success, error code on failure
 */
int mu_bisync_decode(uint8_t *dst, size_t dst_capacity, const uint8_t *encoded, size_t encoded_len);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_BISYNC_H_ */
