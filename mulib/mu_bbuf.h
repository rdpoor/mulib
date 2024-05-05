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

/**
 * @file bbuf.h
 *
 * @brief Safe operatons on an array of bytes
 */

#ifndef _BBUF_H_
#define _BBUF_H_

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

#define MU_BBUF_END INT_MAX
#define MU_BBUF_NOT_FOUND INT_MAX

typedef struct {
    union {
        const uint8_t *bytes_ro; // a buffer of read-only bytes
        uint8_t *bytes_rw;       // a buffer of mutable bytes
    };
    size_t capacity; // the size of the buffer.
} mu_bbuf_t;

/**
 * @brief The signature for a user-supplied predicate to mu_bbuf_match (q.v.) and
 * other string-searching functions (mu_bbuf_trim, etc).
 *
 * @param byte The one-byte character being examined.
 * @param arg The user argument passed to the string-searching function.
 * @return true on a match, false otherwise.
 */
typedef bool (*mu_bbuf_predicate_t)(uint8_t byte, void *arg);

// *****************************************************************************
// Public declarations

/**
 * @brief Initializes a buffer with read-only access to its data.
 *
 * This function sets up a buffer for read-only operations, binding it to a
 * provided data array with specified capacity. It is particularly useful when
 * the data should not be altered after initialization.
 *
 * @param bbuf Pointer to the buffer structure to initialize.
 * @param bytes Pointer to the data array to be associated with the buffer.
 * @param capacity Total number of bytes that the buffer can hold.
 * @return Pointer to the initialized buffer, or NULL if initialization fails.
 */
mu_bbuf_t *mu_bbuf_init_ro(mu_bbuf_t *bbuf, const uint8_t *bytes,
                           size_t capacity);

/**
 * @brief Initializes a buffer with mutable access to its data.
 *
 * Similar to mu_bbuf_init_ro, but the buffer is set up to allow modifications
 * to the data. This function is ideal for scenarios where the buffer's contents
 * need to be updated or manipulated.
 *
 * @param bbuf Pointer to the buffer structure to initialize.
 * @param bytes Pointer to the mutable data array.
 * @param capacity Total number of bytes that the buffer can hold.
 * @return Pointer to the initialized buffer, or NULL if initialization fails.
 */
mu_bbuf_t *mu_bbuf_init_rw(mu_bbuf_t *bbuf, uint8_t *bytes, size_t capacity);

/**
 * @brief Initializes a buffer using a null-terminated C-style string.
 *
 * This function sets up a buffer and copies a C-string into it, making the
 * buffer's content immutable. It is used when the buffer needs to manage a
 * string constant.
 *
 * @param bbuf Pointer to the buffer structure to initialize.
 * @param cstr Pointer to the null-terminated string to copy into the buffer.
 * @return Pointer to the initialized buffer, or NULL if initialization fails.
 */
mu_bbuf_t *mu_bbuf_init_cstr(mu_bbuf_t *bbuf, const char *cstr);

/**
 * @brief Creates a shallow copy of one buffer to another.
 *
 * This function duplicates the metadata of a source buffer into a destination
 * buffer, not the data itself. It is useful for creating a new buffer reference
 * to the same data, without copying the actual bytes.
 *
 * @param dst Pointer to the destination buffer to initialize.
 * @param src Pointer to the source buffer to copy from.
 * @return Pointer to the destination buffer after copying the metadata.
 */
mu_bbuf_t *mu_bbuf_copy(mu_bbuf_t *dst, mu_bbuf_t *src);

/**
 * @brief Retrieves a read-only pointer to the buffer's data.
 *
 * @param bbuf Pointer to the buffer from which to retrieve data.
 * @return Pointer to the read-only data array of the buffer.
 */
const uint8_t *mu_bbuf_bytes_ro(mu_bbuf_t *bbuf);

/**
 * @brief Retrieves a mutable pointer to the buffer's data.
 *
 * @param bbuf Pointer to the buffer from which to retrieve data.
 * @return Pointer to the mutable data array of the buffer.
 */
uint8_t *mu_bbuf_bytes_rw(mu_bbuf_t *bbuf);

