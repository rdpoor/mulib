/**
 * @file: mu_str.h
 * 
 * @brief Safe, in-place string operations without the null terminator.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MU_STR_END PTRDIFF_MAX
#define MU_STR_NOT_FOUND PTRDIFF_MAX

typedef struct {
  union {
    uint8_t *bytes;          // pointer to writable byte buffer
    const uint8_t *ro_bytes; // pointer to read-only byte buffer
  };
  size_t len; // length of buffer in bytes
} mu_str_t;

/**
 * @brief The signature for a user-supplied predicate to mu_str_trim (q.v.).
 * 
 * The function should return true on a match, false otherwise.
 */
typedef bool (*mu_str_predicate)(uint8_t byte, void *arg);

/**
 * @brief Initialize a mu_str with a data buffer and a length.
 */
mu_str_t *mu_str_init(mu_str_t *str, uint8_t *bytes, size_t len);

/**
 * @brief Initialize a mu_str with a read-only data buffer and a length.
 */
mu_str_t *mu_str_ro_init(mu_str_t *str, const uint8_t *bytes, size_t len);

/**
 * @brief Initialize a mu_str with null-terminated C-style string.
 */
mu_str_t *mu_str_cstr_init(mu_str_t *str, const char *cstr);

/**
 * @brief Return the mu_str's data buffer.
 */
uint8_t *mu_str_bytes(mu_str_t *str);

/**
 * @brief Return the mu_str's read-only data buffer.
 */
const uint8_t *mu_str_ro_bytes(mu_str_t *str);

/**
 * @brief Return the number of bytes in the mu_str's data buffer.
 */
size_t mu_str_length(mu_str_t *str);

/**
 * @brief Return true if there are no bytes in the mu_str's data buffer.
 */
bool mu_str_is_empty(mu_str_t *str);

/**
 * @brief Make a shallow copy of a mu_str.
 */
mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src);

/**
 * @brief Fill a mu_str.
 */
mu_str_t *mu_str_fill(mu_str_t *str, uint8_t byte);

/**
 * @brief Fill a mu_str.
 */
mu_str_t *mu_str_fill(mu_str_t *str, uint8_t byte);

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
mu_str_t *mu_str_slice(mu_str_t *dst,
                       mu_str_t *src,
                       ptrdiff_t start,
                       ptrdiff_t end);

/**
 * @brief Find a substring within a string, returning the indes of the first 
 * byte or MU_STR_NOT_FOUND if not found.
 */
size_t mu_str_find(mu_str_t *str, mu_str_t *substr);

/**
 * @brief Find a substring within a string searching backwards from the end,
 * returning the indes of the first byte or MU_STR_NOT_FOUND if not found.
 */
size_t mu_str_rfind(mu_str_t *str, mu_str_t *substr);

/**
 * @brief Remove bytes from the start of str for which predicate returns true.
 *
 * @return modified str.
 */
mu_str_t *mu_str_ltrim(mu_str_t *str, mu_str_predicate predicate, void *arg);

/**
 * @brief Remove bytes from the end of str for which predicate returns true.
 *
 * @return modified str.
 */
mu_str_t *mu_str_rtrim(mu_str_t *str, mu_str_predicate predicate, void *arg);

/**
 * @brief Remove bytes from the start and end of str for which predicate returns
 * true.
 *
 * @return modified str.
 */
mu_str_t *mu_str_trim(mu_str_t *str, mu_str_predicate predicate, void *arg);
