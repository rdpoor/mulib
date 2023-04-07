/**
 * @file mu_jstr.h
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
 * @brief Parse and traverse JSON-formatted strings.
 */

#ifndef _MU_JSTR_H_
#define _MU_JSTR_H_

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

#define MU_JSTR_END PTRDIFF_MAX
#define MU_JSTR_NOT_FOUND PTRDIFF_MAX

typedef enum {
  MU_JSTR_ERR_NONE,
  MU_JSTR_ERR_ALLOC,
  MU_JSTR_ERR_INVALID,
  MU_JSTR_ERR_INCOMPLETE,
} mu_jstr_err_t;

typedef enum {
  MU_JSTR_TYPE_UNDEFINED,
  MU_JSTR_TYPE_ARRAY,     // array of elements (element_length == N)
  MU_JSTR_TYPE_OBJECT,    // name:value pairs (element_length == N)
  MU_JSTR_TYPE_STRING,    // double-quoted string (element_length == 1)
  MU_JSTR_TYPE_PRIMITIVE, // number, true, false, null (element_length == 1)
} mu_jstr_token_type_t;

typedef struct {
  mu_jstr_token_type_t type; // the token type
  size_t element_count;      // # of tokens to skip to get to next sibling
  mu_str_t contents;         // a string that spans the token's text
} mu_jstr_token_t;

typedef struct {
  mu_jstr_token_t *tokens; // user supplied array of tokens
  size_t token_capacity;   // # of tokens in array
  size_t token_count;      // # of tokens in use
} mu_jstr_t;

// *****************************************************************************
// Public declarations

// inline mu_jstr_token_type_t mu_jstr_token_type(mu_jstr_token_t *token) {
// inline size_t mu_jstr_token_sub_element_count(mu_jstr_token_t *token) {
// inline mu_str_t *mu_jstr_token_content(mu_jstr_token_t *token) {

/**
 * @brief Parse a JSON string into its component tokens.
 *
 * @param tree A mu_jstr object.
 * @param json A mu_str string of the JSON to be parsed.
 * @param tokens An array of tokens to hold parsed results.
 * @param capacity The number of tokens in the tokens array.
 * @return An error code of the result of parsing.
 */
mu_jstr_err_t mu_jstr_parse(mu_jstr_t *tree, mu_str_t *json,
                            mu_jstr_token_t *tokens, size_t capacity);

/**
 * @brief Parse a null-terminated JSON string into its component tokens.
 *
 * @param tree A mu_jstr object.
 * @param json A null-terminated string of the JSON to be parsed.
 * @param tokens An array of tokens to hold parsed results.
 * @param capacity The number of tokens in the tokens array.
 * @return An error code of the result of parsing.
 */
mu_jstr_err_t mu_jstr_parse_cstr(mu_jstr_t *tree, const char *json,
                                 mu_jstr_token_t *tokens, size_t capacity);

/**
 * @brief Get the maximum number of tokens available for mu_jstr_parse
 *
 * @param tree A previoulsy parsed mu_jstr object.
 * @return The maximum number of mu_jstr_token_t elements.
 */
static inline size_t mu_jstr_token_capacity(mu_jstr_t *tree) {
  return tree->token_capacity;
}

/**
 * @brief Get the number of tokens resulting from a call to mu_jstr_parse
 *
 * @param tree A previoulsy parsed mu_jstr object.
 * @return The number of mu_jstr_token_t elements.
 */
static inline size_t mu_jstr_token_count(mu_jstr_t *tree) {
  return tree->token_count;
}

/**
 * @brief Access an individual token in a parsed tree.
 *
 * @param tree A previoulsy parsed mu_jstr object.
 * @param index The index of the token to reference.
 * @return the referenced token, or NULL if index is greater than or equal to
 *         the number of tokens returned by mu_jstr_token_count().
 */
mu_jstr_token_t *mu_jstr_token_ref(mu_jstr_t *tree, size_t index);

/**
 * @brief Return the number of top-level sub-elements in the tree.
 */
size_t mu_jstr_subtree_count(mu_jstr_t *tree);

/**
 * @brief Return a subtree of the tree.
 *
 * @param subtree mu_jstr_t in which to store the result.
 * @param tree A previoulsy parsed mu_jstr object.
 * @param n The index of the subtree to reference.
 * @return the n'th subtree, or NULL if n is greater than or equal to
 *         the number of subtrees returned by mu_jstr_subtree_count()
 */
mu_jstr_t *mu_jstr_subtree_ref(mu_jstr_t *subtree, mu_jstr_t *tree, size_t n);

/**
 * @brief Match parsed json against a template.
 *
 * @param target A previously parsed mu_jstr containing the JSON to be
 * matched
 * @param pattern A previoulsy parsed mu_jstr containing a JSON pattern
 * @param allow_extras If true, target count may exceed pattern count.  Else
 *        target count must exactly match pattern_count.
 */
bool mu_jstr_match(mu_jstr_t *target, mu_jstr_t *pattern, bool allow_extras);

/**
 * @brief Return the token type of a token.
 *
 * @param token A parsed token.
 * @return the type of the token.
 */
static inline mu_jstr_token_type_t mu_jstr_token_type(mu_jstr_token_t *token) {
  return token->type;
}

/**
 * @brief Return the number of tokens comprising this element.
 */
static inline size_t mu_jstr_token_element_count(mu_jstr_token_t *token) {
  return token->element_count;
}

/**
 * @brief Return the token in string form.
 */
static inline mu_str_t *mu_jstr_token_contents(mu_jstr_token_t *token) {
  return &token->contents;
}

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JSTR_H_ */
