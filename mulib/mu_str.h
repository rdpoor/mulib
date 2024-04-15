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
 * A mu_str represents a slice of a string of bytes, represented by a pointer
 * the first byte of the string and the number of bytes in the string.
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
    const uint8_t *bytes; // pointer to read-only byte buffer
    int len;              // length of buffer in bytes
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
mu_str_t *mu_str_init(mu_str_t *str, const uint8_t *bytes, int len);

/**
 * @brief Initialize a mu_str with null-terminated C-style string.
 */
mu_str_t *mu_str_init_cstr(mu_str_t *str, const char *cstr);

/**
 * @brief Return the mu_str's data buffer.
 */
const uint8_t *mu_str_bytes(mu_str_t *str);

/**
 * @brief Return the number of bytes in the mu_str's data buffer.
 */
int mu_str_length(mu_str_t *str);

/**
 * @brief Return true if there are no bytes in the mu_str's data buffer.
 */
bool mu_str_is_empty(mu_str_t *str);

/**
 * @brief Make a shallow copy of a mu_str.
 */
mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src);

/**
 * @brief Compare two strings.
 *
 * mu_str_compare compares the first N bytes of s1 and s2, where N is the lesser
 * of mu_str_length(s1) and mu_str_length(s2), and returns:
 *   zero if the s1 and s2 are equal;
 *   a negative value if s1 is less than s2;
 *   a positive value if s1 is greater than s2.
 * If the two strings are equal up through the first N bytes, it returns:
 *   zero if the two strings are the same length
 *   a negative value is s1 is shorter than s2
 *   a positive value if s1 is longter than s2
 * This emulates the behavior of strncmp(), albeit without comparing against a
 * terminating '\0' (since mu_str doesn't have a terminating '\0').
 */
int mu_str_compare(mu_str_t *s1, mu_str_t *s2);

/**
 * @brief Compare a mu_str against a null-terminated C-style string.
 *
 * mu_str_compare compares the first N bytes of s1 and cstr, where N is the
 * lesser of mu_str_length(s1) and strlen(cstr), and returns:
 *   zero if the s1 and cstr are equal;
 *   a negative value if s1 is less than cstr;
 *   a positive value if s1 is greater than cstr.
 * If the two strings are equal up through the first N bytes, it returns:
 *   zero if the two strings are the same length
 *   a negative value is s1 is shorter than cstr
 *   a positive value if s1 is longter than cstr
 */
int mu_str_compare_cstr(mu_str_t *s1, const char *cstr);

/**
 * @brief Slice a substring of a mu_str.
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
 */
mu_str_t *mu_str_bisect(mu_str_t *left, mu_str_t *right, mu_str_t *src,
                        int index);

/**
 * @brief Return true if s2 is an exact prefix of s1
 */
bool mu_str_has_prefix(mu_str_t *s1, mu_str_t *s2);

/**
 * @brief Return true if cstr is an exact prefix of s1
 */
bool mu_str_has_prefix_cstr(mu_str_t *s1, const char *cstr);

/**
 * @brief Return true if s2 is an exact suffix of s1
 */
bool mu_str_has_suffix(mu_str_t *s1, mu_str_t *s2);

/**
 * @brief Return true if cstr is an exact suffix of s1
 */
bool mu_str_has_suffix_cstr(mu_str_t *s1, const char *cstr);

/**
 * @brief Search forward to find a substring within a string.
 *
 * @param str The source string to search
 * @param substr The substring to search
 * @param skip_substr Indicates whether to include or exclude substr.
 * @return If substr is not found in str, return MU_STR_NOT_FOUND.  If
 *         skip_substr is false, returns index of first byte of substr,
 *         else returns index of last byte of substr.
 */
int mu_str_find_substr(mu_str_t *str, mu_str_t *substr, bool skip_substr);

/**
 * @brief Search forward to find a substring within a string.
 *
 * @param str The source string to search
 * @param substr A null-terminated C-style string to search for.
 * @param skip_substr Indicates whether to include or exclude substr.
 * @return If substr is not found in str, return MU_STR_NOT_FOUND.  If
 *         skip_substr is false, returns index of first byte of substr,
 *         else returns index of last byte of substr.
 */
