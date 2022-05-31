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

#include "mu_irq.h"

#include "mu_spsc.h"
#include "mu_task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#ifndef MU_IRQ_MAX_TASKS
#define MU_IRQ_MAX_TASKS 8 // must be a power of two!
#endif

// *****************************************************************************
// Private (static) storage

static mu_spsc_item_t s_irq_tasks[MU_IRQ_MAX_TASKS];
static mu_spsc_t s_irq_spsc;

// *****************************************************************************
// Private (forward) declarations

// *****************************************************************************
// Public code

void mu_irq_init(void) {
  mu_spsc_init(&s_irq_spsc, s_irq_tasks, MU_IRQ_MAX_TASKS);
}

mu_task_t *mu_irq_queue_task(mu_task_t *task) {
  if (mu_spsc_put(&s_irq_spsc, task) == MU_SPSC_ERR_FULL) {
    return NULL;
  }
  return task;
}

void mu_irq_process_irqs(void) {
  mu_spsc_item_t item;
  while (mu_spsc_get(&s_irq_spsc, &item) == MU_SPSC_ERR_NONE) {
      mu_task_call((mu_task_t *)item, NULL);
  }
}

// *****************************************************************************
// Private (static) code
