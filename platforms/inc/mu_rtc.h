/**
 * MIT License
 *
 * Copyright (c) 2020 R. Dunbar Poor <rdpoor@gmail.com>
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

#ifndef _MU_RTC_H_
#define _MU_RTC_H_

#ifdef __cplusplus
extern "C";
#endif

// =============================================================================
// includes

#include "mu_time.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// types and definitions
#define RTC_FREQUENCY ((mu_duration_t)1024)

#define MU_TIME_MS_TO_DURATION(ms) ((mu_duration_t)(((((mu_duration_t)ms)*MS_PER_SECOND))/RTC_FREQUENCY))

typedef void (*mu_rtc_alarm_cb_t)(void);

// =============================================================================
// declarations

/**
 * @brief Initialize the Real Time Clock.  Must be called before any other rtc
 * functions are called.
 */
void mu_rtc_init(void);

/**
 * @brief Get the current time.
 */
mu_time_t mu_rtc_now(void);

/**
 * @brief Busy wait for the given number of RTC ticks.
 */
void mu_rtc_busy_wait(mu_duration_t duration);

/**
 * @brief Set the time at which the RTC should trigger a callback.
 */
void mu_rtc_set_alarm(mu_time_t count);

/**
 * @brief Get the time at which the RTC should trigger a callback.
 */
mu_time_t mu_rtc_get_alarm(void);

/**
 * @brief Set the function to be called when the RTC count matches.
 *
 * Pass NULL for the CB to disable RTC match count callbacks.
 */
void mu_rtc_set_alarm_cb(mu_rtc_alarm_cb_t cb);

// =============================================================================
// These are not public functions, but need to be declared for the RTC ISR
// functions.

// Design pattern: this is one of the few places where we don't provide a
// declaration in the .h file and instead use an extern declaration in the
// driver_isr.c file instead.

// void mu_rtc_on_compare_interrupt(void);

// void mu_rtc_on_overflow_interrupt(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _MU_RTC_H_