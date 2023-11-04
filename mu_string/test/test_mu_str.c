#include "fff.h"
#include "mu_str.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

// helper functions

__attribute__((unused)) static void print_str(mu_str_t *str) {
    int len = mu_str_length(str);
    printf("\n[%d]: '%.*s'", len, (int)len, mu_str_bytes(str));
}

__attribute__((unused)) static int local_strncmp(const char *s1, const char *s2,
                                                 register int n) {
    unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

// Return true if str->bytes equals cstr
__attribute__((unused)) static bool cstr_eq(mu_str_t *str, const char *cstr) {
    return local_strncmp((const char *)mu_str_bytes(str), cstr,
                         mu_str_length(str)) == 0;
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

__attribute__((unused)) static bool is_hexadecimal(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, "0123456789abcdefABCDEF");
}

__attribute__((unused)) static bool is_whitespace(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, " \t\r\n\f\v");
}

__attribute__((unused)) static bool is_x(uint8_t byte, void *arg) {
    (void)arg; // unused
    return byte == 'x';
}

__attribute__((unused)) static bool is_never(uint8_t byte, void *arg) {
    (void)arg; // unused
    return false;
}

__attribute__((unused)) static bool is_always(uint8_t byte, void *arg) {
    (void)arg; // unused
    return true;
}

void setUp(void) {}

void tearDown(void) {
    // nothing yet
}

void test_mu_str_init(void) {
    const uint8_t buf[10];
    mu_str_t s1;
    TEST_ASSERT_EQUAL_PTR(&s1, mu_str_init(&s1, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_PTR(buf, mu_str_bytes(&s1));
    TEST_ASSERT_EQUAL_INT(sizeof(buf), mu_str_length(&s1));
    TEST_ASSERT_FALSE(mu_str_is_empty(&s1));

    const char *cstr = "asdf";
    mu_str_t s2;
    TEST_ASSERT_EQUAL_PTR(&s2, mu_str_init_cstr(&s2, cstr));
    TEST_ASSERT_EQUAL_PTR(cstr, mu_str_bytes(&s2));
    TEST_ASSERT_EQUAL_INT(strlen(cstr), mu_str_length(&s2));
    TEST_ASSERT_FALSE(mu_str_is_empty(&s2));

    const uint8_t mt[0];
    mu_str_t s3;
    TEST_ASSERT_EQUAL_PTR(&s3, mu_str_init(&s3, mt, 0));
    TEST_ASSERT_EQUAL_PTR(mt, mu_str_bytes(&s3));
    TEST_ASSERT_EQUAL_INT(sizeof(mt), mu_str_length(&s3));
    TEST_ASSERT_TRUE(mu_str_is_empty(&s3));
}

void test_mu_str_copy(void) {
    mu_str_t s1, s2;
    uint8_t buf[10];
    mu_str_init(&s1, buf, sizeof(buf));

    TEST_ASSERT_EQUAL_PTR(&s2, mu_str_copy(&s2, &s1));
    TEST_ASSERT_EQUAL_PTR(mu_str_bytes(&s2), mu_str_bytes(&s1));
    TEST_ASSERT_EQUAL_INT(mu_str_length(&s2), mu_str_length(&s1));
}

void test_mu_str_compare(void) {
    mu_str_t s1, s2;

    mu_str_init_cstr(&s1, "abcd");
    // strings are equal in content and length
    TEST_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcd")) == 0);
    // s1 is lexographically higher
    TEST_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcc")) > 0);
    // s1 is lexographically lower
    TEST_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abce")) < 0);
    // s1 is longer
    TEST_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abc")) > 0);
    // s1 is shorter
    TEST_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcde")) < 0);
    // both empty
    TEST_ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""),
                               mu_str_init_cstr(&s2, "")) == 0);
    // s2 empty
    TEST_ASSERT(mu_str_compare(mu_str_init_cstr(&s1, "abcd"),
                               mu_str_init_cstr(&s2, "")) > 0);
    // s1 empty
    TEST_ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""),
                               mu_str_init_cstr(&s2, "abcd")) < 0);
}

