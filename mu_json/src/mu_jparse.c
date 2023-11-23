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

General strategy:

In mu_jparse, each JSON entity is captured in a mu_jparse_token_t structure:

typedef struct {
    mu_str_t str;                // slice of the original JSON string
    mu_jparse_token_type_t type; // token type (ARRAY, OBJECT, STRING, ...)
    int depth;                   // 0 = toplevel, 1 = child of toplevel...
} mu_jparse_token_t;

Parsing the JSON string is done with a compact table-driven push-down parser.
Rather than keep an explicit stack of parser states, the list of parsed tokens
provides the information needed for the parser.  For example, the JSON string:

  {"a":[1,2.5],"b":true}

will parse into a list of seven tokens:

i: slice                 , type    , depth, container, siblings
0: {"a":[1,2.5],"b":true}, OBJECT  , 0    , NULL     , 0
1: "a"                   , STRING  , 1    , OBJECT   , 0
2: [1,2]                 , ARRAY   , 1    , OBJECT   , 1
3: 1                     , INTEGER , 2    , ARRAY    , 0
4: 2.5                   , NUMBER  , 2    , ARRAY    , 1
5: "b"                   , STRING  , 1    , OBJECT   , 2
6: true                  , TRUE    , 1    , OBJECT   , 3

A few things to note:

depth tells us how many levels deep the parser is
container is the type of the container (or NULL to top level)
siblings is how many elements are in the current container.  For OBJECT
  containers, an even-numbered sibling is always the key, odd is the value.

Therefore, the container determines the parser that is in effect after an
element is parsed.



  ch prev_state    action      depth

     value
  {  object
  "    in_string     open(str)   1
  a  in_string      in_string     -           1
  "  in_string      expect_key    close(str)  1
  :  expect_key     expect_entity -           1
  [  expect_entity  expect_entity open(arr)   2
  1  in_number      in_number     open(num)   2
  ,  in_number      expect_entity close(num)  2
  2  expect_entity  in_number     open(num)   2
  ]  in_number
  ,
  "
  b
  "
  :
  t
  r
  u
  e
  }


 */

/**
 *
 * SEE:
 *   https://github.com/nst/JSONTestSuite
 *   https://www.json.org/json-en.html
 *   https://seriot.ch/projects/parsing_json.html
 *   https://github.com/douglascrockford/JSON-c

entity is OBJECT, ARRAY, STRING, NUMBER, TRUE, FALSE, NULL.
current_level() is 0 for top level, 1 inside OBJECT or ARRAY, etc.
containing_token() is either NULL (for top level) or OBJECT or ARRAY.
token_in_progress() is an entity that's been started but not completed

scanning for entity: [whitespace, \{, \[, \", digit, t, f, n] starts new current token
upon completing token:
  set level of current_token(), add to tree
  set next state:
    if containing_token() is NULL, state = WHITESPACE_OR_EOS
    if containing_token() is ARRAY, state = WHITESPACE_OR_COMMA
    if containing_token() is OBJECT:
        if is_even(child_count(containing_token)), state = WHITESPACE_OR_COMMA
        else state = WHITESPACE_OR_COLON
  if containing_token() is NULL then only whitespace or EOS are valid
  if containing_token() is ARRAY, then only whitespace or ',' are valid
  if containing_token() is OBJECT, then whitespace or ':' or ',' are valid
    (depending on # of children of containing_token())

some states:
  S_START_TOPLEVEL: whitespace, [, {, ", digit, t, f, n, EOS
  S_END_TOPLEVEL: whitespace, EOS
  S_EXPECT_ARRAY_VALUE: whitespace, ',', ']'
  S_EXPECT_OBJECT_KEY: whitespace, '"', '}'
  S_GOT_OBJECT_KEY: whitespace, ':'

some functions:
  containing_token()
  child_count(token)
  is_in_object()
  is_in_array()
  is_toplevel()
  char_class(ch)
  current_state()

 */

#if 0
char_class = get_classified_char();
if (char_class == CH_ILLEGAL) {
	tree->status = MU_JPARSE_ERR_BAD_FORMAT;
	return tree;
} else if (char_class == CH_EOF) {
	if (tree_is_complete(tree)) {
	    tree->status = token_index;
	} else {
		tree->status = MU_JPARSE_ERR_BAD_FORMAT;
	}
	return tree;
}

