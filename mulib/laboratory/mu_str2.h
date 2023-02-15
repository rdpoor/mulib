/**
 * @file: mu_str2.h
 * @brief safe, zero-copy string operations.
 */

#define MU_STR_EOS INT_MAX

typedef struct {
    const char *buf;
    size_t len;
} mu_str_ro_t;

typedef struct {
    char *buf;
    size_t len;
} mu_str_rw_t;

mu_str_ro_t *mu_str_init_ro(mu_str_ro_t *str, const char *buf, size_t len);
mu_str_wr_t *mu_str_init_wr(mu_str_wr_t *str, char *buf, size_t len);

const char *mu_str_chars_ro(mu_str_ro_t *str);
const char *mu_str_chars_wr(mu_str_wr_t *str);

size_t mu_str_len_ro(mu_str_ro_t *str)
size_t mu_str_len_wr(mu_str_wr_t *str)

/**
 * @brief Return a slice of a mu_str.
 *
 * Note: out of bounds start and end are always clamped such that
 *       0 <= start <= end <= str->end
 *
 * @param substr The mu_str to receive the result.
 * @param str The "outer" string.
 * @param start Index of the starting char (inclusive).  If negative, indexes
 *        from end.
 * @parem end Indes of the ending char (exclusive).  If negative, indexes from
 *        end.
 */
mu_str_ro_t *mu_str_slice_ro(mu_str_ro_t *substr, mu_str_ro_t *str, int start, int end);
mu_str_wr_t *mu_str_slice_wr(mu_str_wr_t *substr, mu_str_wr_t *str, int start, int end);
