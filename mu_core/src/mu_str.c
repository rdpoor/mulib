/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. D. Poor <rdpoor@gmail.com>
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

// *****************************************************************************
// Includes

#include "mu_str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef USE_STRING_LIB
#include <string.h>
// memrchr lacks prototype in string.h?
void *memrchr(const void *s, int c, size_t n);
#endif

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

#ifndef USE_STRING_LIB

static unsigned long strlen(const char *str) {
    unsigned long len = 0;
    while (*str++ != '\0') {
        len += 1;
    }
    return len;
}

static void *memchr(const void *s, int c_in, size_t n) {
    // re-cast
    unsigned char c = (unsigned char)c_in;
    const unsigned char *p = (const unsigned char *)s;

    for (int i=0; i<n; i++) {
        if (p[i] == c) {
            return (void *)&p[i];
        }
    }
    return NULL;
}

static void *memrchr(const void *s, int c_in, size_t n) {
    // re-cast
    unsigned char c = (unsigned char)c_in;
    const unsigned char *p = (const unsigned char *)s;

    for (int i=n-1; i>=0; i--) {
        if (p[i] == c) {
            return (void *)&p[i];
        }
    }
    return NULL;
}
#endif

static int str_compare_aux(mu_str_t *s1, const uint8_t *b2, int len2);

static int find_aux(const uint8_t *s, int length, mu_str_predicate_t predicate,
                    void *arg, bool match_if);

static int rfind_aux(const uint8_t *s, int length, mu_str_predicate_t predicate,
                    void *arg, bool match_if);

static int find_substr_aux(mu_str_t *str, const uint8_t *substring,
                           int substring_len, bool skip_substr);

static int rfind_substr_aux(mu_str_t *str, const uint8_t *substring,
                            int substring_len, bool skip_substr);

// *****************************************************************************
// Public code

mu_str_t *mu_str_init(mu_str_t *str, const uint8_t *buf, size_t length) {
    str->buf = buf;
    str->length = length;
    return str;
}

mu_str_t *mu_str_init_cstr(mu_str_t *str, const char *cstr) {
    return mu_str_init(str, (const uint8_t *)cstr, strlen(cstr));
}

const uint8_t *mu_str_buf(mu_str_t *str) { return str->buf; }

size_t mu_str_length(mu_str_t *str) { return str->length; }

bool mu_str_is_empty(mu_str_t *str) { return str->length == 0; }

bool mu_str_get_byte(mu_str_t *str, int index, uint8_t *byte) {
    if (index < 0 || index >= str->length) {
        return false;
    } else {
        *byte = str->buf[index];
        return true;
    }
}

mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src) {
    return mu_str_init(dst, src->buf, src->length);
}

int mu_str_compare(mu_str_t *s1, mu_str_t *s2) {
    return str_compare_aux(s1, s2->buf, s2->length);
}

int mu_str_compare_cstr(mu_str_t *s1, const char *cstr) {
    return str_compare_aux(s1, (const uint8_t *)cstr, strlen(cstr));
}

bool mu_str_equals_cstr(mu_str_t *s1, const char *cstr) {
    return str_compare_aux(s1, (const uint8_t *)cstr, strlen(cstr)) == 0;
}

mu_str_t *mu_str_slice(mu_str_t *dst, mu_str_t *src, int start, int end) {
    if (start < 0) {
        // Negative index indexes from end of string
        start = src->length + start;
    } else if ((start > src->length) || (start == MU_STR_END)) {
        // Limit start to end of src string
        start = src->length;
    }

    if (end < 0) {
        // Negative index indexes from end of string
        end = src->length + end;
    } else if ((end >= src->length) || (end == MU_STR_END)) {
        // Limit end to end of src string
        end = src->length;
    }

    if (end < start) {
        // Enforce start <= end
        end = start;
    }

    return mu_str_init(dst, &src->buf[start], end - start);
}

mu_str_t *mu_str_split(mu_str_t *left, mu_str_t *right, mu_str_t *src,
                       int index) {
    int src_len = src->length;

    if (index < 0) {
        // Negative index indexes from end of src
        index = src_len + index;
    } else if ((index > src_len) || (index == MU_STR_END)) {
        // Limit index to end of src string
        index = src_len;
    }

    mu_str_init(left, src->buf, index);
    mu_str_init(right, &src->buf[index], src_len - index);
    return src;
}

size_t mu_str_find_byte(mu_str_t *s, uint8_t b) {
    // assume library function is faster...
    uint8_t *p = memchr(s->buf, b, s->length);
    if (p) {
        return p - s->buf;
    } else {
        return MU_STR_NOT_FOUND;
    }
}

size_t mu_str_rfind_byte(mu_str_t *s, uint8_t b) {
    // assume library function is faster...
    uint8_t *p = memrchr(s->buf, b, s->length);
    if (p) {
        return p - s->buf;
    } else {
        return MU_STR_NOT_FOUND;
    }
}

