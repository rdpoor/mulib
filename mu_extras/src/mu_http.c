/**
 * MIT License
 *
 * Copyright (c) 2020-2023 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_http.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Local (private, static) forward declarations

static void write_header_aux(mu_http_header_t *header, uint8_t *body,
                             size_t body_len, mu_http_writer_fn writer,
                             void *arg);

/**
 * @brief Return a reference to the idx'th header line of a header.
 */
static mu_http_header_line_t *header_line_ref(mu_http_header_t *hdr,
                                              size_t idx);

static void write_start_line(mu_http_rqst_t *rqst, mu_http_writer_fn writer,
                             void *arg);

static void write_status_line(mu_http_resp_t *resp, mu_http_writer_fn writer,
                              void *arg);

static void write_header_lines(mu_http_header_t *hdr, mu_http_writer_fn writer,
                               void *arg);

static void write_header_line(mu_http_header_line_t *line,
                              mu_http_writer_fn writer, void *arg);

static void write_string(const char *string, mu_http_writer_fn writer,
                         void *arg);

static void write_eol(mu_http_writer_fn writer, void *arg);

static void write_bytes(const uint8_t *bytes, size_t len,
                        mu_http_writer_fn writer, void *arg);

static const char *status_to_string(uint16_t status);

// *****************************************************************************
// Public code

mu_http_rqst_t *mu_http_rqst_init(mu_http_rqst_t *rqst, const char *method,
                                  const char *uri, const char *version) {
    rqst->method = method;
    rqst->uri = uri;
    rqst->version = version;
    return rqst;
}

mu_http_resp_t *mu_http_resp_init(mu_http_resp_t *resp, const char *protocol,
                                  uint16_t status) {
    resp->protocol = protocol;
    resp->status = status;
    return resp;
}

mu_http_header_t *mu_http_header_init(mu_http_header_t *header,
                                      mu_http_header_line_t *lines,
                                      size_t max_lines) {
    header->header_lines = lines;
    header->capacity = max_lines;
    header->count = 0;
    return header;
}

mu_http_header_t *mu_http_add_header_cstr(mu_http_header_t *header,
                                          const char *name, const char *value) {
    return mu_http_add_header_buf(header, name, (const uint8_t *)value,
                                  strlen(value));
}

mu_http_header_t *mu_http_add_header_buf(mu_http_header_t *header,
                                          const char *name,
                                          const uint8_t *value,
                                          size_t value_length) {
    mu_http_header_line_t *line = NULL;

    if (header->count < header->capacity) {
        line = &header->header_lines[header->count++];
    } else {
        return NULL;
    }

    line->name = name;
    line->value = value;
    line->value_length = value_length;
    return header;
}

size_t mu_http_header_line_count(mu_http_header_t *header) {
    return header->count;
}

void mu_http_write_rqst(mu_http_rqst_t *rqst, mu_http_header_t *header,
                        uint8_t *body, size_t body_len,
                        mu_http_writer_fn writer, void *arg) {
    write_start_line(rqst, writer, arg);
    write_header_aux(header, body, body_len, writer, arg);
}

void mu_http_write_resp(mu_http_resp_t *resp, mu_http_header_t *header,
                        uint8_t *body, size_t body_len,
                        mu_http_writer_fn writer, void *arg) {
    write_status_line(resp, writer, arg);
    write_header_aux(header, body, body_len, writer, arg);
}

// *****************************************************************************
// Local (private, static) code

static void write_header_aux(mu_http_header_t *header, uint8_t *body,
                             size_t body_len, mu_http_writer_fn writer,
                             void *arg) {
    if (header) {
        write_header_lines(header, writer, arg);
    }
    write_eol(writer, arg);
    if (body) {
        write_bytes(body, body_len, writer, arg);
    }
}

static mu_http_header_line_t *header_line_ref(mu_http_header_t *hdr,
                                              size_t idx) {
    if (idx < mu_http_header_line_count(hdr)) {
        return &hdr->header_lines[idx];
    } else {
        return NULL;
    }
}

