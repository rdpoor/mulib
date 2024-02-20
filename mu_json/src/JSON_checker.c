/* JSON_checker.c */

/* 2016-11-11 */

/*
Copyright (c) 2005 JSON.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "JSON_checker.h"
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define GOOD 0xBABAB00E
#define __ -1 /* the universal error code */

// #define DEBUG_TRACE
#ifdef DEBUG_TRACE
#include <stdio.h>
#define TRACE_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define TRACE_PRINTF(...)
#endif

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/

enum classes {
    C_SPACE, /* space */
    C_WHITE, /* other whitespace */
    C_LCURB, /* {  */
    C_RCURB, /* } */
    C_LSQRB, /* [ */
    C_RSQRB, /* ] */
    C_COLON, /* : */
    C_COMMA, /* , */
    C_QUOTE, /* " */
    C_BACKS, /* \ */
    C_SLASH, /* / */
    C_PLUS,  /* + */
    C_MINUS, /* - */
    C_POINT, /* . */
    C_ZERO,  /* 0 */
    C_DIGIT, /* 123456789 */
    C_LOW_A, /* a */
    C_LOW_B, /* b */
    C_LOW_C, /* c */
    C_LOW_D, /* d */
    C_LOW_E, /* e */
    C_LOW_F, /* f */
    C_LOW_L, /* l */
    C_LOW_N, /* n */
    C_LOW_R, /* r */
    C_LOW_S, /* s */
    C_LOW_T, /* t */
    C_LOW_U, /* u */
    C_ABCDF, /* ABCDF */
    C_E,     /* E */
    C_ETC,   /* everything else */
    NR_CLASSES
};

