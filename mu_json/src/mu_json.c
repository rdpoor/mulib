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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ****************************************************************************=
// Private types and definitions

#define DEBUG_TRACE
#ifdef DEBUG_TRACE
#include <stdio.h>
#define TRACE_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define TRACE_PRINTF(...)
#endif

#define __ (uint8_t)-1 /* the universal error code */

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
    M(NR_CLASSES)

#define EXPAND_CH_CLASS_ENUMS(_name) _name,
typedef enum { DEFINE_CHAR_CLASSES(EXPAND_CH_CLASS_ENUMS) } ch_class_t;

// The JSON parser uses a finite state machine with the following states.
// States before NR_STATES simply transition from one state to another with
// each character read.  States following (and with mixed-case symbol names)
// perform some action prior to transitioning to another state.
#define DEFINE_STATES(M)                                                       \
    M(GO, "start")                                                             \
    M(OK, "ok")                                                                \
    M(OB, "object")                                                            \
    M(KE, "key")                                                               \
    M(CO, "colon")                                                             \
    M(VA, "value")                                                             \
    M(AR, "array")                                                             \
    M(ST, "string")                                                            \
    M(ES, "escape")                                                            \
    M(U1, "u1")                                                                \
    M(U2, "u2")                                                                \
    M(U3, "u3")                                                                \
    M(U4, "u4")                                                                \
    M(MI, "minus")                                                             \
    M(ZE, "zero")                                                              \
    M(IN, "integer")                                                           \
    M(FR, "fraction")                                                          \
    M(FS, "fraction")                                                          \
    M(E1, "e")                                                                 \
    M(E2, "ex")                                                                \
    M(E3, "exp")                                                               \
    M(T1, "tr")                                                                \
    M(T2, "tru")                                                               \
    M(T3, "true")                                                              \
    M(F1, "fa")                                                                \
    M(F2, "fal")                                                               \
    M(F3, "fals")                                                              \
    M(F4, "false")                                                             \
    M(N1, "nu")                                                                \
    M(N2, "nul")                                                               \
    M(N3, "null")                                                              \
    M(NR_STATES, "actions follow")                                             \
    M(Ec, "empty }")                                                           \
    M(Co, "close }")                                                           \
    M(Ca, "close ]")                                                           \
    M(Oo, "open {")                                                            \
    M(Oa, "open [")                                                            \
    M(Os, "open \"")                                                           \
    M(Cs, "close \"")                                                          \
    M(Cm, "comma")                                                             \
    M(Cl, "colon")

#define EXPAND_STATE_ENUMS(_name, _description) _name,
enum { DEFINE_STATES(EXPAND_STATE_ENUMS) };

typedef struct {
    mu_str_t *json;          // the JSON source string
    mu_json_token_t *tokens; // an array of tokens
    size_t max_tokens;       // maximum number of available tokens
    int token_count;         // # of allocated tokens
    int depth;               // current depth
    int char_pos;            // position of char being parsed
    mu_json_err_t error;     // error status
} parser_t;

// ****************************************************************************=
// Private (static) storage

/**
 * @brief Map an ASCII character to a corresponding character class.  This lets
 * us construct a much more compact state transition table (see below).
 */
