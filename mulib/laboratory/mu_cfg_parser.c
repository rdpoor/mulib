/**
 * @file mu_cfg_parser.c
 *
 * MIT License
 *
 * Copyright (c) 2020 R. D. Poor <rdpoor@gmail.com>
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

// *****************************************************************************
// Includes

#include "mu_cfg_parser.h"
#include <limits.h>
#include <string.h>

#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) forward declarations

static bool is_whitespace(char ch);

// static char *print_mu_str(mu_str_t *str); // debugging

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

mu_cfg_parser_t *mu_cfg_parser_init(mu_cfg_parser_t *parser,
                                    const mu_cfg_parser_token_t *tokens,
                                    size_t n_tokens, bool is_case_sensitive) {
  parser->tokens = tokens;
  parser->n_tokens = n_tokens;
  if (is_case_sensitive) {
    parser->cmp_fn = strncmp;
  } else {
    parser->cmp_fn = strncasecmp;
  }
  return parser;
}

mu_cfg_parser_err_t mu_cfg_parser_read_line(mu_cfg_parser_t *parser,
                                            mu_str_t *line) {
  mu_str_t key, value, trimmed;
  size_t idx;
  size_t key_len;

  mu_str_copy(&trimmed, line); // don't modify line...

  // remove '#' comment char and any chars that follow it
  idx = mu_str_index(&trimmed, '#');
  if (idx != MU_STR_NOT_FOUND) {
    mu_str_slice(&trimmed, &trimmed, 0, idx);
  }

  // strip leading and trailing whitespace
  mu_str_trim(&trimmed, is_whitespace);

  if (mu_str_available_rd(&trimmed) == 0) {
    // Line is empty after removing comment and whitespace
    return MU_CFG_PARSER_ERR_NONE;
  }

  // find '=' that separates key and value
  idx = mu_str_index(&trimmed, '=');
  if (idx == MU_STR_NOT_FOUND) {
    return MU_CFG_PARSER_ERR_BAD_FMT;
  }

  // split the line on either side of the = sign
  mu_str_slice(&key, &trimmed, 0, idx);
  mu_str_slice(&value, &trimmed, idx + 1, INT_MAX);

  // strip leading and trailing whitespace around the key and value
  mu_str_trim(&key, is_whitespace);
  mu_str_trim(&value, is_whitespace);

  // key cannot be empty (but value may be).
  key_len = mu_str_available_rd(&key);
  if (key_len == 0) {
    return MU_CFG_PARSER_ERR_BAD_FMT;
  }

  for (int i = 0; i < parser->n_tokens; i++) {
    const mu_cfg_parser_token_t *token = &parser->tokens[i];
    const char *candidate = token->key;

    if ((strlen(candidate) == key_len) &&
        (parser->cmp_fn(candidate, (const char *)mu_str_ref_rd(&key),
                        key_len) == 0)) {
      // got a match -- invoke the parser
      token->parser_fn(&value, token->arg);
      return MU_CFG_PARSER_ERR_NONE;
    }
  }
  // no matching key was found
  return MU_CFG_PARSER_ERR_KEY_NOT_FOUND;
}

// *****************************************************************************
// Local (private, static) code

static bool is_whitespace(char ch) {
  return (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r');
}

// debugging -- NOTE: returned value gets overwritten at next call
// static char *print_mu_str(mu_str_t *str) {
//   static char buf[50];
//   memset(buf, '\0', sizeof(buf));
//   memcpy(buf, mu_str_ref_rd(str), mu_str_available_rd(str));
//   return buf;
// }

// *****************************************************************************
// standalone test

/*
(gcc -DMU_CFG_PARSER_STANDALONE_TEST -Wall -g -I../core -o mu_cfg_parser \
   mu_cfg_parser.c ../core/mu_str.c ../core/mu_strbuf.c \
   && ./mu_cfg_parser \
   && rm ./mu_cfg_parser)
*/

#ifdef MU_CFG_PARSER_STANDALONE_TEST

#include <assert.h>
#include <stdio.h>
#define ASSERT assert

