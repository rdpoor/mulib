#include "fff.h"
#include "mu_bbuf.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

// helper functions

__attribute__((unused)) static void print_str(mu_bbuf_t *bbuf) {
    int len = mu_bbuf_capacity(bbuf);
    printf("\n[%d]: '%.*s'", len, (int)len, mu_bbuf_bytes_ro(bbuf));
}

__attribute__((unused)) static int local_strncmp(const char *b1, const char *b2,
                                                 register int n) {
    unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char)*b1++;
        u2 = (unsigned char)*b2++;
        if (u1 != u2) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

// Return true if bbuf->bytes equals cstr
__attribute__((unused)) static bool cstr_eq(mu_bbuf_t *bbuf, const char *cstr) {
    return local_strncmp((const char *)mu_bbuf_bytes_ro(bbuf), cstr,
                         mu_bbuf_capacity(bbuf)) == 0;
}

__attribute__((unused)) static bool buf_eq(mu_bbuf_t *bbuf, const uint8_t *expect, size_t expect_len) {
    if (mu_bbuf_capacity(bbuf) != expect_len) {
        return false;
    } else {
        return memcmp(bbuf->bytes_ro, expect, expect_len) == 0;
    }
}

__attribute__((unused)) static bool is_member(uint8_t byte, const char *bytes) {
    while (*bytes != '\0') {
        if (byte == *bytes++) {
            return true;
        }
    }
    return false;
}

__attribute__((unused)) static bool is_numeric(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, "0123456789");
}

__attribute__((unused)) static bool is_not_numeric(uint8_t byte, void *arg) {
    return !is_numeric(byte, arg);
}

__attribute__((unused)) static bool is_hexadecimal(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, "0123456789abcdefABCDEF");
}

__attribute__((unused)) static bool is_not_hexadecimal(uint8_t byte, void *arg) {
    return !is_hexadecimal(byte, arg);
}

__attribute__((unused)) static bool is_whitespace(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, " \t\r\n\f\v");
}

__attribute__((unused)) static bool is_not_whitespace(uint8_t byte, void *arg) {
    return !is_whitespace(byte, arg);
}

__attribute__((unused)) static bool is_x(uint8_t byte, void *arg) {
    (void)arg; // unused
    return byte == 'x';
}

__attribute__((unused)) static bool is_not_x(uint8_t byte, void *arg) {
    return !is_x(byte, arg);
}

__attribute__((unused)) static bool is_never(uint8_t byte, void *arg) {
    (void)arg; // unused
    return false;
}

__attribute__((unused)) static bool is_not_never(uint8_t byte, void *arg) {
    return !is_never(byte, arg);
}

__attribute__((unused)) static bool is_always(uint8_t byte, void *arg) {
    (void)arg; // unused
    return true;
}

__attribute__((unused)) static bool is_not_always(uint8_t byte, void *arg) {
    return !is_not_always(byte, arg);
}

void setUp(void) {}

void tearDown(void) {
    // nothing yet
}

void test_mu_bbuf_init(void) {
    const uint8_t buf_ro[10];
    mu_bbuf_t b1;
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_init_ro(&b1, buf_ro, sizeof(buf_ro)));
    TEST_ASSERT_EQUAL_PTR(buf_ro, mu_bbuf_bytes_ro(&b1));
    TEST_ASSERT_EQUAL_INT(sizeof(buf_ro), mu_bbuf_capacity(&b1));

    const char *cstr = "asdf";
    mu_bbuf_t b2;
    TEST_ASSERT_EQUAL_PTR(&b2, mu_bbuf_init_cstr(&b2, cstr));
    TEST_ASSERT_EQUAL_PTR(cstr, mu_bbuf_bytes_ro(&b2));
    TEST_ASSERT_EQUAL_INT(strlen(cstr), mu_bbuf_capacity(&b2));

    uint8_t buf_rw[10];
    mu_bbuf_t b3;
    TEST_ASSERT_EQUAL_PTR(&b3, mu_bbuf_init_rw(&b3, buf_rw, sizeof(buf_rw)));
    TEST_ASSERT_EQUAL_PTR(buf_rw, mu_bbuf_bytes_rw(&b3));
    TEST_ASSERT_EQUAL_INT(sizeof(buf_rw), mu_bbuf_capacity(&b3));
}

void test_mu_bbuf_copy(void) {
    mu_bbuf_t b1, b2;
    uint8_t buf[10];
    mu_bbuf_init_ro(&b1, buf, sizeof(buf));

    TEST_ASSERT_EQUAL_PTR(&b2, mu_bbuf_copy(&b2, &b1));
    TEST_ASSERT_EQUAL_PTR(mu_bbuf_bytes_ro(&b2), mu_bbuf_bytes_ro(&b1));
    TEST_ASSERT_EQUAL_INT(mu_bbuf_capacity(&b2), mu_bbuf_capacity(&b1));
}

void test_mu_buf_ref(void) {
    const uint8_t buf_ro[10];
    mu_bbuf_t b1;
    mu_bbuf_init_ro(&b1, buf_ro, sizeof(buf_ro));
    TEST_ASSERT_EQUAL_PTR(&buf_ro[0], mu_bbuf_ref_ro(&b1, 0));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_bbuf_ref_ro(&b1, sizeof(buf_ro)));

    uint8_t buf_rw[10];
    mu_bbuf_t b3;
    mu_bbuf_init_rw(&b3, buf_rw, sizeof(buf_rw));
    TEST_ASSERT_EQUAL_PTR(&buf_rw[0], mu_bbuf_ref_rw(&b3, 0));
    TEST_ASSERT_EQUAL_PTR(NULL, mu_bbuf_ref_rw(&b3, sizeof(buf_rw)));
}

void test_mu_bbuf_compare(void) {
    mu_bbuf_t b1, b2;

    mu_bbuf_init_cstr(&b1, "abcd");
    // strings are equal in content and length
    TEST_ASSERT(mu_bbuf_compare(&b1, mu_bbuf_init_cstr(&b2, "abcd")) == 0);
    // b1 is lexographically higher
    TEST_ASSERT(mu_bbuf_compare(&b1, mu_bbuf_init_cstr(&b2, "abcc")) > 0);
    // b1 is lexographically lower
    TEST_ASSERT(mu_bbuf_compare(&b1, mu_bbuf_init_cstr(&b2, "abce")) < 0);
    // b1 is longer
    TEST_ASSERT(mu_bbuf_compare(&b1, mu_bbuf_init_cstr(&b2, "abc")) > 0);
    // b1 is shorter
    TEST_ASSERT(mu_bbuf_compare(&b1, mu_bbuf_init_cstr(&b2, "abcde")) < 0);
    // both empty
    TEST_ASSERT(mu_bbuf_compare(mu_bbuf_init_cstr(&b1, ""),
                               mu_bbuf_init_cstr(&b2, "")) == 0);
    // b2 empty
    TEST_ASSERT(mu_bbuf_compare(mu_bbuf_init_cstr(&b1, "abcd"),
                               mu_bbuf_init_cstr(&b2, "")) > 0);
    // b1 empty
    TEST_ASSERT(mu_bbuf_compare(mu_bbuf_init_cstr(&b1, ""),
                               mu_bbuf_init_cstr(&b2, "abcd")) < 0);
}