void test_mu_str_slice(void) {
    mu_str_t s1, s2;

    mu_str_init_cstr(&s1, "ABCDEFGHIJ");
    // whole slice (indefinite end index)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, MU_STR_END));
    TEST_ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));
    // whole slice (definite end index)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, mu_str_length(&s1)));
    TEST_ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));

    // remove first char (positive start index)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 1, MU_STR_END));
    TEST_ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));
    // remove first char (negative start index)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, -9, MU_STR_END));
    TEST_ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));

    // remove last char (positive end index)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, 9));
    TEST_ASSERT(cstr_eq(&s2, "ABCDEFGHI"));
    // remove last char (negative end index)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -1));
    TEST_ASSERT(cstr_eq(&s2, "ABCDEFGHI"));

    // extract middle chars (positive indeces)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 3, 7));
    TEST_ASSERT(cstr_eq(&s2, "DEFG"));
    // extract middle chars (negative indeces)
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, -7, -3));
    TEST_ASSERT(cstr_eq(&s2, "DEFG"));

    // start == end
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 5, 5));
    TEST_ASSERT(cstr_eq(&s2, ""));
    // start > end
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 6, 5));
    TEST_ASSERT(cstr_eq(&s2, ""));

    // start > end of string
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 20, mu_str_length(&s1)));
    TEST_ASSERT(cstr_eq(&s2, ""));
    // end < beginnig of string
    TEST_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -20));
    TEST_ASSERT(cstr_eq(&s2, ""));
}

void test_mu_str_bisect(void) {
    mu_str_t src, left, right;

    mu_str_init_cstr(&src, "ABCDEFGHIJ");
    TEST_ASSERT(mu_str_bisect(&left, &right, &src, 0) == &src);
    TEST_ASSERT(cstr_eq(&left, ""));
    TEST_ASSERT(cstr_eq(&right, "ABCDEFGHIJ"));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, 1) == &src);
    TEST_ASSERT(cstr_eq(&left, "A"));
    TEST_ASSERT(cstr_eq(&right, "BCDEFGHIJ"));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, 9) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHI"));
    TEST_ASSERT(cstr_eq(&right, "J"));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, 10) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHIJ"));
    TEST_ASSERT(cstr_eq(&right, ""));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, MU_STR_END) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHIJ"));
    TEST_ASSERT(cstr_eq(&right, ""));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, -1) == &src);
    TEST_ASSERT(cstr_eq(&left, "ABCDEFGHI"));
    TEST_ASSERT(cstr_eq(&right, "J"));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, -9) == &src);
    TEST_ASSERT(cstr_eq(&left, "A"));
    TEST_ASSERT(cstr_eq(&right, "BCDEFGHIJ"));

    TEST_ASSERT(mu_str_bisect(&left, &right, &src, -10) == &src);
    TEST_ASSERT(cstr_eq(&left, ""));
    TEST_ASSERT(cstr_eq(&right, "ABCDEFGHIJ"));

    // left may equal src
    mu_str_init_cstr(&src, "ABCDEFGHIJ");
    TEST_ASSERT(mu_str_bisect(&src, &right, &src, 1) == &src);
    TEST_ASSERT(cstr_eq(&src, "A"));
    TEST_ASSERT(cstr_eq(&right, "BCDEFGHIJ"));

    // right may equal src
    mu_str_init_cstr(&src, "ABCDEFGHIJ");
    TEST_ASSERT(mu_str_bisect(&left, &src, &src, 1) == &src);
    TEST_ASSERT(cstr_eq(&left, "A"));
    TEST_ASSERT(cstr_eq(&src, "BCDEFGHIJ"));
}

void test_mu_str_has_suffix_prefix(void) {
    mu_str_t s1, s2;
    mu_str_init_cstr(&s1, "abcd");

    TEST_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "")) == true);
    TEST_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "ab")) == true);
    TEST_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "cd")) == false);
    TEST_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "abcd")) == true);
    TEST_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "abcde")) ==
                false);

    TEST_ASSERT(mu_str_has_prefix_cstr(&s1, "") == true);
    TEST_ASSERT(mu_str_has_prefix_cstr(&s1, "ab") == true);
    TEST_ASSERT(mu_str_has_prefix_cstr(&s1, "cd") == false);
    TEST_ASSERT(mu_str_has_prefix_cstr(&s1, "abcd") == true);
    TEST_ASSERT(mu_str_has_prefix_cstr(&s1, "abcde") == false);

    TEST_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "")) == true);
    TEST_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "ab")) == false);
    TEST_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "cd")) == true);
    TEST_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "abcd")) == true);
    TEST_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "abcde")) ==
                false);

    TEST_ASSERT(mu_str_has_suffix_cstr(&s1, "") == true);
    TEST_ASSERT(mu_str_has_suffix_cstr(&s1, "ab") == false);
    TEST_ASSERT(mu_str_has_suffix_cstr(&s1, "cd") == true);
    TEST_ASSERT(mu_str_has_suffix_cstr(&s1, "abcd") == true);
    TEST_ASSERT(mu_str_has_suffix_cstr(&s1, "abcde") == false);
}

