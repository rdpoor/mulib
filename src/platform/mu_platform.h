MU_PLATFORM/**
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
 * @file mu_platform.h
 *
 * @brief Platform specific definitions and declarations required by mulib.
 *
 * If you are porting mulib to a specific platform, create the required
 * definitions and declarations in mu_platform.h and implementations in
 * mu_platform.c.  If you are using mulib in your project, you must find (or
 & create) the appropriate mu_platform files for your platform.
 */

#ifndef _MU_PLATFORM_H_
#define _MU_PLATFORM_H_

// *****************************************************************************
// Includes

#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

// Required: define a data type to hold absolute time.
// typedef uint32_t mu_time_abs_t;

// Required: define a data type to hold relative time, i.e. an interval between
// two absolute times.  Note that this may be negative.
// typedef int32_t mu_time_rel_t;

// Required: define the maximum relative time.  Note that this will be used to
// distinguish between a long time in the future and a long time in the past.
// define MU_PLATFORM_MAX_TIME_REL INT32_MAX

// Optional: un-comment this if your platform supports floating point operations
// #define MU_PLATFORM_HAS_FLOAT

// Optional: Define the number of events that may be scheduled (with different
// times) in mu_sched.  Leave commented to accept the default.
// #define MU_PLATFORM_MAX_EVENTS <n>

// Optional: Uncomment the following to generate profiling functions for mu_task
// and mu_sched, at the expense of larger storage and slightly slower executiion
// times.
// define MU_PLATFORM_PROFILING_TASKS

// Optional: If implementing power management, the minimum time the system will
// sleep for.
// #define MU_PLATFORM_SLEEP_TIME_MIN mu_time_ms_to_rel(1)

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the mu_time module as needed.  Called once at startup.
 */
void mu_time_init(void);

/**
 * @brief Get the current time.
 *
 * NOTE: If you plan to implement low-power sleep, the timer used for this
 * function must continue to run in sleep mode.
 *
 * Note: mulib library functions properly handle mu_time_abs_t rolling over as
 * long as the relative time is less than MU_PLATFORM_MAX_TIME_REL
 *
 * @return The current absolute time.
 */
mu_time_abs_t mu_time_now(void);

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
mu_time_abs_t mu_time_offset(mu_time_abs_t t, mu_time_rel_t dt);

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
mu_time_rel_t mu_time_difference(mu_time_abs_t t1, mu_time_abs_t t2);

/**
 * @brief Return true if t1 is strictly before t2
 *
 * Note that if you want to know if t1 is before or equal to t2, you can use the
 * construct `!mu_time_follows(t2, t1)``
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 is strictly before t2, false otherwise.
 */
bool mu_time_precedes(mu_time_abs_t t1, mu_time_abs_t t2);

/**
 * @brief Return true if t1 is equal to t2
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 equals t2, false otherwise.
 */
bool mu_time_equals(mu_time_abs_t t1, mu_time_abs_t t2);

/**
 * @brief Return true if t1 is strictly after t2
 *
 * Note that if you want to know if t1 is equal to or after t2, you can use the
 * construct `!mu_time_precedes(t2, t1)``
 *
 * @param t1 A time object
 * @param t2 A time object
 * @return true if t1 is strictly after t2, false otherwise.
 */
bool mu_time_follows(mu_time_abs_t t1, mu_time_abs_t t2);

/**
 * @brief Convert a mu_time_rel_t to milliseconds;
 *
 * @param dt A relative time object
 * @return dt converted to milliseconds
 */
int mu_time_rel_to_ms(mu_time_rel_t dt);

/**
 * @brief Convert milliseconds to a mu_time_rel_t
 *
 * @param ms A relative time in milliseconds
 * @return milliseconds converted to mu_time_rel_t
 */
mu_time_rel_t mu_time_ms_to_rel(int ms);

#ifdef MU_PLATFORM_HAS_FLOAT
/**
 * @brief Convert a duration to seconds.
 *
 * @param dt A relative time
 * @return dt converted to seconds
 */
MU_FLOAT mu_time_rel_to_s(mu_time_rel_t dt);

/**
 * @brief Convert seconds to a duration.
 *
 * @param s A relative time in seconds
 * @return seconds converted to mu_time_rel_t
 */
mu_time_rel_t mu_time_s_to_rel(MU_FLOAT s);
#endif


// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_PLATFORM_H_ */
