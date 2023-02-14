/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. Dunbar Poor <rdpoor@gmail.com>
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
 * @file: mu_msg_queue.h
 *
 * @brief Single level messaging queue
 */

#ifndef _MU_MSG_H_
#define _MU_MSG_H_

// *****************************************************************************
// Includes

#include "mu_task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
  void *obj;
  bool has_obj;
  mu_task_t *on_put;
  mu_task_t *on_get;
} mu_msg_queue_t;

// *****************************************************************************
// Public declarations

mu_msg_queue_t *mu_msg_queue_init(mu_msg_queue_t *msg, mu_task_t *on_put, mu_task_t *on_get);
bool mu_msg_queue_can_put(mu_msg_queue_t *msg);
bool mu_msg_queue_can_get(mu_msg_queue_t *msg);
void *mu_msg_queue_put(mu_msg_queue_t *msg, void *obj);
void *mu_msg_queue_get(mu_msg_queue_t *msg);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_MSG_H_ */
