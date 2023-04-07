/**
 * @file mu_jstr.c
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
 *
 */

/**
 * NOTE: https://www.json.org/json-en.html
 *
 * How it works:
 *
 * The user calls mu_jstr_parse() with a mu_jstr_t object to receive the parsed
 * results, an array of mu_jstr_token_t objects and the JSON string to parse.
 *
 * The parser maintains two indeces: one to index characters in the JSON
 * string, and one to count the number of tokens allocated.  When the parser
 * finds a "{" or "[" character, it allocates a token and marks it as "open".
 * When it finds the corresponding "}" or "]", it searches backwards through the
 * allocated tokens for the nearest open token.  From this, it knows how many
 * tokens are contained in the array or object.  It also knows the extent of the
 * underlying JSON text associated with the array or object, which gets saved
 * in the token.
 *
 * Numbers and primitives are handled similarly: a token is allocated, and the
 * underlying string brackets the token's text.
 */

// *****************************************************************************
// Includes

#include "mu_jstr.h"
#include "mulib/core/mu_str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

typedef struct {
  mu_jstr_t *jstr;     // user visible parser state
  mu_str_t *json;      // JSON string being parsed
  size_t char_idx;     // index into input string.
  size_t next_token;   // next token to allocate
} parser_t;

// *****************************************************************************
// Private (static) storage

static parser_t s_parser; // singleton parser instance.

// *****************************************************************************
// Private (static, forward) declarations

// parser accessors
static void parser_init(mu_jstr_t *jstr, mu_str_t *json);
static mu_jstr_token_t *parser_tokens(void);
static mu_jstr_token_t *parser_token_ref(size_t index);

/**
 * @brief Return the token index of the given token.
 */
static size_t parser_token_index(mu_jstr_token_t *token);

static size_t parser_token_capacity(void);
static size_t parser_token_count(void);
static mu_jstr_token_t *parser_token_alloc(mu_jstr_token_type_t type);
static size_t parser_json_char_index(void);
static bool parser_has_json_char(void);
static uint8_t parser_peek_json_char(void);
static uint8_t parser_get_json_char(void);

// token accessors
static mu_jstr_token_t *token_init(mu_jstr_token_t *token,
                                   mu_str_t *json,
                                   mu_jstr_token_type_t type);
static void token_set_element_count(mu_jstr_token_t *token, size_t element_count);
static void token_set_contents_start(mu_jstr_token_t *token, size_t start);
static void token_set_contents_end(mu_jstr_token_t *token, size_t end);
static bool token_container_is_open(mu_jstr_token_t *token);

// parsing suppoert
static mu_jstr_err_t parse_json(mu_jstr_t *tree, mu_str_t *json,
                                mu_jstr_token_t *tokens, size_t capacity);
static bool is_whitespace(uint8_t c);
static mu_jstr_err_t open_object(void);
static mu_jstr_err_t open_array(void);
static mu_jstr_err_t open_container(mu_jstr_token_type_t type);
static mu_jstr_err_t close_object(void);
static mu_jstr_err_t close_array(void);
static mu_jstr_err_t close_container(mu_jstr_token_type_t type);

/**
 * @brief Search backdwards from the current token to find an open container
 * token.  Return it if found, return NULL if no open container is found.
 */
static mu_jstr_token_t find_open_container(void);

static mu_jstr_err_t parse_string(void);
static bool is_number_prefix(uint8_t c);
static mu_jstr_err_t parse_number(void);
static mu_jstr_err_t parse_comma(void);
static mu_jstr_err_t parse_colon(void);
static mu_jstr_err_t parse_primitive(void);


// *****************************************************************************
// Public code

mu_jstr_err_t mu_jstr_parse(mu_jstr_t *tree, mu_str_t *json,
                            mu_jstr_token_t *tokens, size_t capacity) {
  return parse_json(tree, json, tokens, capacity);
}

mu_jstr_err_t mu_jstr_parse_cstr(mu_jstr_t *tree, const char *str,
                                 mu_jstr_token_t *tokens, size_t capacity) {
    mu_str_t json;
    return parse_json(tree, mu_str_init_cstr(&json, str), tokens, capacity);
}

