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

mu_msg_queue_t *mu_msg_queue_init(mu_msg_queue_t *msg_queue,
                                  void **storage,
                                  size_t capacity,
                                  mu_task_t *on_put,
                                  mu_task_t *on_get) {
  msg_queue->storage = storage;
  msg_queue->capacity = capacity;
  msg_queue->on_put = on_put;
  msg_queue->on_get = on_get;
  return mu_msg_queue_reset(msg_queue);
}

mu_msg_queue_t *mu_msg_queue_reset(mu_msg_queue_t *msg_queue) {
  msg_queue->count = 0;
  msg_queue->index = 0;
  return msg_queue;
}

size_t mu_msg_queue_capacity(mu_msg_queue_t *msg_queue) {
  return msg_queue->capacity;
}

size_t mu_msg_queue_count(mu_msg_queue_t *msg_queue) {
  return msg_queue->count;
}

bool mu_msg_queue_is_empty(mu_msg_queue_t *msg_queue) {
  return msg_queue->count == 0;
}

bool mu_msg_queue_is_full(mu_msg_queue_t *msg_queue) {
  return msg_queue->count == msg_queue->capacity;
}

bool mu_msg_queue_put(mu_msg_queue_t *msg_queue, void *obj) {
  if (!mu_msg_queue_is_full(msg_queue)) {
    msg_queue->storage[msg_queue->index] = obj;
    msg_queue->index = (msg_queue->index + 1) % msg_queue->capacity;
    msg_queue->count += 1;
    mu_task_call(msg_queue->on_put, NULL);
    return true;
  } else {
    return false;
  }
}

bool mu_msg_queue_get(mu_msg_queue_t *msg_queue, void **obj) {
  if (!mu_msg_queue_is_empty(msg_queue)) {
    // idx = (index - count) MOD capacity, using unsigned arithmetic
    size_t idx = msg_queue->capacity + msg_queue->index - msg_queue->count;
    if (idx >= msg_queue->capacity) {
      idx -= msg_queue->capacity;
    }
    *obj = msg_queue->storage[idx];
    msg_queue->count -= 1;
    mu_task_call(msg_queue->on_get, NULL);
    return true;
  } else {
    *obj = NULL;
    return false;
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

#define MSG_QUEUE_CAPACITY 3

static void test_mu_msg_queue(void) {
  printf("\nStarting test_mu_msg_queue...");

  mu_msg_queue_t msg_queue;
  void *storage[MSG_QUEUE_CAPACITY];
  mu_task_t on_put, on_get;
  char *obj_a = "a";
  char *obj_b = "b";
  char *obj_c = "c";
  char *obj_d = "d";
  void *obj;

  mu_task_init(&on_put, on_put_fn, 0, NULL);
  mu_task_init(&on_get, on_get_fn, 0, NULL);

  // init returns sem.
  ASSERT(
      mu_msg_queue_init(&msg_queue, storage, MSG_QUEUE_CAPACITY, NULL, NULL) ==
      &msg_queue);

  // on an empty queue...
  ASSERT(mu_msg_queue_capacity(&msg_queue) == MSG_QUEUE_CAPACITY);
  ASSERT(mu_msg_queue_count(&msg_queue) == 0);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == false);

  // fill the queue...
  ASSERT(mu_msg_queue_put(&msg_queue, obj_a) == true);
  ASSERT(mu_msg_queue_count(&msg_queue) == 1);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  ASSERT(mu_msg_queue_put(&msg_queue, obj_b) == true);
  ASSERT(mu_msg_queue_count(&msg_queue) == 2);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  ASSERT(mu_msg_queue_put(&msg_queue, obj_c) == true);
  ASSERT(mu_msg_queue_count(&msg_queue) == MSG_QUEUE_CAPACITY);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == false);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  // attempting to add to a full queue
  ASSERT(mu_msg_queue_put(&msg_queue, obj_d) == false);
  ASSERT(mu_msg_queue_count(&msg_queue) == MSG_QUEUE_CAPACITY);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == false);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  // FIFO order is preserved on get

  ASSERT(mu_msg_queue_get(&msg_queue, &obj) == true);
  ASSERT(obj == obj_a);
  ASSERT(mu_msg_queue_count(&msg_queue) == 2);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  ASSERT(mu_msg_queue_get(&msg_queue, &obj) == true);
  ASSERT(obj == obj_b);
  ASSERT(mu_msg_queue_count(&msg_queue) == 1);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  ASSERT(mu_msg_queue_get(&msg_queue, &obj) == true);
  ASSERT(obj == obj_c);
  ASSERT(mu_msg_queue_count(&msg_queue) == 0);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == false);

  // attempting to get on an empty queue
  ASSERT(mu_msg_queue_get(&msg_queue, &obj) == false);
  ASSERT(obj == NULL);
  ASSERT(mu_msg_queue_count(&msg_queue) == 0);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == false);

  // mu_msg_queue_reset empties the queue
  ASSERT(mu_msg_queue_put(&msg_queue, obj_a) == true);
  ASSERT(mu_msg_queue_count(&msg_queue) == 1);
  ASSERT(!mu_msg_queue_is_full(&msg_queue) == true);
  ASSERT(!mu_msg_queue_is_empty(&msg_queue) == true);

  // test callback triggers
  ASSERT(mu_msg_queue_init(
             &msg_queue, storage, MSG_QUEUE_CAPACITY, &on_put, &on_get) ==
         &msg_queue);
  ASSERT(put_call_count == 0);
  ASSERT(get_call_count == 0);

  ASSERT(mu_msg_queue_put(&msg_queue, obj_a) == true);
  ASSERT(put_call_count == 1);
  ASSERT(get_call_count == 0);

  ASSERT(mu_msg_queue_get(&msg_queue, &obj) == true);
  ASSERT(put_call_count == 1);
  ASSERT(get_call_count == 1);

  printf("\n...test_mu_msg_queue complete\n");
}

int main(void) {
  test_mu_msg_queue();
}

#endif // #ifdef TEST_MU_MSG_QUEUE
