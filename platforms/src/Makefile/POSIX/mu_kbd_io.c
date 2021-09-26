/**
 * MIT License
 *
 * Copyright (c) 2020 R. D. Poor <rdpoor@gmail.com>
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

// =============================================================================
// Includes


#include "mu_kbd_io.h"
#include "mu_ansi_term.h" // POSIX tty depends on ANSI term to mimic interrupt-driven keypress behavior

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <pthread.h>

#include <termios.h>

// =============================================================================
// Local types and definitions

#define STDIN_FILENO 0

// =============================================================================
// Local storage


static mu_kbd_io_callback_t s_kbd_io_cb = 0;
static struct termios saved_attributes;
static bool _has_saved_attributes = false;
static bool _tty_is_in_non_canonical_mode = false;
pthread_t thread_id;

// =============================================================================
// Local (forward) declarations

static void get_terminal_attributes(struct termios *terminal_attributes);
static void set_terminal_attributes(struct termios *terminal_attributes);
static void enter_noncanonical_mode();
static void exit_noncanonical_mode();
static int get_key_press(void);
static void start_kbd_reader_thread(void);
static void *reader_thread(void* vargp);

// =============================================================================
// Public code

void mu_kbd_io_init(void) {
  start_kbd_reader_thread(); // We use a POSIX thread to simulate keyboard interrupts, allowing us to fire the mu_kbd_io callback per keypress
}

void mu_kbd_io_set_callback(mu_kbd_io_callback_t cb) {
  s_kbd_io_cb = cb;
}

// =============================================================================
// Local (static) code

// These POSIX-dependent functions let us read keyboard input one character at a time, instead of waiting for linefeeds

static void set_terminal_attributes(struct termios *terminal_attributes) {
  tcsetattr(STDIN_FILENO, TCSANOW, terminal_attributes);
}

static void get_terminal_attributes(struct termios *terminal_attributes) {
  tcgetattr(STDIN_FILENO, terminal_attributes);      
}

void enter_noncanonical_mode() {
  static struct termios info;
  bool canonical = false, echo_input = false, wait_for_newlines = false; 
  if(_tty_is_in_non_canonical_mode) return;
  get_terminal_attributes(&saved_attributes); // so we can restore later
  _has_saved_attributes = true;
  _tty_is_in_non_canonical_mode = true;
  tcgetattr(STDIN_FILENO, &info);          
  info.c_lflag &= ((ICANON && canonical) | (ECHO && echo_input)); 
  info.c_cc[VMIN] = wait_for_newlines ? 1 : 0;
  info.c_cc[VTIME] = 0;         /* no timeout */
  set_terminal_attributes(&info);
 }

void exit_noncanonical_mode() {
  if(!_tty_is_in_non_canonical_mode) return;
  _tty_is_in_non_canonical_mode = false;
  if(_has_saved_attributes) {
    set_terminal_attributes(&saved_attributes);
  } else { 
    // no saved attributed so resetting hard
    static struct termios info;
    bool canonical = true, echo_input = true;
    bool wait_for_newlines = true; 
    tcgetattr(STDIN_FILENO, &info);          
    info.c_lflag |= ((ICANON && canonical) | (ECHO && echo_input));
    info.c_cc[VMIN] = wait_for_newlines ? 1 : 0;
    info.c_cc[VTIME] = 0;         /* no timeout */
    set_terminal_attributes(&info);
  }
  mu_ansi_term_reset();
}

// here we simulate the single-threaded tty interrupt system by spawning a posix thread
// to monitor the keyboard, and fire the callback

int get_key_press(void) {
    int ch;
    ch = getchar();
    if (ch < 0) {
        if (ferror(stdin)) { /* there was an error... */ }
        clearerr(stdin); // call clearerr regardless
        /* there was no keypress */
        return 0;
    }
    if(ch == 3) exit(0); // Ctrl-C
    return ch;
}

void start_kbd_reader_thread(void) {
  if(!s_kbd_io_cb)
    printf("warning: start_kbd_reader_thread without callback set.  will swallow keypresses.\n");
  enter_noncanonical_mode(); // so we dont wait for line feeds,  and we dont echo
  pthread_create(&thread_id, NULL, reader_thread, NULL);
  atexit(exit_noncanonical_mode); // restores terminal attributes
}

void *reader_thread(void* vargp)
{
    while(1) {
      char ch = get_key_press(); // assuming we are in noncanonical mode this won't hang
      if(ch && s_kbd_io_cb) 
        s_kbd_io_cb(ch);
    }
}


