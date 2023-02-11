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
Naming thoughts...

bool mu_str_prefix_[cstr_]is() true if prefix exactly matches a string
bool mu_str_suffix_[cstr_]is() true if suffix exactly matches a string
size_t mu_str_[cstr_]find() index of literal string searching forward
size_t mu_str_[cstr_]rfind() index of literal string searching backward
size_t mu_str_match() index of predicate returning true searching forward
size_t mu_str_rmatch() index of predicate returning true searching backward

*/

// *****************************************************************************
// Includes

#include "mu_str.h"

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

static size_t mu_str_find_aux(const uint8_t *haystack,
                              size_t haystack_len,
                              const uint8_t *needle, 
                              size_t needle_len, 
                              bool skip_substr);

static size_t mu_str_rfind_aux(const uint8_t *haystack,
                               size_t haystack_len,
                               const uint8_t *needle, 
                               size_t needle_len, 
                               bool skip_substr);

// *****************************************************************************
// Public code

mu_str_t *mu_str_init(mu_str_t *str, const uint8_t *bytes, size_t len) {
  str->bytes = bytes;
  str->len = len;
  return str;
}

mu_str_t *mu_str_init_cstr(mu_str_t *str, const char *cstr) {
    return mu_str_init(str, (const uint8_t *)cstr, strlen(cstr));
}

const uint8_t *mu_str_bytes(mu_str_t *str) { return str->bytes; }

size_t mu_str_length(mu_str_t *str) { return str->len; }

bool mu_str_is_empty(mu_str_t *str) { return mu_str_length(str) == 0; }

mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src) {
  return mu_str_init(dst, src->bytes, src->len);
}

