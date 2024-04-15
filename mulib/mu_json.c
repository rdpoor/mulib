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

// ****************************************************************************=
// Includes

#include "mu_json.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ****************************************************************************=
// Private types and definitions

#define __   -1     /* the universal error code */

// The 128 ASCII chars map into one of the following character classes.
// See s_c_class_map[]
//
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
typedef enum { DEFINE_CHAR_CLASSES(EXPAND_CH_CLASS_ENUMS) } ch_class_t;

// The JSON parser uses a finite state machine with the following states:
//
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

// States that trigger an action have the 0x80 bit turned on and are handled
// individually.
//
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
    M(A_ES) /* string ended by double quote */                                 \
    M(A_EA) /* entity ended by ] */                                            \
    M(A_EO) /* entity ended by } */                                            \
    M(A_EC) /* entity ended by , */                                            \
    M(A_EK) /* entity ended by : ('key') */                                    \
    M(A_EE) /* entity ended by end of string */

#define EXPAND_ACTION_ENUMS(_name) _name,
typedef enum { DEFINE_ACTIONS(EXPAND_ACTION_ENUMS) } action_t;


// ****************************************************************************=
// Private (static) storage

/**
 * @brief Map an ASCII character to a corresponding character class.  This lets
 * us construct a much more compact state transition table (see below).
 */
