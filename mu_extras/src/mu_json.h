/**
 * @file mu_json.h
 *
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
 * @brief Parse and generate JSON formatted strings.
 *
 * Dependencies: mu_string/mu_str.[ch]
 */

/**
 * DESIGN NOTES

To write the JSON string:
   {"color":"red", "pos":{"x":2, "y":3}, "dims":[4, 5]}
rather than:
   mu_json_object_open(&e);
   mu_json_emit_ckey_cstr(&e, "color", "red");
   mu_json_emit_cstr(&e, "pos");
   mu_json_object_open(&e);
   mu_json_emit_ckey_integer(&e, "x", 2);
   mu_json_emit_ckey_integer(&e, "y", 3);
   mu_json_object_close(&e);
   mu_json_emit_cstr(&e, "dims");
   mu_json_array_open(&e);
   mu_json_emit_integer(&e, 4);
   mu_json_emit_integer(&e, 5);
   mu_json_array_close(&e);
   mu_json_object_close(&e);

would it be possible to write:

    void mu_json_array(mu_json_emitter_t *emitter, ...);
    void mu_json_object(mu_json_emitter_t *emitter, ...);


    mu_json_object(&e,
        mu_json_ckey(&e, "color", mu_json_cstr(&e, "red")),
        mu_json_ckey(&e, "pos",
            mu_json_object(&e,
                mu_json_ckey(&e, "x", mu_json_integer(&e, 2)),
                mu_json_ckey(&e, "y", mu_json_integer(&e, 3)))),
        mu_json_ckey(&e, "dims",
            mu_json_array(&e,
                mu_json_integer(&e, 4),
                mu_json_integer(&e, 5))));

That won't quite work since you need a way to know the end of the varargs list.
You can do that with a slight loss of elegance with NULL terminators:

mu_json_object(&e,
    mu_json_ckey(&e, "color", mu_json_cstr(&e, "red")),
    mu_json_ckey(&e, "pos",
        mu_json_object(&e,
            mu_json_ckey(&e, "x", mu_json_integer(&e, 2)),
            mu_json_ckey(&e, "y", mu_json_integer(&e, 3)),
            NULL)),
    mu_json_ckey(&e, "dims",
        mu_json_array(&e,
            mu_json_integer(&e, 4),
            mu_json_integer(&e, 5),
            NULL)),
    NULL);

Another challenge: the order of evaluation of arguments is unspecified
in C.  So the sub-expression:
    mu_json_object(&e,
        mu_json_ckey(&e, "x", mu_json_integer(&e, 2)),
        mu_json_ckey(&e, "y", mu_json_integer(&e, 3))),
can evaluate the "x" before of after the "y".  This means that mu_json_ckey()
can't actually emit anything; it just needs to produce an object that will emit
"x":2 or "y:3" when asked -- the iterator inside of mu_json_object() will
process them in the desired order.

The caller must provide a buffer for the callee to write into, and the buffer
must be large enough for the longest JSON string you plan to write.  And the
buffer needs to heap or stack allocated at each level.  This feels hugely
wasteful, so maybe its best to stick to the "flat" implementation.
*/

#ifndef _MU_JSON_H_
#define _MU_JSON_H_

// *****************************************************************************
// Includes

#include "mu_str.h"
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef enum {
    MU_JSON_TOKEN_TYPE_UNKNOWN, // ?
    MU_JSON_TOKEN_TYPE_ARRAY,   // [ ... ]
    MU_JSON_TOKEN_TYPE_OBJECT,  // { ... }
    MU_JSON_TOKEN_TYPE_STRING,  // "..."
    MU_JSON_TOKEN_TYPE_NUMBER,  // 123.45
    MU_JSON_TOKEN_TYPE_INTEGER, // 12345
    MU_JSON_TOKEN_TYPE_TRUE,    // true
    MU_JSON_TOKEN_TYPE_FALSE,   // false
    MU_JSON_TOKEN_TYPE_NULL,    // null
} mu_json_token_type_t;

#define MU_JSON_PARSER_BAD_FORMAT -1
#define MU_JSON_PARSER_OVERFLOW -2

typedef struct {
    mu_str_t str;              // slice of the original JSON string
    mu_json_token_type_t type; // token type
    int depth;                 // structure depth
} mu_json_token_t;

typedef struct {
  size_t item_count;  // # of items emitted at this level
  bool is_object;     // if true, use ':' separator else use ',' separator
} mu_json_level_t;

/**
 * @brief Signature for a user-supplied function to write a single JSON byte
 *
 * @param ch Reference destination for byte read
 * @param arg User-supplied argument passed in from mu_json_emit_init()
 * @return true if the character was successfully read
 */
typedef bool (*mu_json_reader_fn)(uint8_t *ch, uintptr_t arg);

/**
 * @brief Signature for a user-supplied function to write a single JSON byte
 *
 * @param ch A byte to be written
 * @param arg User-supplied argument passed in from mu_json_emit_init()
 * @return true if the character was successfully written
 */
typedef bool (*mu_json_writer_fn)(uint8_t ch, uintptr_t arg);

typedef struct {
  mu_json_level_t *levels;   // array of mu_json_level_t objects
  size_t max_level;          // number of mu_json_level_t objects
  size_t curr_level;         // level currently being processed
  mu_json_writer_fn writer;  // user-supplied "write one byte" function
  uintptr_t arg;             // user argument passed to writer_fn
} mu_json_emitter_t;

