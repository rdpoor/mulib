/**
 * MIT License
 *
 * Copyright (c) 2021-2022 R. D. Poor <rdpoor@gmail.com>
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
 * @file mu_fmt.h
 *
 * @brief sprintf-like formatter for mu_str and C-style strings.
 *
 * Note: see mu_printf() for emitting printed strings to stdout or other sinks.
 */

#ifndef _MU_FMT_H_
#define _MU_FMT_H_

// *****************************************************************************
// Includes

#include "mu_str.h"
#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

// *****************************************************************************
// Public declarations

size_t mu_fmt_parse_int(mu_str *s, int *val, int radix);
size_t mu_fmt_render_int(mu_str *s, int val, int radix);

size_t mu_fmt_parse_uint(mu_str *s, unsigned int *val, int radix);
size_t mu_fmt_render_uint(mu_str *s, unsigned int val, int radix);

size_t mu_fmt_parse_bool(mu_str *s, bool *val);
size_t mu_fmt_render_bool(mu_str *s, bool val);

size_t mu_fmt_parse_token(mu_str *s, mu_pvect *tokens, int *index);
size_t mu_fmt_render_token(mu_str *s, mu_pvect *tokens, int index);

#if defined(MU_CONFIG_HAS_FLOAT)
size_t mu_fmt_parse_float(mu_str *s, float *val);
size_t mu_fmt_parse_float(mu_str *s, float *val);
#endif

size_t mu_fmt_parse(mu_str_s *s, const char *fmt, ...);  // scanf-like?
size_t mu_fmt_render(mu_str_s *s, const char *fmt, ...); // printf-like

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_FMT_H_ */
