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
 * @file mu_platform.h
 *
 * @brief Platform-specific definitions and declarations for mulib
 *
 * This is where you define certain compile-time parameters to tailor mulib for
 * your environment.
 *
 * Every mulib source file includes mu_platform.h.  At present, only mu_schedule
 * makes use of the parameters here, but this may be extended in the future.
 */

#ifndef _MU_PLATFORM_H_
#define _MU_PLATFORM_H_

// *****************************************************************************
// Includes

#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

// #define MU_SCHED_MAX_DEFERRED_TASKS 20
// #define MU_SCHED_MAX_IRQ_TASKS 8 // must be a power of two!
// #define MU_SCHED_MAX_IMMED_TASKS 20
// #define MU_HAS_FLOAT

// A default definition of MU_TIME_ABS.  Override as required by your
// platform.
#define MU_TIME_ABS uint64_t

// A default definition of MU_TIME_REL.  Override as required by your
// platform.
#define MU_TIME_REL int64_t

// ============= do not edit below here ================
// This sets defaults for any configuration parameters not set by the user.

#ifndef MU_SCHED_MAX_DEFERRED_TASKS
#define MU_SCHED_MAX_DEFERRED_TASKS 20
#endif

#ifndef MU_SCHED_MAX_IRQ_TASKS
#define MU_SCHED_MAX_IRQ_TASKS 8 // must be a power of two!
#endif

#ifndef MU_SCHED_MAX_IMMED_TASKS
#define MU_SCHED_MAX_IMMED_TASKS 20
#endif

#ifndef MU_TIME_ABS
#error Must define MU_TIME_ABS in mu_platform.h
#endif

#ifndef MU_TIME_REL
#error Must define MU_TIME_REL in mu_platform.h
#endif

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the platform module as needed.  Called once at startup.
 */
void mu_platform_init(void);

/**
 * @brief Get the current time.
 *
 * @return The current absolute time.
 */
MU_TIME_ABS mu_time_now(void);

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
MU_TIME_ABS mu_time_offset(MU_TIME_ABS t, MU_TIME_REL dt);

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
MU_TIME_REL mu_time_difference(MU_TIME_ABS t1, MU_TIME_ABS t2);

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
bool mu_time_precedes(MU_TIME_ABS t1, MU_TIME_ABS t2);

/**
 * @brief Return true if t1 is equal to t2
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 equals t2, false otherwise.
 */
bool mu_time_equals(MU_TIME_ABS t1, MU_TIME_ABS t2);

/**
 * @brief Convert a MU_TIME_REL to milliseconds;
 *
 * Note: the result is undefined if dt cannot be converted to milliseconds.
 *
 * @param dt A relative time object
 * @return dt converted to milliseconds
 */
int mu_time_rel_to_ms(MU_TIME_REL dt);

/**
 * @brief Convert milliseconds to a MU_TIME_REL
 *
 * Note: the result is undefined if ms cannot be converted to MU_TIME_REL.
 *
 * @param ms A relative time in milliseconds
 * @return milliseconds converted to MU_TIME_REL
 */
MU_TIME_REL mu_time_ms_to_rel(int ms);

#ifdef MU_HAS_FLOAT
// Define these functions if the float data type is available on your platform.

/**
 * @brief Convert a MU_TIME_REL to seconds;
 *
 * Note: the result is undefined if dt cannot be converted to seconds.
 *
 * @param dt A relative time object
 * @return dt converted to seconds
 */
float mu_time_rel_to_s(MU_TIME_REL dt);

/**
 * @brief Convert seconds to a MU_TIME_REL
 *
 * Note: the result is undefined if s cannot be converted to MU_TIME_REL.
 *
 * @param seconds A relative time in seconds
 * @return seconds converted to MU_TIME_REL
 */
MU_TIME_REL mu_time_s_to_rel(float seconds);

#endif

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif
