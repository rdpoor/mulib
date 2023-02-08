/**
 * @file: mu_str.h
 * 
 * @brief Safe, in-place string operations without the null terminator.
 */

#include "mu_str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

mu_str_t *mu_str_init(mu_str_t *str, uint8_t *bytes, size_t len) {
  str->bytes = bytes;
  str->len = len;
  return str;
}

mu_str_t *mu_str_ro_init(mu_str_t *str, const uint8_t *bytes, size_t len) {
  str->ro_bytes = bytes;
  str->len = len;
  return str;
}

mu_str_t *mu_str_cstr_init(mu_str_t *str, const char *cstr) {
    return mu_str_ro_init(str, (const uint8_t *)cstr, strlen(cstr));
}

uint8_t *mu_str_bytes(mu_str_t *str) { return str->bytes; }

const uint8_t *mu_str_ro_bytes(mu_str_t *str) { return str->ro_bytes; }

size_t mu_str_length(mu_str_t *str) { return str->len; }

bool mu_str_is_empty(mu_str_t *str) { return mu_str_length(str) == 0; }

mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src) {
  return mu_str_init(dst, src->bytes, src->len);
}

mu_str_t *mu_str_fill(mu_str_t *str, uint8_t byte) {
  memset(str->bytes, byte, str->len);
  return str;
}

int mu_str_compare(mu_str_t *s1, mu_str_t *s2) {
  uint8_t *b1 = mu_str_bytes(s1);
  size_t len1 = mu_str_length(s1);
  uint8_t *b2 = mu_str_bytes(s2);
  size_t len2 = mu_str_length(s2);

  size_t len = (len1 < len2) ? len1 : len2;

  for (int i = 0; i < len; i++) {
    int d = b1[i] - b2[i];
    if (d != 0) {
      return d;
    }
  }
  // The first N bytes of s1 and s2 are equal: return value based on lengths.
  return len1 - len2;
}