switch (state) {
case PARSE_VALUE: {
    switch (char_class) {
    case CH_WHITESPACE: {
        state = PARSE_VALUE;
    } break;
    case CH_DBLQUOTE: {
        start_token(MU_JPARSE_TOKEN_TYPE_STRING);
        state = PARSE_STRING;
    } break;
    case CH_OPENCURLY: {
        start_token(MU_JPARSE_TOKEN_TYPE_OBJECT);
        state = PARSE_OBJECT;
    } break;
    case CH_OPENSQUARE: {
        start_token(MU_JPARSE_TOKEN_TYPE_ARRAY);
        state = PARSE_ARRAY;
    } break;
    case CH_T: {
        start_token(MU_JPARSE_TOKEN_TYPE_TRUE);
        state = PARSE_TRUE;
    } break;
    case CH_F: {
        start_token(MU_JPARSE_TOKEN_TYPE_FALSE);
        state = PARSE_FALSE;
    } break;
    case CH_N: {
        start_token(MU_JPARSE_TOKEN_TYPE_NULL);
        state = PARSE_NULL0;
    } break;
    default: {
	    return bad_fmt(tree);
    } break;
    } // switch(char_class)
} break;
case PARSE_NULL0:
	switch(char_class) {
	case CH_U: {
	    state = PARSE_NULL1;
	} break;
	default:
		return bad_fmt(tree);
	} break;
	} // switch(char_class)
case PARSE_NULL1:
	switch(char_class) {
	case CH_L: {
	    state = PARSE_NULL2;
	} break;
	default:
		return bad_fmt(tree);
	} break;
	} // switch(char_class)
case PARSE_NULL2:
	switch(char_class) {
	case CH_L: {
		end_current_token(MU_JPARSE_TOKEN_TYPE_NULL);
	    state = PARSE_VALUE;
	} break;
	default:
		return bad_fmt(tree);
	} break;
	} // switch(char_class)
} // switch(state)

#endif


// ****************************************************************************=
// Includes

#include "mu_jparse.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ****************************************************************************=
// Private types and definitions

#define __   -1     /* the universal error code */

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
    C_EOS,    /* end of string */
    C_ETC,    /* everything else */
    C_CLASS_MAX
} c_class_t;

typedef enum {
    S_GO,  /* start    */
    S_OK,  /* ok       */
    S_OB,  /* object   */
    S_KE,  /* key      */
    S_CO,  /* colon    */
    S_VA,  /* value    */
    S_AR,  /* array    */
    S_ST,  /* string   */
    S_ES,  /* escape   */
    S_U1,  /* u1       */
    S_U2,  /* u2       */
    S_U3,  /* u3       */
    S_U4,  /* u4       */
    S_MI,  /* minus    */
    S_ZE,  /* zero     */
    S_IN,  /* integer  */
    S_FR,  /* fraction */
    S_FS,  /* fraction */
    S_E1,  /* e        */
    S_E2,  /* ex       */
    S_E3,  /* exp      */
    S_T1,  /* tr       */
    S_T2,  /* tru      */
    S_T3,  /* true     */
    S_F1,  /* fa       */
    S_F2,  /* fal      */
    S_F3,  /* fals     */
    S_F4,  /* false    */
    S_N1,  /* nu       */
    S_N2,  /* nul      */
    S_N3,  /* null     */
    STATE_MAX
} state_t;

/**
 * @brief Set the 0x80 bit to signify this is an action rather than a state
 */
#define A(a) ((state_t)((a) | 0x80))

typedef enum {
	SA,   // start array
	EA,   // end array
	SO,   // start object
	EO,   // end object
	SS,   // start string
	ES,   // end string
	SN,   // start number
	EN,   // end number
	ST,   // start true
	ET,   // end true
	SF,   // start false
	EF,   // end false
	SU,   // start nUll
	EU,   // end nUll
} action_t;

typedef struct {
	mu_jparse_jtree_t *tree;  // the array of tokens and overall count / status
	mu_str_t *json;           // the JSON string being parsed
	int char_index;           // index the n'th byte of the JSON string
} parser_t;

// ****************************************************************************=
// Private (static) storage

// clang-format off
/**
 * @brief Map an ASCII character to a ch_state_t.
 */