/**
 * @brief Returns the total capacity of the buffer in bytes.
 *
 * @param bbuf Pointer to the buffer whose capacity is queried.
 * @return Total number of bytes the buffer can hold.
 */
size_t mu_bbuf_capacity(mu_bbuf_t *bbuf);

/**
 * @brief Provides a read-only reference to a specific byte within the buffer.
 *
 * This function returns a pointer to a byte at a specified index within the
 * buffer, if the index is within the bounds of the buffer's capacity. It is
 * used for accessing specific data without altering it.
 *
 * @param bbuf Pointer to the buffer.
 * @param index The zero-based index of the byte to access.
 * @return Pointer to the byte, or NULL if the index is out of range.
 */
const uint8_t *mu_bbuf_ref_ro(mu_bbuf_t *bbuf, size_t index);

/**
 * @brief Provides a mutable reference to a specific byte within the buffer.
 *
 * This function returns a pointer to a byte at a specified index within the
 * buffer, allowing modification if the index is within the bounds of the
 * buffer's capacity. It is used for altering specific data directly within the
 * buffer.
 *
 * @param bbuf Pointer to the buffer.
 * @param index The zero-based index of the byte to access.
 * @return Pointer to the byte, or NULL if the index is out of range.
 */
uint8_t *mu_bbuf_ref_rw(mu_bbuf_t *bbuf, size_t index);

/**
 * @brief Compare two byte buffers by their content.
 *
 * This function compares the content of two byte buffers up to the length of
 * the shorter buffer. It returns:
 *   - zero if the content of both buffers are equal;
 *   - a negative value if the first buffer is lexicographically less than the
 * second;
 *   - a positive value if the first buffer is lexicographically greater than
 * the second. If both buffers are equal up to the shorter length, the function
 * additionally checks the lengths:
 *   - zero if both buffers are the same length;
 *   - negative if the first buffer is shorter than the second;
 *   - positive if the first buffer is longer than the second.
 * This behavior is similar to strncmp(), but without terminating null byte
 * considerations.
 *
 * @param b1 Pointer to the first buffer.
 * @param b2 Pointer to the second buffer.
 * @return Integer result indicating the result of the comparison.
 */
int mu_bbuf_compare(mu_bbuf_t *b1, mu_bbuf_t *b2);

/**
 * @brief Compare a byte buffer against a null-terminated C-style string.
 *
 * This function compares the content of a byte buffer with a C-style string up
 * to the length of the shorter of the two. The return value is:
 *   - zero if the buffer and the C-string are equal;
 *   - negative if the buffer is less than the C-string;
 *   - positive if the buffer is greater than the C-string.
 * If the content matches up to the shorter length, it further checks the total
 * lengths of both:
 *   - zero if they are the same length;
 *   - negative if the buffer is shorter than the C-string;
 *   - positive if the buffer is longer than the C-string.
 *
 * @param bbuf Pointer to the buffer.
 * @param cstr Pointer to the null-terminated C-style string.
 * @return Integer result of the comparison.
 */
int mu_bbuf_compare_cstr(mu_bbuf_t *bbuf, const char *cstr);

/**
 * @brief Create a slice of a byte buffer.
 *
 * This function extracts a slice from a source buffer starting and ending at
 * specified indices and places it into a destination buffer. If the start or
 * end indices are negative, they are treated as offsets from the end of the
 * buffer. The function returns the destination buffer, potentially allowing for
 * chained operations.
 *
 * @param dst Destination buffer to store the slice.
 * @param src Source buffer from which to slice.
 * @param start Index for the beginning of the slice.
 * @param end Index for the end of the slice (exclusive).
 * @return Pointer to the destination buffer containing the slice.
 */
mu_bbuf_t *mu_bbuf_slice(mu_bbuf_t *dst, mu_bbuf_t *src, int start, int end);