static void write_start_line(mu_http_rqst_t *rqst, mu_http_writer_fn writer,
                             void *arg) {
    write_string(rqst->method, writer, arg);
    write_string(" ", writer, arg);
    write_string(rqst->uri, writer, arg);
    write_string(" ", writer, arg);
    write_string(rqst->version, writer, arg);
    write_eol(writer, arg);
}

static void write_status_line(mu_http_resp_t *resp, mu_http_writer_fn writer,
                              void *arg) {
    char buf[20];
    write_string(resp->protocol, writer, arg);
    write_string(" ", writer, arg);
    snprintf(buf, sizeof(buf), "%d %s", resp->status,
             status_to_string(resp->status));
    write_string(buf, writer, arg);
    write_eol(writer, arg);
}

static void write_header_lines(mu_http_header_t *hdr, mu_http_writer_fn writer,
                               void *arg) {
    for (size_t i = 0; i < mu_http_header_line_count(hdr); i++) {
        write_header_line(header_line_ref(hdr, i), writer, arg);
        write_eol(writer, arg);
    }
}

static void write_header_line(mu_http_header_line_t *line,
                              mu_http_writer_fn writer, void *arg) {
    write_string(line->name, writer, arg);
    write_string(": ", writer, arg);
    write_bytes(line->value, line->value_length, writer, arg);
}

static void write_string(const char *string, mu_http_writer_fn writer,
                         void *arg) {
    while (*string) {
        writer(*string++, arg);
    }
}

static void write_eol(mu_http_writer_fn writer, void *arg) {
    write_string("\r\n", writer, arg);
}

static void write_bytes(const uint8_t *bytes, size_t len,
                        mu_http_writer_fn writer, void *arg) {
    for (size_t i = 0; i < len; i++) {
        writer(*bytes++, arg);
    }
}

static const char *status_to_string(uint16_t status) {
    switch (status) {
    /*####### 1xx - Informational #######*/
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 102:
        return "Processing";
    case 103:
        return "Early Hints";

    /*####### 2xx - Successful #######*/
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi-Status";
    case 208:
        return "Already Reported";
    case 226:
        return "IM Used";

    /*####### 3xx - Redirection #######*/
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";

    /*####### 4xx - Client Error #######*/
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Content Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 418:
        return "I'm a teapot";
    case 421:
        return "Misdirected Request";
    case 422:
        return "Unprocessable Content";
    case 423:
        return "Locked";
    case 424:
        return "Failed Dependency";
    case 425:
        return "Too Early";
    case 426:
        return "Upgrade Required";
    case 428:
        return "Precondition Required";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 451:
        return "Unavailable For Legal Reasons";

    /*####### 5xx - Server Error #######*/
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    case 506:
        return "Variant Also Negotiates";
    case 507:
        return "Insufficient Storage";
    case 508:
        return "Loop Detected";
    case 510:
        return "Not Extended";
    case 511:
        return "Network Authentication Required";

    default:
        return NULL;
    }
}

// *****************************************************************************
// *****************************************************************************
// Standalone tests
// *****************************************************************************
// *****************************************************************************

/**
Run this command in to run the standalone tests.

gcc -Wall -DTEST_MU_HTTP -I../../../mulib -o test_mu_http mu_http.c \
&& ./test_mu_http && rm ./test_mu_http

 */

#ifdef TEST_MU_HTTP

#include <stdio.h>
#include <string.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

#define MAX_HDR_LINES 5

static mu_http_rqst_t s_rqst;
static mu_http_resp_t s_resp;
static mu_http_header_line_t s_hdr_lines[MAX_HDR_LINES];
static mu_http_header_t s_hdr;

static int s_bytes_written = 0;
static char s_buf[1000];

static void writer_fn(char ch, void *arg) {
    (void)arg;
    s_buf[s_bytes_written++] = ch;
}

static void writer_reset(void) {
    s_bytes_written = 0;
    memset(s_buf, 0, sizeof(s_buf));
}

