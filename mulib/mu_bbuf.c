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

#include "mu_bbuf.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

static int str_compare_aux(mu_bbuf_t *b1, const uint8_t *b2, int len2);

static bool has_prefix(mu_bbuf_t *b1, const uint8_t *b2, int len2);

static bool has_suffix(mu_bbuf_t *b1, const uint8_t *b2, int len2);

static int mu_bbuf_find_aux(mu_bbuf_t *bbuf, const uint8_t *substr,
                           int substr_len, bool skip_substr);

static int mu_bbuf_rfind_aux(mu_bbuf_t *bbuf, const uint8_t *substr,
                            int substr_len, bool skip_substr);

static bool is_decimal(uint8_t byte);

// *****************************************************************************
// Public code

mu_bbuf_t *mu_bbuf_init_ro(mu_bbuf_t *bbuf, const uint8_t *bytes, size_t capacity) {
    bbuf->bytes_ro = bytes;
    bbuf->capacity = capacity;
    return bbuf;
}

mu_bbuf_t *mu_bbuf_init_rw(mu_bbuf_t *bbuf, uint8_t *bytes, size_t capacity) {
    bbuf->bytes_rw = bytes;
    bbuf->capacity = capacity;
    return bbuf;
}


mu_bbuf_t *mu_bbuf_init_cstr(mu_bbuf_t *bbuf, const char *cstr) {
    bbuf->bytes_ro = (const uint8_t *)cstr;
    bbuf->capacity = strlen(cstr);
    return bbuf;
}

mu_bbuf_t *mu_bbuf_copy(mu_bbuf_t *dst, mu_bbuf_t *src) {
    dst->bytes_ro = src->bytes_ro;
    dst->capacity = src->capacity;
    return dst;
}

const uint8_t *mu_bbuf_bytes_ro(mu_bbuf_t *bbuf) {
    return bbuf->bytes_ro;
}

uint8_t *mu_bbuf_bytes_rw(mu_bbuf_t *bbuf) {
    return bbuf->bytes_rw;
}

size_t mu_bbuf_capacity(mu_bbuf_t *bbuf) { return bbuf->capacity; }

const uint8_t *mu_bbuf_ref_ro(mu_bbuf_t *bbuf, size_t index) {
    if (index < bbuf->capacity) {
        return &bbuf->bytes_ro[index];
    } else {
        return NULL;
    }
}

uint8_t *mu_bbuf_ref_rw(mu_bbuf_t *bbuf, size_t index) {
    if (index < bbuf->capacity) {
        return &bbuf->bytes_rw[index];
    } else {
        return NULL;
    }
}

int mu_bbuf_compare(mu_bbuf_t *b1, mu_bbuf_t *b2) {
    return str_compare_aux(b1, mu_bbuf_bytes_ro(b2), mu_bbuf_capacity(b2));
}

int mu_bbuf_compare_cstr(mu_bbuf_t *b1, const char *cstr) {
    return str_compare_aux(b1, (const uint8_t *)cstr, strlen(cstr));
}

mu_bbuf_t *mu_bbuf_slice(mu_bbuf_t *dst, mu_bbuf_t *src, int start, int end) {
    if (start < 0) {
        // Negative index indexes from end of string
        start = src->capacity + start;
    } else if ((start > src->capacity) || (start == MU_BBUF_END)) {
        // Limit start to end of src string
        start = src->capacity;
    }

    if (end < 0) {
        // Negative index indexes from end of string
        end = src->capacity + end;
    } else if ((end >= src->capacity) || (end == MU_BBUF_END)) {
        // Limit end to end of src string
        end = src->capacity;
    }

    if (end < start) {
        // Enforce start <= end
        end = start;
    }

    return mu_bbuf_init_ro(dst, &src->bytes_ro[start], end - start);
}

mu_bbuf_t *mu_bbuf_bisect(mu_bbuf_t *left, mu_bbuf_t *right, mu_bbuf_t *src,
                        int index) {
    int src_capacity = mu_bbuf_capacity(src);

    if (index < 0) {
        // Negative index indexes from end of src
        index = src_capacity + index;
    } else if ((index > src_capacity) || (index == MU_BBUF_END)) {
        // Limit index to end of src string
        index = src_capacity;
    }

    if (left != NULL) {
        mu_bbuf_init_ro(left, mu_bbuf_bytes_ro(src), index);
    }
    if (right != NULL) {
        mu_bbuf_init_ro(right, &mu_bbuf_bytes_ro(src)[index], src_capacity - index);
    }
    return src;
}

