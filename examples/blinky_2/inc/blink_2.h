/**
 * MIT License
 *
 * Copyright (c) 2020 R. D. Poor <rdpoor@gmail.com>
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

#ifndef _BLINK_2_H_
#define _BLINK_2_H_

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Includes

// =============================================================================
// Types and definitions

// =============================================================================
// Declarations

/**
 * @brief Initialize the blink_2 example code.  Called once at startup.
 */
void blink_2_init(void);

/**
 * @brief Step the blink_2 example code.  Called repeatedly.
 *
 * Note: All morse_e_step() does is call mu_sched_step().  This is typical of
 * applications that are based on the mulib scheduler.
 */
void blink_2_step(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _BLINK_2_H_