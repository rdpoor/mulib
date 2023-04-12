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
 *   https://www.json.org/JSON_checker/
 *
 * Implementation Overview:
 *
 * mu_jtree parses a JSON string into structured JSON tokens.  Its main design
 * goals:
 *
 * * static memory efficient: a compact state table drives the main operation
 * * dynamic memory efficient: does not use recursive calls
 * * predictable: No calls to malloc() or free() -- user supplies storage.
 * * fast: uses zero-copy tokens into the original JSON string
 *
 * How it works:
 *
 * The user supplies the JSON string to be parsed along with an array for token
 * storage.  After calling mu_jtree_parser(), each token contains:
 *
 * token_type: one of ARRAY, OBJECT, STRING, INTEGER, FLOAT, TRUE, FALSE, NULL
 * token_contents: a substring referring directly into the user-supplied JSON
 * token_link: a link field that defines # of children for this token
 *
 * The resulting token array can be traversed linearly with mu_jtree_token_ref()
 * for a depth-first tour of the parsed tokens, or you can use the structured
 * token traversal calls:
 *
 * * mu_jtree_siblings: return the next token on this level
 * * mu_jtree_children: return the first child of this token
 * * mu_jtree_parent: return the parent of this token
 * * mu_jtree_root: return the top-level token (contains the entire JSON string)
 *
 * The parser maintains three variables:
 *
 * state - current state in the state transition table (qv)
 * char_index - index into the user-supplied JSON string
 * token_count - the number of tokens allocated from the user-supplied tokens
 *
 * Rather than using the program stack for recursive calls, mu_jtree uses the
 * list of currently allocated tokens to discover the current depth, the parent
 * of the current token, etc.
 *
 */

// *****************************************************************************
// Includes

#include "mu_jtree.h"
#include "mulib/core/mu_str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// *****************************************************************************
// Private types and definitions

#define TOKEN_LINK_IS_OPEN PTRDIFF_MAX

#include <stdint.h>
#include <stdio.h>

#define TRUE  1
#define FALSE 0
#define GOOD 0xBABAB00E
#define __   -1     /* the universal error code */

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/

typedef enum {
    C_SPACE,  /* space */
    C_WHITE,  /* other whitespace */
    C_LCURB,  /* {  */
    C_RCURB,  /* } */
    C_LSQRB,  /* [ */
    C_RSQRB,  /* ] */
    C_COLON,  /* : */
    C_COMMA,  /* , */
    C_QUOTE,  /* " */
    C_BACKS,  /* \ */
    C_SLASH,  /* / */
    C_PLUS,   /* + */
    C_MINUS,  /* - */
    C_POINT,  /* . */
    C_ZERO ,  /* 0 */
    C_DIGIT,  /* 123456789 */
    C_LOW_A,  /* a */
    C_LOW_B,  /* b */
    C_LOW_C,  /* c */
    C_LOW_D,  /* d */
    C_LOW_E,  /* e */
    C_LOW_F,  /* f */
    C_LOW_L,  /* l */
    C_LOW_N,  /* n */
    C_LOW_R,  /* r */
    C_LOW_S,  /* s */
    C_LOW_T,  /* t */
    C_LOW_U,  /* u */
    C_ABCDF,  /* ABCDF */
    C_E,      /* E */
    C_ETC,    /* everything else */
    NR_CLASSES
} char_class_t;

/*
    The state codes.
*/
typedef enum {
    GO,  /* start     */
    OK,  /* ok        */
    OB,  /* object    */
    KE,  /* key       */
    CO,  /* colon     */
    VA,  /* value     */
    AR,  /* array     */
    ST,  /* string    */
    ES,  /* escape    */
    U1,  /* u1        */
    U2,  /* u2        */
    U3,  /* u3        */
    U4,  /* u4        */
    MI,  /* minus     */
    ZE,  /* zero      */
    IN,  /* integer   */
    FR,  /* fraction  */
    FS,  /* fractions */
    E1,  /* e         */
    E2,  /* ex        */
    E3,  /* exp       */
    T1,  /* tr        */
    T2,  /* tru       */
    T3,  /* true      */
    F1,  /* fa        */
    F2,  /* fal       */
    F3,  /* fals      */
    F4,  /* false     */
    N1,  /* nu        */
    N2,  /* nul       */
    N3,  /* null      */
    NR_STATES
} state_t;

