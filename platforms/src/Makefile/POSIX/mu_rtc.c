/**
 */

 // =============================================================================
 // includes

#include "mu_rtc.h"
#include "mu_time.h"
#include "time.h"          // posix time functions
#include <stdio.h>
#include <pthread.h>

// =============================================================================
// local types and definitions

#define NANOSECS_PER_S  (1000000000)
#define NANOSECS_PER_MS (1000000)

// =============================================================================
// local (forward) declarations
void start_alarm_thead();
void *alarm_thread(void* vargp);

// =============================================================================
// local storage

static volatile mu_time_t s_rtc_ticks;
//static mu_time_t s_safe_ticks;
//static mu_rtc_alarm_cb_t s_rtc_cb;

static mu_rtc_alarm_cb_t s_rtc_alarm_cb = 0;
//volatile static uint16_t s_rtc_hi;
//static uint16_t s_match_count_hi;
static mu_time_t s_match_count = 0;

static pthread_t thread_id = 0;

void *reader_thread(void* vargp);


// =============================================================================
// public code

/**
 * @brief Initialize the Real Time Clock.  Must be called before any other rtc
 * functions are called.
 */
void mu_rtc_init(void) {
  s_rtc_ticks = 0;
}

/**
 * @brief Get the current time.
 */
mu_time_t mu_rtc_now(void) {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (now.tv_sec * NANOSECS_PER_S + now.tv_nsec) / NANOSECS_PER_MS;
}

void mu_rtc_busy_wait(mu_duration_t ticks) {
  mu_time_t until  = mu_time_offset(mu_rtc_now(), ticks);
  while (mu_time_precedes(mu_rtc_now(), until)) {
    asm(" nop");
    // buzz...
  }
}

/**
 * @brief Set the time at which the RTC should trigger a callback.
 */
void mu_rtc_set_alarm(mu_time_t count) {
  s_match_count = count;
  if(!s_rtc_alarm_cb) {
    printf("Warning -- mu_rtc_set_alarm with no callback yet set\n");
    return;
  }
  start_alarm_thead();
}
/**
 * @brief Get the time at which the RTC should trigger a callback.
 */
mu_time_t mu_rtc_get_alarm(void) {
  //return (uint32_t)s_match_count_hi << 16 | RTC.COMP;
  return s_match_count;
}

/**
 * @brief Set the function to be called when the RTC count matches.
 *
 * Pass NULL for the CB to disable RTC compare callbacks.
 */
void mu_rtc_set_alarm_cb(mu_rtc_alarm_cb_t cb) {
  s_rtc_alarm_cb = cb;
  // if(!s_match_count) {
  //   printf("Warning -- no s_match_count for alarm\n");
  //   return;
  // }
  start_alarm_thead();
}


// =============================================================================
// local (static) code

//assumes both s_rtc_alarm_cb and s_match_count have been set
void start_alarm_thead() {
  if(thread_id) {
    printf("Warning -- thread_id non 0 -- there is already an alarm running\n");
    // cancel and reset?
    return;
  }
  pthread_create(&thread_id, NULL, alarm_thread, NULL);
}

void *alarm_thread(void* vargp)
{
    while(1) {
      if(mu_rtc_now() >= s_match_count) break;
    }
    thread_id = 0;
    s_rtc_alarm_cb();
    return NULL;
}