size_t mu_str_find_substr(mu_str_t *s, mu_str_t *substr) {
    return find_substr_aux(s, substr->buf, substr->length, false);
}

size_t mu_str_find_subcstr(mu_str_t *s, const char *cstr) {
    return find_substr_aux(s, (const uint8_t *)cstr, strlen(cstr), false);
}

size_t mu_str_rfind_substr(mu_str_t *s, mu_str_t *substr) {
    return rfind_substr_aux(s, substr->buf, substr->length, false);
}

size_t mu_str_rfind_subcstr(mu_str_t *s, const char *cstr) {
    return rfind_substr_aux(s, (const uint8_t *)cstr, strlen(cstr), false);
}

size_t mu_str_find(mu_str_t *s, mu_str_predicate_t predicate, void *arg,
                   bool match_if) {
    return find_aux(s->buf, s->length, predicate, arg, match_if);
}

size_t mu_str_find_cstr(const char *cstr, mu_str_predicate_t predicate,
                        void *arg, bool match_if) {
    return find_aux((const uint8_t *)cstr, strlen(cstr), predicate, arg,
                    match_if);
}

size_t mu_str_rfind(mu_str_t *s, mu_str_predicate_t predicate, void *arg,
                    bool match_if) {
    return rfind_aux(s->buf, s->length, predicate, arg, match_if);
}

size_t mu_str_rfind_cstr(const char *cstr, mu_str_predicate_t predicate,
                         void *arg, bool match_if) {
    return rfind_aux((const uint8_t *)cstr, strlen(cstr), predicate, arg,
                     match_if);
}

mu_str_t *mu_str_ltrim(mu_str_t *dst, mu_str_t *str,
                       mu_str_predicate_t predicate, void *arg) {
    size_t idx = mu_str_find(str, predicate, arg, false);
    // index is leftmost byte that does NOT match predicate

    if (idx == MU_STR_NOT_FOUND) {
        // all bytes were trimmed
        return mu_str_slice(dst, str, 0, 0);
    } else {
        return mu_str_slice(dst, str, idx, MU_STR_END);
    }
}

mu_str_t *mu_str_rtrim(mu_str_t *dst, mu_str_t *str,
                       mu_str_predicate_t predicate, void *arg) {
    int idx = mu_str_rfind(str, predicate, arg, false);
    // index is rightmost byte that does NOT match predicate

    if (idx == MU_STR_NOT_FOUND) {
        // all bytes were trimmed
        return mu_str_slice(dst, str, 0, 0);
    } else {
        return mu_str_slice(dst, str, 0, idx+1);
    }
}

mu_str_t *mu_str_trim(mu_str_t *dst, mu_str_t *str,
                      mu_str_predicate_t predicate, void *arg) {
    return mu_str_rtrim(dst, mu_str_ltrim(dst, str, predicate, arg), predicate,
                        arg);
}

bool mu_str_is_whitespace(uint8_t byte, void *arg) {
    (void)arg;
    return ((byte == ' ') || (byte == '\t') || (byte == '\n') ||
            (byte == '\r') || (byte == '\f') || (byte == '\v'));
}

bool mu_str_is_digit(uint8_t byte, void *arg) {
    (void)arg;
    return ((byte >= '0') && (byte <= '9'));
}

bool mu_str_is_hex(uint8_t byte, void *arg) {
    (void)arg;
    return (((byte >= '0') && (byte <= '9')) ||
            ((byte >= 'a') && (byte <= 'f')) ||
            ((byte >= 'A') && (byte <= 'F')));
}

bool mu_str_is_word(uint8_t byte, void *arg) {
    (void)arg;
    return (((byte >= '0') && (byte <= '9')) ||
            ((byte >= 'a') && (byte <= 'z')) ||
            ((byte >= 'A') && (byte <= 'Z')) || (byte == '_'));
}

#define DEFINE_INT_PARSER(_name, _type)                                        \
    _type _name(mu_str_t *str) {                                               \
        const uint8_t *buf = str->buf;                                         \
        int len = str->length;                                                 \
        _type v = 0;                                                           \
        bool is_negative = false;                                              \
        if ((len >= 1) && *buf == '-') {                                       \
            len -= 1;                                                          \
            buf++;                                                             \
            is_negative = true;                                                \
        }                                                                      \
                                                                               \
        while ((len-- > 0) && mu_str_is_digit(*buf, NULL)) {                   \
            v = (v * 10) + (*buf++ - '0');                                     \
        }                                                                      \
        return is_negative ? -v : v;                                           \
    }

#define DEFINE_UINT_PARSER(_name, _type)                                       \
    _type _name(mu_str_t *str) {                                               \
        const uint8_t *buf = str->buf;                                         \
        int len = str->length;                                                 \
        _type v = 0;                                                           \
                                                                               \
        while ((len-- > 0) && mu_str_is_digit(*buf, NULL)) {                   \
            v = (v * 10) + (*buf++ - '0');                                     \
        }                                                                      \
        return v;                                                              \
    }

