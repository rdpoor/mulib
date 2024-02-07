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

#define EOL_DEPTH -1

// ****************************************************************************=
// Private (static) storage

// ****************************************************************************=
// Private (forward) declarations

static int parse(mu_json_parser_t *parser, mu_str_t *mu_str);

static bool is_first_token(mu_json_token_t *token);

static bool is_last_token(mu_json_token_t *token);


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

mu_json_token_t *mu_json_token_prev(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    if (is_first_token(token)) {
        return NULL;
    } else {
        return &token[-1];
    }
}

mu_json_token_t *mu_json_token_next(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    if (is_last_token(token)) {
        return NULL;
    } else {
        return &token[1];
    }
}

mu_json_token_t *mu_json_token_root(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    while (!is_first_token(token)) {
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
    // search backwards until a token with equal depth is found
    int depth = mu_json_token_depth(token);
    mu_json_token_t *prev = token;

    do {
        prev = mu_json_token_prev(prev);
    } while (prev != NULL && mu_json_token_depth(prev) > depth);
    return prev;
}

mu_json_token_t *mu_json_token_next_sibling(mu_json_token_t *token) {
    if (token == NULL) {
        return NULL;
    }
    // search forwards until a token with equal depth is found
    int depth = mu_json_token_depth(token);
    mu_json_token_t *next = token;

    do {
        next = mu_json_token_next(next);
    } while (next != NULL && mu_json_token_depth(next) > depth);
    return next;
}

void *mu_json_token_traverse(mu_json_token_t *root, mu_json_visit_fn fn,
                             void *arg, int max_depth) {
    // STUB
    return NULL;
}

// ****************************************************************************=
// Private (static) code

static int parse(mu_json_parser_t *parser, mu_str_t *mu_str) {
    // STUB
    return MU_JSON_ERR_BAD_FORMAT;
    }

static bool is_first_token(mu_json_token_t *token) {
    return token->depth == 0;
}

static bool is_last_token(mu_json_token_t *token) {
    return token->depth == EOL_DEPTH;
}