// *****************************************************************************
// Public declarations

int mu_json_parse_stream(mu_json_tokens_t *tokens, // user-supplied token store
                         size_t max_tokens,        // max number of tokens
                         mu_json_reader_fn reader, // user-supplied char reader
                         void *reader_arg);        // arg passed to reader
int mu_json_parse_mu_str(mu_json_tokens_t *tokens, // user-supplied token store
                         size_t max_tokens,        // max number of tokens
                         mu_str_t *str);           // mu_str of JSON
int mu_json_parse_cstr(mu_json_tokens_t *tokens,   // user-supplied token store
                       size_t max_tokens,          // max number of tokens
                       const char *cstr);          // const char *JSON string

mu_json_token_type_t mu_json_token_type(mu_json_token_t *token);
int mu_json_token_level(mu_json_token_t *token);
const char *mu_json_token_string(mu_json_token_t *token);
int mu_json_token_string_length(mu_json_token_t *token);

// Parsing one token at a time.  Is this possible without keeping an array of
// parsed tokens?  Repeatedly scanning sub-elements doesn't work in a stream
// context (you can't back up).  But if MU_JSON_TOKEN_TYPE_OBJECT doesn't span
// the whole string to the closing '}', then it could work.
bool mu_json_parser_init_from_stream(mu_json_parser_t *parser,
                                mu_json_reader_fn reader,
                                void *reader_arg);
bool mu_json_parser_init_from_mu_str(mu_json_parser_t *parser,
                                     mu_str_t *json);
bool mu_json_parser_init_from_cstr(mu_json_parser_t *parser,
                                   const char *json);
int mu_json_parse_token(mu_json_parser_t *parser, mu_json_token_t *token);

mu_json_emitter_t *mu_json_emit_init(mu_json_emitter_t *emitter,
                                     mu_json_level_t *levels, size_t max_levels,
                                     mu_json_writer_fn writer, uintptr_t arg);

/**
 * @brief Return the current expression depth.
 */
size_t mu_json_emit_curr_level(mu_json_emitter_t *emitter);

/**
 * @brief Return the number of items emitted at this level.
 */
size_t mu_json_emit_item_count(mu_json_emitter_t *emitter);

bool mu_json_array_open(mu_json_emitter_t *emitter);
bool mu_json_array_close(mu_json_emitter_t *emitter);
bool mu_json_object_open(mu_json_emitter_t *emitter);
bool mu_json_object_close(mu_json_emitter_t *emitter);

bool mu_json_emit_cstr(mu_json_emitter_t *emitter, const char *cstr);
bool mu_json_emit_str(mu_json_emitter_t *emitter, mu_str_t *str);
bool mu_json_emit_number(mu_json_emitter_t *emitter, double value);
bool mu_json_emit_integer(mu_json_emitter_t *emitter, int64_t value);
bool mu_json_emit_bool(mu_json_emitter_t *emitter, bool boolean);
bool mu_json_emit_true(mu_json_emitter_t *emitter);
bool mu_json_emit_false(mu_json_emitter_t *emitter);
bool mu_json_emit_null(mu_json_emitter_t *emitter);

bool mu_json_emit_literal_buf(mu_json_emitter_t *emitter, uint8_t *buf, size_t n_bytes);
bool mu_json_emit_literal_byte(mu_json_emitter_t *emitter, uint8_t byte);
bool mu_json_emit_literal_cstr(mu_json_emitter_t *emitter, const char *cstr);
bool mu_json_emit_literal_str(mu_json_emitter_t *emitter, mu_str_t *str);

bool mu_json_emit_key_cstr(mu_json_emitter_t *emitter, mu_str_t *key, const char *cstr);
bool mu_json_emit_key_str(mu_json_emitter_t *emitter, mu_str_t *key, mu_str_t *str);
bool mu_json_emit_key_number(mu_json_emitter_t *emitter, mu_str_t *key, double value);
bool mu_json_emit_key_integer(mu_json_emitter_t *emitter, mu_str_t *key, int64_t value);
bool mu_json_emit_key_bool(mu_json_emitter_t *emitter, mu_str_t *key, bool boolean);
bool mu_json_emit_key_true(mu_json_emitter_t *emitter, mu_str_t *key);
bool mu_json_emit_key_false(mu_json_emitter_t *emitter, mu_str_t *key);
bool mu_json_emit_key_null(mu_json_emitter_t *emitter, mu_str_t *key);

bool mu_json_emit_ckey_cstr(mu_json_emitter_t *emitter, const char *key, const char *cstr);
bool mu_json_emit_ckey_str(mu_json_emitter_t *emitter, const char *key, mu_str_t *str);
bool mu_json_emit_ckey_number(mu_json_emitter_t *emitter, const char *key, double value);
bool mu_json_emit_ckey_integer(mu_json_emitter_t *emitter, const char *key, int64_t value);
bool mu_json_emit_ckey_bool(mu_json_emitter_t *emitter, const char *key, bool boolean);
bool mu_json_emit_ckey_true(mu_json_emitter_t *emitter, const char *key);
bool mu_json_emit_ckey_false(mu_json_emitter_t *emitter, const char *key);
bool mu_json_emit_ckey_null(mu_json_emitter_t *emitter, const char *key);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JSON_H_ */