int mu_str_find_subcstr(mu_str_t *str, const char *substr, bool skip_substr);

/**
 * @brief Search in reverse to find a substring within a string.
 *
 * @param str The source string to search
 * @param substr The substring to search
 * @param skip_substr Indicates whether to include or exclude substr.
 * @return If substr is not found in str, return MU_STR_NOT_FOUND.  If
 *         skip_substr is false, returns index of first byte of substr,
 *         else returns index of last byte of substr.
 */
int mu_str_rfind_substr(mu_str_t *str, mu_str_t *substr, bool skip_substr);

/**
 * @brief Search in reverse to find a substring within a string.
 *
 * @param str The source string to search
 * @param substr A null-terminated C-style string to search for.
 * @param skip_substr Indicates whether to include or exclude substr.
 * @return If substr is not found in str, return MU_STR_NOT_FOUND.  If
 *         skip_substr is false, returns index of first byte of substr,
 *         else returns index of last byte of substr.
 */
int mu_str_rfind_subcstr(mu_str_t *str, const char *substr, bool skip_substr);

/**
 * @brief Search forward in a string until predicate returns true.
 *
 * @param str The string being searched
 * @param predicate the user-supplied search function
 * @param arg The user-supplied argument passed to predicate
 * @param break_if It true, break when predcate returns true, else break when
 * the predicate returns false.
 * @return the index of the first char for which predicate returns break_if, or
 * MU_STR_NOT_FOUND if there was no match.
 */
int mu_str_index(mu_str_t *str, mu_str_predicate_t predicate, void *arg,
                  bool break_if);

/**
 * @brief Search backward in a string until predicate returns true.
 *
 * @param str The string being searched
 * @param predicate the user-supplied search function
 * @param arg The user-supplied argument passed to predicate
 * @param break_if It true, break when predcate returns true, else break when
 * the predicate returns false.
 * @return the index of the first char for which predicate returns break_if, or
 * MU_STR_NOT_FOUND if there was no match.
 */
int mu_str_rindex(mu_str_t *str, mu_str_predicate_t predicate, void *arg,
                  bool break_if);

/**
 * @brief Trim bytes from the beginning of a string.
 *
 * @param dst The string to receive the trimmed result.  Note that dst may equal
 * src.
 * @param src The string being trimmed.
 * @param predicate The user-suppliced matching function
 * @param arg The user-supplied argument passed to predicate
 * @return dst
 *
 * Any leading characters for which predicate returns true are removed from src.
 */
mu_str_t *mu_str_ltrim(mu_str_t *dst, mu_str_t *src,
                       mu_str_predicate_t predicate, void *arg);

/**
 * @brief Trim bytes from the end of a string.
 *
 * @param dst The string to receive the trimmed result.  Note that dst may equal
 * src.
 * @param src The string being trimmed.
 * @param predicate The user-suppliced matching function
 * @param arg The user-supplied argument passed to predicate
 * @return dst
 *
 * Any trailing characters for which predicate returns true are removed from
 * src.
 */
mu_str_t *mu_str_rtrim(mu_str_t *dst, mu_str_t *src,
                       mu_str_predicate_t predicate, void *arg);

/**
 * @brief Trim bytes from the beginning and end of a string.
 *
 * @param dst The string to receive the trimmed result.  Note that dst may equal
 * src.
 * @param src The string being trimmed.
 * @param predicate The user-suppliced matching function
 * @param arg The user-supplied argument passed to predicate
 * @return dst
 *
 * Any leading or trailing characters for which predicate returns true are
 * removed from src.
 */
mu_str_t *mu_str_trim(mu_str_t *dst, mu_str_t *src,
                      mu_str_predicate_t predicate, void *arg);

/**
 * @brief Copy the contents of a mu_str plus a null terminator to a buffer.
 *
 * @param str The mu_str to copy
 * @param buf A byte array to hold the null-terminated string.
 * @param capacity The size of buf
 * @return true if buf is large enough to hold the bytes plus the null
 *         termination, false otherwise.
 */
bool mu_str_to_cstr(mu_str_t *str, char *buf, int capacity);

/**
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
