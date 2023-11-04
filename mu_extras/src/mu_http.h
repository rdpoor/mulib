/**
 * MIT License
 *
 * Copyright (c) 2020-2023 R. D. Poor <rdpoor@gmail.com>
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
 * @brief Utilities for generating and processing HTTP.
 */

#ifndef _MU_HTTP_H_
#define _MU_HTTP_H_

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

/**
 * @brief Signature for the mu_http_writer_fn, used by mu_http_write_rqst() and
 *        mu_http_write_resp() functions.
 *
 * @param ch A byte to be written.
 * @param arg A user-supplied argument.
 */
typedef void (*mu_http_writer_fn)(char ch, void *arg);

typedef struct {
    const char *method;
    const char *uri;
    const char *version;
} mu_http_rqst_t;

typedef struct {
    const char *protocol;
    uint16_t status;
} mu_http_resp_t;

typedef struct {
    const char *name;
    const uint8_t *value;
    size_t value_length;
} mu_http_header_line_t;

typedef struct {
    mu_http_header_line_t *header_lines;
    size_t capacity;
    size_t count;
} mu_http_header_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize an HTTP request.
 *
 * @param rqst The mu_http_rqst_t structure to be initialized.
 * @param method A recognized method, e.g. "GET", "PUT", POST, etc.
 * @param uri The URI for the header, e.g. "/api/authors"
 * @param version A valid protocol, e.g. "HTTP/1.1"
 * @return rqst
 */
mu_http_rqst_t *mu_http_rqst_init(mu_http_rqst_t *rqst, const char *method,
                                  const char *uri, const char *version);

/**
 * @brief Initialize an HTTP response.
 *
 * @param resp The mu_http_resp_t structure to be initialized.
 * @param version A valid protocol version, e.g. "HTTP/1.1"
 * @param status The status code, e.g. 200, 404, 302 etc.
 * @return resp
 */
mu_http_resp_t *mu_http_resp_init(mu_http_resp_t *resp, const char *protocol,
                                  uint16_t status);

/**
 * @brief Initialize an HTTP header object.
 *
 * @param header A user-supplied header object.
 * @param lines A user-supplied array of header lines.  May be NULL.
 * @param max_lines The number of lines in the array.
 */
mu_http_header_t *mu_http_header_init(mu_http_header_t *header,
                                      mu_http_header_line_t *lines,
                                      size_t max_lines);

/**
 * @brief Add a name: value pair to the headers.
 *
 * @param headers A previously initialized mu_http_headers_t object.
 * @param name The name of the header field
 * @param value The value of the header field as a null-terminated C string
 * @return headers if valid, NULL if max_headers has been exceeded
 */
mu_http_header_t *mu_http_add_header_cstr(mu_http_header_t *header,
                                          const char *name, const char *value);

/**
 * @brief Add a name: value pair to the headers.
 *
 * @param header A previously initialized mu_http_headers_t object.
 * @param name The name of the header field
 * @param value The value of the header field
 * @param value_length The number of bytes in value
 * @return headers if valid, NULL if max_headers has been exceeded
 */
mu_http_header_t *mu_http_add_header_buf(mu_http_header_t *header,
                                         const char *name, const uint8_t *value,
                                         size_t value_length);

/**
 * @brief Return the number of header lines added to this header.
 */
size_t mu_http_header_line_count(mu_http_header_t *header);

/**
 * @brief Write the HTTP request.
 *
 * Note: the user-supplied writer function is called to write each byte.
 *
 * @param rqst A mu_http_rqst object.
 * @param header A mu_header object.
 * @param body The body to be written.  May be null.
 * @param body_len The number of bytes in the body.
 * @param writer The user-supplied writer function.
 * @param arg A user-supplied argument, passed to the writer function.
 */
void mu_http_write_rqst(mu_http_rqst_t *rqst, mu_http_header_t *header,
                        uint8_t *body, size_t body_len,
                        mu_http_writer_fn writer, void *arg);

/**
 * @brief Write the HTTP response.
 *
 * Note: the user-supplied writer function is called to write each byte.
 *
 * @param resp A mu_http_resp object.
 * @param header A mu_header object.
 * @param body The body to be written.  May be null.
 * @param body_len The number of bytes in the body.
 * @param writer The user-supplied writer function.
 * @param arg A user-supplied argument, passed to the writer function.
 */
void mu_http_write_resp(mu_http_resp_t *resp, mu_http_header_t *header,
                        uint8_t *body, size_t body_len,
                        mu_http_writer_fn writer, void *arg);

// *****************************************************************************
// End of File

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_HTTP_H_ */