bool mu_bbuf_matches(mu_bbuf_t *b1, mu_bbuf_t *b2) {
    return str_compare_aux(b1, mu_bbuf_bytes_ro(b2), mu_bbuf_capacity(b2)) == 0;
}

bool mu_bbuf_matches_cstr(mu_bbuf_t *b1, const char *cstr) {
    return str_compare_aux(b1, (const uint8_t *)cstr, strlen(cstr)) == 0;
}

bool mu_bbuf_has_prefix(mu_bbuf_t *b1, mu_bbuf_t *b2) {
    return has_prefix(b1, mu_bbuf_bytes_ro(b2), mu_bbuf_capacity(b2));
}

bool mu_bbuf_has_prefix_cstr(mu_bbuf_t *b1, const char *cstr) {
    return has_prefix(b1, (const uint8_t *)cstr, strlen(cstr));
}

bool mu_bbuf_has_suffix(mu_bbuf_t *b1, mu_bbuf_t *b2) {
    return has_suffix(b1, mu_bbuf_bytes_ro(b2), mu_bbuf_capacity(b2));
}

bool mu_bbuf_has_suffix_cstr(mu_bbuf_t *b1, const char *cstr) {
    return has_suffix(b1, (const uint8_t *)cstr, strlen(cstr));
}

int mu_bbuf_find_substr(mu_bbuf_t *bbuf, mu_bbuf_t *substr, bool skip_substr) {
    return mu_bbuf_find_aux(bbuf, mu_bbuf_bytes_ro(substr),
                           mu_bbuf_capacity(substr), skip_substr);
}

int mu_bbuf_find_subcstr(mu_bbuf_t *bbuf, const char *cstr,
                        bool skip_substr) {
    return mu_bbuf_find_aux(bbuf, (const uint8_t *)cstr, strlen(cstr),
                           skip_substr);
}

int mu_bbuf_rfind_substr(mu_bbuf_t *bbuf, mu_bbuf_t *substr, bool skip_substr) {
    return mu_bbuf_rfind_aux(bbuf, mu_bbuf_bytes_ro(substr),
                            mu_bbuf_capacity(substr), skip_substr);
}

int mu_bbuf_rfind_subcstr(mu_bbuf_t *bbuf, const char *cstr,
                         bool skip_substr) {
    return mu_bbuf_rfind_aux(bbuf, (const uint8_t *)cstr, strlen(cstr),
                            skip_substr);
}

int mu_bbuf_find_byte(mu_bbuf_t *bbuf, uint8_t byte) {
    for (int i = 0; i < bbuf->capacity; i++) {
        if (bbuf->bytes_ro[i] == byte) {
            return i;
        }
    }
    return MU_BBUF_NOT_FOUND;
}

int mu_bbuf_rfind_byte(mu_bbuf_t *bbuf, uint8_t byte) {
    for (int i = bbuf->capacity - 1; i >= 0; --i) {
        if (bbuf->bytes_ro[i] == byte) {
            return i;
        }
    }
    return MU_BBUF_NOT_FOUND; // stub
}

int mu_bbuf_find_predicate(mu_bbuf_t *bbuf, mu_bbuf_predicate_t predicate, void *arg) {
    int bbuf_capacity = mu_bbuf_capacity(bbuf);
    const uint8_t *bytes = mu_bbuf_bytes_ro(bbuf);

    for (int idx = 0; idx < bbuf_capacity; idx++) {
        if (!predicate(*bytes++, arg)) {
            return idx;
        }
    }
    return MU_BBUF_NOT_FOUND;
}

int mu_bbuf_rfind_predicate(mu_bbuf_t *bbuf, mu_bbuf_predicate_t predicate, void *arg) {
    int bbuf_capacity = mu_bbuf_capacity(bbuf);
    const uint8_t *bytes = &mu_bbuf_bytes_ro(bbuf)[bbuf_capacity]; // for predecrement

    for (int idx = bbuf_capacity; idx > 0; idx--) {
        if (!predicate(*--bytes, arg)) {
            return idx - 1;
        }
    }
    return MU_BBUF_NOT_FOUND;
}

