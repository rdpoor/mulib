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
 * @file mu_json.h
 *
 * @brief Parsing and emitting JSON
 */

#ifndef _MU_JSON_H_
#define _MU_JSON_H_

// ****************************************************************************=
// Includes

#include "mu_str.h"
#include <stddef.h>
#include <stdint.h>

// ****************************************************************************=
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************=
// Public types and definitions

#define MU_JSON_TOKEN_NOT_FOUND -1

typedef enum {
    MU_JSON_ERR_EMPTY_STRING = 0, // empty string or whitespace only
    MU_JSON_ERR_BAD_FORMAT = -1,  // illegal JSON format
    MU_JSON_ERR_OVERFLOW = -2,    // not enough tokens
} mu_json_err_t;

typedef enum {
    MU_JSON_TOKEN_TYPE_UNKNOWN, // ?
    MU_JSON_TOKEN_TYPE_ARRAY,   // [ ... ]
    MU_JSON_TOKEN_TYPE_OBJECT,  // { ... }
    MU_JSON_TOKEN_TYPE_STRING,  // "..."
    MU_JSON_TOKEN_TYPE_NUMBER,  // 123.45
    MU_JSON_TOKEN_TYPE_INTEGER, // 12345 (specialized number)
    MU_JSON_TOKEN_TYPE_TRUE,    // true
    MU_JSON_TOKEN_TYPE_FALSE,   // false
    MU_JSON_TOKEN_TYPE_NULL,    // null
} mu_json_token_type_t;

typedef enum {
    MU_JSON_TOKEN_FLAG_SOL = 1,
    MU_JSON_TOKEN_FLAG_EOL = 2
} mu_json_token_flags_t;

typedef struct {
    mu_str_t str;  // slice of the original JSON string
    uint8_t type;  // mu_parse_token_type cast to uint8_t
    uint8_t flags; // mu_json_token_flags_t cast to uint8_t
    int16_t depth; // 0 = toplevel, n+1 = child of n...
} mu_json_token_t;

typedef struct {
    mu_json_token_t *tokens; // an array of tokens
    size_t max_tokens;       // maximum number of available tokens
    int status;              // status < 0 indicates error, else # of tokens
} mu_json_parser_t;

/**
 * @brief Signature for a traversal function, see @ref mu_json_token_traverse.
 */
typedef void *(*mu_json_visit_fn)(mu_json_token_t *token, void *arg);

// ****************************************************************************=
// Public declarations

mu_json_parser_t *mu_json_parser_init(mu_json_parser_t *parser,
                                      mu_json_token_t *token_store,
                                      size_t max_tokens);

int mu_json_parse_c_str(mu_json_parser_t *parser, const char *c_str);

int mu_json_parse_mu_str(mu_json_parser_t *parser, mu_str_t *mu_str);

int mu_json_parse_buf(mu_json_parser_t *parser, const uint8_t *buf,
                      size_t buf_length);

mu_json_token_type_t mu_json_token_type(mu_json_token_t *token);

mu_json_token_flags_t mu_json_token_flags(mu_json_token_t *token);

int mu_json_token_depth(mu_json_token_t *token);

mu_json_token_t *mu_json_token_prev(mu_json_token_t *token);

mu_json_token_t *mu_json_token_next(mu_json_token_t *token);

mu_json_token_t *mu_json_token_root(mu_json_token_t *token);

mu_json_token_t *mu_json_token_parent(mu_json_token_t *token);

mu_json_token_t *mu_json_token_child(mu_json_token_t *token);

mu_json_token_t *mu_json_token_prev_sibling(mu_json_token_t *token);

mu_json_token_t *mu_json_token_next_sibling(mu_json_token_t *token);

/**
 * @brief Traverse a tree or subtree of a parsed JSON strng.
 *
 * The user-supplied visit_fn is called with each token in turn, starting with
 * the given root.  It the user function returns a non-NULL value, traversal
 * stops and mu_json_token_traverse() returns that value.  Otherwise, traversal
 * continues until all sub-tokens have been visited.
 *
 * @param root The starting node to visit.
 * @param fn The user-supplied visit fn.
 * @param arg A user-supplied argument, passed as the second argument to fn.
 * @param max_depth The maximum depth.  mu_json_token_traverse() will visit
 * nodes at this depth, but will not descend into objects or arrays at this
 * depth.
 */
void *mu_json_token_traverse(mu_json_token_t *root, mu_json_visit_fn fn,
                             void *arg, int max_depth);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JSON_H_ */