mu_jstr_token_t *mu_jstr_token_ref(mu_jstr_t *tree, size_t index) {
  if (index < mu_jstr_token_count(tree)) {
    return &tree->tokens[index];
  } else {
    return NULL;
  }
}

size_t mu_jstr_subtree_count(mu_jstr_t *tree) {
  // something...
  return 0;
}

mu_jstr_t *mu_jstr_subtree_ref(mu_jstr_t *subtree, mu_jstr_t *tree, size_t n) {
    // TBD
    return NULL;
}

bool mu_jstr_match(mu_jstr_t *target, mu_jstr_t *pattern, bool allow_extras) {
  return false;
}

// *****************************************************************************
// Private (static) code

// **************************************************
// parser accessors

static void parser_init(mu_jstr_t *jstr, mu_str_t *json) {
  s_parser.jstr = jstr;
  s_parser.jstr->token_count = 0;
  s_parser.json = json;
  s_parser.char_idx = 0;
}

static mu_jstr_token_t *parser_tokens(void) {
    return s_parser.jstr->tokens;
}

static mu_jstr_token_t *parser_token_ref(size_t idx) {
    return &s_parser.jstr->tokens[idx];
}

static size_t parser_token_index(mu_jstr_token_t *token) {
    return (parser_tokens() - token)/sizeof(mu_jstr_token_t);
}

static size_t parser_token_capacity(void) {
    return s_parser.jstr->token_capacity;
}

static size_t parser_token_count(void) {
    return s_parser.jstr->token_count;
}

static mu_jstr_token_t *parser_token_alloc(mu_jstr_token_type_t type) {
    if (parser_token_count() >= parser_token_capacity()) {
        return NULL;
    } else {
        mu_jstr_token_t *token = parser_token_ref(s_parser.jstr->token_count++);
        return token_init(token, s_parser.json, type);
    }
}

static size_t parser_json_length(void) {
    return mu_str_length(s_parser.json);
}

static size_t parser_json_char_index(void) {
    return s_parser.char_idx;
}

static bool parser_has_json_char(void) {
    return s_parser.char_idx < parser_json_length();
}

static uint8_t parser_peek_json_char(void) {
    return mu_str_bytes(s_parser.json)[s_parser.char_idx];
}

static uint8_t parser_get_json_char(void) {
    return mu_str_bytes(s_parser.json)[s_parser.char_idx++];
}

// **************************************************
// token accessors

static mu_jstr_token_t *token_init(mu_jstr_token_t *token,
                                   mu_str_t *json,
                                   mu_jstr_token_type_t type) {
    token->type = type;
    token->element_count = 0;
    // Initialize token's contents to span the entire JSON string.  This will be
    // adjusted in token_set_contents_start() and token_set_contents_end().
    mu_str_copy(&token->contents, json);
    return token;
}

static void token_set_element_count(mu_jstr_token_t *token, size_t element_count) {
    token->element_count = element_count;
}

static void token_set_contents_start(mu_jstr_token_t *token, size_t start) {
    // NOTE: this actually sets the END of the contents to the start value.
    // This overly-clever trick saves us allocating another field in token_t.
    // Read token_set_contents_end() how we use this.
    mu_str_slice(&token->contents, &token->contents, 0, start);
}

static void token_set_contents_end(mu_jstr_token_t *token, size_t end) {
    mu_str_t *str = &token->contents;
    // Note: length was set in a previous call to token_set_contents_start(),
    // which we now use as the start.  Reset the start of the contents to length
    // and the length to end - start.
    size_t start = mu_str_length(str);
    mu_str_init(str, &mu_str_bytes(str)[start], end-start);
}

static bool token_container_is_open(mu_jstr_token_t *token) {
    return token->element_count == 0;
}

// **************************************************
// Accessing parser state

static mu_jstr_err_t parse_json(mu_jstr_t *tree, mu_str_t *json,
                                mu_jstr_token_t *tokens, size_t capacity) {
    tree->tokens = tokens;
    tree->token_capacity = capacity;
    tree->token_count = 0;
    parser_init(tree, json);

    mu_jstr_err_t err = MU_JSTR_ERR_NONE;

    while (parser_has_json_char() && err == MU_JSTR_ERR_NONE) {
        uint8_t c = parser_peek_json_char();
        if (is_whitespace(c)) {
            // skip over whitespace
            parser_get_json_char();

        } else if (c == '{') {
            err = open_object();

        } else if (c == '}') {
            err = close_object();

        } else if (c == '[') {
            err = open_array();

        } else if (c == ']') {
            err = close_array();

        } else if (c == '"') {
            err = parse_string();

        } else if (is_number_prefix(c)) {
            err = parse_number();

        } else if (c == ',') {
            err = parse_comma();

        } else if (c == ':') {
            err = parse_colon();

        } else {
            err = parse_primitive();
        }
    }
    return err;
}