typedef struct {
  mu_jtree_t *jtree; // user supplied token array
  mu_str_t *json;    // the JSON string being parsed
  state_t state;     // the current state
  int char_idx;      // index into input string.
  int depth;         // current depth
} parser_t;

// *****************************************************************************
// Private (static) storage

static const int8_t s_ascii_classes[128] = {
/*
    This array maps the 128 ASCII characters into character classes.
    The remaining Unicode characters should be mapped to C_ETC.
    Non-whitespace control characters are errors.
*/
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      C_WHITE, C_WHITE, __,      __,      C_WHITE, __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,

    C_SPACE, C_ETC,   C_QUOTE, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_PLUS,  C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_ZERO,  C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT, C_DIGIT, C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

    C_ETC,   C_ABCDF, C_ABCDF, C_ABCDF, C_ABCDF, C_E,     C_ABCDF, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LSQRB, C_BACKS, C_RSQRB, C_ETC,   C_ETC,

    C_ETC,   C_LOW_A, C_LOW_B, C_LOW_C, C_LOW_D, C_LOW_E, C_LOW_F, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_LOW_L, C_ETC,   C_LOW_N, C_ETC,
    C_ETC,   C_ETC,   C_LOW_R, C_LOW_S, C_LOW_T, C_LOW_U, C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LCURB, C_ETC,   C_RCURB, C_ETC,   C_ETC
};

