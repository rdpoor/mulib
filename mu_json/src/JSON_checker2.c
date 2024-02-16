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

#include <stdlib.h>
#include <stdio.h>
#include "JSON_checker.h"

#define TRUE  1
#define FALSE 0
#define GOOD 0xBABAB00E
#define __   -1     /* the universal error code */

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/

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
enum { DEFINE_CHAR_CLASSES(EXPAND_CH_CLASS_ENUMS) };

#define EXPAND_CH_CLASS_NAMES(_name) #_name,
static const char *s_char_class_names[] = {DEFINE_CHAR_CLASSES(EXPAND_CH_CLASS_NAMES)};

static const char *class_name(int char_class) {
    if ((char_class < 0) || (char_class >= NR_CLASSES)) {
        return "_______";
    } else {
        return s_char_class_names[char_class];
    }
}

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

#define DEFINE_STATES(M)                                                       \
    M(GO) /* start    */                                                       \
    M(OK) /* ok       */                                                       \
    M(OB) /* object   */                                                       \
    M(KE) /* key      */                                                       \
    M(CO) /* colon    */                                                       \
    M(VA) /* value    */                                                       \
    M(AR) /* array    */                                                       \
    M(ST) /* string   */                                                       \
    M(ES) /* escape   */                                                       \
    M(U1) /* u1       */                                                       \
    M(U2) /* u2       */                                                       \
    M(U3) /* u3       */                                                       \
    M(U4) /* u4       */                                                       \
    M(MI) /* minus    */                                                       \
    M(ZE) /* zero     */                                                       \
    M(IN) /* integer  */                                                       \
    M(FR) /* fraction */                                                       \
    M(FS) /* fraction */                                                       \
    M(E1) /* e        */                                                       \
    M(E2) /* ex       */                                                       \
    M(E3) /* exp      */                                                       \
    M(T1) /* tr       */                                                       \
    M(T2) /* tru      */                                                       \
    M(T3) /* true     */                                                       \
    M(F1) /* fa       */                                                       \
    M(F2) /* fal      */                                                       \
    M(F3) /* fals     */                                                       \
    M(F4) /* false    */                                                       \
    M(N1) /* nu       */                                                       \
    M(N2) /* nul      */                                                       \
    M(N3) /* null     */                                                       \
    M(NR_STATES)

#define EXPAND_STATE_ENUMS(_name) _name,
enum { DEFINE_STATES(EXPAND_STATE_ENUMS) };

#define EXPAND_STATE_NAMES(_name) #_name,
static const char *s_state_names[] = {DEFINE_STATES(EXPAND_STATE_NAMES)};

static const char *s_action_names[] = {
    "EMPTY-}",  /* -9 */
    "CLOSE-}",  /* -8 */
    "CLOSE-]",  /* -7 */
    "OPEN-{ ",  /* -6 */
    "OPEN=[ ",  /* -5 */
    "QUOTE  ",  /* -4 */
    "COMMA  ",  /* -3 */
    "COLON  ",  /* -2 */
    "UNKNOWN",  /* -1 */

};

static const char *state_name(int state) {
    if ((state < -9) || (state >= NR_STATES)) {
        return "__";
    } else if (state < 0) {
        return s_action_names[state + 9];
    } else {
        return s_state_names[state];
    }
}