void test_mu_bbuf_compare_cstr(void) {
    mu_bbuf_t b1;

    mu_bbuf_init_cstr(&b1, "abcd");
    // strings are equal in content and length
    TEST_ASSERT(mu_bbuf_compare_cstr(&b1, "abcd") == 0);
    // b1 is lexographically higher
    TEST_ASSERT(mu_bbuf_compare_cstr(&b1, "abcc") > 0);
    // b1 is lexographically lower
    TEST_ASSERT(mu_bbuf_compare_cstr(&b1, "abce") < 0);
    // b1 is longer
    TEST_ASSERT(mu_bbuf_compare_cstr(&b1, "abc") > 0);
    // b1 is shorter
    TEST_ASSERT(mu_bbuf_compare_cstr(&b1, "abcde") < 0);
    // b2 empty
    TEST_ASSERT(mu_bbuf_compare_cstr(&b1, "") > 0);
    // both empty
    TEST_ASSERT(mu_bbuf_compare_cstr(mu_bbuf_init_cstr(&b1, ""), "") == 0);
    // b1 empty
    TEST_ASSERT(mu_bbuf_compare_cstr(mu_bbuf_init_cstr(&b1, ""), "abcd") < 0);
}

void test_mu_bbuf_slice(void) {
    mu_bbuf_t b1, b2;

    mu_bbuf_init_cstr(&b1, "ABCDEFGHIJ");
    // whole slice (indefinite end index)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 0, MU_BBUF_END));
    TEST_ASSERT(cstr_eq(&b2, "ABCDEFGHIJ"));
    // whole slice (definite end index)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 0, mu_bbuf_capacity(&b1)));
    TEST_ASSERT(cstr_eq(&b2, "ABCDEFGHIJ"));

    // remove first char (positive start index)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 1, MU_BBUF_END));
    TEST_ASSERT(cstr_eq(&b2, "BCDEFGHIJ"));
    // remove first char (negative start index)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, -9, MU_BBUF_END));
    TEST_ASSERT(cstr_eq(&b2, "BCDEFGHIJ"));

    // remove last char (positive end index)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 0, 9));
    TEST_ASSERT(cstr_eq(&b2, "ABCDEFGHI"));
    // remove last char (negative end index)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 0, -1));
    TEST_ASSERT(cstr_eq(&b2, "ABCDEFGHI"));

    // extract middle chars (positive indeces)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 3, 7));
    TEST_ASSERT(cstr_eq(&b2, "DEFG"));
    // extract middle chars (negative indeces)
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, -7, -3));
    TEST_ASSERT(cstr_eq(&b2, "DEFG"));

    // start == end
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 5, 5));
    TEST_ASSERT(cstr_eq(&b2, ""));
    // start > end
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 6, 5));
    TEST_ASSERT(cstr_eq(&b2, ""));

    // start > end of string
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 20, mu_bbuf_capacity(&b1)));
    TEST_ASSERT(cstr_eq(&b2, ""));
    // end < beginnig of string
    TEST_ASSERT(&b2 == mu_bbuf_slice(&b2, &b1, 0, -20));
    TEST_ASSERT(cstr_eq(&b2, ""));
}

void test_mu_bbuf_bisect(void) {
    mu_bbuf_t src, left, right;

    mu_bbuf_init_cstr(&src, "ABCDEFGHIJ");
    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, 0) == &src);
    TEST_ASSERT(cstr_eq(&left, ""));
    TEST_ASSERT(cstr_eq(&right, "ABCDEFGHIJ"));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, 1) == &src);
    TEST_ASSERT(cstr_eq(&left, "A"));
    TEST_ASSERT(cstr_eq(&right, "BCDEFGHIJ"));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, 9) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHI"));
    TEST_ASSERT(cstr_eq(&right, "J"));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, 10) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHIJ"));
    TEST_ASSERT(cstr_eq(&right, ""));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, MU_BBUF_END) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHIJ"));
    TEST_ASSERT(cstr_eq(&right, ""));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, -1) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHI"));
    TEST_ASSERT(cstr_eq(&right, "J"));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, -9) == &src);
    TEST_ASSERT(cstr_eq(&left, "A"));
    TEST_ASSERT(cstr_eq(&right, "BCDEFGHIJ"));

    TEST_ASSERT(mu_bbuf_bisect(&left, &right, &src, -10) == &src);
    TEST_ASSERT(cstr_eq(&left, ""));
    TEST_ASSERT(cstr_eq(&right, "ABCDEFGHIJ"));

    // left may equal src
    mu_bbuf_init_cstr(&src, "ABCDEFGHIJ");
    TEST_ASSERT(mu_bbuf_bisect(&src, &right, &src, 1) == &src);
    TEST_ASSERT(cstr_eq(&src, "A"));
    TEST_ASSERT(cstr_eq(&right, "BCDEFGHIJ"));

    // right may equal src
    mu_bbuf_init_cstr(&src, "ABCDEFGHIJ");
    TEST_ASSERT(mu_bbuf_bisect(&left, &src, &src, 1) == &src);
    TEST_ASSERT(cstr_eq(&left, "A"));
    TEST_ASSERT(cstr_eq(&src, "BCDEFGHIJ"));
}