// clang-format off
static int ascii_class[128] = {
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
// clang-format on

/*
    The state codes.
*/
#define DEFINE_STATES(M) \
    M(GO, "start") \
    M(OK, "ok") \
    M(OB, "object") \
    M(KE, "key") \
    M(CO, "colon") \
    M(VA, "value") \
    M(AR, "array") \
    M(ST, "string") \
    M(ES, "escape") \
    M(U1, "u1") \
    M(U2, "u2") \
    M(U3, "u3") \
    M(U4, "u4") \
    M(MI, "minus") \
    M(ZE, "zero") \
    M(IN, "integer") \
    M(FR, "fraction") \
    M(FS, "fraction") \
    M(E1, "e") \
    M(E2, "ex") \
    M(E3, "exp") \
    M(T1, "tr") \
    M(T2, "tru") \
    M(T3, "true") \
    M(F1, "fa") \
    M(F2, "fal") \
    M(F3, "fals") \
    M(F4, "false") \
    M(N1, "nu") \
    M(N2, "nul") \
    M(N3, "null") \
    M(NR_STATES, "actions follow") \
    M(Ec, "empty }") \
    M(Co, "close }") \
    M(Ca, "close ]") \
    M(Oo, "open {") \
    M(Oa, "open [") \
    M(Cs, "close \"") \
    M(Cm, "comma") \
    M(Cl, "colon")

#define EXPAND_STATE_ENUMS(_name, _description) _name,
enum { DEFINE_STATES(EXPAND_STATE_ENUMS) };

// clang-format off
static int state_transition_table[NR_STATES * NR_CLASSES] = {
/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is signified by a
    mixed-case symbol and has a value greater than NR_STATES. A JSON text is 
    accepted if at the end of the text the state is OK and if the mode is 
    MODE_DONE.

               white                                      1-9                                   ABCDF  etc
           space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ GO,GO,Oo,__,Oa,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*ok     OK*/ OK,OK,__,Co,__,Ca,__,Cm,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*object OB*/ OB,OB,__,Ec,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*key    KE*/ KE,KE,__,__,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*colon  CO*/ CO,CO,__,__,__,__,Cl,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,
/*value  VA*/ VA,VA,Oo,__,Oa,__,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__,
/*array  AR*/ AR,AR,Oo,__,Oa,Ca,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__,
/*string ST*/ ST,__,ST,ST,ST,ST,ST,ST,Cs,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,
/*escape ES*/ __,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__,
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

/*
    These modes can be pushed on the stack.
*/
#define DEFINE_MODES(M) \
    M(MODE_ARRAY) \
    M(MODE_DONE) \
    M(MODE_KEY) \
    M(MODE_OBJECT)

#define EXPAND_MODE_ENUMS(_name) _name,
enum { DEFINE_MODES(EXPAND_MODE_ENUMS) };

// *****************************************************************************
// start DEBUG_TRACE support
#ifdef DEBUG_TRACE

#define EXPAND_STATE_NAMES(_name, _description) #_name,
__attribute__((unused))
static const char *s_state_names[] = {DEFINE_STATES(EXPAND_STATE_NAMES)};

#define EXPAND_STATE_DESCRIPTIONS(_name, _description) _description,
__attribute__((unused))
static const char *s_state_descriptions[] = {DEFINE_STATES(EXPAND_STATE_DESCRIPTIONS)};

#define N_STATES sizeof(s_state_names)/sizeof(s_state_names[0])

__attribute__((unused))
static const char *state_name(int state) {
    if (state >= 0 && state < N_STATES) {
        return s_state_names[state];
    } else {
        return "__";
    }
}
__attribute__((unused))
static const char *state_description(int state) {
    if (state >= 0 && state < N_STATES) {
        return s_state_descriptions[state];
    } else {
        return "unknown state";
    }
}

#define EXPAND_MODE_NAMES(_name) #_name,
__attribute__((unused))
static const char *s_mode_names[] = {DEFINE_MODES(EXPAND_MODE_NAMES)};

#define N_MODES sizeof(s_mode_names)/sizeof(s_mode_names[0])

__attribute__((unused))
static const char *mode_name(int mode) {
    if (mode >= 0 && mode < N_MODES) {
        return s_mode_names[mode];
    } else {
        return "unknown mode";
    }
}

#endif
// end DEBUG_TRACE support
// *****************************************************************************

static void destroy(JSON_checker jc) {
    /*
        Delete the JSON_checker object.
    */
    jc->valid = 0;
    free((void *)jc->stack);
    free((void *)jc);
}

static int reject(JSON_checker jc) {
    /*
        Delete the JSON_checker object.
    */
    destroy(jc);
    return FALSE;
}

static int push(JSON_checker jc, int mode) {
    /*
        Push a mode onto the stack. Return false if there is overflow.
    */
    TRACE_PRINTF("push %s => ", mode_name(mode));
    jc->top += 1;
    if (jc->top >= jc->depth) {
        TRACE_PRINTF("stack overflow\n");
        return FALSE;
    }
    jc->stack[jc->top] = mode;
    TRACE_PRINTF("OK\n");
    return TRUE;
}

static int pop(JSON_checker jc, int mode) {
    /*
        Pop the stack, assuring that the current mode matches the expectation.
        Return false if there is underflow or if the modes mismatch.
    */
    TRACE_PRINTF("pop %s => ", mode_name(mode));
    if (jc->top < 0) {
        TRACE_PRINTF("stack underflow\n");
        return FALSE;
    } else if (jc->stack[jc->top] != mode) {
        TRACE_PRINTF("mode mismatch\n");
        return FALSE;
    }
    jc->top -= 1;
    TRACE_PRINTF("OK\n");
    return TRUE;
}

JSON_checker new_JSON_checker(int depth) {
    /*
        new_JSON_checker starts the checking process by constructing a
       JSON_checker object. It takes a depth parameter that restricts the level
       of maximum nesting.

        To continue the process, call JSON_checker_char for each character in
       the JSON text, and then call JSON_checker_done to obtain the final
       result. These functions are fully reentrant.

        The JSON_checker object will be deleted by JSON_checker_done.
        JSON_checker_char will delete the JSON_checker object if it sees an
       error.
    */
    JSON_checker jc = (JSON_checker)malloc(sizeof(struct JSON_checker_struct));
    jc->valid = GOOD;
    jc->state = GO;
    jc->depth = depth;
    jc->top = -1;
    jc->stack = (int *)calloc(depth, sizeof(int));
    TRACE_PRINTF("===== new_JSON_checker()\n");
    push(jc, MODE_DONE);
    return jc;
}

static int lookup_state(int state, int char_class) {
    return state_transition_table[state * NR_CLASSES + char_class];
}

int JSON_checker_char(JSON_checker jc, int next_char) {
    /*
        After calling new_JSON_checker, call this function for each character
       (or partial character) in your JSON text. It can accept UTF-8, UTF-16, or
        UTF-32. It returns TRUE if things are looking ok so far. If it rejects
       the text, it destroys the JSON_checker object and returns false.
    */
    int next_class, next_state;
    /*
        Determine the character's class.
    */
    if (jc->valid != GOOD) {
        return FALSE;
    }
    if (next_char < 0) {
        return reject(jc);
    }
    if (next_char >= 128) {
        next_class = C_ETC;
    } else {
        next_class = ascii_class[next_char];
        if (next_class <= __) {
            return reject(jc);
        }
    }
    /*
        Get the next state from the state transition table.
    */
    next_state = lookup_state(jc->state, next_class);

    if (next_state >= 0 && next_state < NR_STATES) {
        /* Transition the state. */
        jc->state = next_state;

    } else {
        /* Perform an action and transition the state */
        switch (next_state) {
        case Ec:
            /* } - close empty object */
            if (!pop(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

        case Co:
            /* } - close object */ 
            if (!pop(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

        case Ca:
            /* ] - close array */ 
            if (!pop(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

        case Oo:
            /* { - open object */ 
            if (!push(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OB;
            break;

        case Oa:
            /* [ - open array */ 
            if (!push(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = AR;
            break;

        case Cs:
            /* close quote seen */ 
            switch (jc->stack[jc->top]) {
            case MODE_KEY:
                /* If in key mode, expect a colon next */
                jc->state = CO;
                break;
            case MODE_ARRAY:
            case MODE_OBJECT:
                jc->state = OK;
                break;
            default:
                return reject(jc);
            }
            break;

        case Cm:
            /* , */ 
            switch (jc->stack[jc->top]) {
            case MODE_OBJECT:
                /* A comma causes a flip from object mode to key mode. */
                if (!pop(jc, MODE_OBJECT) || !push(jc, MODE_KEY)) {
                    return reject(jc);
                }
                jc->state = KE;
                break;
            case MODE_ARRAY:
                jc->state = VA;
                break;
            default:
                return reject(jc);
            }
            break;

        case Cl:
            /* : */ 
            /* A colon causes a flip from key mode to object mode. */
            if (!pop(jc, MODE_KEY) || !push(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = VA;
            break;
        default:
            /* Bad action. */
            return reject(jc);
        }
    }
    return TRUE;
}

int JSON_checker_done(JSON_checker jc) {
    /*
        The JSON_checker_done function should be called after all of the
       characters have been processed, but only if every call to
       JSON_checker_char returned true. This function deletes the JSON_checker
       and returns true if the JSON text was accepted.
    */
    if (jc->valid != GOOD) {
        return FALSE;
    }
    int result = jc->state == OK && pop(jc, MODE_DONE);
    destroy(jc);
    return result;
}