/**
 * @brief Divide a byte buffer into two at a specified index.
 *
 * This function splits a source buffer at a given index into two distinct
 * buffers, left and right. The byte at the index is the first byte of the right
 * buffer and the last byte of the left buffer is the previous one. If the index
 * is negative, it is considered as an offset from the end of the buffer. If
 * either left or right buffer is the same as the source, behavior is defined by
 * their overlap and could be unpredictable.
 *
 * @param left Buffer to receive the left part of the split.
 * @param right Buffer to receive the right part of the split.
 * @param src Source buffer to be split.
 * @param index Index at which to split the buffer.
 * @return Pointer to the original source buffer.
 */
mu_bbuf_t *mu_bbuf_bisect(mu_bbuf_t *left, mu_bbuf_t *right, mu_bbuf_t *src,
                          int index);

/**
 * @brief Check if two buffers are exactly the same.
 *
 * This function checks if every byte in two buffers are identical.
 *
 * @param b1 Pointer to the first buffer.
 * @param b2 Pointer to the second buffer.
 * @return true if the buffers are identical, false otherwise.
 */
bool mu_bbuf_matches(mu_bbuf_t *b1, mu_bbuf_t *b2);

/**
 * @brief Check if a buffer matches a null-terminated C-style string exactly.
 *
 * This function compares a buffer with a C-style string to see if they are
 * identical.
 *
 * @param b1 Pointer to the buffer.
 * @param cstr Pointer to the null-terminated string.
 * @return true if they are identical, false otherwise.
 */
bool mu_bbuf_matches_cstr(mu_bbuf_t *b1, const char *cstr);

/**
 * @brief Check if one buffer is a prefix of another.
 *
 * This function determines if the second buffer is a prefix of the first
 * buffer.
 *
 * @param b1 Pointer to the buffer being checked.
 * @param b2 Pointer to the buffer that may be the prefix.
 * @return true if b2 is a prefix of b1, false otherwise.
 */
bool mu_bbuf_has_prefix(mu_bbuf_t *b1, mu_bbuf_t *b2);

/**
 * @brief Check if a C-style string is a prefix of a buffer.
 *
 * This function checks if a C-style string is a prefix of the given buffer.
 *
 * @param b1 Pointer to the buffer being checked.
 * @param cstr Pointer to the C-style string that may be the prefix.
 * @return true if the string is a prefix of the buffer, false otherwise.
 */
bool mu_bbuf_has_prefix_cstr(mu_bbuf_t *b1, const char *cstr);

/**
 * @brief Check if one buffer is a suffix of another.
 *
 * This function determines if the second buffer is a suffix of the first
 * buffer.
 *
 * @param b1 Pointer to the buffer being checked.
 * @param b2 Pointer to the buffer that may be the suffix.
 * @return true if b2 is a suffix of b1, false otherwise.
 */
bool mu_bbuf_has_suffix(mu_bbuf_t *b1, mu_bbuf_t *b2);

/**
 * @brief Check if a C-style string is a suffix of a buffer.
 *
 * This function checks if a C-style string is a suffix of the given buffer.
 *
 * @param b1 Pointer to the buffer being checked.
 * @param cstr Pointer to the C-style string that may be the suffix.
 * @return true if the string is a suffix of the buffer, false otherwise.
 */
bool mu_bbuf_has_suffix_cstr(mu_bbuf_t *b1, const char *cstr);

/**
 * @brief Find a substring within a buffer, searching forward.
 *
 * This function searches for a substring within a buffer, optionally skipping
 * the substring itself in the return index. If the substring is not found, it
 * returns MU_BBUF_NOT_FOUND. If found, the index returned depends on the
 * skip_substr flag:
 *   - false: returns the index of the first byte of the substring;
 *   - true: returns the index immediately after the last byte of the substring.
 *
 * @param bbuf The buffer to search within.
 * @param substr The buffer representing the substring to find.
 * @param skip_substr Whether to skip over the substring in the returned index.
 * @return Index of the substring or the position immediately after, or
 * MU_BBUF_NOT_FOUND.
 */
int mu_bbuf_find_substr(mu_bbuf_t *bbuf, mu_bbuf_t *substr, bool skip_substr);