static const uint8_t s_states[] = {
/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is represented as a
    negative number. A JSON text is accepted if at the end of the text the
    state is OK and if the mode is MODE_DONE.
    sS
    sE
    oS
    oE
    aS
    aE
    nS
    nE
    lS
    lE


               white                                      1-9                                   ABCDF  etc
           space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ GO,GO,-6,__,-5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*ok     OK*/ OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*object OB*/ OB,OB,__,-8,__,__,__,__,-9,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*key    KE*/ KE,KE,__,__,__,__,__,__,-9,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*colon  CO*/ CO,CO,__,__,__,__,-2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*value  VA*/ VA,VA,-6,__,-5,__,__,__,-9,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__,
/*array  AR*/ AR,AR,-6,__,-5,-7,__,__,-9,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__,
/*string ST*/ ST,__,ST,ST,ST,ST,ST,ST,-4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,
/*escape ES*/ __,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__,
/*u1     U1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__,
/*u2     U2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__,
/*u3     U3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__,
/*u4     U4*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__,
/*minus  MI*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*zero   ZE*/ OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,__,__,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__,
/*int    IN*/ OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__,
/*frac   FR*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,FS,FS,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*fracs  FS*/ OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,FS,FS,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__,
/*e      E1*/ __,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*ex     E2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*exp    E3*/ OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*tr     T1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__,
/*tru    T2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__,
/*true   T3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__,
/*fa     F1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*fal    F2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__,
/*fals   F3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__,
/*false  F4*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__,
/*nu     N1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__,
/*nul    N2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__,
/*null   N3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,
};


// *****************************************************************************
// Private (static, forward) declarations

static mu_jtree_err_t parse_json(mu_jtree_t *jtree, mu_str_t *json);

static mu_jtree_err_t parse_char(parser_t *parser, uint8_t c);

static uint8_t get_char(parser_t *parser, mu_str_t *json, uint8_t *ch);

// parser accessors

static parser_t *parser_init(parser_t *parser,
                             mu_jtree_t *jtree,
                             mu_str_t *json,
                             state_t initial_state);

static mu_jtree_t parser_jtree(parser_t *parser);

static mu_str_t *parser_json(parser_t *parser);

static state_t parser_state(parser_t *parser);

static int parser_char_index(parser_t *parser);

static int parser_token_count(parser_t *parser);

static mu_jtree_token_t *parser_tokens(parser_t *parser);

static int parser_token_capacity(parser_t *parser);

static int parser_token_count(parser_t *parser);

static int parser_depth(parser_t *parser);

static void parser_increase_depth(parser_t *parser);

static void parser_decrease_depth(parser_t *parser);

// Token access & tree walking

static mu_jtree_token_t *token_alloc(parser_t *parser);

static mu_jtree_token_t *token_init(mu_jtree_token_t *token,
                                    mu_jtree_token_type_t type,
                                    mu_str_t json,
                                    int depth);

static mu_jtree_token_t *token_stack_ref(parser_t *parser, int idx);

static mu_jtree_token_t *token_stack_top(parser_t *parser);

static mu_jtree_token_t *token_stack_root(parser_t *parser);

static int token_stack_index(parser_t *parser, mu_jtree_token_t *token);

static mu_jtree_token_t *token_sibling(parser_t *parser,
                                       mu_jtree_token_t *token);

static mu_jtree_token_t *token_children(parser_t *parser,
                                        mu_jtree_token_t *token);

static mu_jtree_token_t *token_parent(parser_t *parser,
                                      mu_jtree_token_t *token);

static void token_set_contents_start(mu_jtree_token_t *token, int start);

static void token_set_contents_end(mu_jtree_token_t *token, int end);

static bool token_is_container(mu_jtree_token_t *token);

static mu_jtree_err_t open_array(parser_t *parser);
static mu_jtree_err_t open_object(parser_t *parser);
static mu_jtree_err_t open_container(parser_t *parser,
                                     mu_jtree_token_type_t type);

static mu_jtree_err_t close_array(parser_t *parser);
static mu_jtree_err_t close_object(parser_t *parser);
static mu_jtree_err_t close_container(parser_t *parser,
                                      mu_jtree_token_type_t type);

static mu_jtree_err_t open_string(parser_t *parser);
static mu_jtree_err_t close_string(parser_t *parser);

static int8_t char_class(uint8_t c);

// *****************************************************************************
// Public code

mu_jtree_t *mu_jtree_init(mu_jtree_t *jtree,
                          mu_jtree_token_t *tokens,
                          int capacity) {
  jtree->tokens = tokens;
  jtree->token_capacity = capacity;
  jtree->token_count = 0;
  return jtree;
}

mu_jtree_err_t mu_jtree_parse(mu_jtree_t *jtree,
                              const uint8_t *json,
                              int length) {
  mu_str_t str;
  return mu_jtree_parse_mu_str(jtree, mu_str_init(&str, json, length));
}

mu_jtree_err_t mu_jtree_parse_cstr(mu_jtree_t *jtree, const char *json) {
  mu_str_t str;
  return mu_jtree_parse_mu_str(
      jtree, mu_str_init(&str, (uint8_t *)json, strlen(json)));
}

mu_jtree_err_t mu_jtree_parse_mu_str(mu_jtree_t *jtree, mu_str_t *json) {
  return parse_json(jtree, json);
}

bool mu_jtree_match(mu_jtree_t *target,
                    mu_jtree_t *pattern,
                    bool allow_extras) {
  // TODO: TBD
  return false;
}

// *****************************************************************************
// Private (static) code

static mu_jtree_err_t parse_json(mu_jtree_t *jtree, mu_str_t *json) {
  mu_jtree_err_t err = MU_JTREE_ERR_NONE;
  parser_t parser;
  uint8_t c;

  parser_init(&parser, jtree, json, GO);
  while (get_char(&parser, json, &c) && (err != MU_JTREE_ERR_NONE)) {
    err = parse_char(&parser, c);
  }

  if (err != MU_JTREE_ERR_NONE) {
    return err;
  } else if (parser.state != OK) {
    return MU_JTREE_ERR_INCOMPLETE;
  } else {
    return MU_JTREE_ERR_NONE;
  }
}

static mu_jtree_err_t parse_char(parser_t *parser, uint8_t c) {
  mu_jtree_err_t err;
  int8_t next_class = char_class(c);
  if (next_class == __) {
    // illegal char in JSON string
    return MU_JTREE_ERR_INVALID;
  }

  int8_t next_state = s_states[parser->state * NR_CLASSES + next_class];
  if (next_state >= 0) {
    // non-negative state means just continue parsing
    parser->state = next_state;

  } else {
    // negative state means some processing needs to happen.
    switch(next_state) {

    case -9: {
      // Process opening '"'
      if ((err = open_string(parser)) != MU_JTREE_ERR_NONE) {
        return err;
      } else {
        parser->state = ST;  // parsing contents of a string.
      }
    } break;

    case -8: {
      // Process '}'
      if ((err == close_object(parser)) != MU_JTREE_ERR_NONE) {
        return err;
      } else {
        parser->state = OK; // parsing contents of an object
      }
    } break;

    case -7: {
      // Process ']'
      if ((err == close_array(parser)) != MU_JTREE_ERR_NONE) {
        return err;
      } else {
        parser->state = OK; // parsing contents of an array
      }
    } break;

    case -6: {
      // Process '{'
      if ((err == open_object(parser)) != MU_JTREE_ERR_NONE) {
        return err;
      } else {
        parser->state = OB; // parsing contents of an object
      }
    } break;

    case -5: {
      // Process '['
      if ((err == open_array(parser)) != MU_JTREE_ERR_NONE) {
        return err;
      } else {
        parser->state = OB; // parsing contents of an object
      }
    } break;

    case -4: {
      // Process '"' (closing quote)
      if ((err == close_string(parser)) != MU_JTREE_ERR_NONE) {
        return err;
      }
      mu_jtree_token_t *token = token_stack_top(parser);
      if (token == NULL) {
        return MU_JTREE_ERR_SYSTEM;  // should not happen
      }
      if (is_object_key(parser, token)) {
        parser->state = CO;  // expect a colon delimiter
      } else {
        parser->state = OK;
      }
    } break;

    case -3: {
      // Process ','
      mu_jtree_token_t *token = token_stack_top(parser);
      if (token == NULL) {
        return MU_JTREE_ERR_SYSTEM;  // should not happen
      }
      if (is_object_value(parser, token)) {
        // just processed an object value, expect a key
        parser->state = KE;
      } else if (is_array_value(parser, token)) {
        // just processed an array element
        parser->state = VA;
      } else {
        return MU_JTREE_ERR_INVALID;
      }
    } break;

    case -2: {
      // Process ':'
      mu_jtree_token_t *container = find_open_container(parser, jtree);
      if ((container == NULL) || (container->type != MU_JTREE_TYPE_OBJECT)) {
        // could not find opening {
        return MU_JTREE_ERR_INVALID;
      }
      int distance = token_distance(container, token_stack_top(parser, jtree));
      if (!is_odd(distance)) {
          // need a key before a :
          return MU_JTREE_ERR_INVALID;
      }
      // searching for a value
      parser->state = VA;
    } break;

    default: {
      return false;
    } break;
    } // switch

  }
  return true;
}

static uint8_t get_char(parser_t *parser, mu_str_t *json, uint8_t *ch) {
  if (parser->char_idx >= mu_str_length(json)) {
    return false;
  } else {
    *ch = mu_str_bytes(json)[parser->char_idx++];
    return true;
  }
}

// **************************************************
// **************************************************
// parser accessors

static parser_t *parser_init(parser_t *parser,
                             mu_jtree_t *jtree,
                             mu_str_t *json,
                             state_t initial_state {
  parser->jtree = jtree;
  parser->json = json;
  parser->state = initial_state;
  parser->char_idx = 0;
  parser->depth = 0;
  return parser;
}

static mu_jtree_t *parser_jtree(parser_t * parser) {
  return parser->jtree;
}

static mu_str_t *parser_json(parser_t * parser) {
  return parser->json;
}

static state_t parser_state(parser_t *parser) {
  return parser->state;
}

static int parser_char_index(parser_t *parser) {
  return parser->char_idx;
}

static int parser_token_count(parser_t *parser) {
  return parser->jtree->token_count;
}

static mu_jtree_token_t *parser_tokens(parser_t *parser) {
  return parser->jtree->tokens;
};

static int parser_token_capacity(parser_t *parser) {
  return parser->jtree->token_capacity;
}

static int parser_token_count(parser_t *parser) {
  return parser->jtree->token_count;
}

static int parser_depth(parser_t *parser) {
  return parser->depth;
}

static void parser_increase_depth(parser_t *parser) {
  parser->depth += 1;
}

static void parser_decrease_depth(parser_t *parser) {
  parser->depth -= 1;
}

// **************************************************
// **************************************************
// Token access & tree walking

static mu_jtree_token_t *token_alloc(parser_t *parser) {
  if (parser_token_count(parser) >= parser_token_capacity(parser)) {
    return NULL;
  } else {
    return &(parser_tokens(parser)[parser_jtree(parser)->token_count++]);
  }
}

static mu_jtree_token_t *token_init(mu_jtree_token_t *token,
                                    mu_jtree_token_type_t type,
                                    mu_str_t json,
                                    int depth) {
  token->type = type;
  mu_str_copy(&token->contents, json);
  token->depth = depth;
  return token;
}

static mu_jtree_token_t *token_stack_ref(parser_t *parser, int idx) {
  if ((idx < 0) || (idx >= parser_token_count(parser))) {
    return NULL;
  } else {
    return &(parser_tokens(parser)[idx]);
  }
}

static mu_jtree_token_t *token_stack_top(parser_t *parser) {
  return token_stack_ref(parser, parser_token_count(parser) - 1);
}

static mu_jtree_token_t *token_stack_root(parser_t *parser) {
  return token_stack_ref(parser, 0);
}

static int token_stack_index(parser_t *parser, mu_jtree_token_t *token) {
  if (token == NULL) {
    return -1;
  } else {
    return &token - &(token_stack_root(parser));
  }
}

static mu_jtree_token_t *token_sibling(parser_t *parser,
                                       mu_jtree_token_t *token) {
  if (token == NULL) {
    return NULL;
  }
  int index = token_stack_index(parser, token);
  int depth = token->depth;
  while ((token = token_stack_ref(parser, index + 1)) != NULL) {
    if (token->depth == depth) {
      return token;               // found another token at this depth
    }
    index += 1;                   // examine next token in the array
  }
  return NULL;                    // ran off end looking for a sibling
}

static mu_jtree_token_t *token_children(parser_t *parser,
                                        mu_jtree_token_t *token) {
  if (token == NULL) {
    return NULL;
  }
  int index = token_stack_index(parser, token);
  mu_jtree_token_t *next = token_stack_ref(parser, index + 1);
  if ((next != NULL) && (next->depth == token->depth + 1)) {
    return next;
  } else {
    return NULL;
  }
}

static mu_jtree_token_t *token_parent(parser_t *parser,
                                      mu_jtree_token_t *token) {
  if (token == NULL) {
    return NULL;
  }
  int index = token_stack_index(parser, token);
  int depth = token->depth;
  mu_jtree_token_t *candidate;

  while ((candidate = token_stack_ref(parser, index - 1)) != NULL) {
    if (candidate->depth = depth - 1) {
      return token;                 // found a token one level higher
    }
    index -= 1;                     // keep searching backwards
  }
  return NULL;
}

static bool token_is_container(mu_jtree_token_t *token) {
  return (token != NULL) && ((token->type == MU_JTREE_TYPE_ARRAY) || (token->type == MU_JTREE_TYPE_OBJECT));
}

static mu_jtree_err_t open_array(parser_t *parser, mu_str_t *json) {
  return open_container(parser, MU_JTREE_TYPE_ARRAY);
}

static mu_jtree_err_t open_object(parser_t *parser, mu_str_t *json) {
  return open_container(parser, MU_JTREE_TYPE_OBJECT);
}

static mu_jtree_err_t open_container(parser_t *parser, mu_str_t *json,
                                     mu_jtree_token_type_t type) {
  // Here when a '{' or '[' is spotted
  mu_jtree_token_t *token = token_alloc(parser);
  if (token == NULL) {
    // ran out of tokens
    return MU_JTREE_ERR_ALLOC;
  }
  // Initialize the token at the current depth and capture the starting index
  // of the JSON token.
  token_init(token, type, json, parser_depth(parser));
  token_set_contents_start(token, parser_char_index(parser));
  // Increment current depth: added elements will be children to this token.
  parser_increase_depth(parser);
  return MU_JTREE_ERR_NONE;
}

static void token_set_contents_start(mu_jtree_token_t *token, int start) {
  // NOTE: this actually sets the END of the contents to the start value.
  // This overly-clever trick saves us allocating another field in token_t.
  // Read token_set_contents_end() how we use this.
  mu_str_slice(&token->contents, &token->contents, 0, start);
}

static void token_set_contents_end(mu_jtree_token_t *token, int end) {
  mu_str_t *str = &token->contents;
  // Note: length was set in a previous call to token_set_contents_start().
  // length now becomes the start, and length becomes end - start
  int start = mu_str_length(str);
  mu_str_init(str, &mu_str_bytes(str)[start], end - start);
}

static mu_jtree_err_t close_array(parser_t *parser) {
  return close_container(parser, MU_JTREE_TYPE_ARRAY);
}

static mu_jtree_err_t close_object(parser_t *parser) {
  return close_container(parser, MU_JTREE_TYPE_OBJECT);
}

static mu_jtree_err_t close_container(parser_t *parser,
                                      mu_jtree_token_type_t type) {
  // parser is currently at parser_depth().  search backward from the top of
  // token stack for the first token at parser_depth()-1 -- that will be the
  // array or object container.
  mu_jtree_token_t *token = token_stack_top(parser);
  int index = token_stack_index(token);

  while (token != NULL) {
    if (token_depth(token) == parser_depth(parser) - 1) {
      // found the parent container
      break;
    }
    index -= 1;
    token = token_stack_ref(parser, index);
  }
  if (token == NULL) {
    return MU_JTREE_ERR_INVALID;  // could not find parent container
  } else if (token->type != type) {
    // cannot interleave { ... ] or [ ... }
    return MU_JTREE_ERR_INVALID;  // parser should have prevented this...
  } else {
    // Include the closing '}' or ']' in the token contents.  Set
    // link to the number of tokens spanned by the container.
    token_set_contents_end(token, parser->char_idx);
    // Decrement depth: added elements will be siblings to this container.
    parser_increase_depth(parser);
    return MU_JTREE_ERR_NONE;
  }
}

static int8_t char_class(uint8_t c) {
  if (c > sizeof(s_ascii_classes)) {
    return C_ETC;
  } else {
    return s_ascii_classes[c];
  }
}

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
                              int count) {
  for (int i = 0; i < count; i++) {
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
static bool check_element_lengths(mu_jtree_t *jtree, int *lengths,
                                  int count) {
  for (int i = 0; i < count; i++) {
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
                                   int count) {
  mu_str_t expect;

  for (int i = 0; i < count; i++) {
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
  int token_count;

  printf("\nStarting test_mu_jtree_parse...");
  fflush(stdout);

  mu_str_init_cstr(&str, "[]");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_ARRAY}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"[]"}, token_count));

  mu_str_init_cstr(&str, "{}");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_OBJECT}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"{}"}, token_count));

  mu_str_init_cstr(&str, "\"a\"");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_STRING}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
  ASSERT(
      check_element_contents(&jtree, (const char *[]){"\"a\""}, token_count));

  mu_str_init_cstr(&str, "1");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"1"}, token_count));

  mu_str_init_cstr(&str, "true");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  token_count = mu_jtree_token_count(&jtree);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
  ASSERT(check_element_contents(&jtree, (const char *[]){"true"}, token_count));

  mu_str_init_cstr(&str, "false");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
  ASSERT(
      check_element_contents(&jtree, (const char *[]){"false"}, token_count));

  mu_str_init_cstr(&str, "null");
  ASSERT(mu_jtree_parse(&jtree, &str, s_tokens, MAX_TOKENS) ==
         MU_JTREE_ERR_NONE);
  ASSERT(token_count == 1);
  ASSERT(check_token_types(
      &jtree, (mu_jtree_token_type_t[]){MU_JTREE_TYPE_PRIMITIVE}, token_count));
  ASSERT(check_element_lengths(&jtree, (int[]){1}, token_count));
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
  ASSERT(check_element_lengths(&jtree, (int[]){4, 1, 1, 1}, token_count));
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
  ASSERT(check_element_lengths(&jtree, (int[]){7, 1, 1, 1, 1, 1, 1},
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
      check_element_lengths(&jtree, (int[]){6, 1, 3, 1, 1, 1}, token_count));
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
      &jtree, (int[]){10, 1, 1, 1, 4, 1, 1, 1, 1, 1}, token_count));
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
      &jtree, (int[]){11, 1, 8, 1, 5, 1, 2, 1, 1, 1}, token_count));
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
