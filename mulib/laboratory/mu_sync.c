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

// *****************************************************************************
// Includes

#include "mu_sync.h"

#include "mu_pstore.h"
#include "mu_sched.h"
#include "mu_task.h"
#include "mu_time.h"
#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

static void sync_timeout(void *ctx, void *arg);

// *****************************************************************************
// Public code

mu_sync_t *mu_sync_init(mu_sync_t *sync, void *store, size_t capacity) {
    mu_pstore_init(&sync->waiting_on, store, capacity);
    sync->on_completion = NULL;
    return sync;
}


size_t mu_sync_capacity(mu_sync_t *sync) {
    return mu_pstore_capacity(&sync->waiting_on);
}

mu_sync_t *mu_sync_reset(mu_sync_t *sync) {
    mu_pstore_reset(&sync->waiting_on);
    return sync;
}

mu_sync_err_t mu_sync_add_task(mu_sync_t *sync, mu_task_t *task) {
    // Add the task to the list of waited-on tasks
    if (mu_pstore_contains(&sync->waiting_on, task)) {
        return MU_SYNC_ERR_ALREADY_WAITING;
    } else if (mu_pstore_push(&sync->waiting_on, task) == MU_PSTORE_ERR_FULL) {
        return MU_SYNC_ERR_FULL;
    } else {
        return MU_SYNC_ERR_NONE;
    }
}

mu_sync_err_t mu_sync_remove_task(mu_sync_t *sync, mu_task_t *task) {
    // Remove the task from the list of waited-on tasks
    if (mu_pstore_delete(&sync->waiting_on, task) == NULL) {
        return MU_SYNC_ERR_NOT_WAITING;
    } else if (mu_pstore_count(&sysnc->waiting_on) == 0) {
        // removed last task -- cancel timeout (if present) and invoke callback.
        mu_sched_remove_task(&sync->task);
        mu_task_call(store->on_callback, NULL);
        return MU_SYNC_ERR_NONE;
    }
}

mu_sync_err_t mu_sync_wait(mu_sync_t *sync,
                           mu_task_t *on_completion,
                           mu_time_rel_t timeout_in) {
    if (timeout_in != 0) {
        mu_task_init(&sync->task, sync_timeout, sync, "Sync Timeout");
        if (mu_sched_defer_for(&sync->task, timeout_in) == MU_SCHED_ERR_FULL) {
            return MU_SYNC_ERR_SCHED_FULL;
        }
    }
    sync->on_completion = on_completion;
    return MU_SYNC_ERR_NONE;
}

// *****************************************************************************
// Private (static) code

static void sync_timeout(void *ctx, void *arg) {
    // Arrive here when timeout has arrived.
    mu_sync_t *sync = (mu_sync_t *)ctx;
    mu_task_call(store->on_callback, NULL);
    mu_pstore_reset(&sync->waiting_on);
}

// *****************************************************************************
// Standalone tests

// Run this command to run the standalone tests.
// (gcc -Wall -DMU_PSYNC_STANDALONE_TESTS -I../core -I../platform/posix \
// -o mu_sync_test mu_sync.c ../core/mu_sched.c ../core/mu_task.c \
// ../platform/posix/mu_time.c && ./mu_sync_test && rm ./mu_sync_test)

#ifdef MU_SYNC_STANDALONE_TESTS

#include <stdio.h>

#define CAPACITY 4
#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)

static void *store[CAPACITY];
static mu_sync_t s_q;

static void assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

static void setup(void) {
}

int main(void) {
  printf("\nStarting mu_sync tests...");
  printf("\n...tests complete\n");
  return 0;
}

#endif // #ifdef MU_SYNC_STANDALONE_TESTS