mu_bbuf_t *mu_bbuf_trim(mu_bbuf_t *dst, mu_bbuf_t *bbuf,
                      mu_bbuf_predicate_t predicate, void *arg) {
    return mu_bbuf_rtrim(dst, mu_bbuf_ltrim(dst, bbuf, predicate, arg), predicate,
                        arg);
}

mu_bbuf_t *mu_bbuf_ltrim(mu_bbuf_t *dst, mu_bbuf_t *bbuf,
                       mu_bbuf_predicate_t predicate, void *arg) {
    int idx = mu_bbuf_find_predicate(bbuf, predicate, arg);

    if (idx == MU_BBUF_NOT_FOUND) {
        return mu_bbuf_copy(dst, bbuf);
    } else {
        return mu_bbuf_slice(dst, bbuf, idx, MU_BBUF_END);
    }
}

mu_bbuf_t *mu_bbuf_rtrim(mu_bbuf_t *dst, mu_bbuf_t *bbuf,
                       mu_bbuf_predicate_t predicate, void *arg) {
    int idx = mu_bbuf_rfind_predicate(bbuf, predicate, arg);

    if (idx == MU_BBUF_NOT_FOUND) {
        return mu_bbuf_copy(dst, bbuf);
    } else {
        return mu_bbuf_slice(dst, bbuf, 0, idx);
    }
}