static bool check_buf(const char *expected) {
    return strcmp(s_buf, expected) == 0;
}

static void test_mu_http(void) {
    printf("\nStarting test_mu_http...");

    writer_reset();
    ASSERT(&s_rqst == mu_http_rqst_init(&s_rqst, "GET", "/", "HTTP/1.1"));
    mu_http_write_rqst(&s_rqst, NULL, NULL, 0, writer_fn, NULL);
    ASSERT(check_buf("GET / HTTP/1.1\r\n\r\n"));

    writer_reset();
    ASSERT(&s_rqst == mu_http_rqst_init(&s_rqst, "GET", "/", "HTTP/1.1"));
    ASSERT(&s_hdr == mu_http_header_init(&s_hdr, s_hdr_lines, MAX_HDR_LINES));
    ASSERT(mu_http_header_line_count(&s_hdr) == 0);
    ASSERT(&s_hdr ==
           mu_http_add_header_cstr(&s_hdr, "Host", "localhost:8000"));
    ASSERT(&s_hdr ==
           mu_http_add_header_cstr(&s_hdr, "User-Agent", "Mozilla/5.0"));
    ASSERT(mu_http_header_line_count(&s_hdr) == 2);
    mu_http_write_rqst(&s_rqst, &s_hdr, NULL, 0, writer_fn, NULL);
    ASSERT(check_buf("GET / HTTP/1.1\r\n"
                     "Host: localhost:8000\r\n"
                     "User-Agent: Mozilla/5.0\r\n"
                     "\r\n"));

    writer_reset();
    ASSERT(&s_rqst == mu_http_rqst_init(&s_rqst, "GET", "/", "HTTP/1.1"));
    ASSERT(&s_hdr == mu_http_header_init(&s_hdr, s_hdr_lines, MAX_HDR_LINES));
    ASSERT(mu_http_header_line_count(&s_hdr) == 0);
    ASSERT(&s_hdr ==
           mu_http_add_header_cstr(&s_hdr, "Host", "localhost:8080"));
    ASSERT(&s_hdr ==
           mu_http_add_header_cstr(&s_hdr, "User-Agent", "Mozilla/6.0"));
    ASSERT(mu_http_header_line_count(&s_hdr) == 2);
    const char *body = "abcde";

    mu_http_write_rqst(&s_rqst, &s_hdr, (uint8_t *)body, strlen(body),
                       writer_fn, NULL);
    ASSERT(check_buf("GET / HTTP/1.1\r\n"
                     "Host: localhost:8080\r\n"
                     "User-Agent: Mozilla/6.0\r\n"
                     "\r\n"
                     "abcde"));

    writer_reset();
    ASSERT(&s_resp == mu_http_resp_init(&s_resp, "HTTP/1.1", 200));
    mu_http_write_resp(&s_resp, NULL, NULL, 0, writer_fn, NULL);
    ASSERT(check_buf("HTTP/1.1 200 OK\r\n\r\n"));

    writer_reset();
    ASSERT(&s_resp == mu_http_resp_init(&s_resp, "HTTP/1.1", 404));
    ASSERT(&s_hdr == mu_http_header_init(&s_hdr, s_hdr_lines, MAX_HDR_LINES));
    ASSERT(mu_http_header_line_count(&s_hdr) == 0);
    ASSERT(&s_hdr ==
           mu_http_add_header_cstr(&s_hdr, "Host", "localhost:8000"));
    ASSERT(&s_hdr ==
           mu_http_add_header_cstr(&s_hdr, "User-Agent", "Mozilla/5.0"));
    ASSERT(mu_http_header_line_count(&s_hdr) == 2);
    mu_http_write_resp(&s_resp, &s_hdr, NULL, 0, writer_fn, NULL);
    ASSERT(check_buf("HTTP/1.1 404 Not Found\r\n"
                     "Host: localhost:8000\r\n"
                     "User-Agent: Mozilla/5.0\r\n"
                     "\r\n"));

    printf("\n...test_mu_http complete\n");
}