void test_mu_str_find_substr(void) {
    mu_str_t s1, s2;

    //                     0123456789
    mu_str_init_cstr(&s1, "abXcdabYcd");
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, ""), false) == 0);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, ""), true) == 0);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, "ab"), false) ==
                0);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, "ab"), true) ==
                2);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, "cd"), false) ==
                3);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, "cd"), true) ==
                5);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, "cdX"), false) ==
                MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_find_substr(&s1, mu_str_init_cstr(&s2, "cdX"), true) ==
                MU_STR_NOT_FOUND);

    TEST_ASSERT(mu_str_find_subcstr(&s1, "", false) == 0);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "", true) == 0);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "ab", false) == 0);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "ab", true) == 2);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "cd", false) == 3);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "cd", true) == 5);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_find_subcstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);

    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, ""), false) ==
                10);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, ""), true) ==
                10);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, "ab"), false) ==
                5);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, "ab"), true) ==
                7);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, "cd"), false) ==
                8);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, "cd"), true) ==
                10);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, "cdX"), false) ==
                MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_rfind_substr(&s1, mu_str_init_cstr(&s2, "cdX"), true) ==
                MU_STR_NOT_FOUND);

    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "", false) == 10);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "", true) == 10);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "ab", false) == 5);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "ab", true) == 7);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "cd", false) == 8);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "cd", true) == 10);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_rfind_subcstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);
}

void test_mu_str_index(void) {
    mu_str_t s1;

    mu_str_init_cstr(&s1, "0123");
    // finds the very first char '0'
    TEST_ASSERT(mu_str_index(&s1, is_numeric, NULL, true) == 0); // found '0'
    // hits the end of string without a "not match"
    TEST_ASSERT(mu_str_index(&s1, is_numeric, NULL, false) == MU_STR_NOT_FOUND);
    // finds the very last char '3'
    TEST_ASSERT(mu_str_rindex(&s1, is_numeric, NULL, true) == 3); // found '3'
    // hits the begining of string without a "not match"
    TEST_ASSERT(mu_str_rindex(&s1, is_numeric, NULL, false) == MU_STR_NOT_FOUND);

    //                     00000000001111111 1 1 1 2 2
    //                     01234567890123456 7 8 9 0 1
    mu_str_init_cstr(&s1, "0123456789abcDEF \r\n\t\v\f");

    TEST_ASSERT(mu_str_index(&s1, is_numeric, NULL, true) == 0);
    TEST_ASSERT(mu_str_index(&s1, is_numeric, NULL, false) == 10);
    TEST_ASSERT(mu_str_index(&s1, is_hexadecimal, NULL, true) == 0);
    TEST_ASSERT(mu_str_index(&s1, is_hexadecimal, NULL, false) == 16);
    TEST_ASSERT(mu_str_index(&s1, is_whitespace, NULL, true) == 16);
    TEST_ASSERT(mu_str_index(&s1, is_whitespace, NULL, false) == 0);
    TEST_ASSERT(mu_str_index(&s1, is_x, NULL, true) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_index(&s1, is_x, NULL, false) == 0);

    TEST_ASSERT(mu_str_rindex(&s1, is_numeric, NULL, true) == 9);
    TEST_ASSERT(mu_str_rindex(&s1, is_numeric, NULL, false) == 21);
    TEST_ASSERT(mu_str_rindex(&s1, is_hexadecimal, NULL, true) == 15);
    TEST_ASSERT(mu_str_rindex(&s1, is_hexadecimal, NULL, false) == 21);
    TEST_ASSERT(mu_str_rindex(&s1, is_whitespace, NULL, true) == 21);
    TEST_ASSERT(mu_str_rindex(&s1, is_whitespace, NULL, false) == 15);
    TEST_ASSERT(mu_str_rindex(&s1, is_x, NULL, true) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_rindex(&s1, is_x, NULL, false) == 21);

    mu_str_init_cstr(&s1, "");
    TEST_ASSERT(mu_str_index(&s1, is_never, NULL, true) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_rindex(&s1, is_never, NULL, false) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_index(&s1, is_always, NULL, true) == MU_STR_NOT_FOUND);
    TEST_ASSERT(mu_str_rindex(&s1, is_always, NULL, false) == MU_STR_NOT_FOUND);
}