typedef enum {
  FRUIT_TYPE = 1,
  WEIGHT_TYPE,
  COLOR_TYPE,
} key_type_t;

static void string_parser(mu_str_t *value, const void *arg);

static const mu_cfg_parser_token_t s_tokens[] = {
    {"fruit", (void *)FRUIT_TYPE, string_parser},
    {"weight", (void *)WEIGHT_TYPE, string_parser},
    {"color", (void *)COLOR_TYPE, string_parser}};

static mu_str_t *s_matched_value;
static const void *s_matched_key_type;

static const size_t N_TOKENS = sizeof(s_tokens) / sizeof(mu_cfg_parser_token_t);

static void string_parser(mu_str_t *value, const void *arg) {
  s_matched_value = value;
  s_matched_key_type = arg;
}

static bool matched_value_equals(const char *str) {
  return strncmp(str, (const char *)mu_str_ref_rd(s_matched_value),
                 mu_str_available_rd(s_matched_value)) == 0;
}

static mu_cfg_parser_err_t parser_test(mu_cfg_parser_t *parser,
                                       const char *line) {
  mu_strbuf_t strbuf;
  mu_str_t str;

  fflush(stdout);

  mu_strbuf_init_from_cstr(&strbuf, line);
  mu_str_init_rd(&str, &strbuf);
  return mu_cfg_parser_read_line(parser, &str);
}

int main(void) {
  mu_cfg_parser_t parser;
  printf("Beginning standalone tests...\n");

  // Case sensitive parser.
  ASSERT(mu_cfg_parser_init(&parser, s_tokens, N_TOKENS, true) == &parser);

  // a blank line is legal
  ASSERT(parser_test(&parser, "") == MU_CFG_PARSER_ERR_NONE);

  // a line with only a comment is legal
  ASSERT(parser_test(&parser, "# a line containing only a comment") ==
         MU_CFG_PARSER_ERR_NONE);

  // legitimate key/value
  ASSERT(parser_test(&parser, "fruit=apple") == MU_CFG_PARSER_ERR_NONE);
  ASSERT(matched_value_equals("apple"));
  ASSERT(s_matched_key_type == (const void *)FRUIT_TYPE);

  // legitimate key/value with whitespace
  ASSERT(parser_test(&parser, " weight = 23 ") == MU_CFG_PARSER_ERR_NONE);
  ASSERT(matched_value_equals("23"));
  ASSERT(s_matched_key_type == (const void *)WEIGHT_TYPE);

  // legitimate key/value with trailing comment
  ASSERT(parser_test(&parser, " color = red # trailing comment") ==
         MU_CFG_PARSER_ERR_NONE);
  ASSERT(matched_value_equals("red"));
  ASSERT(s_matched_key_type == (const void *)COLOR_TYPE);

  // missing = delimeter
  ASSERT(parser_test(&parser, " color") == MU_CFG_PARSER_ERR_BAD_FMT);

  // not fooled by trailing comment
  ASSERT(parser_test(&parser, " color # = red") == MU_CFG_PARSER_ERR_BAD_FMT);

  // unrecognized key
  ASSERT(parser_test(&parser, " price = 15") ==
         MU_CFG_PARSER_ERR_KEY_NOT_FOUND);

  // missing key
  ASSERT(parser_test(&parser, " = 15") == MU_CFG_PARSER_ERR_BAD_FMT);

  // =====
  // Case insensitive parser.
  ASSERT(mu_cfg_parser_init(&parser, s_tokens, N_TOKENS, false) == &parser);

  // legitimate key/value
  ASSERT(parser_test(&parser, "fruit=apple") == MU_CFG_PARSER_ERR_NONE);
  ASSERT(matched_value_equals("apple"));
  ASSERT(s_matched_key_type == (const void *)FRUIT_TYPE);

  // legitimate key/value
  ASSERT(parser_test(&parser, "FRUIT=apple") == MU_CFG_PARSER_ERR_NONE);
  ASSERT(matched_value_equals("apple"));
  ASSERT(s_matched_key_type == (const void *)FRUIT_TYPE);

  printf("...done\n");
}

#endif