int main(void) { test_mu_http(); }

#endif // #ifdef TEST_MU_HTTP

#if 0

void test_mu_str_example(void) {
    // Parse an HTML message, extracting the Date: from the header and the
    // contents of the body.
    printf("\nStarting mu_str_example...");
    fflush(stdout);

    const char *HTML = "HTTP/1.1 200 OK\r\n"
                       "Date: Wed, 26 Oct 2022 17:17:34 GMT\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: 27\r\n"
                       "Connection: keep-alive\r\n"
                       "X-Javatime: 1666804654506\r\n"
                       "\r\n"
                       "{\"code\":200,\"message\":\"ok\"}";

    mu_str_t html, date_value, body;
    int idx;

    mu_str_init_cstr(&html, HTML);

    // find "Wed, 26 Oct 2022 17:17:34 GMT"
    // Extract the text following "Date: " to end of line
    idx = mu_str_find_subcstr(&html, "Date: ", true);
    TEST_ASSERT(idx != MU_STR_NOT_FOUND);
    mu_str_slice(&date_value, &html, idx, MU_STR_END);
    idx = mu_str_find_subcstr(&date_value, "\r\n", false);
    TEST_ASSERT(idx != MU_STR_NOT_FOUND);
    mu_str_slice(&date_value, &date_value, 0, idx);
    TEST_ASSERT(cstr_eq(&date_value, "Wed, 26 Oct 2022 17:17:34 GMT"));

    // find "{\"code\":200,\"message\":\"ok\"}"
    // blank \r\n\r\n signifies end of HTML header and start of body
    idx = mu_str_find_subcstr(&html, "\r\n\r\n", true);
    TEST_ASSERT(idx != MU_STR_NOT_FOUND);
    mu_str_slice(&body, &html, idx, MU_STR_END);
    TEST_ASSERT(cstr_eq(&body, "{\"code\":200,\"message\":\"ok\"}"));

    printf("\n...mu_str_example complete\n");
}

typedef struct {
    const char *host_name;
    int host_name_len;
    uint16_t host_port;
    bool use_tls;
} http_params_t;

#define HTTP_PREFIX "http://"
#define HTTPS_PREFIX "https://"

static bool is_hostname(uint8_t byte, void *arg) {
    if ((byte >= 'a') && (byte <= 'z')) {
        return true;
    } else if ((byte >= '0') && (byte <= '9')) {
        return true;
    } else if (byte == '.') {
        return true;
    } else if (byte == '-') {
        return true;
    } else {
        return false;
    }
}

static bool is_decimal(uint8_t byte, void *arg) {
    if ((byte >= '0') && (byte <= '9')) {
        return true;
    }
    return false;
}

static uint16_t parse_uint16(const uint8_t *buf, int len) {
    uint16_t v = 0;

    while (len-- > 0) {
        v = (v * 10) + (*buf++ - '0');
    }
    return v;
}