mu_str_t *mu_str_slice(mu_str_t *dst,
                       mu_str_t *src,
                       ptrdiff_t start,
                       ptrdiff_t end) {
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

size_t mu_str_find(mu_str_t *str, mu_str_t *substr) {
  size_t needle_len = mu_str_length(substr);

  if (needle_len == 0) {
    return 0;
  }

  size_t haystack_len = mu_str_length(str);
  uint8_t *needle = mu_str_bytes(substr);
  int j;

  // First scan through haystack looking for a byte that matches the first byte
  // of needle.  Micro-optimization: We stop searching when we get within 
  // needle_len bytes of the end of haystack, since beyond that, the full-length
  // search will always fail.
  for (int i=0; i<haystack_len-needle_len; i++) {
    uint8_t *haystack = &mu_str_bytes(str)[i];
    if (*haystack == *needle) {
      // first byte matches.  Do the rest of the bytes match?
      for (j=0; j<needle_len; j++) {
        if (haystack[j] != needle[j]) {
          // mismatch: advance to next char in haystack
          break;
        }
      }
      if (j == needle_len) {
        // found: &haystack[i] matched all of *needle
        return i;
      }
    }
    // advance to next byte in haystack
  }
  // got to end of haystack without a match.
  return MU_STR_NOT_FOUND;
}

size_t mu_str_rfind(mu_str_t *str, mu_str_t *substr) {
  size_t needle_len = mu_str_length(substr);

  if (needle_len == 0) {
    return 0;
  }

  size_t haystack_len = mu_str_length(str);
  uint8_t *needle = mu_str_bytes(substr);
  int j;

  // First scan through haystack looking for a byte that matches the first byte
  // of needle.  Micro-optimization: We start searching at haystack_end -  
  // needle_len bytes of the end of haystack, since beyond that, the full-length
  // search will always fail.
  for (int i=haystack_len-needle_len; i>=0; --i) {
    uint8_t *haystack = &mu_str_bytes(str)[i];
    if (*haystack == *needle) {
      // first byte matches.  Do the rest of the bytes match?
      for (j=0; j<needle_len; j++) {
        if (haystack[j] != needle[j]) {
          // mismatch: advance to next char in haystack
          break;
        }
      }
      if (j == needle_len) {
        // found: &haystack[i] matched all of *needle
        return i;
      }
    }
    // advance to next byte in haystack
  }
  // got to end of haystack without a match.
  return MU_STR_NOT_FOUND;
}

mu_str_t *mu_str_ltrim(mu_str_t *str, mu_str_predicate predicate, void *arg) {
  uint8_t *bytes = mu_str_bytes(str);
  size_t idx;

  for (idx=0; idx<mu_str_length(str); idx++) {
    if (!predicate(bytes[idx], arg)) {
      break;
    }
  }
	return mu_str_slice(str, str, idx, MU_STR_END);
}

mu_str_t *mu_str_rtrim(mu_str_t *str, mu_str_predicate predicate, void *arg) {
  uint8_t *bytes = mu_str_bytes(str);
  size_t idx = 0;  // suppress spurious compiler warning?

  for (size_t idx=mu_str_length(str); idx>0; idx--) {
    if (!predicate(bytes[idx-1], arg)) {
      break;
    }
  }
  return mu_str_slice(str, str, 0, idx);
}

mu_str_t *mu_str_trim(mu_str_t *str, mu_str_predicate predicate, void *arg) {
	return mu_str_rtrim(mu_str_ltrim(str, predicate, arg), predicate, arg);
}

// *****************************************************************************
// Standalone tests

// Run this command in to run the standalone tests.
// (gcc -Wall -DTEST_MU_STR -o test_mu_str mu_str.c && ./test_mu_str && rm ./test_mu_str)

#ifdef TEST_MU_STR

#include <stdio.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

// Return true if str->bytes equals cstr
static bool cstr_eq(mu_str_t *str, const char *cstr) {
  return strncmp((const char *)mu_str_bytes(str), cstr, mu_str_length(str)) == 0;
}

static bool is_whitespace(uint8_t byte, void *arg) {
  (void)arg;
	return (byte == ' ') || (byte == '\t') || (byte == '\r') || (byte == '\n') || (byte == '\v') || (byte == '\f');
}

__attribute__((unused))
static void print_str(mu_str_t *str) {
	size_t len = mu_str_length(str);
	printf("\n[%ld]: '%.*s'", len, (int)len, mu_str_bytes(str));
}

int main(void) {

  printf("\nStarting mu_str tests...");

  // mu_str_init
  do {
    mu_str_t s1;
    uint8_t buf[10];

    ASSERT(&s1 == mu_str_init(&s1, buf, sizeof(buf)));
    ASSERT(mu_str_bytes(&s1) == buf);
    ASSERT(mu_str_length(&s1) == sizeof(buf));
  } while (false);

  // mu_str_ro_init
  do {
    mu_str_t s1;
    char *buf = "ABCDEFGHIJ";

    ASSERT(&s1 == mu_str_ro_init(&s1, (const uint8_t *)buf, strlen(buf)));
    ASSERT(mu_str_bytes(&s1) == (const uint8_t *)buf);
    ASSERT(mu_str_length(&s1) == strlen(buf));
  } while (false);

  // mu_str_cstr_init
  do {
    mu_str_t s1;
    char *cstr = "ABCDEFGHIJ";

    ASSERT(&s1 == mu_str_cstr_init(&s1, cstr));
    ASSERT(mu_str_bytes(&s1) == (const uint8_t *)cstr);
    ASSERT(mu_str_length(&s1) == strlen(cstr));
  } while (false);

  // mu_str_copy
  do {
    mu_str_t s1, s2;
    uint8_t buf[10];
    mu_str_init(&s1, buf, sizeof(buf));

    ASSERT(&s2 == mu_str_copy(&s2, &s1));
    ASSERT(mu_str_bytes(&s2) == mu_str_bytes(&s1));
    ASSERT(mu_str_length(&s2) == mu_str_length(&s1));
  } while (false);

  // mu_str_fill
  do {
    mu_str_t s1;
    uint8_t buf[5];

    mu_str_init(&s1, buf, sizeof(buf));
    ASSERT(&s1 == mu_str_fill(&s1, '5'));
    for (int i=0; i<mu_str_length(&s1); i++) {
      ASSERT(mu_str_bytes(&s1)[i] == '5');    
    }
  } while (false);

  // mu_str_compare
  do {
    mu_str_t s1, s2;

    mu_str_cstr_init(&s1, "abcd");
    // strings are equal in content and length
    ASSERT(mu_str_compare(&s1, mu_str_cstr_init(&s2, "abcd")) == 0);
    // s1 is lexographically higher
    ASSERT(mu_str_compare(&s1, mu_str_cstr_init(&s2, "abcc")) > 0);
    // s1 is lexographically lower
    ASSERT(mu_str_compare(&s1, mu_str_cstr_init(&s2, "abce")) < 0);
    // s1 is longer
    ASSERT(mu_str_compare(&s1, mu_str_cstr_init(&s2, "abc")) > 0);
    // s1 is shorter
    ASSERT(mu_str_compare(&s1, mu_str_cstr_init(&s2, "abcde")) < 0);
    // both empty
    ASSERT(mu_str_compare(mu_str_cstr_init(&s1, ""), mu_str_cstr_init(&s2, "")) == 0);
    // s2 empty
    ASSERT(mu_str_compare(mu_str_cstr_init(&s1, "abcd"), mu_str_cstr_init(&s2, "")) > 0);
    // s1 empty
    ASSERT(mu_str_compare(mu_str_cstr_init(&s1, ""), mu_str_cstr_init(&s2, "abcd")) < 0);
  } while (false);

  // mu_str_slice
  do {
    mu_str_t s1, s2;

    mu_str_cstr_init(&s1, "ABCDEFGHIJ");
    // whole slice (indefinite end index)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, MU_STR_END));
    ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));
    // whole slice (definite end index)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, mu_str_length(&s1)));
    ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));

    // remove first char (positive start index)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 1, MU_STR_END));
    ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));
    // remove first char (negative start index)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, -9, MU_STR_END));
    ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));

    // remove last char (positive end index)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, 9));
    ASSERT(cstr_eq(&s2, "ABCDEFGHI"));
    // remove last char (negative end index)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -1));
    ASSERT(cstr_eq(&s2, "ABCDEFGHI"));

    // extract middle chars (positive indeces)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 3, 7));
    ASSERT(cstr_eq(&s2, "DEFG"));
    // extract middle chars (negative indeces)
    ASSERT(&s2 == mu_str_slice(&s2, &s1, -7, -3));
    ASSERT(cstr_eq(&s2, "DEFG"));

    // start == end
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 5, 5));
    ASSERT(cstr_eq(&s2, ""));
    // start > end
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 6, 5));
    ASSERT(cstr_eq(&s2, ""));

    // start > end of string
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 20, mu_str_length(&s1)));
    ASSERT(cstr_eq(&s2, ""));
    // end < beginnig of string
    ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -20));
    ASSERT(cstr_eq(&s2, ""));

  } while (false);

  // mu_str_find, mu_str_rfind
  do {
    mu_str_t s1, s2;

    //                     0123456789
    mu_str_cstr_init(&s1, "abXcdabYcd");
    ASSERT(mu_str_find(&s1, mu_str_cstr_init(&s2, "")) == 0);
    ASSERT(mu_str_find(&s1, mu_str_cstr_init(&s2, "ab")) == 0);
    ASSERT(mu_str_find(&s1, mu_str_cstr_init(&s2, "cd")) == 3);
    ASSERT(mu_str_find(&s1, mu_str_cstr_init(&s2, "cdX")) == MU_STR_NOT_FOUND);

    ASSERT(mu_str_rfind(&s1, mu_str_cstr_init(&s2, "")) == 0);
    ASSERT(mu_str_rfind(&s1, mu_str_cstr_init(&s2, "ab")) == 5);
    ASSERT(mu_str_rfind(&s1, mu_str_cstr_init(&s2, "cd")) == 8);
    ASSERT(mu_str_rfind(&s1, mu_str_cstr_init(&s2, "cdX")) == MU_STR_NOT_FOUND);
  } while (false);

  // mu_str_ltrim, mu_str_rtrim, mu_str_trim
  do {
    mu_str_t s1;

    mu_str_cstr_init(&s1, "  abcde  ");
    ASSERT(&s1 == mu_str_ltrim(&s1, is_whitespace, NULL));
    ASSERT(cstr_eq(&s1, "abcde  "));

    mu_str_cstr_init(&s1, "  abcde  ");
    ASSERT(&s1 == mu_str_rtrim(&s1, is_whitespace, NULL));
    ASSERT(cstr_eq(&s1, "  abcde"));

    mu_str_cstr_init(&s1, "  abcde  ");
    ASSERT(&s1 == mu_str_trim(&s1, is_whitespace, NULL));
    ASSERT(cstr_eq(&s1, "abcde"));
  } while (false);

  printf("\n...tests complete\n");
  return 0;
}

#endif // #ifdef TEST_MU_STR