int mu_str_compare(mu_str_t *s1, mu_str_t *s2) {
  const uint8_t *b1 = mu_str_bytes(s1);
  size_t len1 = mu_str_length(s1);
  const uint8_t *b2 = mu_str_bytes(s2);
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

size_t mu_str_find(mu_str_t *str, mu_str_t *substr, bool skip_substr) {
  return mu_str_find_aux(mu_str_bytes(str),
                         mu_str_length(str), 
                         mu_str_bytes(substr), 
                         mu_str_length(substr), 
                         skip_substr);
}

size_t mu_str_find_cstr(mu_str_t *str, const char *substr, bool skip_substr) {
  return mu_str_find_aux(mu_str_bytes(str),
                         mu_str_length(str), 
                         (const uint8_t *)substr,
                         strlen(substr),
                         skip_substr);
}

size_t mu_str_rfind(mu_str_t *str, mu_str_t *substr, bool skip_substr) {
  return mu_str_rfind_aux(mu_str_bytes(str),
                          mu_str_length(str), 
                          mu_str_bytes(substr), 
                          mu_str_length(substr), 
                          skip_substr);
}

size_t mu_str_rfind_cstr(mu_str_t *str, const char *substr, bool skip_substr) {
  return mu_str_rfind_aux(mu_str_bytes(str),
                          mu_str_length(str), 
                          (const uint8_t *)substr,
                          strlen(substr),
                          skip_substr);
}

mu_str_t *mu_str_ltrim(mu_str_t *str, mu_str_predicate predicate, void *arg) {
  const uint8_t *bytes = mu_str_bytes(str);
  size_t idx;

  for (idx=0; idx<mu_str_length(str); idx++) {
    if (!predicate(bytes[idx], arg)) {
      break;
    }
  }
	return mu_str_slice(str, str, idx, MU_STR_END);
}

mu_str_t *mu_str_rtrim(mu_str_t *str, mu_str_predicate predicate, void *arg) {
  const uint8_t *bytes = mu_str_bytes(str);
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
// Private (static) code

static size_t mu_str_find_aux(const uint8_t *haystack,
                              size_t haystack_len,
                              const uint8_t *needle, 
                              size_t needle_len, 
                              bool skip_substr) {
  if (needle_len == 0) {
    return 0;
  }

  int j;

  // First scan through haystack looking for a byte that matches the first byte
  // of needle.  Micro-optimization: We stop searching when we get within 
  // needle_len bytes of the end of haystack, since beyond that, the full-length
  // search will always fail.
  for (int i=0; i<haystack_len-needle_len; i++) {
    const uint8_t *h2 = &haystack[i];
    if (*h2 == *needle) {
      // first byte matches.  Do the rest of the bytes match?
      for (j=1; j<needle_len; j++) {
        if (h2[j] != needle[j]) {
          // mismatch: advance to next char in haystack
          break;
        }
      }
      if (j == needle_len) {
        // found: &haystack[i] matched all of *needle
        return skip_substr ? i + needle_len : i;
      }
    }
    // advance to next byte in haystack
  }
  // got to end of haystack without a match.
  return MU_STR_NOT_FOUND;
}

static size_t mu_str_rfind_aux(const uint8_t *haystack,
                               size_t haystack_len,
                               const uint8_t *needle, 
                               size_t needle_len, 
                               bool skip_substr) {
  if (needle_len == 0) {
    return 0;
  }

  int j;

  // First scan through haystack looking for a byte that matches the first byte
  // of needle.  Micro-optimization: We start searching at haystack_end -  
  // needle_len bytes of the end of haystack, since beyond that, the full-length
  // search will always fail.
  for (int i=haystack_len-needle_len; i>=0; --i) {
    const uint8_t *h2 = &haystack[i];
    if (*h2 == *needle) {
      // first byte matches.  Do the rest of the bytes match?
      for (j=1; j<needle_len; j++) {
        if (h2[j] != needle[j]) {
          // mismatch: advance to next char in haystack
          break;
        }
      }
      if (j == needle_len) {
        // found: &haystack[i] matched all of *needle
        return skip_substr ? i + needle_len : i;
      }
    }
    // advance to next byte in haystack
  }
  // got to end of haystack without a match.
  return MU_STR_NOT_FOUND;
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

void test_mu_str(void) {

  printf("\nStarting mu_str tests...");

  // mu_str_init
  do {
    mu_str_t s1;
    const uint8_t buf[] = {65, 66, 67, 68, 69, 70, 71, 72, 73, 74};

    ASSERT(&s1 == mu_str_init(&s1, buf, sizeof(buf)));
    ASSERT(mu_str_bytes(&s1) == buf);
    ASSERT(mu_str_length(&s1) == sizeof(buf));
  } while (false);

  // mu_str_init_cstr
  do {
    mu_str_t s1;
    char *cstr = "ABCDEFGHIJ";

    ASSERT(&s1 == mu_str_init_cstr(&s1, cstr));
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

  // mu_str_compare
  do {
    mu_str_t s1, s2;

    mu_str_init_cstr(&s1, "abcd");
    // strings are equal in content and length
    ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcd")) == 0);
    // s1 is lexographically higher
    ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcc")) > 0);
    // s1 is lexographically lower
    ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abce")) < 0);
    // s1 is longer
    ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abc")) > 0);
    // s1 is shorter
    ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcde")) < 0);
    // both empty
    ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""), mu_str_init_cstr(&s2, "")) == 0);
    // s2 empty
    ASSERT(mu_str_compare(mu_str_init_cstr(&s1, "abcd"), mu_str_init_cstr(&s2, "")) > 0);
    // s1 empty
    ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""), mu_str_init_cstr(&s2, "abcd")) < 0);
  } while (false);

  // mu_str_slice
  do {
    mu_str_t s1, s2;

    mu_str_init_cstr(&s1, "ABCDEFGHIJ");
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
    mu_str_init_cstr(&s1, "abXcdabYcd");
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, ""), false) == 0);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, ""), true) == 0);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "ab"), false) == 0);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "ab"), true) == 2);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cd"), false) == 3);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cd"), true) == 5);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cdX"), false) == MU_STR_NOT_FOUND);
    ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cdX"), true) == MU_STR_NOT_FOUND);

    ASSERT(mu_str_find_cstr(&s1, "", false) == 0);
    ASSERT(mu_str_find_cstr(&s1, "", true) == 0);
    ASSERT(mu_str_find_cstr(&s1, "ab", false) == 0);
    ASSERT(mu_str_find_cstr(&s1, "ab", true) == 2);
    ASSERT(mu_str_find_cstr(&s1, "cd", false) == 3);
    ASSERT(mu_str_find_cstr(&s1, "cd", true) == 5);
    ASSERT(mu_str_find_cstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
    ASSERT(mu_str_find_cstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);

    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, ""), false) == 0);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, ""), true) == 0);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "ab"), false) == 5);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "ab"), true) == 7);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cd"), false) == 8);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cd"), true) == 10);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cdX"), false) == MU_STR_NOT_FOUND);
    ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cdX"), true) == MU_STR_NOT_FOUND);

    ASSERT(mu_str_rfind_cstr(&s1, "", false) == 0);
    ASSERT(mu_str_rfind_cstr(&s1, "", true) == 0);
    ASSERT(mu_str_rfind_cstr(&s1, "ab", false) == 5);
    ASSERT(mu_str_rfind_cstr(&s1, "ab", true) == 7);
    ASSERT(mu_str_rfind_cstr(&s1, "cd", false) == 8);
    ASSERT(mu_str_rfind_cstr(&s1, "cd", true) == 10);
    ASSERT(mu_str_rfind_cstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
    ASSERT(mu_str_rfind_cstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);
  } while (false);

  // mu_str_ltrim, mu_str_rtrim, mu_str_trim
  do {
    mu_str_t s1;

    mu_str_init_cstr(&s1, "  abcde  ");
    ASSERT(&s1 == mu_str_ltrim(&s1, is_whitespace, NULL));
    ASSERT(cstr_eq(&s1, "abcde  "));

    mu_str_init_cstr(&s1, "  abcde  ");
    ASSERT(&s1 == mu_str_rtrim(&s1, is_whitespace, NULL));
    ASSERT(cstr_eq(&s1, "  abcde"));

    mu_str_init_cstr(&s1, "  abcde  ");
    ASSERT(&s1 == mu_str_trim(&s1, is_whitespace, NULL));
    ASSERT(cstr_eq(&s1, "abcde"));
  } while (false);

  printf("\n...tests complete\n");
}