http_params_t *parse_http_params(http_params_t *params, const char *url) {
    mu_str_t s1;
    int idx;

    mu_str_init_cstr(&s1, url);
    // printf("\n==== parsing '%s'", url);

    if (mu_str_has_prefix_cstr(&s1, HTTP_PREFIX)) {
        // starts with http://
        // printf("\nStarts with http://");
        params->use_tls = false;
        params->host_port = 80; // unless over-ridden by a :<port> spec...
        mu_str_slice(&s1, &s1, strlen(HTTP_PREFIX), MU_STR_END);

    } else if (mu_str_has_prefix_cstr(&s1, HTTPS_PREFIX)) {
        // starts with https://
        // printf("\nStarts with https://");
        params->use_tls = true;
        params->host_port = 443; // unless over-ridden by a :<port> spec...
        mu_str_slice(&s1, &s1, strlen(HTTPS_PREFIX), MU_STR_END);

    } else {
        return NULL;
    }

    // hostname may not start with a '-'
    if (mu_str_has_prefix_cstr(&s1, "-")) {
        // printf("\nhostname may not start with -");
        return NULL;
    }

    // search for first non-hostname char
    idx = mu_str_match(&s1, is_hostname, NULL, false);

    if (idx == MU_STR_NOT_FOUND) {
        // hit end of string without finding any non-hostname chars
        idx = mu_str_length(&s1);
    }

    if (idx == 0) {
        // zero length hostname not allowed.
        // printf("\nZero length hostname not allowed");
        return NULL;

    } else if (mu_str_bytes(&s1)[idx] != ':') {
        // hostname not terminated with a port number
        params->host_name = (const char *)mu_str_bytes(&s1);
        params->host_name_len = idx;
        return params;
    }

    // hostname terminated with a ':' -- parse the following port number
    mu_str_slice(&s1, &s1, idx + 1, MU_STR_END);

    idx = mu_str_match(&s1, is_decimal, NULL, false);
    if (idx == MU_STR_NOT_FOUND) {
        // hit end of string without finding any non-decimal chars
        idx = mu_str_length(&s1);
    }

    if (idx == 0) {
        // printf("\nzero length port # not allowed");
        // zero length port # not allowed
        return NULL;
    }

    // here, all bytes between s1[0] and s1[idx] are guaranteed to be integers.
    // NOTE: we don't check for overflow (values >= 65536)
    params->host_port = parse_uint16(mu_str_bytes(&s1), idx);

    return params;
}

void test_parse_url(void) {
    http_params_t params;

    printf("\nStarting test_parse_url example...");
    fflush(stdout);

    TEST_ASSERT(parse_http_params(&params, "http://example.com") == &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 80);
    TEST_ASSERT(params.use_tls == false);

    TEST_ASSERT(parse_http_params(&params, "https://example.com") == &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 443);
    TEST_ASSERT(params.use_tls == true);

    TEST_ASSERT(parse_http_params(&params, "http://example.com:8080") == &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 8080);
    TEST_ASSERT(params.use_tls == false);

    TEST_ASSERT(parse_http_params(&params, "https://example.com:8080") == &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 8080);
    TEST_ASSERT(params.use_tls == true);

    TEST_ASSERT(parse_http_params(&params, "https://example.com/extra") == &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 443);
    TEST_ASSERT(params.use_tls == true);

    TEST_ASSERT(parse_http_params(&params, "https://example.com:123/extra") ==
           &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 123);
    TEST_ASSERT(params.use_tls == true);

    TEST_ASSERT(parse_http_params(&params, "https://example.com/") == &params);
    TEST_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    TEST_ASSERT(params.host_port == 443);
    TEST_ASSERT(params.use_tls == true);

    TEST_ASSERT(parse_http_params(&params, "https://192.168.12.34/") == &params);
    TEST_ASSERT(strncmp(params.host_name, "192.168.12.34", params.host_name_len) ==
           0);
    TEST_ASSERT(params.host_port == 443);
    TEST_ASSERT(params.use_tls == true);

    // These are the valid chars that end a port #
    // "#?/\"

    // pathologies
    TEST_ASSERT(parse_http_params(&params, "") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http:") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http:/") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http://") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http://:") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http://:123") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http://example.com:") == NULL);
    TEST_ASSERT(parse_http_params(&params, "http://example.com:abc") == NULL);
    TEST_ASSERT(parse_http_params(&params, "ftp://example.com:123") == NULL);
    // parse_http_params() doesn't investigate the URL past the hostname / port#
    // ASSERT(parse_http_params(&params, "http://example.com/extra:123") ==
    // NULL); This should be caught, but we don't handle it at present.
    // ASSERT(parse_http_params(&params, "http://example.com:123abc") == NULL);
    printf("\n...test_parse_url example complete\n");
}

int main(void) {
    test_mu_str();
    test_mu_str_example();
    test_parse_url();
}

#endif // #ifdef TEST_MU_STR