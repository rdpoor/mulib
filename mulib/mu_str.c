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

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

// Avoid requiring string.h
static unsigned long strlen(const char *str) {
    unsigned long len = 0;
    while (*str++ != '\0') {
        len += 1;
    }
    return len;
}

static int str_compare_aux(mu_str_t *s1, const uint8_t *b2, int len2);

static bool has_prefix(mu_str_t *s1, const uint8_t *b2, int len2);

static bool has_suffix(mu_str_t *s1, const uint8_t *b2, int len2);

static int mu_str_find_aux(mu_str_t *str, const uint8_t *substring,
                           int substring_len, bool skip_substr);

static int mu_str_rfind_aux(mu_str_t *str, const uint8_t *substring,
                            int substring_len, bool skip_substr);

static bool is_decimal(uint8_t byte);

// *****************************************************************************
// Public code

mu_str_t *mu_str_init(mu_str_t *str, const uint8_t *bytes, int len) {
    str->bytes = bytes;
    str->len = len;
    return str;
}

mu_str_t *mu_str_init_cstr(mu_str_t *str, const char *cstr) {
    return mu_str_init(str, (const uint8_t *)cstr, strlen(cstr));
}

const uint8_t *mu_str_bytes(mu_str_t *str) { return str->bytes; }

int mu_str_length(mu_str_t *str) { return str->len; }

bool mu_str_is_empty(mu_str_t *str) { return mu_str_length(str) == 0; }

mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src) {
    return mu_str_init(dst, src->bytes, src->len);
}

int mu_str_compare(mu_str_t *s1, mu_str_t *s2) {
    return str_compare_aux(s1, mu_str_bytes(s2), mu_str_length(s2));
}

int mu_str_compare_cstr(mu_str_t *s1, const char *cstr) {
    return str_compare_aux(s1, (const uint8_t *)cstr, strlen(cstr));
}

mu_str_t *mu_str_slice(mu_str_t *dst, mu_str_t *src, int start, int end) {
    if (start < 0) {
        // Negative index indexes from end of string
        start = src->len + start;
    } else if ((start > src->len) || (start == MU_STR_END)) {
        // Limit start to end of src string
        start = src->len;
    }

    if (end < 0) {
        // Negative index indexes from end of string
        end = src->len + end;
    } else if ((end >= src->len) || (end == MU_STR_END)) {
        // Limit end to end of src string
        end = src->len;
    }

    if (end < start) {
        // Enforce start <= end
        end = start;
    }

    return mu_str_init(dst, &src->bytes[start], end - start);
}

mu_str_t *mu_str_bisect(mu_str_t *left, mu_str_t *right, mu_str_t *src,
                        int index) {
    int src_len = mu_str_length(src);

    if (index < 0) {
        // Negative index indexes from end of src
        index = src_len + index;
    } else if ((index > src_len) || (index == MU_STR_END)) {
        // Limit index to end of src string
        index = src_len;
    }

    mu_str_init(left, mu_str_bytes(src), index);
    mu_str_init(right, &mu_str_bytes(src)[index], src_len - index);
    return src;
}

bool mu_str_has_prefix(mu_str_t *s1, mu_str_t *s2) {
    return has_prefix(s1, mu_str_bytes(s2), mu_str_length(s2));
}

bool mu_str_has_prefix_cstr(mu_str_t *s1, const char *cstr) {
    return has_prefix(s1, (const uint8_t *)cstr, strlen(cstr));
}

bool mu_str_has_suffix(mu_str_t *s1, mu_str_t *s2) {
    return has_suffix(s1, mu_str_bytes(s2), mu_str_length(s2));
}

bool mu_str_has_suffix_cstr(mu_str_t *s1, const char *cstr) {
    return has_suffix(s1, (const uint8_t *)cstr, strlen(cstr));
}

int mu_str_find_substr(mu_str_t *str, mu_str_t *substring, bool skip_substr) {
    return mu_str_find_aux(str, mu_str_bytes(substring),
                           mu_str_length(substring), skip_substr);
}

int mu_str_find_subcstr(mu_str_t *str, const char *substring,
                        bool skip_substr) {
    return mu_str_find_aux(str, (const uint8_t *)substring, strlen(substring),
                           skip_substr);
}

int mu_str_rfind_substr(mu_str_t *str, mu_str_t *substring, bool skip_substr) {
    return mu_str_rfind_aux(str, mu_str_bytes(substring),
                            mu_str_length(substring), skip_substr);
}

int mu_str_rfind_subcstr(mu_str_t *str, const char *substring,
                         bool skip_substr) {
    return mu_str_rfind_aux(str, (const uint8_t *)substring, strlen(substring),
                            skip_substr);
}

int mu_str_index(mu_str_t *str, mu_str_predicate_t predicate, void *arg,
                 bool break_if) {
    int str_len = mu_str_length(str);
    const uint8_t *bytes = mu_str_bytes(str);

    for (int idx = 0; idx < str_len; idx++) {
        if (predicate(*bytes++, arg) == break_if) {
            return idx;
        }
    }
    return MU_STR_NOT_FOUND;
}