void test_mu_str_example(void) {
  // Parse an HTML message, extracting the Date: from the header and the
  // contents of the body.  
  printf("\nStarting mu_str_example...");

  const char *HTML =
    "HTTP/1.1 200 OK\r\n"
    "Date: Wed, 26 Oct 2022 17:17:34 GMT\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 27\r\n"
    "Connection: keep-alive\r\n"
    "X-Javatime: 1666804654506\r\n"
    "\r\n"
    "{\"code\":200,\"message\":\"ok\"}";

  mu_str_t html, date_value, body;
  size_t idx;

  mu_str_init_cstr(&html, HTML);

  // find "Wed, 26 Oct 2022 17:17:34 GMT"
  // Extract the text following "Date: " to end of line
  idx = mu_str_find_cstr(&html, "Date: ", true);
  ASSERT(idx != MU_STR_NOT_FOUND);
  mu_str_slice(&date_value, &html, idx, MU_STR_END);
  idx = mu_str_find_cstr(&date_value, "\r\n", false);
  ASSERT(idx != MU_STR_NOT_FOUND);
  mu_str_slice(&date_value, &date_value, 0, idx);
  ASSERT(cstr_eq(&date_value, "Wed, 26 Oct 2022 17:17:34 GMT"));

  // find "{\"code\":200,\"message\":\"ok\"}"
  // blank \r\n\r\n signifies end of HTML header and start of body
  idx = mu_str_find_cstr(&html, "\r\n\r\n", true);
  ASSERT(idx != MU_STR_NOT_FOUND);
  mu_str_slice(&body, &html, idx, MU_STR_END);
  ASSERT(cstr_eq(&body, "{\"code\":200,\"message\":\"ok\"}"));
  
  printf("\n...mu_str_example complete\n");

}

