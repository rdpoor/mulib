/**
 * @file mu_jtree.c
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
 * NOTES:
 * See
 *   https://www.json.org/json-en.html
 * for syntax.
 *   https://github.com/nst/JSONTestSuite/jtree/master/test_parsing
 * for comprehensive unit tests.
 *   https://seriot.ch/projects/parsing_json.html#1
 * for discourse on the lack of standards.
 *   https://code.google.com/archive/p/json-test-suite/
 * for another test suite (but see warning on previous link):
 *   http://www.json.org/JSON_checker/
 *
 * How it works:
 *
 * The user calls mu_jtree_parse() with a mu_jtree_t object to receive the
 * parsed results, an array of mu_jtree_token_t objects and the JSON string to
 * parse.
 *
 * Upon success, each parsed token contains a type (object, array, string,
 * primitive or number) and a mu_str that spans the underlying text.  It also
 * has a link field: -1 means this node has no sibling, 0 means this node has
 * a child at the next slot in the array, >= 1 is the relative index to the next
 * sibling.
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

#include "mu_jtree.h"
#include "mulib/core/mu_str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define TOKEN_LINK_OPEN PTRDIFF_MAX

typedef struct {
  size_t char_idx;   // index into input string.
  size_t next_token; // next token to allocate
} parser_t;

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

static parser_t *parser_init(paser_t *parser);

static mu_tree_err_t parse_tree(parser_t *parser, mu_jtree_t *jtree,
                                mu_str_t *json);

// *****************************************************************************
// Public code

mu_jtree_t *mu_jtree_init(mu_jtree_t *jtree, mu_jtree_token_t *tokens,
                          size_t capacity) {
  jtree->tokens = tokens;
  jtree->token_capacity = capacity;
  jtree->token_count = 0;
  return jtree;
}

mu_jtree_err_t mu_jtree_parse(mu_jtree_t *jtree, mu_str_t *json) {
  return parse_tree(jtree, json);
}

mu_jtree_err_t mu_jtree_parse_cstr(mu_jtree_t *jtree, const char *json) {
  mu_str_t str;
  return mu_jtree_parse(jtree, mu_str_init_cstr(&str, json));
}

// link = 1 means the node has no children, else children are at . + 1.
size_t mu_jtree_children_count(mu_jtree_t *jtree, mu_jtree_token_t *token) {
  (void)jtree;
  if (token == NULL) {
    return 0;
  } else {
    return token->link - 1;
  }
}

mu_jtree_token_t *mu_jtree_children(mu_jtree_t *jtree,
                                    mu_jtree_token_t *token) {
  if (token == NULL) {
    return NULL; // null token
  } else if (mu_jtree_children_count(jtree, token) == 0) {
    return NULL; // has no children
  } else {
    // first child is next token in the token array.
    return mu_jtree_token_ref(jtree, 1);
  }
}

// link = 0 means this node has no sibling, else is offset to sibling
size_t mu_jtree_sibling_count(mu_jtree_t *jtree, mu_jtree_token_t *token) {
  size_t sibling_count = 0;
  size_t index = 0;

  if (token == NULL) {
    return sibling_count;
  } else {
    // get index of this token.
    index = token_index(jtree, token);
  }
  while ((token != NULL) && (token->link > 0)) {
    index += token->link;
    token = mu_jtree_token_ref(jtree, index);
    sibling_count += 1;
  }
  return sibling_count;
}

mu_jtree_token_t *mu_jtree_sibling(mu_jtree_t *jtree, mu_jtree_token_t *token) {
  if (token == NULL) {
    return NULL;
  } else if (token->link == 0) {
    return NULL; // no siblings
  } else {
    // at least one sibling.  form subtree starting with next sibling.
    size_t index = token_index(jtree, token) + token->link;
    return mu_jtree_token_ref(jtree, index);
  }
}

mu_jtree_token_t *mu_jtree_parent(mu_jtree_t *jtree, mu_jtree_token_t *token) {

  if (token == NULL) {
    return NULL;
  }
  // search backwards from current token for an array or object token
  mu_jtree_token_t *parent;
  size_t index = token_index(jtree, token) - 1;
  while ((parent = mu_jtree_token_ref(index)) != NULL) {
    if (token_is_container(parent)) {
      break;
    }
    index -= 1;
  }
  return parent;
}

bool mu_jtree_match(mu_jtree_t *target, mu_jtree_t *pattern,
                    bool allow_extras) {
  return false;
}

// *****************************************************************************
// Private (static) code

static mu_tree_err_t parse_tree(mu_jtree_t *jtree, mu_str_t *json) {
  parser_t parser;
  uint8_t c;

  parser_init(&parser);
  mu_jtree_err_t err = MU_JTREE_ERR_NONE;

  while (peek_char(parser, json, &c) && err == MU_JTREE_ERR_NONE) {

    if (is_whitespace(c)) {
      // skip over whitespace
      get_char(parser, json, &c);

    } else if (c == '[') {
      err = open_array(parser, jtree);
      get_char(parser, json, &c);

    } else if (c == '{') {
      err = open_object(parser, jtree);
      get_char(parser, json, &c);

    } else if (c == ']') {
      err = close_array(parser, jtree);
      get_char(parser, json, &c);

    } else if (c == '}') {
      err = close_object(parser, jtree);
      get_char(parser, json, &c);

    } else if (c == '"') {
      err = parse_string(parser, jtree, json);

    } else if (is_number_prefix(c)) {
      err = parse_number(parser, jtree, json, c);

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

static parser_t *parser_init(paser_t *parser) {
  parser->char_idx = 0;
  parser->token_idx = 0;
  return parser;
}

static bool has_char(parser_t *parser, mu_str_t *json) {
  return parser->char_idx < mu_str_length(json);
}

static bool peek_char(parser_t *parser, mu_str_t *json, uint8_t *ch) {
  if (parser->char_idx >= mu_str_length(json)) {
    return false;
  } else {
    *ch = mu_str_bytes(json)[parser->char_idx];
    return true;
  }
}

static uint8_t get_char(parser_t *parser, mu_str_t *json, uint8_t *ch) {
  if (parser->char_idx >= mu_str_length(json)) {
    return false;
  } else {
    *ch = mu_str_bytes(json)[parser->char_idx++];
    return true;
  }
}

static bool is_whitespace(uint8_t c) {
  return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

static mu_jtree_err_t open_array(parser_t *parser, mu_jtree_t *jtree) {
  return open_container(parser, jtree, MU_JTREE_TYPE_ARRAY);
}

static mu_jtree_err_t open_object(parser_t *parser, mu_jtree_t *jtree) {
  return open_container(parser, jtree, MU_JTREE_TYPE_OBJECT);
}

static mu_jtree_err_t open_container(parser_t *parser, mu_jtree_t *jtree,
                                     mu_str_t *json,
                                     mu_jtree_token_type_t type) {
  // Here when a '{' or '[' is spotted
  mu_jtree_token_t *token = token_alloc(parser, jtree);
  if (token == NULL) {
    // ran out of tokens
    return MU_JTREE_ERR_ALLOC;
  }
  // Initialize this token:
  // - set link to TOKEN_OPEN to signify "unclosed" container.
  // - capture starting character index for this token,
  // ... then consume the character.
  token_init(token, type, TOKEN_LINK_OPEN, json);
  token_set_contents_start(token, parser->char_idx++);
  return MU_JTREE_ERR_NONE;
}

static mu_jtree_token_t *token_alloc(parser_t *parser, mu_jtree_t *jtree) {
  if (parser->token_idx >= jtree->token_capacity) {
    return NULL;
  } else {
    return &jtree->tokens[parser->token_idx++];
  }
}

static mu_jtree_token_t *token_init(mu_jtree_token_t *token,
                                    mu_jtree_token_type_t type, size_t link,
                                    mu_str_t json) {
  token->type = type;
  token->link = link;
  mu_str_copy(&link->contents, json);
  return token;
}

static void token_set_contents_start(mu_jtree_token_t *token, size_t start) {
  // NOTE: this actually sets the END of the contents to the start value.
  // This overly-clever trick saves us allocating another field in token_t.
  // Read token_set_contents_end() how we use this.
  mu_str_slice(&token->contents, &token->contents, 0, start);
}

static void token_set_contents_end(mu_jtree_token_t *token, size_t end) {
  mu_str_t *str = &token->contents;
  // Note: length was set in a previous call to token_set_contents_start().
  // length now becomes the start, and length becomes end - start
  size_t start = mu_str_length(str);
  mu_str_init(str, &mu_str_bytes(str)[start], end - start);
}

static mu_jtree_err_t close_object(parser_t *parser, mu_jtree_t *jtree) {
  return close_container(parser, jtree, MU_JTREE_TYPE_OBJECT);
}

static mu_jtree_err_t close_array(parser_t *parser, mu_jtree_t *jtree) {
  return close_container(parser, jtree, MU_JTREE_TYPE_ARRAY);
}

static mu_jtree_err_t close_container(parser_t *parser, mu_jtree_t *jtree,
                                      mu_jtree_token_type_t type) {
  // Here when a '}' or ']' is spotted.  Search backwards through the array
  // of tokens to find the nearest "unclosed" container token, and assure that
  // the container type (object or array) matches the closing character.

  mu_jtree_token_t *token = find_open_container(parser, jtree);
  if (token == NULL) {
    return MU_JTREE_ERR_INVALID;

  } else if (token->type != type) {
    // cannot interleave { ... ] or [ ... }
    return MU_JTREE_ERR_INVALID;

  } else {
    // Include the closing '}' or ']' in the token contents.  Set
    // link to the number of tokens spanned by the container.
    token_set_contents_end(token, parser->char_idx + 1);
    token->link = parser->token_count - token_index(jtree, token);
    return MU_JTREE_ERR_NONE;
  }
}

static mu_jtree_token_t *find_open_container(parser_t *parser,
                                             mu_jtree_t *jtree) {
  if (parser->token_index == 0) {
    // no tokens at all
    return NULL;
  }
  size_t idx = parser->token_index - 1;

  while (1) {
    mu_jtree_token_t *token = &jtree->tokens[idx];
    if (token->link == TOKEN_LINK_OPEN) {
      return token;
    } else if (idx == 0) {
      return NULL;
    } else {
      idx -= 1;
    }
  }
}

static bool token_is_container(mu_jtree_token_t *token) {
  mu_jtree_token_type_t type = token->type;
  return (type == MU_JTREE_TYPE_ARRAY) || (type == MU_JTREE_TYPE_OBJECT);
}

static size_t token_index(mu_jtree_t *jtree, mu_jtree_token_t *token) {
  // This works because jtree->tokens is a dense array of tokens.
  return (token - jtree->tokens) / sizeof(mu_jtree_token);
}

typedef enum {
    SS_TOP,
    SS_ESC,
    SS_U,
    SS_U1,
    SS_U2,
    SS_U3,
    SS_ERR,
    SS_DONE,
} string_state_t;

static mu_jtree_err_t parse_string(parser_t *parser, mu_jtree_t *jtree,
                                   mu_str_t *json) {
  // Here when an opening '"' spotted.  Collect everything up to closing '"'
  uint8_t c;
  size_t start = parser->char_idx++; // capture starting char index
  string_state_t ss = SS_TOP;

  while ((ss != SS_ERR) && (ss != SS_DONE)) {
    if (get_char(parser, json, &c) == false) {
      // ran off end of string
      ss = SS_ERR;
    }
    switch (ss) {
    case SS_TOP: {
      // inside toplevel quote
      if (c < ' ') {
        ss = SS_ERR; // control char cannot appear within string
      } else if (c == '\\') {
        ss = SS_ESC; // backslash starts an escape sequene
      } else if (c == '\"') {
        ss = SS_DONE; // closing quote finishes string
      } else {
        // stay in this state
      }
    } break;
    case SS_ESC: {
      // previous char was backslash
      if (char_is_member(c, "\"\\/bfnrt")) {
        // valid single-escaped char
        ss = ss_TOP;
      } else if (c == 'u') {
        // unicode lead-in: expect 4 chars
        ss = ss_U;
      } else {
        ss = SS_ERR; // illegal excape char
      }
    } break;
    case SS_U: {
      // previous chars were \u - expect 4 hex digits
      if (char_is_hex(c)) {
        ss = SS_U1;
      } else {
        ss = SS_ERR;
      }
    } break;
    case SS_U1: {
      // previous chars were \uX - expect 3 hex digits
      if (char_is_hex(c)) {
        ss = SS_U2;
      } else {
        ss = SS_ERR;
      }
    } break;
    case SS_U2: {
      // previous chars were \uXX - expect 2 hex digits
      if (char_is_hex(c)) {
        ss = SS_U3;
      } else {
        ss = SS_ERR;
      }
    } break;
    case SS_U3: {
      // previous chars were \uXXX - expect 1 hex digit
      if (char_is_hex(c)) {
        ss = SS_TOP;
      } else {
        ss = SS_ERR;
      }
    } break;
    case SS_ERR: {
      // terminal state: encountered an error
    } break;
    case SS_DONE: {
      // terminal state: success
    } break;
    } // switch
  }   // while()

  if (ss == SS_ERR) {
    return MU_JTREE_ERR_INVALID;
  } else if ((token = token_alloc(parser, jtree)) == NULL) {
    return MU_JTREE_ERR_ALLOC;
  } else {
    // TODO: set link field of previous token
    token_init(token, json, MU_JTREE_TYPE_STRING);
    mu_str_slice(&token->contents, &token->contents, start, parser->char_idx);
    return MU_JTREE_ERR_NONE;
  }
}

static bool char_is_member(uint8_t c, const char *chars) {
  for (i = 0; i < strlen(chars); i++) {
    if (c == chars[i]) {
      return true;
    }
  }
  return false;
}

static bool char_is_hex(uint8_t c) {
  return char_is_member(c, "0123456789abcdefABCDEF");
}

static bool char_is_number_prefix(c) {
    return char_is_member(c, "01234567890-");
}

typedef enum {
    NS_MI,
    NS_ZE,
    NS_IN,
    NS_F0,
    NS_FR,
    NS_E1,
    NS_E2,
    NS_e3,
    NS_DONE_FLOAT,
    NS_DONE_INT,
} number_state_t;

static mu_jtree_err_t parse_number(parser_t *parser, mu_jtree_t *jtree,
                                   mu_str_t *json, uint8_t c) {
  // Here when a number prefix was spotted.  Collect the number and create a
  // token for it.
  string_state_t ns;

  // Set initial state based on prefix.
  if (c == '-') {
    ns = NS_MI;
  } else if (c == '0') {
    ns = NS_ZE;
  } else {
    ns = NS_IN;
  }
  // capture starting char index
  size_t start = parser->char_idx;

  while (!is_terminal_ns_state(ns)) {
    if (get_char(parser, json, &c) == false) {
      // ran off end of string
      ns = NS_ERR;
    }
    switch (ns) {
    case NS_MI: {
        if (c == '0') {
            ns = NS_ZE;
        } else if (is_decimal(c)) {
            ns = NS_IN;
        } else {
            ns = NS_ERR;
        }
    } break;
    case NS_ZE: {

    } break;
    case NS_TOP: {
        // what was the character that got us here?
        uint8_t prefix = mu_str_bytes(json)[parser->char_idex - 1];
        if (prefix == '-') {
            ns = NS_MINUS;
        } else if (prefix == '0') {
            ns = NS_ZERO;
        } else {

        }

    }
    } // switch
  }   // while
}

// **************************************************
// parser accessors

static void parser_init(mu_jtree_t *jtree, mu_str_t *json) {
  s_parser.jtree = jtree;
  s_parser.jtree->token_count = 0;
  s_parser.json = json;
  s_parser.char_idx = 0;
}

static size_t parser_token_capacity(void) {
  return s_parser.jtree->token_capacity;
}

static size_t parser_token_count(void) { return s_parser.jtree->token_count; }

static mu_jtree_token_t *parser_tokens(void) { return parser_token_ref(0); }

static mu_jtree_token_t *parser_token_ref(size_t idx) {
  return &s_parser.jtree->tokens[idx];
}

static size_t parser_token_index(mu_jtree_token_t *token) {
  return (parser_tokens() - token) / sizeof(mu_jtree_token_t);
}

static uint8_t parser_token_offset(mu_jtree_token_t *token, size_t offset) {
  size_t idx = parser_token_index(token) + offset;
  return parse_token_ref(idx);
}

static mu_jtree_token_t *parser_token_alloc(mu_jtree_token_type_t type) {
  if (parser_token_count() >= parser_token_capacity()) {
    return NULL;
  } else {
    mu_jtree_token_t *token = parser_token_ref(s_parser.jtree->token_count++);
    return token_init(token, s_parser.json, type);
  }
}

static size_t parser_json_length(void) { return mu_str_length(s_parser.json); }

static size_t parser_json_char_index(void) { return s_parser.char_idx; }

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

static mu_jtree_token_t *token_init(mu_jtree_token_t *token, mu_str_t *json,
                                    mu_jtree_token_type_t type) {
  token->type = type;
  token->element_count = 0;
  // Initialize token's contents to span the entire JSON string.  This will be
  // adjusted in token_set_contents_start() and token_set_contents_end().
  mu_str_copy(&token->contents, json);
  return token;
}

static void token_set_element_count(mu_jtree_token_t *token,
                                    size_t element_count) {
  token->element_count = element_count;
}

static bool token_container_is_open(mu_jtree_token_t *token) {
  return token->element_count == 0;
}

// **************************************************
// Accessing parser state

static bool is_whitespace(uint8_t c) {
  return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

static mu_jtree_err_t open_object(void) {
  return open_container(MU_JTREE_TYPE_OBJECT);
}

static mu_jtree_err_t open_array(void) {
  return open_container(MU_JTREE_TYPE_ARRAY);
}

static mu_jtree_err_t open_container(mu_jtree_token_type_t type) {
  // Here when a '{' or '[' is spotted
  mu_jtree_token_t *token = parser_token_alloc(type);
  if (token == NULL) {
    // ran out of tokens
    return MU_JTREE_ERR_ALLOC;
  }
  // Capture the vital stats for this token:
  // - set  element_length to zero to signify "unclosed" container.
  // - capture starting character index for this token,
  // ... then consume the character.
  token_set_element_count(token, 0);
  token_set_contents_start(token, parser_json_char_index());
  parser_get_json_char();
  return MU_JTREE_ERR_NONE;
}

static mu_jtree_err_t close_object(void) {
  return close_container(MU_JTREE_TYPE_OBJECT);
}

static mu_jtree_err_t close_array(void) {
  return close_container(MU_JTREE_TYPE_ARRAY);
}

static mu_jtree_err_t close_container(mu_jtree_token_type_t type) {
  // Here when a '}' or ']' is spotted.  Search backwards through the array
  // of tokens to find the nearest "unclosed" container token, and assure that
  // the container type (object or array) matches the closing character.
  mu_jtree_token_t *token = find_open_container();

  if (token == NULL) {
    // Not inside an open container.
    return MU_JTREE_ERR_INVALID;

  } else if (mu_jtree_token_type(token) != type) {
    // cannot interleave { ... ] or [ ... }
    return MU_JTREE_ERR_INVALID;

  } else {
    // Include the closing '}' or ']' in the token contents.  Set
    // the element count to the number of tokens spanned by the
    // container.
    parser_get_json_char();
    token_set_contents_end(token, parser_json_char_index());
    token_set_element_count(token, parser_token_count() - i);
    return MU_JTREE_ERR_NONE;
  }
}

static mu_jtree_token_t *find_open_container(void) {
  for (int i = parser_token_count() - 1; i >= 0; i--) {
    mu_jtree_token_t *token = parser_token_ref(i);
    if (token_container_is_open(token)) {
      return token;
    }
  }
  return NULL;
}

static mu_jtree_err_t parse_string(void) { return MU_JTREE_ERR_INVALID; }

static bool is_number_prefix(uint8_t c) { return false; }

static mu_jtree_err_t parse_comma(void) {
  // Here when a comma is found.  If not inside a container ()
  return MU_JTREE_ERR_INVALID;
}

static mu_jtree_err_t parse_colon(void) { return MU_JTREE_ERR_INVALID; }

static mu_jtree_err_t parse_number(void) { return MU_JTREE_ERR_INVALID; }

static mu_jtree_err_t parse_primitive(void) { return MU_JTREE_ERR_INVALID; }

// *****************************************************************************
// *****************************************************************************
// Standalone Unit Tests
// *****************************************************************************
// *****************************************************************************

// Run this command in a shell to run the standalone tests.
/*
gcc -g -Wall -DTEST_MU_JTREE -I../.. -o test_mu_jtree mu_jtree.c \
../core/mu_str.c && ./test_mu_jtree && rm ./test_mu_jtree
*/