static int state_transition_table[NR_STATES][NR_CLASSES] = {
/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is represented as a
    negative number. A JSON text is accepted if at the end of the text the
    state is OK and if the mode is MODE_DONE.

                 white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ {GO,GO,-6,__,-5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ok     OK*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*object OB*/ {OB,OB,__,-9,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*key    KE*/ {KE,KE,__,__,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*colon  CO*/ {CO,CO,__,__,__,__,-2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*value  VA*/ {VA,VA,-6,__,-5,__,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*array  AR*/ {AR,AR,-6,__,-5,-7,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*string ST*/ {ST,__,ST,ST,ST,ST,ST,ST,-4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST},
/*escape ES*/ {__,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__},
/*u1     U1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
/*u2     U2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
/*u3     U3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
/*u4     U4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__},
/*minus  MI*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*zero   ZE*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,__,__,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*int    IN*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*frac   FR*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,FS,FS,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fracs  FS*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,FS,FS,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*e      E1*/ {__,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ex     E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*exp    E3*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*tr     T1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
/*tru    T2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
/*true   T3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*fa     F1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fal    F2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
/*fals   F3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
/*false  F4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*nu     N1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
/*nul    N2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
/*null   N3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__}
};


/*
    These modes can be pushed on the stack.
*/
enum modes {
    MODE_ARRAY,
    MODE_DONE,
    MODE_KEY,
    MODE_OBJECT
};

static void
destroy(JSON_checker jc)
{
/*
    Delete the JSON_checker object.
*/
    jc->valid = 0;
    free((void*)jc->stack);
    free((void*)jc);
}


static int
reject(JSON_checker jc)
{
/*
    Delete the JSON_checker object.
*/
    destroy(jc);
    return FALSE;
}


static int
push(JSON_checker jc, int mode)
{
/*
    Push a mode onto the stack. Return false if there is overflow.
*/
    jc->top += 1;
    if (jc->top >= jc->depth) {
        return FALSE;
    }
    jc->stack[jc->top] = mode;
    return TRUE;
}


static int
pop(JSON_checker jc, int mode)
{
/*
    Pop the stack, assuring that the current mode matches the expectation.
    Return false if there is underflow or if the modes mismatch.
*/
    if (jc->top < 0 || jc->stack[jc->top] != mode) {
        return FALSE;
    }
    jc->top -= 1;
    return TRUE;
}


JSON_checker
new_JSON_checker(int depth)
{
/*
    new_JSON_checker starts the checking process by constructing a JSON_checker
    object. It takes a depth parameter that restricts the level of maximum
    nesting.

    To continue the process, call JSON_checker_char for each character in the
    JSON text, and then call JSON_checker_done to obtain the final result.
    These functions are fully reentrant.

    The JSON_checker object will be deleted by JSON_checker_done.
    JSON_checker_char will delete the JSON_checker object if it sees an error.
*/
    JSON_checker jc = (JSON_checker)malloc(sizeof(struct JSON_checker_struct));
    jc->valid = GOOD;
    jc->state = GO;
    jc->depth = depth;
    jc->top = -1;
    jc->stack = (int*)calloc(depth, sizeof(int));
    push(jc, MODE_DONE);
    return jc;
}


int
JSON_checker_char(JSON_checker jc, int next_char)
{
/*
    After calling new_JSON_checker, call this function for each character (or
    partial character) in your JSON text. It can accept UTF-8, UTF-16, or
    UTF-32. It returns TRUE if things are looking ok so far. If it rejects the
    text, it destroys the JSON_checker object and returns false.
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
    next_state = state_transition_table[jc->state][next_class];

    fprintf(stderr, "\n'%c' %s %s => %s", next_char, class_name(next_class),
            state_name(jc->state), state_name(next_state));

    if (next_state >= 0) {
        /*
            Change the state.
        */
        jc->state = next_state;
/*
    Or perform one of the actions.
*/
    } else {
        switch (next_state) {
/* empty } */
        case -9:
            if (!pop(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OK;
            fprintf(stderr, "=> %s", state_name(jc->state));
            break;

/* } */ case -8:
            if (!pop(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = OK;
            fprintf(stderr, "=> %s", state_name(jc->state));
            break;

/* ] */ case -7:
            if (!pop(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = OK;
            fprintf(stderr, "=> %s", state_name(jc->state));
            break;

/* { */ case -6:
            if (!push(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OB;
            fprintf(stderr, "=> %s", state_name(jc->state));
            break;

/* [ */ case -5:
            if (!push(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = AR;
            fprintf(stderr, "=> %s", state_name(jc->state));
            break;

/* " */ case -4:
            switch (jc->stack[jc->top]) {
            case MODE_KEY:
                jc->state = CO;
                fprintf(stderr, "=> %s", state_name(jc->state));
                break;
            case MODE_ARRAY:
            case MODE_OBJECT:
                jc->state = OK;
                fprintf(stderr, "=> %s", state_name(jc->state));
                break;
            default:
                return reject(jc);
            }
            break;

/* , */ case -3:
            switch (jc->stack[jc->top]) {
            case MODE_OBJECT:
/*
    A comma causes a flip from object mode to key mode.
*/
                if (!pop(jc, MODE_OBJECT) || !push(jc, MODE_KEY)) {
                    return reject(jc);
                }
                jc->state = KE;
                fprintf(stderr, "=> %s", state_name(jc->state));
                break;
            case MODE_ARRAY:
                jc->state = VA;
                fprintf(stderr, "=> %s", state_name(jc->state));
                break;
            default:
                return reject(jc);
            }
            break;

/* : */ case -2:
/*
    A colon causes a flip from key mode to object mode.
*/
            if (!pop(jc, MODE_KEY) || !push(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = VA;
            fprintf(stderr, "=> %s", state_name(jc->state));
            break;
/*
    Bad action.
*/
        default:
            return reject(jc);
        }
    }
    return TRUE;
}


int
JSON_checker_done(JSON_checker jc)
{
/*
    The JSON_checker_done function should be called after all of the characters
    have been processed, but only if every call to JSON_checker_char returned
    true. This function deletes the JSON_checker and returns true if the JSON
    text was accepted.
*/
    if (jc->valid != GOOD) {
        return FALSE;
    }
    int result = jc->state == OK && pop(jc, MODE_DONE);
    destroy(jc);
    return result;
}


/**
gcc -Wall JSON_checker2.c -o JSON_checker2
 */
int main(int argc, char* argv[]) {
/*
    Read STDIN. Exit with a message if the input is not well-formed JSON text.

    jc will contain a JSON_checker with a maximum depth of 20.
*/
    JSON_checker jc = new_JSON_checker(20);
    for (;;) {
        int next_char = getchar();
        if (next_char <= 0) {
            break;
        }
        if (!JSON_checker_char(jc, next_char)) {
            fprintf(stderr, "JSON_checker_char: syntax error\n");
            exit(1);
        }
    }
    if (!JSON_checker_done(jc)) {
        fprintf(stderr, "JSON_checker_end: syntax error\n");
        exit(1);
    }
    fprintf(stderr, "JSON_checker: success\n");
}