typedef struct {
    const char *host_name;
    size_t host_name_len;
    uint16_t host_port;
    bool use_tls;
} http_params_t;

http_params_t *parse_http_params(http_params_t *params, const char *url) {
  mu_str_t url_str;
  size_t idx;

  mu_str_init_cstr(&url_str, url);

  if ((idx = mu_str_find_cstr(&url_str, "https://", true)) == 0) {
      // found "https://" at position 0
      params->host_port = 443;  // until proven otherwise...
      params->use_tls = true;
      break;
  } else if ((idx = mu_str_find_cstr(&url_str, "http://", true)) == 0) {
      // found "http://" at position 0
      params->host_port = 80;  // until proven otherwise...
      params->use_tls = false;
      break;
  } else {
    return NULL;
  }

  // skip over "http://" (or "https://")
  mu_str_slice(&url_str, &url_str, idx, MU_STR_END);

  // host_name terminates with ':', '/', or end-of-line

  if ((idx = mu_str_find_cstr(&url_str, ":", false)) != MU_STR_NOT_FOUND) {
    // found a ':' in the url.  candidate host name is everything before ':'
    mu_str_t candidate;
    mu_str_slice(&candidate, &url_str, 0, idx);
    if ((idx2 = mu_str_find_cstr(&candidate, "/", true) != MU_STR_NOT_FOUND)) {
      // found a '/' preceding the ':' -- not allowed.
      // TODO: are there other illegal chars we should look for?
      // TODO: bool mu_str_contains_chars(str, cstring charlist) would be useful...
      // TODO: bool mu_str_contains_char(str, char ch) also...
      return NULL;
    }
    // host name terminates with ':' and looks okay - capture it.
    params->host_name = mu_str_bytes(candidate);
    params->host_name_len = idx;
    idx += 1;                      // skip over ':'

  }


  }
  // success
  return params;
}

void test_parse_url() {
  http_params_t params;

  printf("\nStarting test_parse_url example...");

  ASSERT(parse_http_params(&params, "http://example.com") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 80);
  ASSERT(params->use_tls == false);

  ASSERT(parse_http_params(&params, "https://example.com") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 443);
  ASSERT(params->use_tls == true);

  ASSERT(parse_http_params(&params, "http://example.com:8080") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 8080);
  ASSERT(params->use_tls == false);

  ASSERT(parse_http_params(&params, "https://example.com:8080") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 8080);
  ASSERT(params->use_tls == true);

  ASSERT(parse_http_params(&params, "https://example.com/extra") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 443);
  ASSERT(params->use_tls == true);

  ASSERT(parse_http_params(&params, "https://example.com:123/extra") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 123);
  ASSERT(params->use_tls == true);

  ASSERT(parse_http_params(&params, "https://example.com/") == &params);
  ASSERT(strcmp(params->host_name, "example.com") == 0);
  ASSERT(params->host_port == 443);
  ASSERT(params->use_tls == true);

  // pathologies
  ASSERT(parse_http_params(&params, "") == NULL);
  ASSERT(parse_http_params(&params, "http") == NULL);
  ASSERT(parse_http_params(&params, "http:") == NULL);
  ASSERT(parse_http_params(&params, "http:/") == NULL);
  ASSERT(parse_http_params(&params, "http://") == NULL);
  ASSERT(parse_http_params(&params, "http://:") == NULL);
  ASSERT(parse_http_params(&params, "http://:123") == NULL);
  ASSERT(parse_http_params(&params, "http://example.com:") == NULL);
  ASSERT(parse_http_params(&params, "http://example.com:abc") == NULL);
  ASSERT(parse_http_params(&params, "http://example.com/extra:123") == NULL);
  ASSERT(parse_http_params(&params, "ftp://example.com:123") == NULL);

  printf("\n...test_parse_url example complete\n");
}

int main(void) {
  test_mu_str();
  test_mu_str_example();
  // test_parse_url();
}

#endif // #ifdef TEST_MU_STR