static bool is_whitespace(uint8_t c) {
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

static mu_jstr_err_t open_object(void) {
    return open_container(MU_JSTR_TYPE_OBJECT);
}

static mu_jstr_err_t open_array(void) {
    return open_container(MU_JSTR_TYPE_ARRAY);
}

static mu_jstr_err_t open_container(mu_jstr_token_type_t type) {
    // Here when a '{' or '[' is spotted
    mu_jstr_token_t *token = parser_token_alloc(type);
    if (token == NULL) {
        // ran out of tokens
        return MU_JSTR_ERR_ALLOC;
    }
    // Capture the vital stats for this token:
    // - set  element_length to zero to signify "unclosed" container.
    // - capture starting character index for this token,
    // ... then consume the character.
    token_set_element_count(token, 0);
    token_set_contents_start(token, parser_json_char_index());
    parser_get_json_char();
    return MU_JSTR_ERR_NONE;
}

static mu_jstr_err_t close_object(void) {
    return close_container(MU_JSTR_TYPE_OBJECT);
}

static mu_jstr_err_t close_array(void) {
    return close_container(MU_JSTR_TYPE_ARRAY);
}

static mu_jstr_err_t close_container(mu_jstr_token_type_t type) {
    // Here when a '}' or ']' is spotted.  Search backwards through the array
    // of tokens to find the nearest "unclosed" container token, and assure that
    // the container type (object or array) matches the closing character.
    mu_jstr_token_t *token = find_open_container();

    if (token == NULL) {
        // Not inside an open container.
        return MU_JSTR_ERR_INVALID;

    } else if (mu_jstr_token_type(token) != type) {
        // cannot interleave { ... ] or [ ... }
        return MU_JSTR_ERR_INVALID;

    } else {
        // Include the closing '}' or ']' in the token contents.  Set
        // the element count to the number of tokens spanned by the
        // container.
        parser_get_json_char();
        token_set_contents_end(token, parser_json_char_index());
        token_set_element_count(token, parser_token_count() - i);
        return MU_JSTR_ERR_NONE;
    }
}

static mu_jstr_token_t find_open_container(void) {
    for (int i=parser_token_count()-1; i>=0; i--) {
        mu_jstr_token_t *token = parser_token_ref(i);
        if (token_container_is_open(token)) {
            return token;
        }
    }
    return NULL;
}

static mu_jstr_err_t parse_string(void) {
    return MU_JSTR_ERR_INVALID;
}

static bool is_number_prefix(uint8_t c) {
    return false;
}

static mu_jstr_err_t parse_comma(void) {
    // Here when a comma is found.  If not inside a container ()
    return MU_JSTR_ERR_INVALID;
}

static mu_jstr_err_t parse_colon(void) {
    return MU_JSTR_ERR_INVALID;
}

static mu_jstr_err_t parse_number(void) {
    return MU_JSTR_ERR_INVALID;
}

static mu_jstr_err_t parse_primitive(void) {
    return MU_JSTR_ERR_INVALID;
}

// *****************************************************************************
// *****************************************************************************
// Standalone Unit Tests
// *****************************************************************************
// *****************************************************************************

// Run this command in a shell to run the standalone tests.
/*
gcc -g -Wall -DTEST_MU_JSTR -I../.. -o test_mu_jstr mu_jstr.c \
../core/mu_str.c && ./test_mu_jstr && rm ./test_mu_jstr
*/

#ifdef TEST_MU_JSTR

#include <stdio.h>
#include <string.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

#define MAX_TOKENS 20
static mu_jstr_token_t s_tokens[MAX_TOKENS];

/**
 * @brief Return true if each the type of each token in tree matches the
 * corresponding elements of the types array.
 */
static bool check_token_types(mu_jstr_t *tree, mu_jstr_token_type_t *types,
                              size_t count) {
  for (size_t i = 0; i < count; i++) {
    mu_jstr_token_t *token = mu_jstr_token_ref(tree, i);
    if (mu_jstr_token_type(token) != types[i]) {
      // match failed
      return false;
    }
  }
  // all elements matched
  return true;
}

/**
 * @brief Return true if each the element length of each token in tree matches
 * corresponding member of the lengths array.
 */
static bool check_element_lengths(mu_jstr_t *tree, size_t *lengths,
                                  size_t count) {
  for (size_t i = 0; i < count; i++) {
    mu_jstr_token_t *token = mu_jstr_token_ref(tree, i);
    if (mu_jstr_token_element_count(token) != lengths[i]) {
      // match failed
      return false;
    }
  }
  // all elements matched
  return true;
}

static bool check_element_contents(mu_jstr_t *tree, const char *strings[], size_t count) {
  mu_str_t expect;

  for (size_t i = 0; i < count; i++) {
    mu_jstr_token_t *token = mu_jstr_token_ref(tree, i);
    mu_str_init_cstr(&expect, strings[i]);
    if (mu_str_compare(mu_jstr_token_contents(token), &expect) != 0) {
      // match failed
      return false;
    }
  }
  // all elements matched
  return true;
}
static void test_mu_jstr_parse(void) {
  mu_jstr_t tree;
  mu_str_t str;
  size_t token_count;

  printf("\nStarting test_mu_jstr_parse...");
  fflush(stdout);

  mu_str_init_cstr(&str, "[]");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  token_count = mu_jstr_token_count(&tree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(&tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_ARRAY},
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"[]"}, token_count));


  mu_str_init_cstr(&str, "{}");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  token_count = mu_jstr_token_count(&tree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(&tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_OBJECT},
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"{}"}, token_count));

  mu_str_init_cstr(&str, "\"a\"");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  token_count = mu_jstr_token_count(&tree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(&tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_STRING},
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"\"a\""}, token_count));

  mu_str_init_cstr(&str, "1");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  token_count = mu_jstr_token_count(&tree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"1"}, token_count));

  mu_str_init_cstr(&str, "true");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  token_count = mu_jstr_token_count(&tree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"true"}, token_count));

  mu_str_init_cstr(&str, "false");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"false"}, token_count));

  mu_str_init_cstr(&str, "null");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &tree, (mu_jstr_token_type_t[]){MU_JSTR_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&tree, (const char *[]){"null"}, token_count));

  mu_str_init_cstr(&str, "[1, 2, 3]");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 4);
  ASSERT(check_token_types(&tree,
                           (mu_jstr_token_type_t[]){
                               MU_JSTR_TYPE_ARRAY,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_PRIMITIVE,
                           },
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){4, 1, 1, 1}, token_count));
  ASSERT(check_element_contents(
      &tree, (const char *[]){"[1, 2, 3]", "1", "2", "3"}, token_count));

  mu_str_init_cstr(&str, "{\"a\":1, \"b\":true, \"c\":\"a string\"}");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 10);
  ASSERT(check_token_types(&tree,
                           (mu_jstr_token_type_t[]){
                               MU_JSTR_TYPE_OBJECT,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_STRING,
                           },
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){7, 1, 1, 1, 1, 1, 1},
                               token_count));
  ASSERT(check_element_contents(
      &tree,
      (const char *[]){"{\"a\":1, \"b\":true, \"c\":\"a string\"}", "\"a\"",
                       "1", "\"b\"", "true", "\"c\"", "\"a string\""},
      token_count));

  mu_str_init_cstr(&str, "[1, {\"a\":2}, 3]");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 6);
  ASSERT(check_token_types(&tree,
                           (mu_jstr_token_type_t[]){
                               MU_JSTR_TYPE_ARRAY,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_OBJECT,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_STRING,
                           },
                           token_count));
  ASSERT(
      check_element_lengths(&tree, (size_t[]){6, 1, 3, 1, 1, 1}, token_count));
  ASSERT(
      check_element_contents(&tree,
                             (const char *[]){"[1, {\"a\":2}, 3]", "1",
                                              "{\"a\":2}", "\"a\"", "2", "3"},
                             token_count));

  mu_str_init_cstr(&str, "{\"a\":1, \"b\":[4, 5, 6], \"c\":\"a string\"}");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 10);
  ASSERT(check_token_types(&tree,
                           (mu_jstr_token_type_t[]){
                               MU_JSTR_TYPE_OBJECT,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_ARRAY,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_PRIMITIVE,
                               MU_JSTR_TYPE_STRING,
                               MU_JSTR_TYPE_STRING,
                           },
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){10, 1, 1, 1, 4, 1, 1, 1, 1, 1},
                               token_count));
  ASSERT(check_element_contents(
      &tree,
      (const char *[]){"{\"a\":1, \"b\":[4, 5, 6], \"c\":\"a string\"}",
                       "\"a\"", "1", "\"b\"", "[4, 5, 6]", "4", "5", "6",
                       "\"c\"", "\"a string\""},
      token_count));

  mu_str_init_cstr(&str, "[1, [2, [3, [4], 5], 6], 7]");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) == MU_JSTR_ERR_NONE);
  ASSERT(token_count == 11);
  ASSERT(check_token_types(&tree,
                           (mu_jstr_token_type_t[]){
                               MU_JSTR_TYPE_ARRAY,     // [
                               MU_JSTR_TYPE_PRIMITIVE, // 1
                               MU_JSTR_TYPE_ARRAY,     // [
                               MU_JSTR_TYPE_PRIMITIVE, // 2
                               MU_JSTR_TYPE_ARRAY,     // [
                               MU_JSTR_TYPE_PRIMITIVE, // 3
                               MU_JSTR_TYPE_ARRAY,     // [
                               MU_JSTR_TYPE_PRIMITIVE, // 4
                               MU_JSTR_TYPE_PRIMITIVE, // 5
                               MU_JSTR_TYPE_PRIMITIVE, // 6
                               MU_JSTR_TYPE_PRIMITIVE, // 7
                           },
                           token_count));
  ASSERT(check_element_lengths(&tree, (size_t[]){11, 1, 8, 1, 5, 1, 2, 1, 1, 1},
                               token_count));
  ASSERT(check_element_contents(
      &tree,
      (const char *[]){"[1, [2, [3, [4], 5], 6], 7]",
                       "1", "[2, [3, [4], 5], 6]", "2",
                       "[3, [4], 5]", "3", "[4]", "4", "5", "6", "7"},
      token_count));

  printf("\n...test_mu_jstr_parse complete\n");
}

