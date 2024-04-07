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
 * @file mu_time.h
 *
 * @brief Declaration of the time functions required by mu_sched.
 *
 * Note: each platform must provide:
 * - definitions of MU_SCHED_ABS_TIME and MU_SCHED_REL_TIME in mu_platform.h
 * - mu_time.c with implementations of the function prototypes defined here.
 */


#ifndef _MU_TIME_H_
#define _MU_TIME_H_

// *****************************************************************************
// Includes

#include "mu_platform.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#ifndef MU_SCHED_ABS_TIME
#error mu_platform.h must define MU_SCHED_ABS_TIME
#endif

#ifndef MU_SCHED_REL_TIME
#error mu_platform.h must define MU_SCHED_REL_TIME
#endif

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the mu_time module as needed.  Called once at startup.
 */
void mu_time_init(void);

/**
 * @brief Get the current time.
 *
 * @return The current absolute time.
 */
MU_SCHED_ABS_TIME mu_time_now(void);

/**
 * @brief Add a time and a duration.
 *
 * `mu_time_offset` adds an absolute time and a relative time to produce a new
 * absolute time.
 *
 * @param t1 a time object
 * @param dt a duration object
 * @return t1 offset by dt
 */
MU_SCHED_ABS_TIME mu_time_offset(MU_SCHED_ABS_TIME t, MU_SCHED_REL_TIME dt);

/**
 * @brief Take the difference between two time objects
 *
 * `mu_time_difference` subtracts absolute time t2 from absolute time t1 to
 * produce a relative time.
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return (t1-t2) as a relative time
 */
MU_SCHED_REL_TIME mu_time_difference(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2);

/**
 * @brief Return true if t1 precedes t2
 *
 * Note that if you want to know if t1 precedes or is equal to t2, you can use
 * the construct `!mu_time_follows(t2, t1)``
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 is strictly before t2, false otherwise.
 */
bool mu_time_precedes(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2);

/**
 * @brief Return true if t1 is equal to t2
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 equals t2, false otherwise.
 */
bool mu_time_equals(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2);

/**
 * @brief Return true if t1 follows t2
 *
 * Note that if you want to know if t1 is equal to follows t2, you can use the
 * construct `!mu_time_precedes(t2, t1)``
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 is strictly after t2, false otherwise.
 */
bool mu_time_follows(MU_SCHED_ABS_TIME t1, MU_SCHED_ABS_TIME t2);

/**
 * @brief Convert a MU_SCHED_REL_TIME to milliseconds;
 *
 * Note: the result is undefined if dt cannot be converted to milliseconds.
 *
 * @param dt A relative time object
 * @return dt converted to milliseconds
 */
int mu_time_rel_to_ms(MU_SCHED_REL_TIME dt);

/**
 * @brief Convert milliseconds to a MU_SCHED_REL_TIME
 *
 * Note: the result is undefined if ms cannot be converted to MU_SCHED_REL_TIME.
 *
 * @param ms A relative time in milliseconds
 * @return milliseconds converted to MU_SCHED_REL_TIME
 */
MU_SCHED_REL_TIME mu_time_ms_to_rel(int ms);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_PLATFORM_H_ */