DEFINE_INT_PARSER(mu_str_parse_int, int)
DEFINE_UINT_PARSER(mu_str_parse_unsigned_int, unsigned int)
DEFINE_INT_PARSER(mu_str_parse_int8, int8_t)
DEFINE_UINT_PARSER(mu_str_parse_uint8, uint8_t)
DEFINE_INT_PARSER(mu_str_parse_int16, int16_t)
DEFINE_UINT_PARSER(mu_str_parse_uint16, uint16_t)
DEFINE_INT_PARSER(mu_str_parse_int32, int32_t)
DEFINE_UINT_PARSER(mu_str_parse_uint32, uint32_t)
DEFINE_INT_PARSER(mu_str_parse_int64, int64_t)
DEFINE_UINT_PARSER(mu_str_parse_uint64, uint64_t)

unsigned int mu_str_parse_hex(mu_str_t *str) {
    const uint8_t *buf = str->buf;
    int len = str->length;
    unsigned int v = 0;

    while (len-- > 0) {
        char ch = *buf++;
        if ((ch >= '0') && (ch <= '9')) {
            v <<= 4;
            v += (ch - '0');
        } else if ((ch >= 'a') && (ch <= 'f')) {
            v <<= 4;
            v += (ch - 'a' + 10);
        } else if ((ch >= 'A') && (ch <= 'F')) {
            v <<= 4;
            v += (ch - 'A' + 10);
        } else {
            break;
        }
    }
    return v;
}

// *****************************************************************************
// Private (static) code

static int str_compare_aux(mu_str_t *s1, const uint8_t *b2, int len2) {
    const uint8_t *b1 = s1->buf;
    int len1 = s1->length;
    int len = (len1 < len2) ? len1 : len2; // min(len1, len2)

    for (int i = 0; i < len; i++) {
        int d = b2[i] - b1[i];
        if (d != 0) {
            return d;
        }
    }
    // The first len bytes are equal: return value based on lengths.
    return len2 - len1;
}

static int find_aux(const uint8_t *s, int length, mu_str_predicate_t predicate,
                    void *arg, bool match_if) {
    for (int i = 0; i < length; i++) {
        if (predicate(s[i], arg) == match_if) {
            return i;
        }
    }
    return MU_STR_NOT_FOUND;
}

static int rfind_aux(const uint8_t *s, int length, mu_str_predicate_t predicate,
                    void *arg, bool match_if) {
    for (int i = length-1; i >= 0; --i) {
        if (predicate(s[i], arg) == match_if) {
            return i;
        }
    }
    return MU_STR_NOT_FOUND;
}

static int find_substr_aux(mu_str_t *str, const uint8_t *substring,
                           int substring_len, bool skip_substr) {
    const uint8_t *hay_bytes = str->buf;
    int hay_len = str->length;
    int j;

    if (substring_len == 0) {
        // null substring matches immediately
        return 0;
    }

    // First scan through str looking for a byte that matches the first
    // byte of substring.  Micro-optimization: We stop searching when we get
    // within substring_len bytes of the end of str, since beyond that, the
    // full-length search will always fail.
    for (int i = 0; i <= hay_len - substring_len; i++) {
        const uint8_t *h2 = &hay_bytes[i];
        if (*h2 == *substring) {
            // first byte matches.  Do the rest of the bytes match?
            for (j = 1; j < substring_len; j++) {
                if (h2[j] != substring[j]) {
                    // mismatch: advance to next char in str
                    break;
                }
            }
            if (j == substring_len) {
                // found: &hay_bytes[i] matched all of *substring
                return skip_substr ? i + substring_len : i;
            }
        }
        // advance to next byte in str
    }
    // got to end of str without a match.
    return MU_STR_NOT_FOUND;
}

static int rfind_substr_aux(mu_str_t *str, const uint8_t *substring,
                            int substring_len, bool skip_substr) {
    const uint8_t *hay_bytes = str->buf;
    int hay_len = str->length;
    int j;

    if (substring_len == 0) {
        // null substring matches immediately
        return hay_len;
    }

    // First scan through str looking for a byte that matches the first
    // byte of substring.  Micro-optimization: We start searching at str_end -
    // substring_len bytes of the end of str, since beyond that, the
    // full-length search will always fail.
    for (int i = hay_len - substring_len; i >= 0; --i) {
        const uint8_t *h2 = &hay_bytes[i];
        if (*h2 == *substring) {
            // first byte matches.  Do the rest of the bytes match?
            for (j = 1; j < substring_len; j++) {
                if (h2[j] != substring[j]) {
                    // mismatch: advance to next char in str
                    break;
                }
            }
            if (j == substring_len) {
                // found: &hay_bytes[i] matched all of *substring
                return skip_substr ? i + substring_len : i;
            }
        }
        // advance to next byte in str
    }
    // got to end of str without a match.
    return MU_STR_NOT_FOUND;
}

