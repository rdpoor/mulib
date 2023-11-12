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
 * @brief Platform-specific configuration parameters for mulib
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

// A default definition of MU_SCHED_ABS_TIME.  Override as required by your
// platform.
#define MU_SCHED_ABS_TIME uint32_t

// A default definition of MU_SCHED_REL_TIME.  Override as required by your
// platform.
#define MU_SCHED_REL_TIME int32_t

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

#ifndef MU_SCHED_ABS_TIME
#error Must define MU_SCHED_ABS_TIME in mu_platform.h
#endif

#ifndef MU_SCHED_REL_TIME
#error Must define MU_SCHED_REL_TIME in mu_platform.h
#endif

// *****************************************************************************
// Public declarations

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_PLATFORM_H_ */
