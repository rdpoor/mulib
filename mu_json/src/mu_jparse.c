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
    NR_CH_CLASSES
} ch_class_t;

typedef struct {
	mu_jparse_jtree_t *tree;  // the array of tokens and overall count / status
	mu_str_t *json;           // the JSON string being parsed
	int char_index;           // index the n'th byte of the JSON string
} parser_t;

// ****************************************************************************=
// Private (static) storage

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
	return false;
}