// clang-format off
static ch_class_t s_c_class_map[] = {
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

/**
 * @brief The state transition table.  JSON parsing proceeds as follows:
 * - Each input character is mapped to its character class.
 * - The character class selects a column in the current state (row).
 * - A value of '__' (-1) signifies an illegal character for that state
 * - A value of S_xx immediately transitions to that state for the next char.
 * - A value of A_xx dispatches to a function for additional action processing
 * - Parsing continues until the end of string is reached.
 * - If the final state is S_OK and all arrays and objects are closed, the
 *   parsing has completed without error.
 */
// clang-format off
static uint8_t s_state_map[STATE_MAX * C_CLASS_MAX] = {
//
//          space white    {    }    [    ]    :    ,    "    \    /    +    -    .    0  1-9    a    b    c    d    e    f    l    n    r    s    t    u ABCDF   E  etc */
/*value  VA*/ S_VA,S_VA,A_SO,  __,A_SA,  __,  __,  __,A_SS,  __,  __,  __,A_SM,  __,A_SZ,A_SN,  __,  __,  __,  __,  __,A_SF,  __,A_SU,  __,  __,A_ST,  __,  __,  __,  __,
/*ok     OK*/ S_OK,S_OK,  __,A_EO,  __,A_EA,  __,A_EC,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*object OB*/ S_OB,S_OB,  __,A_EO,  __,  __,  __,  __,S_ST,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*key    KE*/ S_KE,S_KE,  __,  __,  __,  __,  __,  __,S_ST,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*colon  CO*/ S_CO,S_CO,  __,  __,  __,  __,A_KE,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*array  AR*/ S_AR,S_AR,A_SO,  __,A_SA,A_EA,  __,  __,S_ST,  __,  __,  __,S_MI,  __,S_ZE,S_IN,  __,  __,  __,  __,  __,S_F1,  __,S_N1,  __,  __,S_T1,  __,  __,  __,  __,
/*string ST*/ S_ST,  __,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,A_ES,S_ES,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,
/*escape ES*/   __,  __,  __,  __,  __,  __,  __,  __,S_ST,S_ST,S_ST,  __,  __,  __,  __,  __,  __,S_ST,  __,  __,  __,S_ST,  __,S_ST,S_ST,  __,S_ST,S_U1,  __,  __,  __,
/*u1     U1*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_U2,S_U2,S_U2,S_U2,S_U2,S_U2,S_U2,S_U2,  __,  __,  __,  __,  __,  __,S_U2,S_U2,  __,
/*u2     U2*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_U3,S_U3,S_U3,S_U3,S_U3,S_U3,S_U3,S_U3,  __,  __,  __,  __,  __,  __,S_U3,S_U3,  __,
/*u3     U3*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_U4,S_U4,S_U4,S_U4,S_U4,S_U4,S_U4,S_U4,  __,  __,  __,  __,  __,  __,S_U4,S_U4,  __,
/*u4     U4*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,S_ST,  __,  __,  __,  __,  __,  __,S_ST,S_ST,  __,
/*minus  MI*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_ZE,S_IN,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*zero   ZE*/ S_OK,S_OK,  __,A_EO,  __,A_EA,  __,A_EC,  __,  __,  __,  __,  __,S_FR,  __,  __,  __,  __,  __,  __,S_E1,  __,  __,  __,  __,  __,  __,  __,  __,S_E1,  __,
/*int    IN*/ S_OK,S_OK,  __,A_EO,  __,A_EA,  __,A_EC,  __,  __,  __,  __,  __,S_FR,S_IN,S_IN,  __,  __,  __,  __,S_E1,  __,  __,  __,  __,  __,  __,  __,  __,S_E1,  __,
/*frac   FR*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_FS,S_FS,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*fracs  FS*/ S_OK,S_OK,  __,A_EO,  __,A_EA,  __,A_EC,  __,  __,  __,  __,  __,  __,S_FS,S_FS,  __,  __,  __,  __,S_E1,  __,  __,  __,  __,  __,  __,  __,  __,S_E1,  __,
/*e      E1*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_E2,S_E2,  __,S_E3,S_E3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*ex     E2*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_E3,S_E3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*exp    E3*/ S_OK,S_OK,  __,A_EO,  __,A_EA,  __,A_EC,  __,  __,  __,  __,  __,  __,S_E3,S_E3,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*tr     T1*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_T2,  __,  __,  __,  __,  __,  __,
/*tru    T2*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_T3,  __,  __,  __,
/*true   T3*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_OK,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*fa     F1*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_F2,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*fal    F2*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_F3,  __,  __,  __,  __,  __,  __,  __,  __,
/*fals   F3*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_F4,  __,  __,  __,  __,  __,
/*false  F4*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_OK,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,
/*nu     N1*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_N2,  __,  __,  __,
/*nul    N2*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_N3,  __,  __,  __,  __,  __,  __,  __,  __,
/*null   N3*/   __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,  __,S_OK,  __,  __,  __,  __,  __,  __,  __,  __
};
// clang-format on

// #define EXPAND_VISIT_FUNCTIONS(_name, _function) _function,
// static mu_json_visit_fn s_action_map[] = { DEFINE_ACTIONS(EXPAND_VISIT_FUNCTIONS) };


// ****************************************************************************=
// Private (forward) declarations

static int parse(mu_json_parser_t *parser, mu_str_t *mu_str);

/**
 * @brief Return the character class for the given character.
 */
static ch_class_t classify_char(uint8_t ch);

/**
 * @brief Reference the index'th token, or NULL if out of bounds.
 */
static mu_json_token_t *token_ref(mu_json_parser_t *parser, int index);

/**
 * @brief Initialize the given token.
 *
 * Slices the original JSON string from ch_index to end of original string.
 * The slice end will be set in the call to token_end (q.v.)
 */
static void token_initialize(mu_json_token_t *token,
                        mu_str_t *mu_str, int ch_index,
                        mu_json_token_type_t type, int tok_index, int16_t depth);

/**
 * @brief Finalize the given token.
 */
static void token_finalize(mu_json_token_t *token, mu_str_t *mu_str,
                           int ch_index);

// ****************************************************************************=
// Public code

mu_json_parser_t *mu_json_parser_init(mu_json_parser_t *parser,
                                      mu_json_token_t *token_store,
                                      size_t max_tokens) {
    parser->tokens = token_store;
    parser->max_tokens = max_tokens;
    parser->status = 0;
    return parser;
}

int mu_json_parse_c_str(mu_json_parser_t *parser, const char *c_str) {
    mu_str_t mu_str;
    return parse(parser, mu_str_init_cstr(&mu_str, c_str));
}

int mu_json_parse_mu_str(mu_json_parser_t *parser, mu_str_t *mu_str) {
    return parse(parser, mu_str);
}

int mu_json_parse_buf(mu_json_parser_t *parser, const uint8_t *buf,
                      size_t buf_length) {
    mu_str_t mu_str;
    return parse(parser, mu_str_init(&mu_str, buf, buf_length));
}

mu_json_token_type_t mu_json_token_type(mu_json_token_t *token) {
    if (token == NULL) {
        return MU_JSON_TOKEN_TYPE_UNKNOWN;
    } else {
        return token->type;
    }
}

int mu_json_token_depth(mu_json_token_t *token) {
    if (token == NULL) {
        return -1;
    } else {
        return token->depth;
    }
}

bool mu_json_token_is_first(mu_json_token_t *token) {
    return token->flags & MU_JSON_TOKEN_FLAG_IS_FIRST;
}

bool mu_json_token_is_last(mu_json_token_t *token) {
    return token->flags & MU_JSON_TOKEN_FLAG_IS_LAST;
}

mu_json_token_t *mu_json_token_prev(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    } else if (mu_json_token_is_first(token)) {
        return NULL;
    } else {
        return &token[-1];
    }
}

