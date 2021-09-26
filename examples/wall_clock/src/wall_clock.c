/*
 * @file wall_clock.c
 *
 *  Created on: Apr 18, 2021
 *      Author: andy
 */

// =============================================================================
// Includes

#include "wall_clock.h"
#include "ansi_big_font.h"
#include "ansi_nico_font.h"
#include "fb.h"

#include <mu_sched.h>
#include <mu_time.h>
#include <mu_rtc.h>
#include <mu_task.h>
#include <mu_kbd_io.h>
#include <mu_button_io.h>
#include <mu_led_io.h>
#include <mu_ansi_term.h>


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#ifdef HAS_POSIX_TIME
  #include <time.h>
#endif


// =============================================================================
// Local types and definitions

#define CLOCK_POLL_INTERVAL_MS (200)

#define TERM_WIDTH (80)
#define TERM_HEIGHT (25)


typedef struct {
  mu_task_t task;
  unsigned char key_char;
} clock_poll_ctx_t;

// =============================================================================
// Local (forward) declarations

static void establish_clock_offset();

static void clock_poll_fn(void *ctx, void *arg);

static char *local_time_string(void);

static void begin_polling_clock(void);

static void set_clock_offset(char *hhmmss_string);

// =============================================================================
// Local storage

static clock_poll_ctx_t clock_poll_ctx;

static char _local_time_string[16];

static unsigned long clock_offset = 0;

static char s_backing_buf[TERM_WIDTH * TERM_HEIGHT];

static char s_display_buf[TERM_WIDTH * TERM_HEIGHT];


// =============================================================================
// Public code

void wall_clock_init(void) {
  mu_ansi_term_init(); // so we can clear the screen, manipulate the cursor position, etc
  mu_rtc_init();
  mu_sched_init(); // we will poll the clock using a task that reschedules itself
  establish_clock_offset(); // on non-POSIX systems we will prompt the user to enter the current time
  mu_ansi_term_set_cursor_visible(false);
  mu_ansi_term_clear_screen();
  fb_init(TERM_WIDTH, TERM_HEIGHT, s_backing_buf, s_display_buf);
  begin_polling_clock(); 
}

void wall_clock_step(void) {
  mu_sched_step();
}

// =============================================================================
// Local (private) code

static void establish_clock_offset() { 
  #ifndef HAS_POSIX_TIME
    mu_ansi_term_clear_screen();   
    printf("Enter the current time (HH:MM:SS):\n");
    char *user_string = NULL;
    size_t linecap = 0;
    while(1) {
      int characters = getline(&user_string,&linecap,stdin);
      if(characters > 0) {
        if(characters != 9) {
          printf("Please use HH:MM:SS\n");
        } else {
          set_clock_offset(user_string);
          break;
        }
      }
    }
  #else
    struct tm  ts;
    time_t now;
    static char temp_string[16];
    time(&now);
    ts = *localtime(&now);  // Fill in the tm structure
    strftime(temp_string, sizeof(temp_string), "%I:%M:%S", &ts); // %H for 24H
    set_clock_offset(temp_string);
  #endif
}

static void set_clock_offset(char *hhmmss_string) {
  int hour, min, sec;
  sscanf(hhmmss_string , "%d:%d:%d" , &hour,&min,&sec);
  clock_offset = ((hour * 60 * 60) + (min * 60) + sec) * 1000;
}

// non-POSIX systems will need to use mu_rtc_now() / 1000 instead of time(&now)
static char *local_time_string() {
  int hours, mins;
  unsigned long secs = (mu_rtc_now() + clock_offset) / 1000;
  hours = (secs / 3600) % 24;
  mins = (secs % 3600) / 60;
  secs = (secs % 60);
  sprintf(_local_time_string,"%02d:%02d:%02lu",hours,mins,secs);
  return _local_time_string;
}

static void clock_poll_fn(void *ctx, void *arg) {
  (void)ctx;  // unused
  (void)arg;  // unused
  print_string_using_nico_font(local_time_string());
  mu_duration_t delay = MU_TIME_MS_TO_DURATION(CLOCK_POLL_INTERVAL_MS);
  mu_sched_task_in(&clock_poll_ctx.task, delay);
}

static void begin_polling_clock() {
  mu_task_init(&clock_poll_ctx.task, clock_poll_fn, &clock_poll_ctx, "clock_poll");
  mu_sched_task_now(&clock_poll_ctx.task);
}