static void test_mu_jstr_parse_pathologies(void) {
  mu_jstr_t tree;
  mu_str_t str;

  printf("\nStarting test_mu_jstr_parse_pathologies...");
  fflush(stdout);

  // empty string is not JSON
  mu_str_init_cstr(&str, "");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) ==
         MU_JSTR_ERR_INVALID);

  // missing close bracket
  mu_str_init_cstr(&str, "[1, 2, 3");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) ==
         MU_JSTR_ERR_INCOMPLETE);

  // missing close bracket
  mu_str_init_cstr(&str, "{");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) ==
         MU_JSTR_ERR_INCOMPLETE);

  // member name must be a string
  mu_str_init_cstr(&str, "{1:2, 3:4}");
  ASSERT(mu_jstr_parse(&tree, &str, s_tokens, MAX_TOKENS) ==
         MU_JSTR_ERR_INVALID);

  printf("\n...test_mu_jstr_parse_pathologies complete\n");
}

static void test_mu_jstr_token_fns(void) {
  printf("\nStarting test_mu_jstr_token_fns...");
  fflush(stdout);
  mu_jstr_token_t tok;

  // slightly cheating: construct token manually for testing...
  tok.type = MU_JSTR_TYPE_ARRAY;
  tok.element_count = 3;
  mu_str_init_cstr(&tok.contents, "woof");

  ASSERT(mu_jstr_token_type(&tok) == MU_JSTR_TYPE_ARRAY);
  ASSERT(mu_jstr_token_element_count(&tok) == 3);
  ASSERT(mu_jstr_token_contents(&tok) == &tok.contents);

  printf("\n...test_mu_jstr_token_fns complete\n");
}

int main(void) {
  test_mu_jstr_parse();
  test_mu_jstr_parse_pathologies();
  test_mu_jstr_token_fns();
}

#endif

// *****************************************************************************
// End of file