#ifdef TEST_MU_JTREE

#include <stdio.h>
#include <string.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

#define MAX_TOKENS 20
static mu_jtree_token_t s_tokens[MAX_TOKENS];

/**
 * @brief Return true if each the type of each token in jtree matches the
 * corresponding elements of the types array.
 */
static bool check_token_types(mu_jtree_t *jtree, mu_jtree_token_type_t *types,
                              size_t count) {
  for (size_t i = 0; i < count; i++) {
    mu_jtree_token_t *token = mu_jtree_token_ref(jtree, i);
    if (mu_jtree_token_type(token) != types[i]) {
      // match failed
      return false;
    }
  }
  // all elements matched
  return true;
}

/**
 * @brief Return true if each the element length of each token in jtree matches
 * corresponding member of the lengths array.
 */
static bool check_element_lengths(mu_jtree_t *jtree, size_t *lengths,
                                  size_t count) {
  for (size_t i = 0; i < count; i++) {
    mu_jtree_token_t *token = mu_jtree_token_ref(jtree, i);
    if (mu_jtree_token_element_count(token) != lengths[i]) {
      // match failed
      return false;
    }
  }
  // all elements matched
  return true;
}

static bool check_element_contents(mu_jtree_t *jtree, const char *strings[],
                                   size_t count) {
  mu_str_t expect;

  for (size_t i = 0; i < count; i++) {
    mu_jtree_token_t *token = mu_jtree_token_ref(jtree, i);
    mu_str_init_cstr(&expect, strings[i]);
    if (mu_str_compare(mu_jtree_token_contents(token), &expect) != 0) {
      // match failed
      return false;
    }
  }
  // all elements matched
  return true;
}
static void test_mu_jtree_parse(void) {
  mu_jtree_t jtree;
  mu_str_t str;
  size_t token_count;

  printf("\nStarting test_mu_jtree_parse...");
  fflush(stdout);

  mu_str_init_cstr(&str, "[]");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_ARRAY}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"[]"}, token_count));

  mu_str_init_cstr(&str, "{}");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_OBJECT}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"{}"}, token_count));

  mu_str_init_cstr(&str, "\"a\"");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_STRING}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(
      check_element_contents(&jtree, (const char *[]){"\"a\""}, token_count));

  mu_str_init_cstr(&str, "1");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"1"}, token_count));

  mu_str_init_cstr(&str, "true");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"true"}, token_count));

  mu_str_init_cstr(&str, "false");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(
      check_element_contents(&jtree, (const char *[]){"false"}, token_count));

  mu_str_init_cstr(&str, "null");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"null"}, token_count));

  mu_str_init_cstr(&str, "[1, 2, 3]");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 4);
  ASSERT(check_token_types(&jtree,
                           (mu_jtree_token_type_t[]){
                               MU_JTREE_TYPE_ARRAY,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_PRIMITIVE,
                           },
                           token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){4, 1, 1, 1}, token_count));
  ASSERT(check_element_contents(
      &jtree, (const char *[]){"[1, 2, 3]", "1", "2", "3"}, token_count));

  mu_str_init_cstr(&str, "{\"a\":1, \"b\":true, \"c\":\"a string\"}");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 10);
  ASSERT(check_token_types(&jtree,
                           (mu_jtree_token_type_t[]){
                               MU_JTREE_TYPE_OBJECT,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_STRING,
                           },
                           token_count));
  ASSERT(check_element_lengths(&jtree, (size_t[]){7, 1, 1, 1, 1, 1, 1},
                               token_count));
  ASSERT(check_element_contents(
      &jtree,
      (const char *[]){"{\"a\":1, \"b\":true, \"c\":\"a string\"}", "\"a\"",
                       "1", "\"b\"", "true", "\"c\"", "\"a string\""},
      token_count));

  mu_str_init_cstr(&str, "[1, {\"a\":2}, 3]");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 6);
  ASSERT(check_token_types(&jtree,
                           (mu_jtree_token_type_t[]){
                               MU_JTREE_TYPE_ARRAY,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_OBJECT,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_STRING,
                           },
                           token_count));
  ASSERT(
      check_element_lengths(&jtree, (size_t[]){6, 1, 3, 1, 1, 1}, token_count));
  ASSERT(
      check_element_contents(&jtree,
                             (const char *[]){"[1, {\"a\":2}, 3]", "1",
                                              "{\"a\":2}", "\"a\"", "2", "3"},
                             token_count));

  mu_str_init_cstr(&str, "{\"a\":1, \"b\":[4, 5, 6], \"c\":\"a string\"}");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 10);
  ASSERT(check_token_types(&jtree,
                           (mu_jtree_token_type_t[]){
                               MU_JTREE_TYPE_OBJECT,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_ARRAY,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_PRIMITIVE,
                               MU_JTREE_TYPE_STRING,
                               MU_JTREE_TYPE_STRING,
                           },
                           token_count));
  ASSERT(check_element_lengths(
      &jtree, (size_t[]){10, 1, 1, 1, 4, 1, 1, 1, 1, 1}, token_count));
  ASSERT(check_element_contents(
      &jtree,
      (const char *[]){"{\"a\":1, \"b\":[4, 5, 6], \"c\":\"a string\"}",
                       "\"a\"", "1", "\"b\"", "[4, 5, 6]", "4", "5", "6",
                       "\"c\"", "\"a string\""},
      token_count));

  mu_str_init_cstr(&str, "[1, [2, [3, [4], 5], 6], 7]");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 11);
  ASSERT(check_token_types(&jtree,
                           (mu_jtree_token_type_t[]){
                               MU_JTREE_TYPE_ARRAY,     // [
                               MU_JTREE_TYPE_PRIMITIVE, // 1
                               MU_JTREE_TYPE_ARRAY,     // [
                               MU_JTREE_TYPE_PRIMITIVE, // 2
                               MU_JTREE_TYPE_ARRAY,     // [
                               MU_JTREE_TYPE_PRIMITIVE, // 3
                               MU_JTREE_TYPE_ARRAY,     // [
                               MU_JTREE_TYPE_PRIMITIVE, // 4
                               MU_JTREE_TYPE_PRIMITIVE, // 5
                               MU_JTREE_TYPE_PRIMITIVE, // 6
                               MU_JTREE_TYPE_PRIMITIVE, // 7
                           },
                           token_count));
  ASSERT(check_element_lengths(
      &jtree, (size_t[]){11, 1, 8, 1, 5, 1, 2, 1, 1, 1}, token_count));
  ASSERT(check_element_contents(&jtree,
                                (const char *[]){"[1, [2, [3, [4], 5], 6], 7]",
                                                 "1", "[2, [3, [4], 5], 6]",
                                                 "2", "[3, [4], 5]", "3", "[4]",
                                                 "4", "5", "6", "7"},
                                token_count));

  printf("\n...test_mu_jtree_parse complete\n");
}