int mu_str_rindex(mu_str_t *str, mu_str_predicate_t predicate, void *arg,
                  bool break_if) {
    int str_len = mu_str_length(str);
    const uint8_t *bytes = &mu_str_bytes(str)[str_len]; // for predecrement

    for (int idx = str_len; idx > 0; idx--) {
        if (predicate(*--bytes, arg) == break_if) {
            return idx - 1;
        }
    }
    return MU_STR_NOT_FOUND;
}

mu_str_t *mu_str_ltrim(mu_str_t *dst, mu_str_t *str,
                       mu_str_predicate_t predicate, void *arg) {
    int idx = mu_str_index(str, predicate, arg, false);

    if (idx == MU_STR_NOT_FOUND) {
        return mu_str_copy(dst, str);
    } else {
        return mu_str_slice(dst, str, idx, MU_STR_END);
    }
}

mu_str_t *mu_str_rtrim(mu_str_t *dst, mu_str_t *str,
                       mu_str_predicate_t predicate, void *arg) {
    int idx = mu_str_rindex(str, predicate, arg, false);

    if (idx == MU_STR_NOT_FOUND) {
        return mu_str_copy(dst, str);
    } else {
        return mu_str_slice(dst, str, 0, idx);
    }
}

mu_str_t *mu_str_trim(mu_str_t *dst, mu_str_t *str,
                      mu_str_predicate_t predicate, void *arg) {
    return mu_str_rtrim(dst, mu_str_ltrim(dst, str, predicate, arg), predicate,
                        arg);
}

bool mu_str_to_cstr(mu_str_t *str, char *buf, int capacity) {
    int str_length = mu_str_length(str);
    const uint8_t *bytes = mu_str_bytes(str);

    if (str_length >= capacity) {
        return false;
    }
    for (int i = 0; i < str_length; i++) {
        buf[i] = (char)bytes[i];
    }
    buf[str_length] = '\0';
    return true;
}

// TODO: handle zero length str (!!!)

#define DEFINE_INT_PARSER(_name, _type)                                        \
    _type _name(mu_str_t *str) {                                               \
        const uint8_t *buf = mu_str_bytes(str);                                \
        int len = mu_str_length(str);                                          \
        _type v = 0;                                                           \
        bool is_negative = false;                                              \
        if ((len >= 1) && *buf == '-') {                                       \
            len -= 1;                                                          \
            buf++;                                                             \
            is_negative = true;                                                \
        }                                                                      \
                                                                               \
        while ((len-- > 0) && is_decimal(*buf)) {                              \
            v = (v * 10) + (*buf++ - '0');                                     \
        }                                                                      \
        return is_negative ? -v : v;                                           \
    }

#define DEFINE_UINT_PARSER(_name, _type)                                       \
    _type _name(mu_str_t *str) {                                               \
        const uint8_t *buf = mu_str_bytes(str);                                \
        int len = mu_str_length(str);                                          \
        _type v = 0;                                                           \
                                                                               \
        while ((len-- > 0) && is_decimal(*buf)) {                              \
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
    const uint8_t *buf = mu_str_bytes(str);
    int len = mu_str_length(str);
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
    const uint8_t *b1 = mu_str_bytes(s1);
    int len1 = mu_str_length(s1);
    int len = (len1 < len2) ? len1 : len2; // min(len1, len2)

    for (int i = 0; i < len; i++) {
        int d = b1[i] - b2[i];
        if (d != 0) {
            return d;
        }
    }
    // The first len bytes are equal: return value based on lengths.
    return len1 - len2;
}

static bool has_prefix(mu_str_t *s1, const uint8_t *b2, int len2) {
    const uint8_t *b1 = mu_str_bytes(s1);
    int len1 = mu_str_length(s1);
    int idx;

    if (len2 == 0) {
        return true;
    } else if (len2 > len1) {
        return false;
    }

    for (idx = 0; idx < len1 && idx < len2; idx++) {
        if (*b1++ != *b2++) {
            break;
        }
    }
    return idx == len2;
}

static bool has_suffix(mu_str_t *s1, const uint8_t *b2, int len2) {
    const uint8_t *b1 = mu_str_bytes(s1);
    int len1 = mu_str_length(s1);
    int idx;

    if (len2 == 0) {
        return true;
    } else if (len2 > len1) {
        return false;
    }

    // position past last byte for pre-decrement
    b1 = &b1[len1];
    b2 = &b2[len2];

    for (idx = 0; idx < len1 && idx < len2; idx++) {
        if (*--b1 != *--b2) {
            break;
        }
    }
    return idx == len2;
}

static int mu_str_find_aux(mu_str_t *str, const uint8_t *substring,
                           int substring_len, bool skip_substr) {
    const uint8_t *hay_bytes = mu_str_bytes(str);
    int hay_len = mu_str_length(str);
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

static int mu_str_rfind_aux(mu_str_t *str, const uint8_t *substring,
                            int substring_len, bool skip_substr) {
    const uint8_t *hay_bytes = mu_str_bytes(str);
    int hay_len = mu_str_length(str);
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

static bool is_decimal(uint8_t byte) {
    if ((byte >= '0') && (byte <= '9')) {
        return true;
    }
    return false;
}