void test_mu_bbuf_matches(void) {
    mu_bbuf_t b1, b2;
    mu_bbuf_init_cstr(&b1, "abcd");

    TEST_ASSERT_FALSE(mu_bbuf_matches(&b1, mu_bbuf_init_cstr(&b2, "")));
    TEST_ASSERT_FALSE(mu_bbuf_matches(&b1, mu_bbuf_init_cstr(&b2, "a")));
    TEST_ASSERT_FALSE(mu_bbuf_matches(&b1, mu_bbuf_init_cstr(&b2, "ab")));
    TEST_ASSERT_FALSE(mu_bbuf_matches(&b1, mu_bbuf_init_cstr(&b2, "abc")));
    TEST_ASSERT_TRUE(mu_bbuf_matches(&b1, mu_bbuf_init_cstr(&b2, "abcd")));
    TEST_ASSERT_FALSE(mu_bbuf_matches(&b1, mu_bbuf_init_cstr(&b2, "abcde")));

    TEST_ASSERT_FALSE(mu_bbuf_matches_cstr(&b1, ""));
    TEST_ASSERT_FALSE(mu_bbuf_matches_cstr(&b1, "a"));
    TEST_ASSERT_FALSE(mu_bbuf_matches_cstr(&b1, "ab"));
    TEST_ASSERT_FALSE(mu_bbuf_matches_cstr(&b1, "abc"));
    TEST_ASSERT_TRUE(mu_bbuf_matches_cstr(&b1, "abcd"));
    TEST_ASSERT_FALSE(mu_bbuf_matches_cstr(&b1, "abcde"));
}

