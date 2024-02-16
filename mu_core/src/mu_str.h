/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. Dunbar Poor <rdpoor@gmail.com>
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
 * @file: mu_str.h
 *
 * @brief Safe, in-place string operations without the null terminator.
 *
 * A mu_str represents a string of bytes, represented by a pointer
 * the first byte of the string and the number of bytes in the string.
 *
 * Note that none of the functions here move data bytes; all functions work
 * in-place.
 */

#ifndef _MU_STR_H_
#define _MU_STR_H_

// *****************************************************************************
// Includes

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#define MU_STR_END INT_MAX
#define MU_STR_NOT_FOUND INT_MAX

typedef struct {
    union {
        const uint8_t *buf; // pointer to a read-only byte buffer
        uint8_t *rw_buf;    // pointer to a writable byte buffer
    };
    size_t length; // length of buffer in bytes
} mu_str_t;

/**
 * @brief The signature for a user-supplied predicate to mu_str_match (q.v.) and
 * other string-searching functions (mu_str_trim, etc).
 *
 * @param byte The one-byte character being examined.
 * @param arg The user argument passed to the string-searching function.
 * @return true on a match, false otherwise.
 */
typedef bool (*mu_str_predicate_t)(uint8_t byte, void *arg);

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a mu_str with a readonly data buffer and a length.
 */
mu_str_t *mu_str_init(mu_str_t *str, const uint8_t *buf, size_t length);

/**
 * @brief Initialize a mu_str with null-terminated C-style string.
 */
mu_str_t *mu_str_init_cstr(mu_str_t *str, const char *cstr);

/**
 * @brief Return the length of the mu_str.
 */
size_t mu_str_length(mu_str_t *str);

/**
 * @brief Return true if there are no bytes in the mu_str's data buffer.
 */
bool mu_str_is_empty(mu_str_t *str);

/**
 * @brief Access the index'th byte within str, return false if index is out of
 * range.
 */
bool mu_str_get_byte(mu_str_t *str, int index, uint8_t *byte);

/**
 * @brief Make a shallow copy of a mu_str from src to dst.
 */
mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src);

/**
 * @brief Compare two strings.
 *
 * It returns an integer less than, equal to, or greater than zero if s1 is
 * found, respectively, to be less than, to match, or be greater than s2.
 *
 * Only the N bytes are compared, where N is the lesser of the length of s1 and
 * of s2.
 */
int mu_str_compare(mu_str_t *s1, mu_str_t *s2);

/**
 * @brief Compare a mu_str against a null-terminated C-style string.
 *
 * It returns an integer less than, equal to, or greater than zero if s1 is
 * found, respectively, to be less than, to match, or be greater than cstr.
 *
 * Only the N bytes are compared, where N is the lesser of the length of s1 and
 * strlen(cstr).
 */
int mu_str_compare_cstr(mu_str_t *s1, const char *cstr);

/**
 * @brief Take a slice of a mu_str.
 *
 * @param dst The substring to receive the slice.  (Note: may be same as src).
 * @param src The string being sliced.
 * @param start Index into src, which becomes the first byte of dst.  If start
 *        is negative, indexes from the end of src.
 * @param end Index into src, which becomes the last byte (exclusive) of dst.
 *        If end is negative, indexes from the end of src.
 * @return dst
 *
 * Note that MU_STR_END may be used as an argument for start or end, signifying
 * the end (exclusive) of src.
 *
 * Example:
 *   mu_str_t src, dst;
 *   mu_str_init_cstr(&src, "potato");
 *   mu_str_slice(&dst, &src, -2, MU_STR_END);
 *   // dst now equals "to"
 */
mu_str_t *mu_str_slice(mu_str_t *dst, mu_str_t *src, int start, int end);

/**
 * @brief Slice a string into two substrings.
 *
 * @param left The left substring, i.e. all bytes to the left of index.
 * @param right The right subtring, i.e. all bytes the right of index.
 * @param src The string being sliced.
 * @param index Index into src, referencing the last byte (exclusive) of left
 * and the first byte (inclusive) of right.  If index is negative, it indexes
 * from the end of src.
 * @return src.
 *
 * Note: either left or right may equal src.  If both are equal to src, the
 * results are undefined.
 *
 * Example:
 *   mu_str_t left, right, src;
 *   mu_str_init_cstr(&src, "parking");
 *   mu_str_split(&left, &right, &src, 3);
 *   // left now equals "par", right equals "king"
 *   mu_str_split(&left, &right, &src, -1);
 *   // left now equals "parkin", right equals "g"
 */
mu_str_t *mu_str_split(mu_str_t *left, mu_str_t *right, mu_str_t *src,
                        int index);

/*******************
 * mu_str_find_xxx and mu_str_rfind_xxx search for a pattern and return an index
 * to the start of the pattern or MU_STR_NOT_FOUND if not found.
 */

