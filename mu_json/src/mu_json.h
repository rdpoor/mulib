/**
 * MIT License
 *
 * Copyright (c) 2024 R. D. Poor <rdpoor@gmail.com>
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
 * @brief mu_json is a compact, efficient JSON parser.
 * 
 * mu_json takes a JSON formatted string and parses it into its constituent 
 * tokens.  Once parsed, mu_json provides functions to navigate among the
 * resulting tokens via parent, child, next_sibling, prev_sibling, etc.
 * 
 * Parsing is done entirely in place: strings are not copied.  The user provies
 * token storage, so mu_json never calls malloc() or free().
 * 
 * Thanks to:
 *   Douglas Crockford for creating JSON in the first place, and for the compact
 *   JSON_checker.c design on which much of mu_json is based.
 *   Serge Zaitsev for creating JSMN, an efficient copy-free JSON parser, which
 *   informed the "sliced token" approach used by mu_json.
 * 
 * Unlike many parsers, mu_json does not try to interpret the parsed values.
 * Rather, it simply slices each token into a substring and associates a JSON
 * type to it.
 * 
 * Here's a simple example:
 * 
 * @code
 * 
 * 
 * 
 * TODO:
 * * Start numbers as INTEGER type, promote to NUMBER type only as needed.
 * * Extend `finish_token()` to check that the token type being finished 
 *   matches the expected type, and write unit test to verify.
 * * Create mu_json_parser_info_t structure, pass it into the parsing functions.
 *   If non-null, when the parsing function returns, it will contain extra info
 *   such as the char position where parsing failed, etc.
 */

#ifndef _MU_JSON_H_
#define _MU_JSON_H_

// *****************************************************************************
// Includes

#include "mu_str.h"
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef enum {
    MU_JSON_ERR_NONE = 0,        // no error
    MU_JSON_ERR_BAD_FORMAT = -1, // illegal JSON format
    MU_JSON_ERR_NO_TOKENS = -2,  // not enough tokens provided
    MU_JSON_ERR_INCOMPLETE = -3  // JSON ended with unterminated form
} mu_json_err_t;

typedef enum {
    MU_JSON_TOKEN_FLAG_IS_FIRST = 1,  // token is first in token list
    MU_JSON_TOKEN_FLAG_IS_LAST = 2,   // token is last in token list
    MU_JSON_TOKEN_FLAG_IS_SEALED = 4  // token end has been found
} mu_json_token_flags_t;

#define DEFINE_MU_JSON_TOKEN_TYPES(M)                                          \
    M(MU_JSON_TOKEN_TYPE_UNKNOWN) /* ?       */                                \
    M(MU_JSON_TOKEN_TYPE_ARRAY)   /* [ ... ] */                                \
    M(MU_JSON_TOKEN_TYPE_OBJECT)  /* { ... } */                                \
    M(MU_JSON_TOKEN_TYPE_STRING)  /* "..."   */                                \
    M(MU_JSON_TOKEN_TYPE_NUMBER)  /* 123.45  */                                \
    M(MU_JSON_TOKEN_TYPE_INTEGER) /* 12345 (specialized number) */             \
    M(MU_JSON_TOKEN_TYPE_TRUE)    /* true    */                                \
    M(MU_JSON_TOKEN_TYPE_FALSE)   /* false   */                                \
    M(MU_JSON_TOKEN_TYPE_NULL)    /* null    */

#define EXPAND_MU_JSON_TOKEN_TYPE_ENUMS(_name) _name,
typedef enum {
    DEFINE_MU_JSON_TOKEN_TYPES(EXPAND_MU_JSON_TOKEN_TYPE_ENUMS)
} mu_json_token_type_t;

typedef struct {
    mu_str_t json; // slice of the original JSON string
    uint8_t type;  // mu_json_token_type cast to uint8_t
    uint8_t flags; // mu_json_token_flags_t cast to uint8_t
    int16_t depth; // 0 = toplevel, n+1 = child of n...
} mu_json_token_t;

// *****************************************************************************
// Public declarations

int mu_json_parse_c_str(mu_json_token_t *token_store, size_t max_tokens,
                        const char *json);

int mu_json_parse_mu_str(mu_json_token_t *token_store, size_t max_tokens,
                         mu_str_t *mu_json);

int mu_json_parse_buffer(mu_json_token_t *token_store, size_t max_tokens,
                         const uint8_t *buf, size_t buflen);

mu_str_t *mu_json_token_mu_str(mu_json_token_t *token);

mu_json_token_type_t mu_json_token_type(mu_json_token_t *token);

int mu_json_token_depth(mu_json_token_t *token);

bool mu_json_token_is_first(mu_json_token_t *token);

bool mu_json_token_is_last(mu_json_token_t *token);

mu_json_token_t *mu_json_token_prev(mu_json_token_t *token);

mu_json_token_t *mu_json_token_next(mu_json_token_t *token);

mu_json_token_t *mu_json_token_root(mu_json_token_t *token);

mu_json_token_t *mu_json_token_parent(mu_json_token_t *token);

mu_json_token_t *mu_json_token_child(mu_json_token_t *token);

mu_json_token_t *mu_json_token_prev_sibling(mu_json_token_t *token);

mu_json_token_t *mu_json_token_next_sibling(mu_json_token_t *token);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JSON_H_ */