void test_mu_bbuf_has_suffix_prefix(void) {
    mu_bbuf_t b1, b2;
    mu_bbuf_init_cstr(&b1, "abcd");

    TEST_ASSERT_TRUE(mu_bbuf_has_prefix(&b1, mu_bbuf_init_cstr(&b2, "")));
    TEST_ASSERT_TRUE(mu_bbuf_has_prefix(&b1, mu_bbuf_init_cstr(&b2, "ab")));
    TEST_ASSERT_FALSE(mu_bbuf_has_prefix(&b1, mu_bbuf_init_cstr(&b2, "cd")));
    TEST_ASSERT_TRUE(mu_bbuf_has_prefix(&b1, mu_bbuf_init_cstr(&b2, "abcd")));
    TEST_ASSERT_FALSE(mu_bbuf_has_prefix(&b1, mu_bbuf_init_cstr(&b2, "abcde")));

    TEST_ASSERT_TRUE(mu_bbuf_has_prefix_cstr(&b1, ""));
    TEST_ASSERT_TRUE(mu_bbuf_has_prefix_cstr(&b1, "ab"));
    TEST_ASSERT_FALSE(mu_bbuf_has_prefix_cstr(&b1, "cd"));
    TEST_ASSERT_TRUE(mu_bbuf_has_prefix_cstr(&b1, "abcd"));
    TEST_ASSERT_FALSE(mu_bbuf_has_prefix_cstr(&b1, "abcde"));

    TEST_ASSERT_TRUE(mu_bbuf_has_suffix(&b1, mu_bbuf_init_cstr(&b2, "")));
    TEST_ASSERT_FALSE(mu_bbuf_has_suffix(&b1, mu_bbuf_init_cstr(&b2, "ab")));
    TEST_ASSERT_TRUE(mu_bbuf_has_suffix(&b1, mu_bbuf_init_cstr(&b2, "cd")));
    TEST_ASSERT_TRUE(mu_bbuf_has_suffix(&b1, mu_bbuf_init_cstr(&b2, "abcd")));
    TEST_ASSERT_FALSE(mu_bbuf_has_suffix(&b1, mu_bbuf_init_cstr(&b2, "abcde")));

    TEST_ASSERT_TRUE(mu_bbuf_has_suffix_cstr(&b1, ""));
    TEST_ASSERT_FALSE(mu_bbuf_has_suffix_cstr(&b1, "ab"));
    TEST_ASSERT_TRUE(mu_bbuf_has_suffix_cstr(&b1, "cd"));
    TEST_ASSERT_TRUE(mu_bbuf_has_suffix_cstr(&b1, "abcd"));
    TEST_ASSERT_FALSE(mu_bbuf_has_suffix_cstr(&b1, "abcde"));
}

void test_mu_bbuf_find_substr(void) {
    mu_bbuf_t b1, b2;

    //                     0123456789
    mu_bbuf_init_cstr(&b1, "abXcdabYcd");
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, ""), false) == 0);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, ""), true) == 0);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, "ab"), false) ==
                0);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, "ab"), true) ==
                2);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, "cd"), false) ==
                3);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, "cd"), true) ==
                5);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, "cdX"), false) ==
                MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_find_substr(&b1, mu_bbuf_init_cstr(&b2, "cdX"), true) ==
                MU_BBUF_NOT_FOUND);

    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "", false) == 0);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "", true) == 0);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "ab", false) == 0);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "ab", true) == 2);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "cd", false) == 3);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "cd", true) == 5);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "cdX", false) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_find_subcstr(&b1, "cdX", true) == MU_BBUF_NOT_FOUND);

    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, ""), false) ==
                10);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, ""), true) ==
                10);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, "ab"), false) ==
                5);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, "ab"), true) ==
                7);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, "cd"), false) ==
                8);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, "cd"), true) ==
                10);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, "cdX"), false) ==
                MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_rfind_substr(&b1, mu_bbuf_init_cstr(&b2, "cdX"), true) ==
                MU_BBUF_NOT_FOUND);

    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "", false) == 10);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "", true) == 10);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "ab", false) == 5);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "ab", true) == 7);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "cd", false) == 8);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "cd", true) == 10);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "cdX", false) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_rfind_subcstr(&b1, "cdX", true) == MU_BBUF_NOT_FOUND);
}

void test_mu_bbuf_find_byte(void) {
    mu_bbuf_t b;
    const char *cstr1 = "abac";
    mu_bbuf_init_cstr(&b, cstr1);
    TEST_ASSERT_EQUAL_INT(0, mu_bbuf_find_byte(&b, 'a'));
    TEST_ASSERT_EQUAL_INT(3, mu_bbuf_find_byte(&b, 'c'));
    TEST_ASSERT_EQUAL_INT(MU_BBUF_NOT_FOUND, mu_bbuf_find_byte(&b, 'z'));

    const char *cstr2 = "caba";
    mu_bbuf_init_cstr(&b, cstr2);
    TEST_ASSERT_EQUAL_INT(3, mu_bbuf_rfind_byte(&b, 'a'));
    TEST_ASSERT_EQUAL_INT(0, mu_bbuf_rfind_byte(&b, 'c'));
    TEST_ASSERT_EQUAL_INT(MU_BBUF_NOT_FOUND, mu_bbuf_rfind_byte(&b, 'z'));
}