static void test_mu_jtree_parse_pathologies(void) {
  mu_jtree_t jtree;
  mu_str_t str;

  printf("\nStarting test_mu_jtree_parse_pathologies...");
  fflush(stdout);

  // empty string is not JSON
  mu_str_init_cstr(&str, "");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_INVALID);

  // missing close bracket
  mu_str_init_cstr(&str, "[1, 2, 3");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_INCOMPLETE);

  // missing close bracket
  mu_str_init_cstr(&str, "{");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_INCOMPLETE);

  // member name must be a string
  mu_str_init_cstr(&str, "{1:2, 3:4}");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_INVALID);

  printf("\n...test_mu_jtree_parse_pathologies complete\n");
}

static void test_mu_jtree_token_fns(void) {
  printf("\nStarting test_mu_jtree_token_fns...");
  fflush(stdout);
  mu_jtree_token_t tok;

  // slightly cheating: construct token manually for testing...
  tok.type = MU_JTREE_TYPE_ARRAY;
  tok.element_count = 3;
  mu_str_init_cstr(&tok.contents, "woof");

  ASSERT(mu_jtree_token_type(&tok) == MU_JTREE_TYPE_ARRAY);
  ASSERT(mu_jtree_token_element_count(&tok) == 3);
  ASSERT(mu_jtree_token_contents(&tok) == &tok.contents);

  printf("\n...test_mu_jtree_token_fns complete\n");
}

int main(void) {
  test_mu_jtree_parse();
  test_mu_jtree_parse_pathologies();
  test_mu_jtree_token_fns();
}

#endif

// *****************************************************************************
// End of file