/**
 * @brief Find the first occurance of a byte in a mu_str.
 *
 * Returns the index of the first occurance of b in str, or MU_STR_NOT_FOUND if
 * not found.
 *
 * Example:
 *   mu_str_t str;
 *   mu_str_init_cstr(&str, "C:/home/test.txt");
 *   size_t index = mu_str_find_byte(&str, ':');
 *   mu_str_slice(&src, &src, index+1, MU_STR_END);
 *   // src now = "/home/test.txt"
 */
size_t mu_str_find_byte(mu_str_t *s, uint8_t b);

/**
 * @brief Find the last occurance of a byte in a mu_str.
 *
 * Returns the index of the last occurance of b in str, or MU_STR_NOT_FOUND if
 * not found.
 *
 * Example:
 *   mu_str_t str, ext;
 *   mu_str_init_cstr(&str, "C:/home/test.txt");
 *   size_t index = mu_str_rfind_byte(&str, '.');
 *   mu_str_split(&src, &ext, &src, index, MU_STR_END);
 *   // src now = "/home/test", ext = ".ext"
 */
size_t mu_str_rfind_byte(mu_str_t *s, uint8_t b);

size_t mu_str_find_substr(mu_str_t *s, mu_str_t *substr);
size_t mu_str_find_subcstr(mu_str_t *s, const char *cstr);
size_t mu_str_rfind_substr(mu_str_t *s, mu_str_t *substr);
size_t mu_str_rfind_subcstr(mu_str_t *s, const char *cstr);

size_t mu_str_find(mu_str_t *s, mu_str_predicate_t predicate, void *arg, bool invert);
size_t mu_str_find_cstr(const char *cstr, mu_str_predicate_t predicate, void *arg, bool invert);
size_t mu_str_rfind(mu_str_t *s, mu_str_predicate_t predicate, void *arg, bool invert);
size_t mu_str_rfind_cstr(const char *cstr, mu_str_predicate_t predicate, void *arg, bool invert);

/*******************
 * mu_str_trim and related functions strip bytes from the beginning or end of
 * a string.
 */

/**
 * @brief Trim bytes from the beginning of a string.
 *
 * @param dst The string to receive the trimmed result.  dst may equal src.
 * @param src The string being trimmed.
 * @param predicate The user-supplied matching function
 * @param arg The user-supplied argument passed to predicate
 * @return dst
 *
 * All leading bytes for which predicate returns true are removed.
 */
mu_str_t *mu_str_ltrim(mu_str_t *dst, mu_str_t *src,
                       mu_str_predicate_t predicate, void *arg);

/**
 * @brief Trim bytes from the beginning of a string.
 *
 * @param dst The string to receive the trimmed result.  dst may equal src.
 * @param src The string being trimmed.
 * @param predicate The user-supplied matching function
 * @param arg The user-supplied argument passed to predicate
 * @return dst
 *
 * All trailing bytes for which predicate returns true are removed.
 */
mu_str_t *mu_str_rtrim(mu_str_t *dst, mu_str_t *src,
                       mu_str_predicate_t predicate, void *arg);

/**
 * @brief Trim bytes from the beginning and end of a string.
 *
 * @param dst The string to receive the trimmed result.  dst may equal src.
 * @param src The string being trimmed.
 * @param predicate The user-supplied matching function
 * @param arg The user-supplied argument passed to predicate
 * @return dst
 *
 * Any leading or trailing bytes for which predicate returns true are removed.
 */
mu_str_t *mu_str_trim(mu_str_t *dst, mu_str_t *src,
                      mu_str_predicate_t predicate, void *arg);

/*******************
 * mu_str_is_xxx are a set common predicates, provided as a convenience.
 */

bool mu_str_is_whitespace(uint8_t byte, void *arg); // [ \t\n\r\f\v]
bool mu_str_is_digit(uint8_t byte, void *arg);      // [0-9]
bool mu_str_is_hex(uint8_t byte, void *arg);        // [0-9a-fA-F]
bool mu_str_is_word(uint8_t byte, void *arg);       // [a-zA-Z0-9_]

/*******************
 * @brief A collection of simple integer parsers.
 *
 * @param str The string being parsed.
 * @return The converted integer
 *
 * Notes:
 * - Conversion stops on the at the first non-digit.
 * - If the first character is a non-digit, the parse function returns 0.
 * - Conversion continues to the end of string.  If the parsed number exceeds
 * - returned data type, the result is truncated to the low-order bits.
 */
int mu_str_parse_int(mu_str_t *str);
unsigned int mu_str_parse_unsigned_int(mu_str_t *str);
int8_t mu_str_parse_int8(mu_str_t *str);
uint8_t mu_str_parse_uint8(mu_str_t *str);
int16_t mu_str_parse_int16(mu_str_t *str);
uint16_t mu_str_parse_uint16(mu_str_t *str);
int32_t mu_str_parse_int32(mu_str_t *str);
uint32_t mu_str_parse_uint32(mu_str_t *str);
int64_t mu_str_parse_int64(mu_str_t *str);
uint64_t mu_str_parse_uint64(mu_str_t *str);
unsigned int mu_str_parse_hex(mu_str_t *str); // accepts [0-9][a-f][A-F]

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_STR_H_ */