void test_mu_bbuf_find_predicate(void) {
    mu_bbuf_t b1;

    mu_bbuf_init_cstr(&b1, "0123");
    // finds the very first char '0'
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_numeric, NULL) == 0); // found '0'
    // hits the end of string without a "not match"
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_numeric, NULL) == MU_BBUF_NOT_FOUND);
    // finds the very last char '3'
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_not_numeric, NULL) == 3); // found '3'
    // hits the begining of string without a "not match"
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_numeric, NULL) == MU_BBUF_NOT_FOUND);

    //                     00000000001111111 1 1 1 2 2
    //                     01234567890123456 7 8 9 0 1
    mu_bbuf_init_cstr(&b1, "0123456789abcDEF \r\n\t\v\f");

    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_numeric, NULL) == 0);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_numeric, NULL) == 10);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_hexadecimal, NULL) == 0);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_hexadecimal, NULL) == 16);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_whitespace, NULL) == 16);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_whitespace, NULL) == 0);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_x, NULL) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_x, NULL) == 0);

    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_not_numeric, NULL) == 9);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_numeric, NULL) == 21);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_not_hexadecimal, NULL) == 15);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_hexadecimal, NULL) == 21);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_not_whitespace, NULL) == 21);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_whitespace, NULL) == 15);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_not_x, NULL) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_x, NULL) == 21);

    mu_bbuf_init_cstr(&b1, "");
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_never, NULL) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_never, NULL) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_find_predicate(&b1, is_not_always, NULL) == MU_BBUF_NOT_FOUND);
    TEST_ASSERT(mu_bbuf_rfind_predicate(&b1, is_always, NULL) == MU_BBUF_NOT_FOUND);
}

void test_mu_bbuf_trim(void) {
    mu_bbuf_t src;
    mu_bbuf_t dst;

    mu_bbuf_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&dst == mu_bbuf_ltrim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, "abcde  "));

    mu_bbuf_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&dst == mu_bbuf_rtrim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, "  abcde"));

    mu_bbuf_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&dst == mu_bbuf_trim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, "abcde"));

    // src may equal dst
    mu_bbuf_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&src == mu_bbuf_ltrim(&src, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&src, "abcde  "));

    mu_bbuf_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&src == mu_bbuf_rtrim(&src, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&src, "  abcde"));

    mu_bbuf_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&src == mu_bbuf_trim(&src, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&src, "abcde"));

    // no trimming
    mu_bbuf_init_cstr(&src, "");
    TEST_ASSERT(&dst == mu_bbuf_ltrim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, ""));

    mu_bbuf_init_cstr(&src, "");
    TEST_ASSERT(&dst == mu_bbuf_rtrim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, ""));
}

void test_mu_bbuf_parse_int(void) {
    mu_bbuf_t s;
    int v_int;
    unsigned int v_uint;
    int8_t v_int8;
    uint8_t v_uint8;
    int16_t v_int16;
    uint16_t v_uint16;
    int32_t v_int32;
    uint32_t v_uint32;
    int64_t v_int64;
    uint64_t v_uint64;

    // with an empty string
    mu_bbuf_init_cstr(&s, "");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with a non-numeric string
    mu_bbuf_init_cstr(&s, "z123");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with "0"
    mu_bbuf_init_cstr(&s, "0");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with "-0"
    mu_bbuf_init_cstr(&s, "-0");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with "1"
    mu_bbuf_init_cstr(&s, "1");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int8);
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint64);

    // with "-1"
    mu_bbuf_init_cstr(&s, "-1");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with 2^8-1
    mu_bbuf_init_cstr(&s, "255");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);    // truncation
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint8);
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int16);
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint64);

    // with 2^16-1
    mu_bbuf_init_cstr(&s, "65535");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_int);
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);      // truncation
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint8);    // truncation
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int16);     // truncation
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint16);
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_int32);
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint64);

    // with 2^32-1
    mu_bbuf_init_cstr(&s, "4294967295");
    v_int = mu_bbuf_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int);       // truncation
    v_uint = mu_bbuf_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_uint);
    v_int8 = mu_bbuf_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);      // truncation
    v_uint8 = mu_bbuf_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint8);    // truncation
    v_int16 = mu_bbuf_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int16);     // truncation
    v_uint16 = mu_bbuf_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint16); // truncation
    v_int32 = mu_bbuf_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int32);     // truncation
    v_uint32 = mu_bbuf_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_uint32);
    v_int64 = mu_bbuf_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_int64);
    v_uint64 = mu_bbuf_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_uint64);

}

