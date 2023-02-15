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

// *****************************************************************************
// Includes

#include "mu_msg_queue.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

// *****************************************************************************
// Public code

mu_msg_queue_t *mu_msg_queue_init(mu_msg_queue_t *msg_queue, mu_task_t *on_put,
                                  mu_task_t *on_get) {
  msg_queue->on_put = on_put;
  msg_queue->on_get = on_get;
  msg_queue->has_obj = false;
  return msg_queue;
}

bool mu_msg_queue_can_put(mu_msg_queue_t *msg_queue) {
  return !msg_queue->has_obj;
}

bool mu_msg_queue_can_get(mu_msg_queue_t *msg_queue) {
  return msg_queue->has_obj;
}

void *mu_msg_queue_put(mu_msg_queue_t *msg_queue, void *obj) {
  if (mu_msg_queue_can_put(msg_queue)) {
    msg_queue->obj = obj;
    msg_queue->has_obj = true;
    mu_task_call(msg_queue->on_put, NULL);
    return obj;
  } else {
    return NULL;
  }
}

void *mu_msg_queue_get(mu_msg_queue_t *msg_queue) {
  if (mu_msg_queue_can_get(msg_queue)) {
    void *obj = msg_queue->obj;
    msg_queue->has_obj = false;
    mu_task_call(msg_queue->on_get, NULL);
    return obj;
  } else {
    return NULL;
  }
}

// *****************************************************************************
// Private (static) code

// *****************************************************************************
// *****************************************************************************
// Standalone tests
// *****************************************************************************
// *****************************************************************************

// Run this command in to run the standalone tests.
// gcc -Wall -DTEST_MU_MSG_QUEUE -o test_mu_msg_queue mu_msg_queue.c mu_task.c
// && ./test_mu_msg_queue && rm ./test_mu_msg_queue

#ifdef TEST_MU_MSG_QUEUE

#include <stdio.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

__attribute__((unused)) static void print_sem(mu_msg_queue_t *sem) {}

static int put_call_count = 0;
static int get_call_count = 0;

static void on_put_fn(mu_task_t *task, void *arg) {
  (void)arg;
  put_call_count++;
}

static void on_get_fn(mu_task_t *task, void *arg) {
  (void)arg;
  get_call_count++;
}

void test_mu_msg_queue(void) {
  printf("\nStarting test_mu_msg_queue...");

  mu_msg_queue_t msg_queue;
  mu_task_t on_put, on_get;
  const char *obj = "01234";

  mu_task_init(&on_put, on_put_fn, 0, NULL);
  mu_task_init(&on_get, on_get_fn, 0, NULL);

  // init returns sem.
  ASSERT(mu_msg_queue_init(&msg_queue, NULL, NULL) == &msg_queue);

  // on an empty queue...
  ASSERT(mu_msg_queue_can_put(&msg_queue) == true);
  ASSERT(mu_msg_queue_can_get(&msg_queue) == false);
  ASSERT(mu_msg_queue_get(&msg_queue) == NULL);
  ASSERT(mu_msg_queue_put(&msg_queue, &obj) == &obj);
  // on a full sem...
  ASSERT(mu_msg_queue_can_put(&msg_queue) == false);
  ASSERT(mu_msg_queue_can_get(&msg_queue) == true);
  ASSERT(mu_msg_queue_put(&msg_queue, &obj) == NULL);
  ASSERT(mu_msg_queue_get(&msg_queue) == &obj);

  // test callback triggers
  ASSERT(mu_msg_queue_init(&msg_queue, &on_put, &on_get) == &msg_queue);
  ASSERT(put_call_count == 0);
  ASSERT(get_call_count == 0);

  // on an empty queue...
  ASSERT(mu_msg_queue_get(&msg_queue) == NULL);
  ASSERT(mu_msg_queue_put(&msg_queue, &obj) == &obj);
  ASSERT(put_call_count == 1);
  ASSERT(get_call_count == 0);

  // on a full sem...
  ASSERT(mu_msg_queue_put(&msg_queue, &obj) == NULL);
  ASSERT(mu_msg_queue_get(&msg_queue) == &obj);
  ASSERT(put_call_count == 1);
  ASSERT(get_call_count == 1);

  printf("\n...test_mu_msg_queue complete\n");
}

int main(void) { test_mu_msg_queue(); }

#endif // #ifdef TEST_MU_MSG_QUEUE
