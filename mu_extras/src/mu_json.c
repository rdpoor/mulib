/**
 * @file mu_json.c
 *
 * MIT License
 *
 * Copyright (c) 2020-2023 R. D. Poor <rdpoor@gmail.com>
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
 * IMPLEMENTATION
 * Based on JSON parser in https://github.com/douglascrockford/JSON-c
 */

// *****************************************************************************
// Includes

#include "mu_json.h"

// *****************************************************************************
// Private types and definitions

typedef struct {
    mu_str_t *str;
    int index;
} default_reader_state_t

// *****************************************************************************
// Private (static) storage

// clang-format off
static int state_transition_table[NR_STATES][NR_CLASSES] = {
/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is represented as a
    negative number. A JSON text is accepted if at the end of the text the
    state is OK and if the mode is MODE_DONE.

                 white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ { GO, GO,AM6, __,AM5, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*ok     OK*/ { OK, OK, __,AM8, __,AM7, __,AM3, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*object OB*/ { OB, OB, __,AM9, __, __, __, __, ST, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*key    KE*/ { KE, KE, __, __, __, __, __, __, ST, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*colon  CO*/ { CO, CO, __, __, __, __,AM2, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*value  VA*/ { VA, VA,AM6, __,AM5, __, __, __, ST, __, __, __, MI, __, ZE, IN, __, __, __, __, __, F1, __, N1, __, __, T1, __, __, __, __},
/*array  AR*/ { AR, AR,AM6, __,AM5,AM7, __, __, ST, __, __, __, MI, __, ZE, IN, __, __, __, __, __, F1, __, N1, __, __, T1, __, __, __, __},
/*string ST*/ { ST, __, ST, ST, ST, ST, ST, ST,AM4, ES, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST},
/*escape ES*/ { __, __, __, __, __, __, __, __, ST, ST, ST, __, __, __, __, __, __, ST, __, __, __, ST, __, ST, ST, __, ST, U1, __, __, __},
/*u1     U1*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, U2, U2, U2, U2, U2, U2, U2, U2, __, __, __, __, __, __, U2, U2, __},
/*u2     U2*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, U3, U3, U3, U3, U3, U3, U3, U3, __, __, __, __, __, __, U3, U3, __},
/*u3     U3*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, U4, U4, U4, U4, U4, U4, U4, U4, __, __, __, __, __, __, U4, U4, __},
/*u4     U4*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, ST, ST, ST, ST, ST, ST, ST, ST, __, __, __, __, __, __, ST, ST, __},
/*minus  MI*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, ZE, IN, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*zero   ZE*/ {AZE,AZE, __,AM8, __,AM7, __,AM3, __, __, __, __, __, FR, __, __, __, __, __, __, E1, __, __, __, __, __, __, __, __, E1, __},
/*int    IN*/ {AIN,AIN, __,AM8, __,AM7, __,AM3, __, __, __, __, __, FR, IN, IN, __, __, __, __, E1, __, __, __, __, __, __, __, __, E1, __},
/*frac   FR*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, FS, FS, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*fracs  FS*/ {AFS,AFS, __,AM8, __,AM7, __,AM3, __, __, __, __, __, __, FS, FS, __, __, __, __, E1, __, __, __, __, __, __, __, __, E1, __},
/*e      E1*/ { __, __, __, __, __, __, __, __, __, __, __, E2, E2, __, E3, E3, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*ex     E2*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, E3, E3, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*exp    E3*/ {AE3,AE3, __,AM8, __,AM7, __,AM3, __, __, __, __, __, __, E3, E3, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*tr     T1*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, T2, __, __, __, __, __, __},
/*tru    T2*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, T3, __, __, __},
/*true   T3*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,AT3, __, __, __, __, __, __, __, __, __, __},
/*fa     F1*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, F2, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
/*fal    F2*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, F3, __, __, __, __, __, __, __, __},
/*fals   F3*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, F4, __, __, __, __, __},
/*false  F4*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,AF4, __, __, __, __, __, __, __, __, __, __},
/*nu     N1*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, N2, __, __, __},
/*nul    N2*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, N3, __, __, __, __, __, __, __, __},
/*null   N3*/ { __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,AN3, __, __, __, __, __, __, __, __}
};
// clang-format on

// *****************************************************************************
// Private (static, forward) declarations

bool default_reader(uint8_t *ch, uintptr_t arg);

// *****************************************************************************
// Public code

int mu_json_parse_stream(mu_json_tokens_t *tokens,
                         size_t max_tokens,
                         mu_json_reader_fn reader,
                         void *reader_arg) {
    mu_json_parser_t parser;
    parser_init(&parser, tokens, max_tokens, reader, reader_arg);
    return parse(&parser);
}

int mu_json_parse_mu_str(mu_json_tokens_t *tokens,
                         size_t max_tokens,
                         mu_str_t *str) {
    default_reader_state_t state = {.str=str, .index=0};
    return mu_json_parse_stream(tokens, max_tokens, default_reader, &state);
}

int mu_json_parse_cstr(mu_json_tokens_t *tokens,
                       size_t max_tokens,
                       const char *cstr) {
    mu_str_t str;
    mu_str_init_cstr(&str, cstr);
    return mu_json_parse_mu_str(tokens, max_tokens, str);
}

mu_json_token_type_t mu_json_token_type(mu_json_token_t *token) {
    return token->type;
}
int mu_json_token_level(mu_json_token_t *token) {
    return token->depth;
}
const char *mu_json_token_string(mu_json_token_t *token) {
    return mu_str_bytes(token->str);
}
int mu_json_token_string_length(mu_json_token_t *token) {
    return mu_str_length(token->str);
}

// *****************************************************************************
// Private (static) code

bool default_reader(uint8_t *ch, uintptr_t arg) {
    default_reader_state_t *state = (default_reader_state_t *)arg;
    mu_str_t *str = state->str;
    int length = mu_str_length(str);

    // TODO: create bool mu_str_ref(mu_str_t str, int index, uint8_t *ch)?
    return mu_str_ref(str, state->index++, ch);
}

// *****************************************************************************
// End of file