void test_mu_str_trim(void) {
    mu_str_t src;
    mu_str_t dst;

    mu_str_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&dst == mu_str_ltrim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, "abcde  "));

    mu_str_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&dst == mu_str_rtrim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, "  abcde"));

    mu_str_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&dst == mu_str_trim(&dst, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&dst, "abcde"));

    // src may equal dst
    mu_str_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&src == mu_str_ltrim(&src, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&src, "abcde  "));

    mu_str_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&src == mu_str_rtrim(&src, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&src, "  abcde"));

    mu_str_init_cstr(&src, "  abcde  ");
    TEST_ASSERT(&src == mu_str_trim(&src, &src, is_whitespace, NULL));
    TEST_ASSERT(cstr_eq(&src, "abcde"));
}

void test_mu_str_to_cstr(void) {
    mu_str_t s1;
    char buf[5]; // 4 chars max (plus null termination)

    mu_str_init_cstr(&s1, "abcd");
    TEST_ASSERT(mu_str_to_cstr(&s1, buf, sizeof(buf)) == true);
    TEST_ASSERT(buf[0] == 'a'); // strcmp avoidance...
    TEST_ASSERT(buf[1] == 'b');
    TEST_ASSERT(buf[2] == 'c');
    TEST_ASSERT(buf[3] == 'd');

    // s1 not large enough to hold "abcde"
    mu_str_init_cstr(&s1, "abcde");
    TEST_ASSERT(mu_str_to_cstr(&s1, buf, sizeof(buf)) == false);
}

void test_mu_str_parse_int(void) {
    mu_str_t s;
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
    mu_str_init_cstr(&s, "");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with a non-numeric string
    mu_str_init_cstr(&s, "z123");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with "0"
    mu_str_init_cstr(&s, "0");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with "-0"
    mu_str_init_cstr(&s, "-0");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int8);
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with "1"
    mu_str_init_cstr(&s, "1");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int8);
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(1, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(1, v_uint64);

    // with "-1"
    mu_str_init_cstr(&s, "-1");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(0, v_uint64);

    // with 2^8-1
    mu_str_init_cstr(&s, "255");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);    // truncation
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint8);
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int16);
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(255, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint64);

    // with 2^16-1
    mu_str_init_cstr(&s, "65535");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_int);
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);      // truncation
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint8);    // truncation
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int16);     // truncation
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint16);
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_int32);
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint64);

    // with 2^32-1
    mu_str_init_cstr(&s, "4294967295");
    v_int = mu_str_parse_int(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int);       // truncation
    v_uint = mu_str_parse_unsigned_int(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_uint);
    v_int8 = mu_str_parse_int8(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int8);      // truncation
    v_uint8 = mu_str_parse_uint8(&s);
    TEST_ASSERT_EQUAL_INT(255, v_uint8);    // truncation
    v_int16 = mu_str_parse_int16(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int16);     // truncation
    v_uint16 = mu_str_parse_uint16(&s);
    TEST_ASSERT_EQUAL_INT(65535, v_uint16); // truncation
    v_int32 = mu_str_parse_int32(&s);
    TEST_ASSERT_EQUAL_INT(-1, v_int32);     // truncation
    v_uint32 = mu_str_parse_uint32(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_uint32);
    v_int64 = mu_str_parse_int64(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_int64);
    v_uint64 = mu_str_parse_uint64(&s);
    TEST_ASSERT_EQUAL_INT(4294967295, v_uint64);

}

void test_mu_str_parse_hex(void) {
    mu_str_t s;
    unsigned int v_hex;

    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, ""));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "-1"));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "-0"));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "1"));
    TEST_ASSERT_EQUAL_INT(1, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "f"));
    TEST_ASSERT_EQUAL_INT(15, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "F"));
    TEST_ASSERT_EQUAL_INT(15, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "aa"));
    TEST_ASSERT_EQUAL_INT(170, v_hex);
    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "AA"));
    TEST_ASSERT_EQUAL_INT(170, v_hex);

    v_hex = mu_str_parse_hex(mu_str_init_cstr(&s, "xyzzy"));
    TEST_ASSERT_EQUAL_INT(0, v_hex);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_str_init);
    RUN_TEST(test_mu_str_copy);
    RUN_TEST(test_mu_str_compare);
    RUN_TEST(test_mu_str_slice);
    RUN_TEST(test_mu_str_bisect);
    RUN_TEST(test_mu_str_has_suffix_prefix);
    RUN_TEST(test_mu_str_find_substr);
    RUN_TEST(test_mu_str_index);
    RUN_TEST(test_mu_str_trim);
    RUN_TEST(test_mu_str_to_cstr);
    RUN_TEST(test_mu_str_parse_int);
    RUN_TEST(test_mu_str_parse_hex);

    return UNITY_END();
}