mu_json_token_t *mu_json_token_next(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    } if (mu_json_token_is_last(token)) {
        return NULL;
    } else {
        return &token[1];
    }
}

mu_json_token_t *mu_json_token_root(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    while (!mu_json_token_is_first(token)) {
        token = mu_json_token_prev(token);
    }
    return token;
}

mu_json_token_t *mu_json_token_parent(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    // search backwards until a token with a shallower depth is found
    int depth = mu_json_token_depth(token);
    mu_json_token_t *prev = token;

    do {
        prev = mu_json_token_prev(prev);
    } while (prev != NULL && mu_json_token_depth(prev) >= depth);
    return prev;
}

mu_json_token_t *mu_json_token_child(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    int depth = mu_json_token_depth(token);
    mu_json_token_t *next = mu_json_token_next(token);
    if (mu_json_token_depth(next) > depth) {
        return next;
    } else {
        return NULL;
    }
}

mu_json_token_t *mu_json_token_prev_sibling(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    mu_json_token_t *prev = mu_json_token_prev(token);
    while (true) {
        if (prev == NULL) {
            return NULL;
        } else if (mu_json_token_depth(prev) < mu_json_token_depth(token)) {
            return NULL;
        } else if (mu_json_token_depth(prev) == mu_json_token_depth(token)) {
            return prev;
        } else {
            prev = mu_json_token_prev(prev);
        }
    }
}

mu_json_token_t *mu_json_token_next_sibling(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    mu_json_token_t *next = mu_json_token_next(token);
    while (true) {
        if (next == NULL) {
            return NULL;
        } else if (mu_json_token_depth(next) < mu_json_token_depth(token)) {
            return NULL;
        } else if (mu_json_token_depth(next) == mu_json_token_depth(token)) {
            return next;
        } else {
            next = mu_json_token_next(next);
        }
    }
}

void *mu_json_token_traverse(mu_json_token_t *root, mu_json_visit_fn fn,
                             void *arg, int max_depth) {
    // STUB
    return NULL;
}

// ****************************************************************************=
// Private (static) code

// typedef struct {
//     mu_json_token_t *tokens; // an array of tokens
//     size_t max_tokens;       // maximum number of available tokens
//     int status;              // status < 0 indicates error, else # of tokens
// } mu_json_parser_t;