void test_mu_bbuf_parse_hex(void) {
    mu_bbuf_t s;
    unsigned int v_hex;

    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, ""));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "-1"));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "-0"));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "1"));
    TEST_ASSERT_EQUAL_INT(1, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "f"));
    TEST_ASSERT_EQUAL_INT(15, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "F"));
    TEST_ASSERT_EQUAL_INT(15, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "aa"));
    TEST_ASSERT_EQUAL_INT(170, v_hex);
    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "AA"));
    TEST_ASSERT_EQUAL_INT(170, v_hex);

    v_hex = mu_bbuf_parse_hex(mu_bbuf_init_cstr(&s, "xyzzy"));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
}

void test_mu_bbuf_get_put_byte(void) {
    mu_bbuf_t b;
    const char *cstr = "abcd";
    uint8_t buf[] = {'w', 'x', 'y', 'z'};
    uint8_t ch;

    mu_bbuf_init_cstr(&b, cstr);
    TEST_ASSERT_TRUE(mu_bbuf_get_byte(&b, 0, &ch));
    TEST_ASSERT_EQUAL_INT('a', ch);
    TEST_ASSERT_TRUE(mu_bbuf_get_byte(&b, 3, &ch));
    TEST_ASSERT_EQUAL_INT('d', ch);
    TEST_ASSERT_FALSE(mu_bbuf_get_byte(&b, 4, &ch));  // out of bounds
    TEST_ASSERT_FALSE(mu_bbuf_get_byte(&b, -1, &ch)); // out of bounds

    mu_bbuf_init_rw(&b, buf, sizeof(buf));
    TEST_ASSERT_TRUE(mu_bbuf_put_byte(&b, 0, 'a'));
    TEST_ASSERT_TRUE(cstr_eq(&b, "axyz"));
    TEST_ASSERT_TRUE(mu_bbuf_put_byte(&b, 3, 'd'));
    TEST_ASSERT_TRUE(cstr_eq(&b, "axyd"));
    TEST_ASSERT_FALSE(mu_bbuf_put_byte(&b, 4, '*'));  // out of bounds
    TEST_ASSERT_FALSE(mu_bbuf_put_byte(&b, -1, '*')); // out of bounds

    TEST_ASSERT_TRUE(cstr_eq(&b, "axyd"));
}

void test_mu_bbuf_clear(void) {
    mu_bbuf_t b;
    uint8_t buf[] = {'w', 'x', 'y', 'z'};

    mu_bbuf_init_rw(&b, buf, sizeof(buf));
    TEST_ASSERT_TRUE(cstr_eq(&b, "wxyz"));
    TEST_ASSERT_EQUAL_PTR(&b, mu_bbuf_clear(&b));
    TEST_ASSERT_TRUE(cstr_eq(&b, "\0\0\0\0"));
}