#define DEFINE_INT_PARSER(_name, _type)                                        \
    _type _name(mu_bbuf_t *bbuf) {                                               \
        const uint8_t *buf = mu_bbuf_bytes_ro(bbuf);                                \
        int len = mu_bbuf_capacity(bbuf);                                          \
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
    _type _name(mu_bbuf_t *bbuf) {                                               \
        const uint8_t *buf = mu_bbuf_bytes_ro(bbuf);                                \
        int len = mu_bbuf_capacity(bbuf);                                          \
        _type v = 0;                                                           \
                                                                               \
        while ((len-- > 0) && is_decimal(*buf)) {                              \
            v = (v * 10) + (*buf++ - '0');                                     \
        }                                                                      \
        return v;                                                              \
    }

DEFINE_INT_PARSER(mu_bbuf_parse_int, int)
DEFINE_UINT_PARSER(mu_bbuf_parse_unsigned_int, unsigned int)
DEFINE_INT_PARSER(mu_bbuf_parse_int8, int8_t)
DEFINE_UINT_PARSER(mu_bbuf_parse_uint8, uint8_t)
DEFINE_INT_PARSER(mu_bbuf_parse_int16, int16_t)
DEFINE_UINT_PARSER(mu_bbuf_parse_uint16, uint16_t)
DEFINE_INT_PARSER(mu_bbuf_parse_int32, int32_t)
DEFINE_UINT_PARSER(mu_bbuf_parse_uint32, uint32_t)
DEFINE_INT_PARSER(mu_bbuf_parse_int64, int64_t)
DEFINE_UINT_PARSER(mu_bbuf_parse_uint64, uint64_t)

unsigned int mu_bbuf_parse_hex(mu_bbuf_t *bbuf) {
    const uint8_t *buf = mu_bbuf_bytes_ro(bbuf);
    int len = mu_bbuf_capacity(bbuf);
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
// Operations on mutable buffers

bool mu_bbuf_get_byte(mu_bbuf_t *bbuf, size_t index, uint8_t *byte) {
    if (index >= bbuf->capacity) {
        return false;
    }
    *byte = bbuf->bytes_ro[index];
    return true;
}

bool mu_bbuf_put_byte(mu_bbuf_t *bbuf_rw, size_t index, uint8_t byte) {
    if (index >= bbuf_rw->capacity) {
        return false;
    }
    bbuf_rw->bytes_rw[index] = byte;
    return true;
}

mu_bbuf_t *mu_bbuf_clear(mu_bbuf_t *bbuf_rw) {
    memset(bbuf_rw->bytes_rw, 0, bbuf_rw->capacity);
    return bbuf_rw;
}

size_t mu_bbuf_copy_into(mu_bbuf_t *bbuf_rw, mu_bbuf_t *src, size_t offset) {
    size_t to_copy;

    // calculate # of bytes to copy using unsigned arithmentic.
    if (offset >= bbuf_rw->capacity) {
        to_copy = 0;
    } else {
        to_copy = bbuf_rw->capacity - offset;
    }
    if (to_copy > src->capacity) {
        to_copy = src->capacity;
    }

    if (to_copy > 0) {
        uint8_t *dst_bytes = &bbuf_rw->bytes_rw[offset];
        const uint8_t *src_bytes = &src->bytes_ro[0];
        memmove(dst_bytes, src_bytes, to_copy);
    }
    return to_copy;
}

mu_bbuf_t *mu_bbuf_lshift(mu_bbuf_t *bbuf_rw, size_t count) {
    // stub
    return bbuf_rw;
}

mu_bbuf_t *mu_bbuf_rshift(mu_bbuf_t *bbuf_rw, size_t count) {
    // stub
    return bbuf_rw;
}

// *****************************************************************************
// Private (static) code

static int str_compare_aux(mu_bbuf_t *bbuf, const uint8_t *b2, int len2) {
    const uint8_t *b1 = mu_bbuf_bytes_ro(bbuf);
    int len1 = mu_bbuf_capacity(bbuf);
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

static bool has_prefix(mu_bbuf_t *bbuf, const uint8_t *b2, int len2) {
    const uint8_t *b1 = mu_bbuf_bytes_ro(bbuf);
    int len1 = mu_bbuf_capacity(bbuf);
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

static bool has_suffix(mu_bbuf_t *bbuf, const uint8_t *b2, int len2) {
    const uint8_t *b1 = mu_bbuf_bytes_ro(bbuf);
    int len1 = mu_bbuf_capacity(bbuf);
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

static int mu_bbuf_find_aux(mu_bbuf_t *bbuf, const uint8_t *substr,
                           int substr_len, bool skip_substr) {
    const uint8_t *hay_bytes = mu_bbuf_bytes_ro(bbuf);
    int hay_len = mu_bbuf_capacity(bbuf);
    int j;

    if (substr_len == 0) {
        // null substr matches immediately
        return 0;
    }

    // First scan through bbuf looking for a byte that matches the first
    // byte of substr.  Micro-optimization: We stop searching when we get
    // within substr_len bytes of the end of bbuf, since beyond that, the
    // full-length search will always fail.
    for (int i = 0; i <= hay_len - substr_len; i++) {
        const uint8_t *h2 = &hay_bytes[i];
        if (*h2 == *substr) {
            // first byte matches.  Do the rest of the bytes match?
            for (j = 1; j < substr_len; j++) {
                if (h2[j] != substr[j]) {
                    // mismatch: advance to next char in bbuf
                    break;
                }
            }
            if (j == substr_len) {
                // found: &hay_bytes[i] matched all of *substr
                return skip_substr ? i + substr_len : i;
            }
        }
        // advance to next byte in bbuf
    }
    // got to end of bbuf without a match.
    return MU_BBUF_NOT_FOUND;
}

static int mu_bbuf_rfind_aux(mu_bbuf_t *bbuf, const uint8_t *substr,
                            int substr_len, bool skip_substr) {
    const uint8_t *hay_bytes = mu_bbuf_bytes_ro(bbuf);
    int hay_len = mu_bbuf_capacity(bbuf);
    int j;

    if (substr_len == 0) {
        // null substr matches immediately
        return hay_len;
    }

    // First scan through bbuf looking for a byte that matches the first
    // byte of substr.  Micro-optimization: We start searching at str_end -
    // substr_len bytes of the end of bbuf, since beyond that, the
    // full-length search will always fail.
    for (int i = hay_len - substr_len; i >= 0; --i) {
        const uint8_t *h2 = &hay_bytes[i];
        if (*h2 == *substr) {
            // first byte matches.  Do the rest of the bytes match?
            for (j = 1; j < substr_len; j++) {
                if (h2[j] != substr[j]) {
                    // mismatch: advance to next char in bbuf
                    break;
                }
            }
            if (j == substr_len) {
                // found: &hay_bytes[i] matched all of *substr
                return skip_substr ? i + substr_len : i;
            }
        }
        // advance to next byte in bbuf
    }
    // got to end of bbuf without a match.
    return MU_BBUF_NOT_FOUND;
}

static bool is_decimal(uint8_t byte) {
    if ((byte >= '0') && (byte <= '9')) {
        return true;
    }
    return false;
}