/**
 * @brief Find a substring within a buffer, searching backward.
 *
 * Similar to mu_bbuf_find_substr, but searches from the end of the buffer
 * towards the beginning. The return values and behavior concerning skip_substr
 * are identical to mu_bbuf_find_substr.
 *
 * @param bbuf The buffer to search within.
 * @param substr The buffer representing the substring to find.
 * @param skip_substr Whether to skip over the substring in the returned index.
 * @return Index of the substring or the position immediately after, or
 * MU_BBUF_NOT_FOUND.
 */
int mu_bbuf_rfind_substr(mu_bbuf_t *bbuf, mu_bbuf_t *substr, bool skip_substr);

/**
 * @brief Search for a substring within a buffer, searching forward, using a
 * null-terminated C-style string.
 *
 * This function searches a buffer for a specified null-terminated string,
 * returning the index based on the skip_substr flag:
 *   - false: returns the index of the first byte of the substring;
 *   - true: returns the index immediately after the last byte of the substring.
 * If the substring is not found, MU_BBUF_NOT_FOUND is returned.
 *
 * @param bbuf The buffer to search within.
 * @param cstr Pointer to the null-terminated C-style string to find.
 * @param skip_substr Whether to skip over the substring in the returned index.
 * @return Index of the substring or the position immediately after, or
 * MU_BBUF_NOT_FOUND.
 */
int mu_bbuf_find_subcstr(mu_bbuf_t *bbuf, const char *cstr, bool skip_substr);

/**
 * @brief Search for a substring within a buffer, searching backward, using a
 * null-terminated C-style string.
 *
 * Similar to mu_bbuf_find_subcstr, but searches from the end of the buffer
 * towards the beginning. The return values and behavior concerning skip_substr
 * are identical to mu_bbuf_find_subcstr.
 *
 * @param bbuf The buffer to search within.
 * @param cstr Pointer to the null-terminated C-style string to find.
 * @param skip_substr Whether to skip over the substring in the returned index.
 * @return Index of the substring or the position immediately after, or
 * MU_BBUF_NOT_FOUND.
 */
int mu_bbuf_rfind_subcstr(mu_bbuf_t *bbuf, const char *cstr,
                          bool skip_substr);

/**
 * @brief Search forward to find a specific byte within a buffer.
 *
 * This function searches for a specific byte in a buffer starting at index == 0
 * and moving forward.
 *
 * @param bbuf The buffer to search within.
 * @param byte The byte to search for.
 * @return Index of the byte or MU_BBUF_NOT_FOUND if the byte is not found.
 */
int mu_bbuf_find_byte(mu_bbuf_t *bbuf, uint8_t byte);

/**
 * @brief Search backward to find a specific byte within a buffer.
 *
 * This function searches for a specific byte in a buffer starting at index ==
 * capacity-1 and moving backward.
 *
 * @param bbuf The buffer to search within.
 * @param byte The byte to search for.
 * @return Index of the byte or MU_BBUF_NOT_FOUND if the byte is not found.
 */
int mu_bbuf_rfind_byte(mu_bbuf_t *bbuf, uint8_t byte);

/**
 * @brief Search forward as long as the given predicate returns true.
 *
 * This function iterates over the bytes in a buffer, applying a user-supplied
 * predicate function to each byte, until the predicate returns false. The index
 * of the byte where the search stopped is returned, or MU_BBUF_NOT_FOUND if
 * no such byte exists.
 *
 * @param bbuf The buffer to search within.
 * @param pred The predicate function to apply.
 * @param arg Additional user-supplied argument passed to the predicate.
 * @return Index of the first byte for which the predicate returns false, or
 * MU_BBUF_NOT_FOUND.
 */
int mu_bbuf_find_predicate(mu_bbuf_t *bbuf, mu_bbuf_predicate_t pred,
                           void *arg);

/**
 * @brief Search backward in a buffer until a predicate returns true.
 *
 * Similar to mu_bbuf_find_predicate, but searches from the end of the buffer
 * towards the beginning. The behavior concerning the predicate function is
 * identical to mu_bbuf_find_predicate.
 *
 * @param bbuf The buffer to search within.
 * @param pred The predicate function to apply.
 * @param arg Additional user-supplied argument passed to the predicate.
 * @return Index of the first byte for which the predicate returns false, or
 * MU_BBUF_NOT_FOUND.
 */
