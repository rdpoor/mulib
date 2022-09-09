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

// *****************************************************************************
// Includes

#include "mu_srec.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// *****************************************************************************
// Private types and definitions

#define HEADER_PREAMBLE "S0"

typedef struct {
  const char *data_preamble;
  const char *termination_preamble;
  uint8_t addr_chars;
} srec_descriptor_t;

// *****************************************************************************
// Private (static) storage

static const srec_descriptor_t s_srec16 = {
    .data_preamble = "S1",
    .termination_preamble = "S903",
    .addr_chars = 4,
};

static const srec_descriptor_t s_srec24 = {
    .data_preamble = "S2",
    .termination_preamble = "S804",
    .addr_chars = 6,
};

static const srec_descriptor_t s_srec32 = {
    .data_preamble = "S3",
    .termination_preamble = "S706",
    .addr_chars = 8,
};

// *****************************************************************************
// Private (forward) declarations

/**
 * @brief Print val as a fixed-width hex string, left padded with zeroes.
 *
 * @param val The value to be printed
 * @param buf The buffer in which to print.
 * @param width The width of the resulting string, padded on left with '0's.
 */
static void hex_print(uint32_t val, char *buf, uint8_t width);

// *****************************************************************************
// Public code

srec_handle_t *srec_init(srec_handle_t *handle,
                         srec_addr_size_t addr_size,
                         int skip,
                         char *buf,
                         char *buflen,
                         srec_write_fn writer) {
  if (handle == NULL) {
    return NULL;
  }
  switch (addr_size) {
  case SREC_ADDR_SIZE_16:
    handle->descriptor = &s_srec_16;
    break;
  case SREC_ADDR_SIZE_24:
    handle->descriptor = &s_srec_24;
    break;
  case SREC_ADDR_SIZE_32:
    handle->descriptor = &s_srec_32;
    break;
  default:
    return NULL;
  }
  handle->skip = skip;
  handle->buf = buf;
  handle->buflen = buflen;
  handle->writer = writer;
  return handle;
}

void srec_write_header(srec_handle_t *handle, const char *hdr_message);

void srec_write_data(srec_handle_t *handle,
                     size_t addr,
                     uint8_t *src_data,
                     size_t data_len);

void srec_write_termination(srec_handle_t *handle, size_t addr);


// *****************************************************************************
// Private (static) code

static void hex_printer(uint32_t val, char *buf, uint8_t width) {
  for (int i = (width - 1); i >= 0; i--) {
    uint8_t nibble = val & 0x0f;
    buf[i] = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'a');
    val >>= 4;
  }
}