static c_class_t s_c_class_map[] = {
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
// clang-format on

// clang-format off
static int8_t s_state_map[STATE_MAX][C_CLASS_MAX] = {
//
//                 white                                                                  1-9                                                             ABCDF      etc
//            space  |    {    }    [    ]    :    ,    "    \    /    +    -    .    0    |    a    b    c    d    e    f    l    n    r    s    t    u    |    E    |*/
/*start  GO*/ {GO,  GO,A(SA),  __,  -5,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*ok     OK*/ {OK,  OK,  __,  -8,  __,  -7,  __,  -3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*object OB*/ {OB,  OB,  __,  -9,  __,  __,  __,  __,  ST,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*key    KE*/ {KE,  KE,  __,  __,  __,  __,  __,  __,  ST,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*colon  CO*/ {CO,  CO,  __,  __,  __,  __,  -2,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*value  VA*/ {VA,  VA,  -6,  __,  -5,  __,  __,  __,  ST,  __,  __,  __,  MI,  __,  ZE,  IN,  __,  __,  __,  __,  __,  F1,  __,  N1,  __,  __,  T1,  __,  __,  __,  __},
/*array  AR*/ {AR,  AR,  -6,  __,  -5,  -7,  __,  __,  ST,  __,  __,  __,  MI,  __,  ZE,  IN,  __,  __,  __,  __,  __,  F1,  __,  N1,  __,  __,  T1,  __,  __,  __,  __},
/*string ST*/ {ST,  __,  ST,  ST,  ST,  ST,  ST,  ST,  -4,  ES,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST},
/*escape ES*/ {__,  __,  __,  __,  __,  __,  __,  __,  ST,  ST,  ST,  __,  __,  __,  __,  __,  __,  ST,  __,  __,  __,  ST,  __,  ST,  ST,  __,  ST,  U1,  __,  __,  __},
/*u1     U1*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  U2,  U2,  U2,  U2,  U2,  U2,  U2,  U2,  __,  __,  __,  __,  __,  __,  U2,  U2,  __},
/*u2     U2*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  U3,  U3,  U3,  U3,  U3,  U3,  U3,  U3,  __,  __,  __,  __,  __,  __,  U3,  U3,  __},
/*u3     U3*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  U4,  U4,  U4,  U4,  U4,  U4,  U4,  U4,  __,  __,  __,  __,  __,  __,  U4,  U4,  __},
/*u4     U4*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  ST,  __,  __,  __,  __,  __,  __,  ST,  ST,  __},
/*minus  MI*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  ZE,  IN,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*zero   ZE*/ {OK,  OK,  __,  -8,  __,  -7,  __,  -3,  __,  __,  __,  __,  __,  FR,  __,  __,  __,  __,  __,  __,  E1,  __,  __,  __,  __,  __,  __,  __,  __,  E1,  __},
/*int    IN*/ {OK,  OK,  __,  -8,  __,  -7,  __,  -3,  __,  __,  __,  __,  __,  FR,  IN,  IN,  __,  __,  __,  __,  E1,  __,  __,  __,  __,  __,  __,  __,  __,  E1,  __},
/*frac   FR*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  FS,  FS,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*fracs  FS*/ {OK,  OK,  __,  -8,  __,  -7,  __,  -3,  __,  __,  __,  __,  __,  __,  FS,  FS,  __,  __,  __,  __,  E1,  __,  __,  __,  __,  __,  __,  __,  __,  E1,  __},
/*e      E1*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  E2,  E2,  __,  E3,  E3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*ex     E2*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  E3,  E3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*exp    E3*/ {OK,  OK,  __,  -8,  __,  -7,  __,  -3,  __,  __,  __,  __,  __,  __,  E3,  E3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*tr     T1*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  T2,  __,  __,  __,  __,  __,  __},
/*tru    T2*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  T3,  __,  __,  __},
/*true   T3*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  OK,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*fa     F1*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  F2,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*fal    F2*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  F3,  __,  __,  __,  __,  __,  __,  __,  __},
/*fals   F3*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  F4,  __,  __,  __,  __,  __},
/*false  F4*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  OK,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __},
/*nu     N1*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  N2,  __,  __,  __},
/*nul    N2*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  N3,  __,  __,  __,  __,  __,  __,  __,  __},
/*null   N3*/ {__,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  OK,  __,  __,  __,  __,  __,  __,  __,  __}
};
// clang-format on

// ****************************************************************************=
// Private (forward) declarations

static mu_parse_jtree_t *parse_aux(mu_parse_jtree_t *tree, mu_str_t *json);

/**
 * @brief Set up parser state to parse JSON string.
 */
static parser_t *parser_init(parser_t *parser, mu_parse_tree *tree,
                             mu_str_t *json);

/**
 * @brief Process one JSON byte at a time, return false on completion or error.
 *
 * Note: upon return, tree->status will be set to a negative value on error or
 * a non-zero value on success.
 */
static bool parser_step(parser_t *parser);

/**
 * @brief Read next char from JSON source, map to ch_class.
 *
 * NOTE: this post-increments the character pointer.
 */
static c_class_t classify_next_char(parser_t *parser)


/**
 * @brief Return true if this given state triggers an action, not just a state
 * transition.
 *
 * NOTE: States (and actions) are represented by a uint8_t.  When the 0x80 bit
 * is on, this triggers an action rather than a simple state transtion.
 */
static inline bool state_is_action(state_t state) { return state & 0x80; }


// ****************************************************************************=
// Public code

mu_jparse_jtree_t *mu_jparse_jtree_init(mu_jparse_jtree_t *tree,
                                        mu_jparse_token_t *token_store,
                                        size_t max_tokens) {
	tree->tokens = token_store;
	tree->max_tokens = max_tokens;
	tree->status = 0;
}

mu_jparse_jtree_t *mu_jparse_parse_mu_str(mu_jparse_jtree_t *tree,
                                          mu_str_t *mu_str) {
	return parse_aux(tree, mu_str);
}

mu_jparse_jtree_t *mu_jparse_parse_c_str(mu_jparse_jtree_t *tree,
                                         const char *c_str) {
	mu_str_t mu_str;
	return parse_aux(tree, mu_init_cstr(&mu_str, cstr));
}

mu_parse_jtree_t *mu_jparse_parse_buf(mu_parse_jtree_t *, (const uint8_t *)buf,
                                      size_t buf_length) {
	mu_str_t mu_str;
	return parse_aux(tree, mu_str_init(&mu_str, buf, buf_len));
}

int mu_jparse_jtree_token_index_of(mu_jparse_jtree_t *tree,
                                   mu_jparse_token_t *token) {
	// Separate clauses make coverage tests more useful.
	// TODO: does it generate the same code as if it had one big if statement?
	if (token < &tree->tokens[0]) {
		return MU_JPARSE_TOKEN_NOT_FOUND;
	} else if (token >= &tree->tokens[tree->max_tokens]) {
		return MU_JPARSE_TOKEN_NOT_FOUND;
	} else {
		// Old-school C pointer arithmetic
		return token - tree->tokens;
	}
}

mu_jparse_token_t *mu_jparse_jtree_token_ref(mu_jparse_jtree_t *tree,
                                             int index) {
	if (index < 0) {
		return NULL;
	} else if (index >= tree->max_tokens) {
		return NULL;
	} else {
		return &tree->tokens[index];
	}
}

mu_jparse_token_t *mu_jparse_jtree_next_sibling(mu_jparse_jtree_t *tree,
                                                mu_str_t *mu_str) {
	return NULL;
}

mu_jparse_token_t *mu_jparse_jtree_prev_sibling(mu_jparse_jtree_t *tree,
                                                mu_str_t *mu_str) {
	return NULL;
}

mu_jparse_token_t *mu_jparse_jtree_parent(mu_jparse_jtree_t *tree,
                                          mu_str_t *mu_str) {
	return NULL;
}

mu_jparse_token_t *mu_jparse_jtree_first_child(mu_jparse_jtree_t *tree,
                                               mu_str_t *mu_str) {
	return NULL;
}

// ****************************************************************************=
// Private (static) code

static mu_parse_jtree_t *parse_aux(mu_parse_jtree_t *tree, mu_str_t *json) {
	parser_t parser;
	parser_init(&parser, tree, json);
	while (parser_step(parser)) {
		// process one char at a time until end of string or error
	}
	// Here, tree has been built with status = # of tokens (if non-negative) or
	// status = error code (if negative)
	return tree;
}

static parser_t *parser_init(parser_t *parser, mu_parse_tree *tree,
                             mu_str_t *json) {
    parser->tree = tree;
    parser->json = json;
    tree->status = 0;
    parser->char_index = 0;
    return parser;
}

static bool parser_step(parser_t *parser) {
	c_class_t ch_class = classify_next_char(parser);
	state_t next_state = get_state(parser->state, ch_class);
	if (state_is_action(next_state)) {
		process_action(parser, next_state);
	} else {
		parser->state = next_state;
	}
	return false;
}

static c_class_t classify_next_char(parser_t *parser) {
	if (parser->char_index >= mu_str_length(parser->json)) {
		// At end of JSON string, class => CL_EOS
		return CH_EOS;
	}

    // TODO: implement mu_str_ref()?
	uint8_t ch = mu_str_bytes(parser->json)[parser->char_index++];

	if (ch > sizeof(s_c_class_map)) {
		// ch out of range of lookup table.  class => CL_ETC
		return CH_ETC;
	} else {
		// look up ch_class from table.
		return s_c_class_map[ch];
	}
}

static void process_action(parser_t *parser, state_t next_state) {
	mu_jparse_token_t *token;
	action_t action = next_state & 0x7f; // strip action bit
	// TODO: convert to table dispatch

	switch(action) {
	case SA: {
        // start array
        alloc_token(parser, MU_JPARSE_TOKEN_TYPE_OBJECT, true);
    } break;
	case EA: {
        // end array
    } break;
	case SO: {
        // start object
    } break;
	case EO: {
        // end object
    } break;
	case SS: {
        // start string
    } break;
	case ES: {
        // end string
    } break;
	case SN: {
        // start number
    } break;
	case EN: {
        // end number
    } break;
	case ST: {
        // start true
    } break;
	case ET: {
        // end true
    } break;
	case SF: {
        // start false
    } break;
	case EF: {
        // end false
    } break;
	case SU: {
        // start nUll
    } break;
	case EU: {
        // end nUll
    } break;

	} // switch
}