int mu_bbuf_rfind_predicate(mu_bbuf_t *bbuf, mu_bbuf_predicate_t pred,
                            void *arg);

/**
 * @brief Trim bytes from both the beginning and end of a byte buffer according
 * to a predicate function.
 *
 * This function trims bytes from both the beginning and end of the source
 * buffer where the predicate function returns true. The result is stored in the
 * destination buffer, which can be the same as the source buffer. This
 * operation allows selective trimming based on byte values, making it flexible
 * for different conditions.
 *
 * @param dst Destination buffer to store the trimmed output; can be the same as
 * src.
 * @param src Source buffer from which bytes are trimmed.
 * @param predicate User-supplied function that determines if a byte should be
 * trimmed.
 * @param arg Optional user-supplied argument passed to the predicate function.
 * @return Pointer to the destination buffer containing the trimmed data.
 */
mu_bbuf_t *mu_bbuf_trim(mu_bbuf_t *dst, mu_bbuf_t *src,
                        mu_bbuf_predicate_t predicate, void *arg);

/**
 * @brief Trim bytes from the beginning of a byte buffer based on a predicate
 * function.
 *
 * This function removes bytes from the start of the source buffer as long as
 * the predicate function returns true. The trimming process stops at the first
 * byte for which the predicate returns false. The result is stored in the
 * destination buffer, which may be the same as the source buffer.
 *
 * @param dst Destination buffer to store the trimmed output; can be the same as
 * src.
 * @param src Source buffer from which bytes are trimmed.
 * @param predicate User-supplied function that determines if a byte should be
 * trimmed.
 * @param arg Optional user-supplied argument passed to the predicate function.
 * @return Pointer to the destination buffer containing the trimmed data.
 */
mu_bbuf_t *mu_bbuf_ltrim(mu_bbuf_t *dst, mu_bbuf_t *src,
                         mu_bbuf_predicate_t predicate, void *arg);

/**
 * @brief Trim bytes from the end of a byte buffer based on a predicate
 * function.
 *
 * This function removes bytes from the end of the source buffer, proceeding
 * backward as long as the predicate function returns true. The process stops at
 * the first byte, scanning from the end, for which the predicate returns false.
 * The result is stored in the destination buffer, which may be the same as the
 * source buffer.
 *
 * @param dst Destination buffer to store the trimmed output; can be the same as
 * src.
 * @param src Source buffer from which bytes are trimmed.
 * @param predicate User-supplied function that determines if a byte should be
 * trimmed.
 * @param arg Optional user-supplied argument passed to the predicate function.
 * @return Pointer to the destination buffer containing the trimmed data.
 */
mu_bbuf_t *mu_bbuf_rtrim(mu_bbuf_t *dst, mu_bbuf_t *src,
                         mu_bbuf_predicate_t predicate, void *arg);

/**
 * @brief Parse an integer from a byte buffer and handle different integer
 * types.
 *
 * These functions parse integers from the beginning of a byte buffer up to the
 * first non-digit character or the end of the buffer. The parsing stops at the
 * first non-numeric character, and if the first character is non-numeric, the
 * function returns 0. If the parsed number overflows the target type, the
 * result is truncated to fit the data type.
 *
 * @param bbuf Buffer containing the numeric characters to parse.
 * @return The parsed integer, formatted according to the specific function's
 * return type.
 */
int mu_bbuf_parse_int(mu_bbuf_t *bbuf);
unsigned int mu_bbuf_parse_unsigned_int(mu_bbuf_t *bbuf);
int8_t mu_bbuf_parse_int8(mu_bbuf_t *bbuf);
uint8_t mu_bbuf_parse_uint8(mu_bbuf_t *bbuf);
int16_t mu_bbuf_parse_int16(mu_bbuf_t *bbuf);
uint16_t mu_bbuf_parse_uint16(mu_bbuf_t *bbuf);
int32_t mu_bbuf_parse_int32(mu_bbuf_t *bbuf);
uint32_t mu_bbuf_parse_uint32(mu_bbuf_t *bbuf);
int64_t mu_bbuf_parse_int64(mu_bbuf_t *bbuf);
uint64_t mu_bbuf_parse_uint64(mu_bbuf_t *bbuf);
unsigned int mu_bbuf_parse_hex(
    mu_bbuf_t *bbuf); // Accepts hexadecimal characters [0-9][a-f][A-F].

