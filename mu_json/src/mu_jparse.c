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

  _{"a":[1,2.5],"b":true}

will parse into a list of seven tokens:

i: slice                 , type    , depth, container, siblings
0: {"a":[1,2.5],"b":true}, OBJECT  , 0    , NULL     , 0
1: "a"                   , STRING  , 1    , OBJECT   , 0
2: [1,2.5]               , ARRAY   , 1    , OBJECT   , 1
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
element is parsed.  When the end of any entity is seen, the state becomes
END_ENTITY.  END_ENTITY selects the next state based on the container
(ARRAY, OBJECT or NONE), and in the case of an OBJECT container, whether a
key or value is expected.

START_ENTITY: whitespace, '[', '{', '"', digit, 't', 'f', 'n'
IN_ARRAY: whitespace, ',', ']'
  ',' => state = ENTITY,
  ']' => action = CLOSE_ARRAY, state => END_ENTITY
IN_OBJECT_EVEN: whitespace, ':'
  ':' => EXPECT_STRING
IN_OBJECT_ODD: whitespace, ',', '}'
  ',' => EXPECT_ENTITY
  '}' => action = CLOSE_OBJECT, state = END_ENTITY
EXPECT_STRING: whitespace, '"'
  '"' => IN_STRING
IN_STRING:
IN_NUMBER:
IN_TRUE:
IN_FALSE:
IN_NULL:

  _{"a":[1,2.5],"b":true}

prev_state     ch depth action       comment
--------------+-+------+------------+-------------------------------+
expect_entity     0                  (initial state)
expect_entity  _  0     -
expect_entity  {  0     open_object  depth++, state=>expect_string
expect_string  "  1     open_string  state=>parsing_string
parsing_string a  1     -
parsing_string "  1     close_string state=>expect_colon [1]
expect_colon   :  1     -            state=>expect_entity
expect_entity  [  1     open_array   depth++, state=>expect_entity
expect_entity  1  2     open_number  state=>parsing_number
parsing_number ,  2     close_number state=>expect_entity [2]
expect_entity  2  2     open_number  state=>parsing_number
parsing_number .  2     -
parsing_number 5  2     -
parsing_number ]  2     close_number depth--, state=>expect_comma [3]
expect_comma   ,  1     -            state=>expect_string
expect_string  "  1     open_string  state=>parsing_string
parsing_string b  1     -
parsing_string "  1     close_string state=>expect_colon [4]
expect_colon   :  1     -            state=>expect_entity
expect_entity  t  1     open_true    state=>parsing_true
parsing_true   r  1     -
parsing_true   u  1     -
parsing_true   e  1     close_true   state=>expect_comma [5]
expect_comma   }  1     close_object depth--, state=>expect_entity
expect_entity  ^  0     endgame      end of string && depth==0 => okay

[1] container = OBJECT, container_child_count = 1, ergo state=>expect_colon
[2] container = ARRAY
[3] container = OBJECT, container_child_count = 2, ergo state=>expect_comma
[4] container = OBJECT, container_child_count = 3, ergo state=>expect_colon
[5] container = OBJECT, container_child_count = 4, comma or } okay...