void test_mu_bbuf_copy_into(void) {
    mu_bbuf_t b1;
    mu_bbuf_t b2;
    const uint8_t buf2[] = {11, 12, 13, 14};

    mu_bbuf_init_ro(&b2, buf2, sizeof(buf2));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(4, mu_bbuf_copy_into(&b1, &b2, 0));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){11, 12, 13, 14}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(3, mu_bbuf_copy_into(&b1, &b2, 1));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 11, 12, 13}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(2, mu_bbuf_copy_into(&b1, &b2, 2));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 11, 12}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(1, mu_bbuf_copy_into(&b1, &b2, 3));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 11}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(0, mu_bbuf_copy_into(&b1, &b2, 4));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(3, mu_bbuf_copy_into(&b1, &b2, -1));
    // for (int i=0; i<4; i++) {
    //     printf("\n[%d] =%d", i, b1.bytes_rw[i]);
    // }
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){12, 13, 14, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(2, mu_bbuf_copy_into(&b1, &b2, -2));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){13, 14, 3, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(1, mu_bbuf_copy_into(&b1, &b2, -3));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){14, 2, 3, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_INT(0, mu_bbuf_copy_into(&b1, &b2, -4));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 4}, 4));
}

void test_mu_bbuf_reverse(void) {
    uint8_t buf1[] = {'r', 'a', 't', 's'};
    mu_bbuf_t b1;
    mu_bbuf_init_rw(&b1, buf1, sizeof(buf1));

    TEST_ASSERT_TRUE(cstr_eq(&b1, "rats"));
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_reverse(&b1));
    TEST_ASSERT_TRUE(cstr_eq(&b1, "star"));
}

void test_mu_bbuf_rrotate(void) {
    mu_bbuf_t b1;

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, 0));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, 1));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){4, 1, 2, 3}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, 2));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){3, 4, 1, 2}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, 3));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){2, 3, 4, 1}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, 4));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, -1));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){2, 3, 4, 1}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, -2));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){3, 4, 1, 2}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, -3));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){4, 1, 2, 3}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rrotate(&b1, -4));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 4}, 4));
}

void test_mu_bbuf_rshift(void) {
    mu_bbuf_t b1;

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, 0));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){1, 2, 3, 4}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, 1));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){0, 1, 2, 3}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, 2));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){0, 0, 1, 2}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, 3));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){0, 0, 0, 1}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, 4));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){0, 0, 0, 0}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, -1));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){2, 3, 4, 0}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, -2));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){3, 4, 0, 0}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, -3));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){4, 0, 0, 0}, 4));

    mu_bbuf_init_rw(&b1, (uint8_t[]){1, 2, 3, 4}, 4);
    TEST_ASSERT_EQUAL_PTR(&b1, mu_bbuf_rshift(&b1, -4));
    TEST_ASSERT_TRUE(buf_eq(&b1, (uint8_t[]){0, 0, 0, 0}, 4));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_bbuf_init);
    RUN_TEST(test_mu_bbuf_copy);
    RUN_TEST(test_mu_buf_ref);
    RUN_TEST(test_mu_bbuf_compare);
    RUN_TEST(test_mu_bbuf_compare_cstr);
    RUN_TEST(test_mu_bbuf_slice);
    RUN_TEST(test_mu_bbuf_bisect);
    RUN_TEST(test_mu_bbuf_matches);
    RUN_TEST(test_mu_bbuf_has_suffix_prefix);
    RUN_TEST(test_mu_bbuf_find_substr);
    RUN_TEST(test_mu_bbuf_find_byte);
    RUN_TEST(test_mu_bbuf_find_predicate);
    RUN_TEST(test_mu_bbuf_trim);
    RUN_TEST(test_mu_bbuf_parse_int);
    RUN_TEST(test_mu_bbuf_parse_hex);
    RUN_TEST(test_mu_bbuf_get_put_byte);
    RUN_TEST(test_mu_bbuf_clear);
    RUN_TEST(test_mu_bbuf_copy_into);
    RUN_TEST(test_mu_bbuf_reverse);
    RUN_TEST(test_mu_bbuf_rrotate);
    RUN_TEST(test_mu_bbuf_rshift);

    return UNITY_END();
}