// *****************************************************************************
// Operations on mutable (rw) buffers

/**
 * @brief Retrieve a byte from a specific index in a byte buffer.
 *
 * This function attempts to copy a byte from the specified index of the buffer
 * to the provided byte pointer. If the index is within the range of the buffer,
 * the byte at that position is copied, and the function returns true. If the
 * index is out of range, the function returns false and the byte is not copied.
 *
 * @param bbuf Pointer to the buffer from which the byte is to be retrieved.
 * @param index The zero-based index of the byte within the buffer.
 * @param byte Pointer to store the retrieved byte.
 * @return true if the byte was successfully retrieved, false if the index is
 * out of range.
 */
bool mu_bbuf_get_byte(mu_bbuf_t *bbuf, size_t index, uint8_t *byte);

/**
 * @brief Write a byte to a specific index in a mutable byte buffer.
 *
 * This function writes a byte to the buffer at the specified index. If the
 * index is within the valid range of the buffer, the byte is written, and the
 * function returns true. If the index is out of the buffer's range, the
 * function returns false and no modification is made to the buffer.
 *
 * @param bbuf_rw Pointer to the mutable buffer where the byte is to be written.
 * @param index The zero-based index at which the byte will be written.
 * @param byte The byte to write into the buffer.
 * @return true if the byte was successfully written, false if the index is out
 * of range.
 */
bool mu_bbuf_put_byte(mu_bbuf_t *bbuf_rw, size_t index, uint8_t byte);

/**
 * @brief Clear all bytes in a mutable byte buffer by setting them to zero.
 *
 * This function sets all bytes in the specified mutable buffer to zero.
 *
 * @param bbuf_rw Pointer to the mutable buffer to be cleared.
 * @return Pointer to the cleared buffer.
 */
mu_bbuf_t *mu_bbuf_clear(mu_bbuf_t *bbuf_rw);

/**
 * @brief Copy bytes from a source buffer into a destination buffer starting at
 * a specified offset.
 *
 * This function copies bytes from the source buffer to the destination buffer
 * beginning at the specified offset in the destination. It ensures that the
 * copy operation does not exceed the bounds of the destination buffer nor the
 * source buffer.  It returns the number of bytes actually copied.
 *
 * @param bbuf_rw Pointer to the destination mutable buffer where bytes are to
 * be copied.
 * @param src Pointer to the source buffer from which bytes are copied.
 * @param offset The zero-based index in the destination buffer at which to
 * start copying.  It may be negative.
 * @return The number of bytes successfully copied.
 */
int mu_bbuf_copy_into(mu_bbuf_t *bbuf_rw, mu_bbuf_t *src, int offset);

/**
 * @brief Reverse the bytes in bbuf_rw.
 */
mu_bbuf_t *mu_bbuf_reverse(mu_bbuf_t *bbuf_rw);

/**
 * @brief Rotate the bytes in bbuf_rw to the right by shift, or
 * to the left if shift is negative.
 */
mu_bbuf_t *mu_bbuf_rrotate(mu_bbuf_t *bbuf_rw, int shift);

/**
 * @brief Shift all bytes bbuf_rw to the right by shift, or to the left if
 * shift is negative, padding the left or right end with zeroes.
 *
 * @param bbuf_rw Pointer to the mutable buffer to be shifted.
 * @param shift Number of positions each byte in the buffer is to be
 * shifted right (or left if negative).
 * @return Pointer to the shifted buffer.
 */
mu_bbuf_t *mu_bbuf_rshift(mu_bbuf_t *bbuf_rw, int left_shift);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _BBUF_H_ */
