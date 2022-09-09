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
 * @file mu_srec.h
 *
 * @brief Support for reading and writing Motorola S-records
 */

#ifndef _MU_SREC_H_
#define _MU_SREC_H_

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Includes

#include <stddef.h>

// *****************************************************************************
// Public types and definitions

typedef enum {
  SREC_ADDR_SIZE_16,
  SREC_ADDR_SIZE_24,
  SREC_ADDR_SIZE_32
} srec_addr_size_t;

/**
 * @brief Signature of user-supplied function to receive a formatted s-record.
 */
typedef void (*srec_writer_fn)(const char *str);

typedef struct {
  const struct _srec_descriptor *descriptor;
  int skip;
  char *buf;
  char *buflen;
  srec_write_fn writer;
} srec_handle_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the srec printer.
 *
 * @param addr_size Must be one of SREC_ADDR_SIZE__16, _24 or _32.
 * @param skip If positive, the low-order 8 bits set the skip byte.  If all the
 *        bytes in the payload equal the skip byte, the record is ellided.  If
 *        skip is negative, elliding is disabled.
 * @param buf User-supplied buffer for receiving the formatted s-records.
 * @param buflen Length of the user supplied buffer.  The buffer length
 *        determines the number of bytes per record.  Possible values:
 *                          min  max  bytes per record
 *        SREC_ADDR_SIZE_16  12  267  buflen - 11
 *        SREC_ADDR_SIZE_24  14  269  buflen - 13
 *        SREC_ADDR_SIZE_32  16  271  buflen - 15
 *        So, for example, with SREC_ADDR_SIZE_32, a buflen of 32+15 = 47 will
 *        print 32 bytes per record.
 * @param writer User-supplied function to process the formatted data.  It is
 *        called once for each record generated, passed a pointer to the
 *        formatted string.
 */
srec_handle_t *srec_init(srec_handle_t *handle,
                         srec_addr_size_t addr_size,
                         int skip,
                         char *buf,
                         char *buflen,
                         srec_write_fn writer);

/**
 * @brief Generate an srec header (S0).
 *
 * @param handle The s_record handle as returned by srec_init.
 * @param hdr_message The message to embed in the header record.  Any characters
 *        longer than buflen - 11 will be truncated.
 */
void srec_write_header(srec_handle_t *handle, const char *hdr_message);

/**
 * @brief Generate one or more srec data records (S1, S2, or S3).
 *
 * @param handle The s_record handle as returned by srec_init.
 * @param addr The starting address of the data.
 * @param data A pointer to the binary data to be encoded.
 * @param data_len The number of bytes in the data buffer.
 */
void srec_write_data(srec_handle_t *handle,
                     size_t addr,
                     uint8_t *src_data,
                     size_t data_len);

/**
 * @brief Generate a termination record (S7, S8, S9).
 *
 * @param handle The s_record handle as returned by srec_init.
 * @param addr The starting address of the code (may be 0)
 */
void srec_write_termination(srec_handle_t *handle, size_t addr);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_SREC_H_ */
