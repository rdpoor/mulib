/**
 * @file mu_jtree.h
 *
 * MIT License
 *
 * Copyright (c) 2023 R. Dunbar Poor
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
 * @brief Parse and traverse JSON-formatted strings in-place.

mu-jtree will parse this JSON string:

               '[  1,  "a",  { "b" : true, "c" : null}, [ 2.0  ] ]'

into 10 tokens as follows:

                [  1,  "a",  { "b" : true, "c" : null}, [ 2.0  ] ]
                +-------------------------------------------------^
                |  1   "a"   { "b" : true, "c" : null}  [ 2.0  ]
                |  +^  +--^  +------------------------^ +-------^
                |  |   |     | "b" : true, "c" : null   | 2.0
                |  |   |     | +--^  +---^ +--^  +---^  | +--^
                |  |   |     | |     |     |     |      | |
token string    ^  ^   ^     ^ ^     ^     ^     ^      ^ ^
token idx:      0  1   2     3 4     5     6     7      8 9
token type:     A  I   S     O S     T     S     N      A F
child count:    4  0   0     4 0     0     0     0      1 0
sibling count:  0  3   2     1 3     2     1     0      0 0
token parent:   x  0   0     0 3     3     3     3      0 8
token sibling:  x  2   3     8 5     6     7     x      x x
token children: 1  x   x     4 x     x     x     x      9 x
 */

#ifndef _MU_JTREE_H_
#define _MU_JTREE_H_

// *****************************************************************************
// Includes

#include "../core/mu_str.h"

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

#define MU_JTREE_END PTRDIFF_MAX
#define MU_JTREE_NOT_FOUND PTRDIFF_MAX

typedef enum {
  MU_JTREE_ERR_NONE,
  MU_JTREE_ERR_ALLOC,
  MU_JTREE_ERR_INVALID,
  MU_JTREE_ERR_INCOMPLETE,
} mu_jtree_err_t;

typedef enum {
  MU_JTREE_TYPE_UNDEFINED, // not yet set
  MU_JTREE_TYPE_ARRAY,     // array of elements
  MU_JTREE_TYPE_OBJECT,    // name:value pairs
  MU_JTREE_TYPE_STRING,    // double-quoted string
  MU_JTREE_TYPE_INTEGER,   // an integer
  MU_JTREE_TYPE_FLOAT,     // a floating point value
  MU_JTREE_TYPE_TRUE,      // true value
  MU_JTREE_TYPE_FALSE,     // false value
  MU_JTREE_TYPE_NULL,      // null value
} mu_jtree_token_type_t;

typedef struct {
  mu_jtree_token_type_t type; // the token type
  size_t link;                // 0 = no sibling, 1 = no child, n = next sibling
  mu_str_t contents;          // a string that spans the token's text
} mu_jtree_token_t;

typedef struct {
  mu_jtree_token_t *tokens; // user supplied array of tokens
  size_t token_capacity;    // # of tokens in array
  size_t token_count;       // # of tokens in use
} mu_jtree_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a jtree object.
 *
 * @param jtree A mu_jtree object.
 * @param tokens An array of tokens to hold parsed results.
 * @param capacity The number of tokens in the tokens array.
 * @return jtree.
 */
mu_jtree_t *mu_jtree_init(mu_jtree_t *jtree, mu_jtree_token_t *tokens,
                          size_t capacity);

/**
 * @brief Parse a JSON string into its component tokens.
 *
 * @param jtree A mu_jtree object.
 * @param json A mu_str string of the JSON to be parsed.
 * @return An error code of the result of parsing.
 */
mu_jtree_err_t mu_jtree_parse(mu_jtree_t *jtree, mu_str_t *json);

/**
 * @brief Parse a null-terminated JSON string into its component tokens.
 *
 * @param jtree A mu_jtree object.
 * @param json A null-terminated string of the JSON to be parsed.
 * @return An error code of the result of parsing.
 */
mu_jtree_err_t mu_jtree_parse_cstr(mu_jtree_t *jtree, const char *json);

/**
 * @brief Get the maximum number of tokens available for mu_jtree_parse
 *
 * @param jtree A previoulsy initialized mu_jtree object.
 * @return The maximum number of mu_jtree_token_t elements.
 */
static inline size_t mu_jtree_token_capacity(mu_jtree_t *jtree) {
  return jtree->token_capacity;
}

/**
 * @brief Get the number of tokens resulting from a call to mu_jtree_parse() or
 * mu_jtree_parse_cstr().
 *
 * @param jtree A previously parsed mu_jtree object.
 * @return The number of mu_jtree_token_t elements.
 */
static inline size_t mu_jtree_token_count(mu_jtree_t *jtree) {
  return jtree->token_count;
}

/**
 * @brief Access an individual token in a parsed tree.
 *
 * @param tree A previoulsy parsed mu_jtree object.
 * @param index The index of the token to reference.
 * @return the referenced token, or NULL if index is greater than or equal to
 *         the number of tokens returned by mu_jtree_token_count().
 */
static inline mu_jtree_token_t *mu_jtree_token_ref(mu_jtree_t *jtree,
                                                   size_t idx) {
  return index < mu_jtree_token_count() ? &jtree->tokens[idx] : NULL;
}

/**
 * @brief Return the number of children for the given token.
 */
size_t mu_jtree_children_count(mu_jtree_t *jtree, mu_jtree_token_t *token);

/**
 * @brief Return the first child of the given token, or NULL if there are no
 * children.
 */
mu_jtree_token_t *mu_jtree_children(mu_jtree_t *jtree, mu_jtree_token_t *token);

/**
 * @brief Return the number of siblings of the given token.
 */
size_t mu_jtree_sibling_count(mu_jtree_t *jtree, mu_jtree_token_t *token);

/**
 * @brief Return the next sibling of the given token, or NULL if there are no
 * siblings.
 */
mu_jtree_token_t *mu_jtree_sibling(mu_jtree_t *jtree, mu_jtree_token_t *token);

/**
 * @brief Return the parent of the given token, or NULL if this is the top
 * level token.
 */
mu_jtree_token_t *mu_jtree_parent(mu_jtree_t *jtree, mu_jtree_token_t *token);

/**
 * @brief Return a sub-tree.
 *
 * @param subtree A jtree object to hold the subtree.
 * @param jtree The jtree object
 * @param token The parent token of the subtree.
 * @return subtree if parameters are valid, NULL otherwise.
 */
mu_jtree_t *mu_jtree_subtree(mu_jtree_t *subtree, mu_jtree_t *jtree,
                             mu_jtree_token_t *token);

/**
 * @brief Match a jtree against a template..
 *
 * The pattern matches if each token in target matches the corresponding
 * target in template.  If the template's contents is '?', it matches any
 * token.
 *
 * @param target A previously parsed mu_jtree containing the JSON to be
 * matched
 * @param pattern A previoulsy parsed mu_jtree containing a JSON pattern
 */
bool mu_jtree_match(mu_jtree_t *target, mu_jtree_t *pattern, bool allow_extras);

/**
 * @brief Return the token type of a token.
 *
 * @param token A parsed token.
 * @return the type of the token.
 */
mu_jtree_token_type_t mu_jtree_token_type(mu_jtree_token_t *token);

/**
 * @brief Return the string contents of the token.
 */
mu_str_t *mu_jtree_token_contents(mu_jtree_token_t *token);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JTREE_H_ */
