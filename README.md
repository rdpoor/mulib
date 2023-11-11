# Welcome to `mulib`


`mulib` a highly performant, compact and reliable framework for building complex
applications in resource-constrained embedded systems.  

## `mulib`'s philosophy:  Small, Fast and Trusting

`mulib`'s design is guided by the following tenets:

* **Self-contained, pure C code**. `mulib` minimizes dependencies on external
libraries, except for the most common ones such as `stdint` and `stdbool`.
* **No malloc.  Ever.** `mulib`'s modules accept user-allocated data structures.
If your application already has malloc, you're free to use it, but `mulib` will
never malloc behind your back.
* **Well tested and supported.** `mulib` is validated through ample unit testing
and continuous integration.
* **Super portable.** To port `mulib` to a new target environment, you only
need create `mu_platform.h` and `mu_platform.c`, and those files may already exist
in the mu_examples repository.
* **Fast and Trusting.** `mulib` favors maximizing speed and minimizing code
space over safety.  In general, `mulib` trusts that you're passing valid
parameters and that your code implements argument validation where needed.
* **Single-threaded design.** Much of `mulib`'s small code footprint and
low-overhead is attributed to its single thread / multiple task design.
See [Run To Completion Schedulers](./docs/RunToCompletion.md)
for the reasons -- and implications -- behind this choice.
* **Power-aware scheduling.** `mulib`'s architecture for handling interrupts 
and scheduling make it easy to create low-power embedded applications. 
* **Low threshold, high ceiling.** A suite of well-documented code examples
guide you from the simplest "blink an LED" demo to complex multi-tasking
applications.
* **Yours to use.** The entirety of `mulib` is covered under the permissive MIT
Open Source license.

(And in case you're wondering about the pronunciation, "MOO-lib" is preferred
over "MEW-lib": cows, not kittens.)

## Directory Structure

```
mulib/
  README.md             # mulib overview: installation, usage, philosophy...

  mu_schedule/          # Sources, unit tests and documentation for mulib scheduler
    README.md           # Overview and documentation for the scheduler
    src/                # These are the files for a complete scheduler
      mu_sched_conf.h   # compile time configuration parameters
      mu_mqueue.[ch]
      mu_sched.[ch]
      mu_spsc.[ch]
      mu_task.[ch]
      mu_time.h         # NOTE: implementations provide mu_time.c "elsewhere"
    test/               # unit tests for the scheduler.
      Makefile          # testing makefile
      fff.h             # fake / mock / stub framework
      unity.[ch]        # test runner
      unity_internals.h # test runner
      test_mu_mqueue.c
      test_mu_sched.c
      ... etc

  mu_string/            # in-place, zero copy, safe string manipulation
    README.md           # overview and documentation for mu_string package
    src/                # these are the only files you need to include in your project
      mu_str.[ch]
    test/
      Makefile          # testing makefile
      fff.h             # fake / mock / stub framework
      unity.[ch]        # test runner
      unity_internals.h # test runner
      test_mu_str.c     # unit tests for the mu_string package

  mu_extras/            # generally useful utilities for embedded systems
    README.md           # overview and documentation for the mulib extras package
    src/                # pick and choose the files you want
      mu_http.[ch]      # generate and parse HTTP messages
      mu_json.[ch]      # generate and parse JSON strings
      mu_log.[ch]       # low-overhead logger (TRACE, DEBUG, INFO, WARN, ERROR)
      mu_macros.h       # clean, well-written utility macros (MU_MIN, MU_MAX, ...)
      mu_task_info.[ch] # task tracing utility (track state transitions, etc)
      mu_timer.[ch]     # one-shot delay or repeating mu_task trigger
    test/               # unit tests for the extras package
      Makefile          # testing makefile
      fff.h             # fake / mock / stub framework
      unity.[ch]        # test runner
      unity_internals.h # test runner
      test_mu_http.c
      test_mu_json.c

  examples/             # code examples and complete projects.  Details TBD

```
