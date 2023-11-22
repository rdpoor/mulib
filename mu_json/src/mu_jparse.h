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
 * @file mu_jparse.h
 *
 * @brief Parse JSON into tokens
 */

#ifndef _MU_JPARSE_H_
#define _MU_JPARSE_H_

// ****************************************************************************=
// Includes

#include "mu_str.h"
#include <stddef.h>

// ****************************************************************************=
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************=
// Public types and definitions

#define MU_JPARSE_TOKEN_NOT_FOUND -1

typedef enum {
    MU_JPARSE_ERR_EMPTY_STRING = 0, // empty string or whitespace only
    MU_JPARSE_ERR_BAD_FORMAT = -1,  // illegal JSON format
    MU_JPARSE_ERR_OVERFLOW = -2,    // not enough tokens
} mu_jparse_err_t;

typedef enum {
    MU_JPARSE_TOKEN_TYPE_UNKNOWN, // ?
    MU_JPARSE_TOKEN_TYPE_ARRAY,   // [ ... ]
    MU_JPARSE_TOKEN_TYPE_OBJECT,  // { ... }
    MU_JPARSE_TOKEN_TYPE_STRING,  // "..."
    MU_JPARSE_TOKEN_TYPE_NUMBER,  // 123.45
    MU_JPARSE_TOKEN_TYPE_INTEGER, // 12345 (specialized number)
    MU_JPARSE_TOKEN_TYPE_TRUE,    // true
    MU_JPARSE_TOKEN_TYPE_FALSE,   // false
    MU_JPARSE_TOKEN_TYPE_NULL,    // null
} mu_jparse_token_type_t;

typedef struct {
    mu_str_t str;                // slice of the original JSON string
    mu_jparse_token_type_t type; // token type
    int depth;                   // 0 = toplevel, 1 = child of toplevel...
} mu_jparse_token_t;

typedef struct {
    mu_jparse_token_t *tokens; // an array of tokens
    size_t max_tokens;         // maximum number of available tokens
    int status;                // status < 0 indicates error, else # of tokens
} mu_jparse_jtree_t;

// ****************************************************************************=
// Public declarations

mu_jparse_jtree_t *mu_jparse_jtree_init(mu_jparse_jtree_t *tree,
                                        mu_jparse_token_t *token_store,
                                        size_t max_tokens);

mu_jparse_jtree_t *mu_jparse_parse_c_str(mu_jparse_jtree_t *tree,
                                         const char *c_str);

mu_jparse_jtree_t *mu_jparse_parse_mu_str(mu_jparse_jtree_t *tree,
                                          mu_str_t *mu_str);

mu_parse_jtree_t *mu_jparse_parse_buf(mu_parse_jtree_t *, (const uint8_t *)buf,
                                      size_t buf_length);

int mu_jparse_jtree_token_index_of(mu_jparse_jtree_t *tree,
                                   mu_jparse_token_t *token);

mu_jparse_token_t *mu_jparse_jtree_token_ref(mu_jparse_jtree_t *tree,
                                             int index);

mu_jparse_token_t *mu_jparse_jtree_next_sibling(mu_jparse_jtree_t *tree,
                                                mu_str_t *mu_str);

mu_jparse_token_t *mu_jparse_jtree_prev_sibling(mu_jparse_jtree_t *tree,
                                                mu_str_t *mu_str);

mu_jparse_token_t *mu_jparse_jtree_parent(mu_jparse_jtree_t *tree,
                                          mu_str_t *mu_str);

mu_jparse_token_t *mu_jparse_jtree_first_child(mu_jparse_jtree_t *tree,
                                               mu_str_t *mu_str);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JPARSE_H_ */
