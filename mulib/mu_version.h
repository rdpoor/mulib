/**
 * MIT License
 *
 * Copyright (c) 2020-2024 R. D. Poor <rdpoor@gmail.com>
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
 * @brief Declare the version number of mulib.
 */

#ifndef _MU_VERSION_H_
#define _MU_VERSION_H_

// *****************************************************************************
// Includes

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#define MU_VERSION_MAJOR 1
#define MU_VERSION_MINOR 0
#define MU_VERSION_PATCH 0

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#define MAKE_VERSION_STRING(major, minor, patch) \
    "v" STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch)

#define MU_VERSION_STRING \
    MAKE_VERSION_STRING(MU_VERSION_MAJOR, MU_VERSION_MINOR, MU_VERSION_PATCH)

// *****************************************************************************
// Public declarations

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_VERSION_H_ */