// clang-format off
static int ascii_classes[128] = {
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
 * @brief Map a [state, char_class] to a new state or action.
 *
 * This state transition table takes the current state and the current symbol,
 * and returns either a new state or an action. An action is signified by a
 * mixed-case symbol and has a value greater than NR_STATES. JSON text is
 * accepted if at the end of the text the state is OK and if the mode is
 * MODE_DONE.
 */
// clang-format off
static int state_transition_table[NR_STATES * NR_CLASSES] = {
/*               white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ GO,GO,Oo,__,Oa,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*ok     OK*/ OK,OK,__,Co,__,Ca,__,Cm,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*object OB*/ OB,OB,__,Ec,__,__,__,__,Os,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*key    KE*/ KE,KE,__,__,__,__,__,__,Os,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*colon  CO*/ CO,CO,__,__,__,__,Cl,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*value  VA*/ VA,VA,Oo,__,Oa,__,__,__,Os,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__,
/*array  AR*/ AR,AR,Oo,__,Oa,Ca,__,__,Os,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__,
/*string ST*/ ST,__,ST,ST,ST,ST,ST,ST,Cs,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,
/*escape ES*/ __,__,__,__,__,__,__,__,Os,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__,
/*u1     U1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__,
/*u2     U2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__,
/*u3     U3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__,
/*u4     U4*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__,
/*minus  MI*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*zero   ZE*/ OK,OK,__,Co,__,Ca,__,Cm,__,__,__,__,__,FR,__,__,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__,
/*int    IN*/ OK,OK,__,Co,__,Ca,__,Cm,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__,
/*frac   FR*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,FS,FS,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*fracs  FS*/ OK,OK,__,Co,__,Ca,__,Cm,__,__,__,__,__,__,FS,FS,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__,
/*e      E1*/ __,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*ex     E2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*exp    E3*/ OK,OK,__,Co,__,Ca,__,Cm,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*tr     T1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__,
/*tru    T2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__,
/*true   T3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__,
/*fa     F1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*fal    F2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__,
/*fals   F3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__,
/*false  F4*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__,
/*nu     N1*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__,
/*nul    N2*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__,
/*null   N3*/ __,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__
};
// clang-format on

/**
 * @brief Modes of the parser.
 */
#define DEFINE_MODES(M)                                                        \
    M(MODE_ARRAY)  /* working inside an array */                               \
    M(MODE_DONE)   /* "top of stack", okay if input ends now */                \
    M(MODE_KEY)    /* working inside an object, expecting a key */             \
    M(MODE_OBJECT) /* working inside an object, expecting a value */

#define EXPAND_MODE_ENUMS(_name) _name,
enum { DEFINE_MODES(EXPAND_MODE_ENUMS) };

// *****************************************************************************
// start DEBUG_TRACE support
#ifdef DEBUG_TRACE

#define EXPAND_CH_CLASS_NAMES(_name) #_name,
__attribute__((unused)) static const char *s_ch_class_names[] = {
    DEFINE_CHAR_CLASSES(EXPAND_CH_CLASS_NAMES)};

#define N_CH_CLASSES sizeof(s_ch_class_names) / sizeof(s_ch_class_names[0])

#define EXPAND_STATE_NAMES(_name, _description) #_name,
__attribute__((unused)) static const char *s_state_names[] = {
    DEFINE_STATES(EXPAND_STATE_NAMES)};

#define EXPAND_STATE_DESCRIPTIONS(_name, _description) _description,
__attribute__((unused)) static const char *s_state_descriptions[] = {
    DEFINE_STATES(EXPAND_STATE_DESCRIPTIONS)};

#define N_STATES sizeof(s_state_names) / sizeof(s_state_names[0])

__attribute__((unused)) static const char *ch_class_name(ch_class_t ch_class) {
    if (ch_class >= 0 && ch_class < N_CH_CLASSES) {
        return s_ch_class_names[ch_class];
    } else {
        return "CH_UNK?";
    }
}

__attribute__((unused)) static const char *state_name(int state) {
    if (state >= 0 && state < N_STATES) {
        return s_state_names[state];
    } else {
        return "__";
    }
}

__attribute__((unused)) static const char *state_description(int state) {
    if (state >= 0 && state < N_STATES) {
        return s_state_descriptions[state];
    } else {
        return "unknown state";
    }
}

#define EXPAND_MODE_NAMES(_name) #_name,
__attribute__((unused)) static const char *s_mode_names[] = {
    DEFINE_MODES(EXPAND_MODE_NAMES)};

#define N_MODES sizeof(s_mode_names) / sizeof(s_mode_names[0])

__attribute__((unused)) static const char *mode_name(int mode) {
    if (mode >= 0 && mode < N_MODES) {
        return s_mode_names[mode];
    } else {
        return "unknown mode";
    }
}

__attribute__((unused)) static const char *error_name(mu_json_err_t error) {
    if (error == MU_JSON_ERR_NONE) {
        return "MU_JSON_ERR_NONE";
    } else if (error == MU_JSON_ERR_BAD_FORMAT) {
        return "MU_JSON_ERR_BAD_FORMAT";
    } else if (error == MU_JSON_ERR_NO_TOKENS) {
        return "MU_JSON_ERR_NO_TOKENS";
    } else {
        return "UNKNOWN ERROR";
    }
}

#endif
// end DEBUG_TRACE support
// *****************************************************************************

// ****************************************************************************=
// Private (forward) declarations

static int parse(mu_json_token_t *tokens, size_t max_tokens,
                 mu_str_t *json_input);

/**
 * @brief Return the character class for the given character.
 */
static int classify_char(uint8_t ch);

/**
 * @brief Examine the allocated token list to determine what type of tokens
 * the parser can accept.
 */
static int get_mode(parser_t *parser);

/**
 * @brief Initialize and add a token to the token list.
 *
 * Initialize the token using the current char_pos and depth from the parser.
 *
 * Return false if no tokens are avaialble.
 */
static bool start_token(parser_t *parser, mu_json_token_type_t type);

/**
 * @brief Complete a token.
 * 
 * Trim the tooken's JSON slice end at the current char_pos and seal it.
 */
static void finish_token(parser_t *parser, mu_json_token_t *token);

/**
 * @brief Map a (state, char_class) pair to a new state.
 */
static int lookup_state(int state, int char_class);

/**
 * @broef Return true if this token is "sealed", that is, the end of the token
 * has been found.  For containers (like ARRAY and OBJECT) this means that no
 * more items will be added to it.
 */
static bool token_is_sealed(mu_json_token_t *token);

/**
 * @brief Seal a token.
 */
static void token_seal(mu_json_token_t *token);

/**
 * @brief Return the most recently allocated token or NULL if none have been
 * allocated.
 */
static mu_json_token_t *top_token(parser_t *parser);

// ****************************************************************************=
// Public code

int mu_json_parse_c_str(mu_json_token_t *token_store, size_t max_tokens,
                        const char *json) {
    mu_str_t mu_str;
    return parse(token_store, max_tokens, mu_str_init_cstr(&mu_str, json));
}

int mu_json_parse_mu_str(mu_json_token_t *token_store, size_t max_tokens,
                         mu_str_t *mu_json) {
    return parse(token_store, max_tokens, mu_json);
}

int mu_json_parse_buffer(mu_json_token_t *token_store, size_t max_tokens,
                         const uint8_t *buf, size_t buflen) {
    mu_str_t mu_str;
    return parse(token_store, max_tokens, mu_str_init(&mu_str, buf, buflen));
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
    }
    if (mu_json_token_is_last(token)) {
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

static int parse(mu_json_token_t *token_store, size_t max_tokens,
                 mu_str_t *json_input) {
    parser_t parser;
    parser.json = json_input;
    parser.tokens = token_store;
    parser.max_tokens = max_tokens;
    parser.token_count = 0;
    parser.depth = 0;
    parser.char_pos = 0;
    parser.error = MU_JSON_ERR_NONE;

    uint8_t ch;
    int char_class;
    int curr_state = GO;
    int next_state;

    TRACE_PRINTF("\n==== parsing '%.*s'", (int)mu_str_length(json_input),
                 mu_str_buf(json_input));

    while (true) {
        if (parser.error != MU_JSON_ERR_NONE) {
            // allocation or format error
            break;
        }
        if (!mu_str_get_byte(parser.json, parser.char_pos, &ch)) {
            // end of string reached
            TRACE_PRINTF("\n...eos");
            break;
        }
        if ((char_class = classify_char(ch)) < 0) {
            // illegal character in json_input
            parser.error = MU_JSON_ERR_BAD_FORMAT;
            TRACE_PRINTF("\n'%c': illegal character", ch);
            break;
        }

        next_state = lookup_state(curr_state, char_class);

        TRACE_PRINTF("\n%d %d %10s '%c': %s %s => %s", parser.token_count,
                     parser.depth, mode_name(get_mode(&parser)), ch,
                     ch_class_name(char_class), state_name(curr_state),
                     state_name(next_state));

        if (next_state >= 0 && next_state < NR_STATES) {
            // Simple state transition w/o special action
            curr_state = next_state;

        } else {
            // This state requires special action before transitioning.
            switch (next_state) {
            case Ec: {
                // } - close empty object
                if (get_mode(&parser) != MODE_KEY) {
                    parser.error = MU_JSON_ERR_BAD_FORMAT;
                } else {
                    curr_state = OK;
                    finish_token(&parser, top_token(&parser));
                    token_seal(top_token(&parser));
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                }
                break;
            }
            case Co: {
                // } - close object
                if (get_mode(&parser) != MODE_KEY) {
                    parser.error = MU_JSON_ERR_BAD_FORMAT;
                } else {
                    curr_state = OK;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                }
                break;
            }

            case Ca: {
                // ] - close array
                if (get_mode(&parser) != MODE_ARRAY) {
                    parser.error = MU_JSON_ERR_BAD_FORMAT;
                } else {
                    curr_state = OK;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                }
                break;
            }

            case Oo: {
                // { - open object
                if (!start_token(&parser, MU_JSON_TOKEN_TYPE_OBJECT)) {
                    // out of tokens
                    parser.error = MU_JSON_ERR_NO_TOKENS;
                } else {
                    parser.depth += 1;
                    curr_state = OB;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                }
                break;
            }

            case Oa: {
                // [ - open array
                if (!start_token(&parser, MU_JSON_TOKEN_TYPE_ARRAY)) {
                    // out of tokens
                    parser.error = MU_JSON_ERR_NO_TOKENS;
                } else {
                    parser.depth += 1;
                    curr_state = AR;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                }
                break;
            }

            case Os: {
                // " - open a string
                if (!start_token(&parser, MU_JSON_TOKEN_TYPE_STRING)) {
                    // out of tokens
                    parser.error = MU_JSON_ERR_NO_TOKENS;
                } else {
                    curr_state = ST;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                }
                break;
            }
            case Cs: {
                // " - close string
                finish_token(&parser, top_token(&parser));
                switch (get_mode(&parser)) {
                case MODE_OBJECT: {
                    break;
                }
                case MODE_KEY: {
                    curr_state = CO;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                    break;
                }
                case MODE_ARRAY: {
                    curr_state = OK;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                    break;
                }
                case MODE_DONE: {
                    break;
                }
                } // switch (get_mode(&parser))
                break;
            }

            case Cm: {
                // comma seen
                switch (get_mode(&parser)) {
                case MODE_OBJECT: {
                    break;
                }
                case MODE_KEY: {
                    break;
                }
                case MODE_ARRAY: {
                    curr_state = VA;
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                    break;
                }
                case MODE_DONE: {
                    break;
                }
                } // switch (get_mode(&parser))
                break;
            }

            case Cl: {
                // colon seen
                switch (get_mode(&parser)) {
                case MODE_OBJECT: {
                    break;
                }
                case MODE_KEY: {
                    curr_state = VA;
                    token_seal(top_token(&parser));
                    TRACE_PRINTF(" => %s", state_name(curr_state));
                    break;
                }
                case MODE_ARRAY: {
                    break;
                }
                case MODE_DONE: {
                    break;
                }
                } // switch (get_mode(&parser))
                break;
            }
            default: {
                // Bad action.
                parser.error = MU_JSON_ERR_BAD_FORMAT;
                break;
            }

            } // switch(next_state)
        }

        // advance to next char
        parser.char_pos += 1;

    } // while(true)

    // endgame: do we have a complete form?
    if (parser.error == MU_JSON_ERR_NONE) {
        TRACE_PRINTF("\nSuccess: %d token%s\n", parser.token_count, 
            parser.token_count == 1 ? "" : "s");
        return parser.token_count;
    } else {
        TRACE_PRINTF("\nFailed with %s\n", error_name(parser.error));
        return parser.error;
    }
}

static int classify_char(uint8_t ch) {
    if (ch >= sizeof(ascii_classes) / sizeof(ascii_classes[0])) {
        return C_ETC;
    } else {
        return ascii_classes[ch];
    }
}

// Return one of four values:
// MODE_DONE if the parser is at top level, either with zero elements or with 
// no "unclosed" containers.
// MODE_ARRAY if the parser is inside an array, that is, the next element to be
// added will be a child of the array or will be the closing of the array.
// MODE_KEY if the parser is inside an object and expecting a key or end of
// object
// MODE_OBJECT if the parser is inside an object and expecting a value
static int get_mode(parser_t *parser) {
    // Return the current mode of the parser:
    // '...{'  => mode = MODE_OBJECT (expecing a key)
    // '...{"asdf"'  => mode = MODE_KEY (expecing ': <vlaue>')
    // '...{"asdf": 1' => mode = MODE_OBJECT (expecting ', "<key>')
    if (parser->token_count == 0) {
        // no tokens -- assume done.
        TRACE_PRINTF(" [MODE_DONE - no tokens] ");
        return MODE_DONE; // no tokens, assume done.
    }

    // examine topmost token
    mu_json_token_t *token = top_token(parser);
    if ((token->type == MU_JSON_TOKEN_TYPE_ARRAY) && !token_is_sealed(token)) {
        // token is an empty array 
        TRACE_PRINTF(" [MODE_ARRAY - unsealed ARRAY] ");
        return MODE_ARRAY;
    } else if ((token->type == MU_JSON_TOKEN_TYPE_OBJECT) && !token_is_sealed(token)) {
        // token is an empty object - expecting a key
        TRACE_PRINTF(" [MODE_KEY - unsealed OBJECT] ");
        return MODE_KEY;
    }

    // get parent of topmost token
    mu_json_token_t *parent = mu_json_token_parent(token);
    if (parent == NULL) {
        // token is "naked" toplevel value (not within object or array)
        TRACE_PRINTF(" [MODE_DONE - no parent] ");
        return MODE_DONE;
    } else if (parent->type == MU_JSON_TOKEN_TYPE_ARRAY) {
        // token's parent is an array
        TRACE_PRINTF(" [MODE_ARRAY - ARRAY parent] ");
        return MODE_ARRAY;
    }

    // token is inside an object.  determine if it is in key or value position
    // by counting how many siblings lie between token and parent
    int distance = 0;
    while ((token = mu_json_token_prev_sibling(token)) != NULL) {
        distance += 1;
    }
    if (distance & 0x01) {
        // odd number: token was in value position
        TRACE_PRINTF(" [MODE_OBJECT - distance = %d] ", distance);
        return MODE_OBJECT;
    } else {
        // zero or even number: token is a key
        TRACE_PRINTF(" [MODE_KEY - distance = %d] ", distance);
        return MODE_KEY;
    }
}

static bool start_token(parser_t *parser, mu_json_token_type_t type) {
    if (parser->token_count >= parser->max_tokens) {
        return false;
    }
    mu_json_token_t *token = &parser->tokens[parser->token_count++];
    // Since we haven't parsed to the end of this token yet, initialize the
    // token's string to start at char_pos and extend to the end of the input
    // string.  This will get adjusted in a call to finish_token() [q.v.].
    mu_str_slice(&token->json, parser->json, parser->char_pos, MU_STR_END);
    token->type = type;
    token->flags |= parser->token_count == 1 ? MU_JSON_TOKEN_FLAG_IS_FIRST : 0;
    token->depth = parser->depth;
    return true;
}

static void finish_token(parser_t *parser, mu_json_token_t *token) {
    // TODO: Add mu_json_token_type_t arg to confim we're closing the right one
    // on entry, token->json extends from the token start to the end of
    // the input string.  slice it to end at parser->char_pos.
    //
    // How it works:
    // start_index is the index of the start of the token's string _within the
    // original input string_.  
    int start_index = mu_str_length(parser->json) - mu_str_length(&token->json);
    // Re-slice to start at start_index and end at parser->char_pos.
    mu_str_slice(&token->json, parser->json, start_index, parser->char_pos);
}

static int lookup_state(int state, int char_class) {
    return state_transition_table[state * NR_CLASSES + char_class];
}

static bool token_is_sealed(mu_json_token_t *token) {
    return token->flags & MU_JSON_TOKEN_FLAG_IS_SEALED;
}

static void token_seal(mu_json_token_t *token) {
    token->flags |= MU_JSON_TOKEN_FLAG_IS_SEALED;
}

static mu_json_token_t *top_token(parser_t *parser) {
    if (parser->token_count == 0) {
        return NULL;
    } else {
        return &parser->tokens[parser->token_count - 1];
    }
}