void entity_did_close() {
	if (depth == 0) {
	    if (EOS) {
	        success(); // entity done
	    } else {
	        error();   // can't have multiple top-level entities
	    }
	} else if (container() == ARRAY) {
		// ',' or ']' acceptible
	} else if (container() == OBJECT) {
	    if (child_count is even) {
	        // expect :
	    } else {
	        // , or } is acceptible
	    }
	}
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

#define DEFINE_CHAR_CLASSES(M)                                                 \
    M(C_SPACE) /* space */                                                     \
    M(C_WHITE) /* other whitespace */                                          \
    M(C_LCURB) /* {  */                                                        \
    M(C_RCURB) /* } */                                                         \
    M(C_LSQRB) /* [ */                                                         \
    M(C_RSQRB) /* ] */                                                         \
    M(C_COLON) /* : */                                                         \
    M(C_COMMA) /* , */                                                         \
    M(C_QUOTE) /* " */                                                         \
    M(C_BACKS) /* \ */                                                         \
    M(C_SLASH) /* / */                                                         \
    M(C_PLUS)  /* + */                                                         \
    M(C_MINUS) /* - */                                                         \
    M(C_POINT) /* . */                                                         \
    M(C_ZERO)  /* 0 */                                                         \
    M(C_DIGIT) /* 123456789 */                                                 \
    M(C_LOW_A) /* a */                                                         \
    M(C_LOW_B) /* b */                                                         \
    M(C_LOW_C) /* c */                                                         \
    M(C_LOW_D) /* d */                                                         \
    M(C_LOW_E) /* e */                                                         \
    M(C_LOW_F) /* f */                                                         \
    M(C_LOW_L) /* l */                                                         \
    M(C_LOW_N) /* n */                                                         \
    M(C_LOW_R) /* r */                                                         \
    M(C_LOW_S) /* s */                                                         \
    M(C_LOW_T) /* t */                                                         \
    M(C_LOW_U) /* u */                                                         \
    M(C_ABCDF) /* ABCDF */                                                     \
    M(C_E)     /* E */                                                         \
    M(C_ETC)   /* everything else */                                           \
    M(C_CLASS_MAX)

#define EXPAND_CH_CLASS_ENUMS(_name) _name,
typedef enum { DEFINE_CHAR_CLASSES(EXPAND_CH_CLASS_ENUMS) } c_class_t;

#define DEFINE_STATES(M)                                                       \
    M(S_VA) /* value    */                                                     \
    M(S_OK) /* ok       */                                                     \
    M(S_OB) /* object   */                                                     \
    M(S_KE) /* key      */                                                     \
    M(S_CO) /* colon    */                                                     \
    M(S_AR) /* array    */                                                     \
    M(S_ST) /* string   */                                                     \
    M(S_ES) /* escape   */                                                     \
    M(S_U1) /* u1       */                                                     \
    M(S_U2) /* u2       */                                                     \
    M(S_U3) /* u3       */                                                     \
    M(S_U4) /* u4       */                                                     \
    M(S_MI) /* minus    */                                                     \
    M(S_ZE) /* zero     */                                                     \
    M(S_IN) /* integer  */                                                     \
    M(S_FR) /* fraction */                                                     \
    M(S_FS) /* fraction */                                                     \
    M(S_E1) /* e        */                                                     \
    M(S_E2) /* ex       */                                                     \
    M(S_E3) /* exp      */                                                     \
    M(S_T1) /* tr       */                                                     \
    M(S_T2) /* tru      */                                                     \
    M(S_T3) /* true     */                                                     \
    M(S_F1) /* fa       */                                                     \
    M(S_F2) /* fal      */                                                     \
    M(S_F3) /* fals     */                                                     \
    M(S_F4) /* false    */                                                     \
    M(S_N1) /* nu       */                                                     \
    M(S_N2) /* nul      */                                                     \
    M(S_N3) /* null     */                                                     \
    M(STATE_MAX)

#define EXPAND_STATE_ENUMS(_name) _name,
typedef enum { DEFINE_STATES(EXPAND_STATE_ENUMS) } state_t;

#define DEFINE_ACTIONS(M)                                                      \
    M(A_SA=0x80) /* starting array with [ */                                   \
    M(A_SO) /* starting object with { */                                       \
    M(A_SS) /* starting object with " */                                       \
    M(A_SM) /* starting number with - */                                       \
    M(A_SZ) /* starting number with 0 */                                       \
    M(A_SN) /* starting number with 1-9 */                                     \
    M(A_ST) /* starting true with t */                                         \
    M(A_SF) /* starting false with f */                                        \
    M(A_SU) /* starting null with n */                                         \
    M(A_ES) /* entity ended by space or whitespace */                          \
    M(A_EA) /* entity ended by ] */                                            \
    M(A_EO) /* entity ended by } */                                            \
    M(A_EC) /* entity ended by , */                                            \
    M(A_EK) /* entity ended by : ('key') */                                    \
    M(A_EE) /* entity ended by end of string */

#define EXPAND_ACTION_ENUMS(_name) _name,
typedef enum { DEFINE_ACTIONS(EXPAND_ACTION_ENUMS) } action_t;

typedef enum {
    STATUS_IN_PROGRESS, // parsing in progress
    STATUS_NO_TOKENS,   // ran out of tokens
    STATUS_BAD_FORMAT,  // bad JSON syntax
    STATUS_COMPLETE     // success
} status_t;

typedef struct {
	mu_jparse_jtree_t *tree;  // the array of tokens and overall count / status
	mu_str_t *json;           // the JSON string being parsed
	int char_index;           // index the n'th byte of the JSON string
    int depth;                // depth of current token
    status_t status;          // parser status
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
static uint8_t s_state_map[STATE_MAX][C_CLASS_MAX] = {
//
//             space white     {      }    [     ]     :     ,     "     \     /     +     -     .     0   1-9     a     b     c     d     e     f     l     n     r     s     t     u ABCDF     E   etc */
/*value  VA*/ { S_VA, S_VA, A_SO,   __, A_SA,   __,   __,   __, A_SS,   __,   __,   __, A_SM,   __, A_SZ, A_SN,   __,   __,   __,   __,   __, A_SF,   __, A_SU,   __,   __, A_ST,   __,   __,   __,   __},
/*ok     OK*/ { S_OK, S_OK,   __, A_EO,   __, A_EA,   __, A_EC,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*object OB*/ { S_OB, S_OB,   __, A_EO,   __,   __,   __,   __, S_ST,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*key    KE*/ { S_KE, S_KE,   __,   __,   __,   __,   __,   __, S_ST,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*colon  CO*/ { S_CO, S_CO,   __,   __,   __,   __, A_EK,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*array  AR*/ { S_AR, S_AR, A_SO,   __, A_SA, A_EA,   __,   __, S_ST,   __,   __,   __, S_MI,   __, S_ZE, S_IN,   __,   __,   __,   __,   __, S_F1,   __, S_N1,   __,   __, S_T1,   __,   __,   __,   __},
/*string ST*/ { S_ST,   __, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, A_ES, S_ES, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST},
/*escape ES*/ {   __,   __,   __,   __,   __,   __,   __,   __, S_ST, S_ST, S_ST,   __,   __,   __,   __,   __,   __, S_ST,   __,   __,   __, S_ST,   __, S_ST, S_ST,   __, S_ST, S_U1,   __,   __,   __},
/*u1     U1*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_U2, S_U2, S_U2, S_U2, S_U2, S_U2, S_U2, S_U2,   __,   __,   __,   __,   __,   __, S_U2, S_U2,   __},
/*u2     U2*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_U3, S_U3, S_U3, S_U3, S_U3, S_U3, S_U3, S_U3,   __,   __,   __,   __,   __,   __, S_U3, S_U3,   __},
/*u3     U3*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_U4, S_U4, S_U4, S_U4, S_U4, S_U4, S_U4, S_U4,   __,   __,   __,   __,   __,   __, S_U4, S_U4,   __},
/*u4     U4*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST, S_ST,   __,   __,   __,   __,   __,   __, S_ST, S_ST,   __},
/*minus  MI*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_ZE, S_IN,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*zero   ZE*/ { S_OK, S_OK,   __, A_EO,   __, A_EA,   __, A_EC,   __,   __,   __,   __,   __, S_FR,   __,   __,   __,   __,   __,   __, S_E1,   __,   __,   __,   __,   __,   __,   __,   __, S_E1,   __},
/*int    IN*/ { S_OK, S_OK,   __, A_EO,   __, A_EA,   __, A_EC,   __,   __,   __,   __,   __, S_FR, S_IN, S_IN,   __,   __,   __,   __, S_E1,   __,   __,   __,   __,   __,   __,   __,   __, S_E1,   __},
/*frac   FR*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_FS, S_FS,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*fracs  FS*/ { S_OK, S_OK,   __, A_EO,   __, A_EA,   __, A_EC,   __,   __,   __,   __,   __,   __, S_FS, S_FS,   __,   __,   __,   __, S_E1,   __,   __,   __,   __,   __,   __,   __,   __, S_E1,   __},
/*e      E1*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_E2, S_E2,   __, S_E3, S_E3,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*ex     E2*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_E3, S_E3,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*exp    E3*/ { S_OK, S_OK,   __, A_EO,   __, A_EA,   __, A_EC,   __,   __,   __,   __,   __,   __, S_E3, S_E3,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*tr     T1*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_T2,   __,   __,   __,   __,   __,   __},
/*tru    T2*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_T3,   __,   __,   __},
/*true   T3*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_OK,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*fa     F1*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_F2,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*fal    F2*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_F3,   __,   __,   __,   __,   __,   __,   __,   __},
/*fals   F3*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_F4,   __,   __,   __,   __,   __},
/*false  F4*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_OK,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __},
/*nu     N1*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_N2,   __,   __,   __},
/*nul    N2*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_N3,   __,   __,   __,   __,   __,   __,   __,   __},
/*null   N3*/ {   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __,   __, S_OK,   __,   __,   __,   __,   __,   __,   __,   __}
};
// clang-format on

#define EXPAND_ACTION_FUNCTIONS(_name, _function) _function,
static action_fn s_action_map[] = { DEFINE_ACTIONS(EXPAND_ACTION_FUNCTIONS) };

// ****************************************************************************=
// Private (forward) declarations

static mu_parse_jtree_t *parse_aux(mu_parse_jtree_t *tree, mu_str_t *json);

/**
 * @brief Set up parser state to parse JSON string.
 */
static parser_t *parser_init(parser_t *parser, mu_parse_tree *tree,
                             mu_str_t *json);

/**
 * @brief Process one JSON byte at a time
 *
 * Note: call step until parser->status != STATUS_IN_PROGRESS
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

/**
 * @brief Turn off the "is_action" bit to convert a state to an action.
 */
static inline action_t state_to_action(state_t state) { return state & ~=80; }

/**
 * @brief Look up the next state from the state transition table.
 *
 * Note: if the returned value has the 0x80 bit set, this denotes an action
 * rather than a simple state transition.
 */
static inline state_t get_next_state(state_t curr_state, c_class_t c_class) {
	return s_state_map[curr_state][c_class];
}

// ****************************************************************************=
// Public code

mu_jparse_jtree_t *mu_jparse_jtree_init(mu_jparse_jtree_t *tree,
                                        mu_jparse_token_t *token_store,
                                        size_t max_tokens) {
	tree->tokens = token_store;
	tree->max_tokens = max_tokens;
	tree->count = 0;
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
	parser_t parser;  // parser only exists long enough to parse the JSON

	parser_init(&parser, tree, json);
    while (parser->status == STATUS_IN_PROGRESS) {
        // read and process a char at a time
        parser_step(parser);
    }
	// Finalize tree count
    if (parser->status == STATUS_NO_TOKENS) {
        tree->count = MU_JPARSE_NO_MEMORY;
    } else if (parser->status == STATUS_BAD_FORMAT) {
        tree->count = MU_JPARSE_BAD_FORMAT;
    } else {
        // tree->count already set to # of tokens allocated
    }
	return tree;
}

static parser_t *parser_init(parser_t *parser, mu_parse_tree *tree,
                             mu_str_t *json) {
    parser->tree = tree;
    parser->json = json;
    parser->char_index = 0;
    parser->depth = 0;
    parser->status = STATUS_IN_PROGRESS;
    return parser;
}

static void parser_step(parser_t *parser) {
    uint8_t ch;

    if (!mu_str_ref(parser->json, parser->char_index, &ch)) {
        // at end of string
        process_action(parser, TE);

    } else {
        c_class_t c_class = classify_char(ch);
        state_t next_state = get_next_state(parser->state, c_class);

        if (!state_is_action(next_state)) {
            // no action to process - simply advance state
            parser->state = next_state;

        } else {
            // next_state has action bit turned on.
            process_action(parser, next_state);
        }

        // prepare to read next char
        parser->char_index++;
    }
}

static c_class_t classify_char(parser_t *parser) {
	if (ch > sizeof(s_c_class_map)) {
		// ch out of range of lookup table.  class => C_ETC
		return C_ETC;
	} else {
		// look up character class from table.
		return s_c_class_map[ch];
	}
}

static process_action(parser_t *parser, action_t action) {
    mu_jparse_token_t *container = containing_token(parser);

	switch(action) {
	case A_SA: {
        // start array
        start_token(parser, MU_JPARSE_TOKEN_TYPE_ARRAY, parser->depth++);
        parser->state = IA;  // in array
    } break;
	case A_SO: {
        // start object
		start_token(parser, MU_JPARSE_TOKEN_TYPE_OBJECT, parser->depth++);
        parser->state = KE;  // expect key
    } break;
	case A_SS: {
		start_token(parser, MU_JPARSE_TOKEN_TYPE_STRING, parser->depth);
        parser_state = ST;
    } break;
	case A_SM: {
        // start minus sign
		start_token(parser, MU_JPARSE_TOKEN_TYPE_NUMBER, parser->depth);
        parser_state = MI;
    } break;
	case A_SZ: {
        // start zero
		start_token(parser, MU_JPARSE_TOKEN_TYPE_NUMBER, parser->depth);
        parser_state = ZE;
    } break;
	case A_SN: {
        // start number
		start_token(parser, MU_JPARSE_TOKEN_TYPE_NUMBER, parser->depth);
        parser_state = IN;
    } break;
	case A_ST: {
        // start true
		start_token(parser, MU_JPARSE_TOKEN_TYPE_TRUE, parser->depth);
        parser_state = T1;
    } break;
	case A_SF: {
        // start false
		start_token(parser, MU_JPARSE_TOKEN_TYPE_FALSE, parser->depth);
        parser_state = F1;
    } break;
	case A_SU: {
        // start nUll
		start_token(parser, MU_JPARSE_TOKEN_TYPE_NULL, parser->depth);
        parser_state = N1;
    } break;
    case A_ES: {
        // entity ended by space or whitespace
        end_token(parser);
        parser->state = OK;
    } break;
    case A_EA: {
        // entity ended by ] (parent should == ARRAY)
        if (container && (container->type == MU_JPARSE_TOKEN_TYPE_ARRAY)) {
            end_token(parser);
            parser->depth -= 1;
            parser->state = OK;
        } else {
            parser->status = STATUS_BAD_FORMAT;
        }
    } break;
    case A_EO: {
        // entity ended by } (parent should == OBJECT)
        if (container && (container->type == MU_JPARSE_TOKEN_TYPE_OBJECT)) {
            end_token(parser);
            parser->depth -= 1;
            parser->state = OK;
        } else {
            parser->status = STATUS_BAD_FORMAT;
        }
    } break;

    case A_EC: {
        // entity ended by comma
        if (container == NULL) {
            parser->status = STATUS_BAD_FORMAT;
        } else if (container->type == MU_JPARSE_TOKEN_TYPE_OBJECT) {
            if (is_even(child_count(container))) {
                // even numbered children are keys.  need colon, not comma
                parser->status = STATUS_BAD_FORMAT;
            } else {
                end_token(parser);
                parser->state = OK;
            }
        } else /* container->type == MU_JPARSE_TOKEN_TYPE_ARRAY */ {
            // comma separates items in an array
            end_token(parser);
            parser->state = OK;
        }
    } break;

    case A_EK: {
        // entity ended by colon
        if (container == NULL) {
            parser->status = STATUS_BAD_FORMAT;
        } else if (container->type == MU_JPARSE_TOKEN_TYPE_OBJECT) {
            if (!is_even(child_count(container))) {
                // odd numbered children are values.  need comma, not colon
                parser->status = STATUS_BAD_FORMAT;
            } else {
                end_token(parser);
                parser->state = OK;
            }
        } else /* container->type == MU_JPARSE_TOKEN_TYPE_ARRAY */ {
            // array elements need comma, not colon
            parser->status = STATUS_BAD_FORMAT;
        }
    } break;

    case A_EE: {
        // entity ended by end of string
        if (parser->state != OK) {
            // not at top-level parser
            parser->status = STATUS_BAD_FORMAT;
        } else if (parser->depth != 0) {
            // un-terminated array or object
             parser->status = STATUS_BAD_FORMAT;
        } else {
            end_token(parser);
            parser->status = STATUS_COMPLETE;
        }
    } break;
	} // switch
}

static void start_token(parser_t *parser, mu_jparse_token_type_t type, int depth) {
    mu_jparse_jtree_t *tree = parser->tree;
    if (tree->count >= tree->max_tokens) {
        parser->status = STATUS_NO_TOKENS;
    } else {
        mu_jparse_token_t *token = &tree->tokens[tree->count];
        token->type = type;
        token->depth = depth;
        // capture JSON string starting at current char to end of string.
        // The ending char will be fixed up in end_token() [q.v.].
        mu_str_slice(&token->str, parser->json, parser->char_index, MU_STR_END);
    }
}

static void end_token(parser_t *parser) {
    mu_jparse_jtree_t *tree = parser->tree;
    mu_jparse_token_t *token = &tree->tokens[tree->count++];
    if (token->type == MU_JPARSE_TOKEN_TYPE_UNKNOWN) {
        // should not happen.
        asm("nop");
    }
    // the token string currently extends to end of the JSON string.
    // trim it to parser->char_index.
    // TODO: design sensible way to compute slice end...
    //                    +-----------+
    // +------------------------------+
    //                          ^
    // mu_str_slice(&token->str, &token->str, 0, ???);
}