static int parse(mu_json_parser_t *parser, mu_str_t *mu_str) {
    uint8_t ch;
    ch_class_t ch_class;
    uint8_t curr_state;
    uint8_t next_state;
    mu_token_t *token;

    bool is_valid = true;  // set to false to terminate parsing
    int16_t depth = 0;
    int ch_index = 0;
    int tok_index = 0;
    curr_state = S_VA;

    while (is_valid) {
        if (!mu_str_get_byte(mu_str, ch_index, &ch)) {
            // end of string
            break;
        } else if ((ch_class = classify_char(ch)) == -1) {
            // illegal character class
            is_valid = false;
            break;
        }

        next_state = s_state_map[curr_state * C_CLASS_MAX + ch_class];

        // Within this switch(), only Action states are handled.  Non action
        // states simply proceed to the next character.

        switch (next_state) {
        case A_SA: {
            // starting array with [
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_ARRAY, tok_index, depth);
                depth += 1;
                next_state = S_AR;
            }
            break;
        }
        case A_SO: {
            // starting object with {
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_OBJECT, tok_index, depth);
                depth += 1;
                next_state = S_OB;
            }
            break;
        }
        case A_SS: {
            // starting object with "
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_STRING, tok_index, depth);
                next_state = S_ST;
            }
            break;
        }
        case A_SM: {
            // starting number with -
            // Assume this will be MU_JSON_TOKEN_TYPE_INTEGER, but convert it to
            // MU_JSON_TOKEN_TYPE_NUMBER if a decimal point is subsequently seen
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_INTEGER, tok_index, depth);
                next_state = S_MI;
            }
            break;
        }
        case A_SZ: {
            // starting number with 0
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_INTEGER, tok_index, depth);
                next_state = S_ZE;
            }
            break;
        }
        case A_SN: {
            // starting number with 1-9
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_INTEGER, tok_index, depth);
                next_state = S_IN;
            }
            break;
        }
        case A_ST: {
            // starting true with t (as in true)
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_TRUE, tok_index, depth);
                next_state = S_T1;
            }
            break;
        }
        case A_SF: {
            // starting false with f (as in false)
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_FALSE, tok_index, depth);
                next_state = S_F1;
            }
            break;
        }
        case A_SU: {
            // starting null with n
            if ((token = token_ref(parser, tok_index++)) == NULL) {
                // could not allocate token
                is_valid = false;
            } else {
                token_initialize(token, mu_str, ch_index, MU_JSON_TOKEN_TYPE_NULL, tok_index, depth);
                next_state = S_N1;
            }
            break;
        }
        case A_ES: {
            // string ended by "
            mu_json_token_t *token = token_ref(parser, tok_index);
            token_finalize(token, mu_str, ch_index);
            if (is_object_key(token)) {
                next_state = S_CO;
            } else if (is_in_array(token) || (is_in_object(token))) {
                next_state = S_OK;
            } else {
                // TODO: honor ALLOW_TOP_LEVEL_COMMAS
                next_state = -1;
            }
            break;
        }
        case A_EA: {
            // entity ended by ]
            mu_json_token_t *token = token_ref(parser, tok_index);
            // starting with token, search backwards for [
            while ((token != NULL) && (mu_json_token_type(token) != MU_JSON_TOKEN_TYPE_ARRAY)) {
                token = mu_json_token_prev(token);
            }
            if (token == NULL) {
                // [ not found...
                is_valid = false;
            } else {
                // found opening [
                token_finalize(token, mu_str, ch_index);
                depth -= 1;
                next_state = S_OK;
            }
            break;
        }
        case A_EO: {
            // entity ended by }
            mu_json_token_t *token = token_ref(parser, tok_index);
            // starting with current token, search backwards for {
            while ((token != NULL) && (mu_json_token_type(token) != MU_JSON_TOKEN_TYPE_OBJECT)) {
                token = mu_json_token_prev(token);
            }
            if (token == NULL) {
                // { not found...
                is_valid = false;
            } else {
                // found opening {
                token_finalize(token, mu_str, ch_index);
                depth -= 1;
                next_state = S_OK;
            }
            break;
        }
        case A_EC: {
            // entity ended by ,
            token_end(parser, tok_index, mu_str, ch_index);
            if (is_in_object(token)) {
                if (is_key(token)) {
                    // key needs : termination, not comma
                    next_state = -1;
                } else {
                    // comma signifies start of next key
                    next_state = S_KE;
                }
#ifdef ALLOW_TOP_LEVEL_COMMAS
            } else {
                // ready for any value
                next_state = S_VA;
            }
#elif
            } else if (is_in_array(token)) {
                // ready for any value
                next_state = S_VA;
            } else {
                // commas must be inside array
                next_state = -1;
            }
#endif
            break;
        }
        case A_EK: {
            // entity ended by : ('key') [only if non-string keys are allowed]
            token_end(parser, tok_index, mu_str, ch_index);
            if (is_in_object(token) && is_key(token)) {
                next_state = S_VA;
            } else {
                next_state = -1;
            }
            break;
        }
        case A_EE: {
            // entity ended by end of string
            token_end(parser, tok_index, mu_str, ch_index);
            next_state = ??;
            break;
        }
        case __: {
            // illegal state.
            is_valid = false;
            break;
        }
        } // switch

        if (is_valid) {
            // transition to next state, advance to next character
            curr_state = next_state;
            ch_index += 1;
        }
    } // while(is_valid)

    // Endgame: here on illegal state, illegal char class or end of string
    // STUB
    return MU_JSON_ERR_BAD_FORMAT;
}

static ch_class_t classify_char(uint8_t ch) {
    if (ch >= sizeof(s_c_class_map)) {
        return -1;
    } else {
        return s_c_class_map[ch];
    }
}


static mu_json_token_t *token_ref(mu_json_parser_t *parser, int index) {
    if (index < 0 || index >= parser->max_tokens) {
        return NULL;
    } else {
        return &parser->tokens[index];
    }
}

static void token_initialize(mu_json_token_t *token,
                        mu_str_t *mu_str, int ch_index,
                        mu_json_token_type_t type, int tok_index, int16_t depth) {
    // This token starts at ch_index, but we don't yet know where it ends, so we
    // set it to the end of the input string.  This will be updated in
    // token_finalize() [qv]
    mu_str_slice(&token->str, mu_str, ch_index, MU_STR_END);
    token->type = type;
    token->flags = tok_index == 0 ? MU_JSON_TOKEN_FLAG_IS_FIRST : 0;
    token->depth = depth;
}

static void token_finalize(mu_json_token_t *token, mu_str_t *mu_str,
                           int ch_index) {
    // on entry, token->str extends from the token start to the end of
    // mu_str.  slice it to end at ch_index.
    int start_index = mu_str_length(mu_str) - mu_str_length(&token->str);
    int token_length = ch_index - start_index;
    mu_str_slice(&token->str, &token->str, 0, token_length);
}
