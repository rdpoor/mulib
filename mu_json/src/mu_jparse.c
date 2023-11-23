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

typedef enum {
    CH_SPACE,  /* space */
    CH_WHITE,  /* other whitespace */
    CH_LCURB,  /* {  */
    CH_RCURB,  /* } */
    CH_LSQRB,  /* [ */
    CH_RSQRB,  /* ] */
    CH_COLON,  /* : */
    CH_COMMA,  /* , */
    CH_QUOTE,  /* " */
    CH_BACKS,  /* \ */
    CH_SLASH,  /* / */
    CH_PLUS,   /* + */
    CH_MINUS,  /* - */
    CH_POINT,  /* . */
    CH_ZERO ,  /* 0 */
    CH_DIGIT,  /* 123456789 */
    CH_LOW_A,  /* a */
    CH_LOW_B,  /* b */
    CH_LOW_C,  /* c */
    CH_LOW_D,  /* d */
    CH_LOW_E,  /* e */
    CH_LOW_F,  /* f */
    CH_LOW_L,  /* l */
    CH_LOW_N,  /* n */
    CH_LOW_R,  /* r */
    CH_LOW_S,  /* s */
    CH_LOW_T,  /* t */
    CH_LOW_U,  /* u */
    CH_ABCDF,  /* ABCDF */
    CH_E,      /* E */
    CH_EOS,    /* end of string */
    CH_ETC,    /* everything else */
    CH_CLASS_MAX
} ch_class_t;

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

typedef enum {
	A_SA,   // start array
	A_EA,   // end array
	A_SO,   // start object
	A_EO,   // end object
	A_SS,   // start string
	A_ES,   // end string
	A_SN,   // start number
	A_EN,   // end number
	A_ST,   // start true
	A_ET,   // end true
	A_SF,   // start false
	A_EF,   // end false
	A_SU,   // start nUll
	A_EU,   // end nUll
} action_t;

/**
 * @brief Set the 0x80 bit to signify this is an action rather than a state
 */
#define A(a) ((state_t)((a) | 0x80))

typedef struct {
	mu_jparse_jtree_t *tree;  // the array of tokens and overall count / status
	mu_str_t *json;           // the JSON string being parsed
	int char_index;           // index the n'th byte of the JSON string
} parser_t;

// ****************************************************************************=
// Private (static) storage

/**
 * @brief Map an ASCII character to a ch_state_t.
 */
static ch_class_t s_ch_class_map[] = {
    __,       __,       __,       __,       __,       __,       __,       __,
    __,       CH_WHITE, CH_WHITE, __,       __,       CH_WHITE, __,       __,
    __,       __,       __,       __,       __,       __,       __,       __,
    __,       __,       __,       __,       __,       __,       __,       __,

    CH_SPACE, CH_ETC,   CH_QUOTE, CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,
    CH_ETC,   CH_ETC,   CH_ETC,   CH_PLUS,  CH_COMMA, CH_MINUS, CH_POINT, CH_SLASH,
    CH_ZERO,  CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT,
    CH_DIGIT, CH_DIGIT, CH_COLON, CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,

    CH_ETC,   CH_ABCDF, CH_ABCDF, CH_ABCDF, CH_ABCDF, CH_E,     CH_ABCDF, CH_ETC,
    CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,
    CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,
    CH_ETC,   CH_ETC,   CH_ETC,   CH_LSQRB, CH_BACKS, CH_RSQRB, CH_ETC,   CH_ETC,

    CH_ETC,   CH_LOW_A, CH_LOW_B, CH_LOW_C, CH_LOW_D, CH_LOW_E, CH_LOW_F, CH_ETC,
    CH_ETC,   CH_ETC,   CH_ETC,   CH_ETC,   CH_LOW_L, CH_ETC,   CH_LOW_N, CH_ETC,
    CH_ETC,   CH_ETC,   CH_LOW_R, CH_LOW_S, CH_LOW_T, CH_LOW_U, CH_ETC,   CH_ETC,
    CH_ETC,   CH_ETC,   CH_ETC,   CH_LCURB, CH_ETC,   CH_RCURB, CH_ETC,   CH_ETC
};

static int8_t s_state_map[STATE_MAX][CH_CLASS_MAX] = {
	{},
	{}
};

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
static ch_class_t classify_next_char(parser_t *parser)


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
	ch_class_t ch_class = classify_next_char(parser);
	state_t next_state = get_state(parser->state, ch_class);
	if (state_is_action(next_state)) {
		process_action(parser, next_state);
	} else {
		parser->state = next_state;
	}
	return false;
}

static ch_class_t classify_next_char(parser_t *parser) {
	if (parser->char_index >= mu_str_length(parser->json)) {
		// At end of JSON string, class => CL_EOS
		return CH_EOS;
	}

    // TODO: implement mu_str_ref()?
	uint8_t ch = mu_str_bytes(parser->json)[parser->char_index++];

	if (ch > sizeof(s_ch_class_map)) {
		// ch out of range of lookup table.  class => CL_ETC
		return CH_ETC;
	} else {
		// look up ch_class from table.
		return s_ch_class_map[ch];
	}
}
