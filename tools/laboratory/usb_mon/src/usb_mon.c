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

#include "usb_mon.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mu_sched.h>
#include <mu_task.h>
#include <mu_str.h>
#include <mu_strbuf.h>
//#include <mu_list.h>
#include <mu_ansi_term.h>
#include "mu_platform.h"

// =============================================================================
// Local types and definitions

#define VERSION "0.3"

#define POLL_INTERVAL_MS 6000

// Define the context for a usb_mon task.  When task_fn is called, this
// context is passed in as an argument and gives task_fn all of the
// information it needs to run the task.
typedef struct {
  mu_task_t task;
} ctx_t;

// A struct to hold info about a connected usb device.
typedef struct {
  mu_str_t name;
  mu_str_t product_id;
  mu_str_t location_id;
  mu_str_t manufacturer;
  //mu_list_t list;
} usb_dev_t;

// copious string storage for info from lsusb or system_profiler -- on a busy osx this can easily exceed 20kb
// presumably this is running on a dev machine where ram isn't precious
#define OUTPUT_BUFFER_SIZE (1024 * 32)
#define MAX_CSTR_LENGTH (1024)
#define MAX_DEVS 200

#define min(x, y) ((x) > (y) ? (y) : (x))


// =============================================================================
// Local (forward) declarations

static void task_fn(void *ctx, void *arg);
static int read_output_from_shell_command(char *command, char *output_buffer);
static void parse_info();
static bool extract_next_dev_darwin(mu_str_t *str, usb_dev_t *usb_dev);
static bool extract_next_dev_linux(mu_str_t *str, usb_dev_t *usb_dev);
static void print_string(mu_str_t *reader, int a, int b);
static void print_dev(usb_dev_t *usb_dev, bool isNew);
int mu_str_index_reverse(mu_str_t *str, uint8_t byte);
int mu_str_strcmp(mu_str_t *str1, mu_str_t *str2);
static void mu_str_trim_char(mu_str_t *str, uint8_t byte);

// =============================================================================
// Local storage

static ctx_t s_ctx;

static os_type_t os_type;
extern  bool verbose_flag;
//static usb_dev_t first_device = (usb_dev_t){.list.next = NULL, .age = -1}; 

// buffer to hold output of system query (lsusb or similar)
static char readbuf[OUTPUT_BUFFER_SIZE] = "";
static char readbuf_prev[OUTPUT_BUFFER_SIZE] = "";
// temp buffer for storing c-style strings
static char s_cstr_buf[MAX_CSTR_LENGTH];


static int usb_dev_cnt = 0;
static usb_dev_t s_usb_devs[MAX_DEVS] = {};

// =============================================================================
// Public code

void usb_mon_init(void) {

  #ifdef ON_DARWIN
  os_type = OS_TYPE_DARWIN;
  #else
  os_type = OS_TYPE_LINUX;
  #endif

  printf("os_type %d\n",os_type);

  mu_sched_init();
  mu_ansi_term_init();

  // initialize the mu_task to associate function (task_fn) with context (s_ctx)
  mu_task_init(&s_ctx.task, task_fn, &s_ctx, "USB_MON");

  // Make the first call to the scheduler to start things off.  The task_fn will
  // reschedule itself upon completion.
  mu_sched_task_now(&s_ctx.task);
}


void usb_mon_step(void) {
  // Just run the scheduler
  mu_sched_step();
}

// =============================================================================
// Local (private) code

// task_fn is invoked whenever usb_mon's task is triggered.  
static void task_fn(void *ctx, void *arg) {
  int err;

  if(os_type == OS_TYPE_DARWIN)
    err = read_output_from_shell_command("system_profiler SPUSBDataType -detailLevel mini 2>/dev/null | tr \"\\n\" \"^\" | tr -d \"\\t\"", readbuf);
  else 
    err = read_output_from_shell_command("lsusb | tr \"\\n\" \"^\" | tr -d \"\\t\"", readbuf);

  if(err) {
    printf("read_output_from_shell_command err %d\n", err);
  } else {
    mu_ansi_term_clear_screen();
    mu_ansi_term_home();
    parse_info();
    strncpy(readbuf_prev, readbuf, OUTPUT_BUFFER_SIZE);
  }
  mu_sched_reschedule_in(POLL_INTERVAL_MS); // POLL_INTERVAL_MS
}

static int read_output_from_shell_command(char *command, char *output_buffer) {
  FILE *input;
  input = popen (command, "r");
  if (!input) {
    fprintf (stderr, "incorrect parameters.\n");
    return -1;
  }

  while(fgets(output_buffer, OUTPUT_BUFFER_SIZE, input)) {}
  
  if (pclose (input) != 0) {
    fprintf (stderr, "Could not run shell command or other error.\n");
    return -1;
  }
  return 0;
}

int mu_str_index_reverse(mu_str_t *str, uint8_t byte) {
  for (int i=str->s; i>=0; i--) {
    uint8_t b = mu_strbuf_rdata(str->buf)[i];
    if (b == byte) 
      return i - str->s;
  }
  // not found
  return -1;
}

int mu_str_strcmp(mu_str_t *str1, mu_str_t *str2) {
  int len = min(str1->e - str1->s, str2->e - str2->s);
  return strncmp((char *)&str1->buf->rdata[str1->s], (char *)&str2->buf->rdata[str2->s], len);
}

static void mu_str_trim_char(mu_str_t *str, uint8_t byte) {
  for (int i=str->s; i<str->e; i++) {
    str->s = i;
    uint8_t b = mu_strbuf_rdata(str->buf)[i];
    if (b != byte) 
      return;
  }
}

static void print_string(mu_str_t *str, int a, int b) {
  int st = a > 0 ? a : str->s;
  int len = b > 0 ? b : str->e;
  mu_str_t inspector_str;
  inspector_str.buf = str->buf;
  inspector_str.s = st;
  inspector_str.e = len;
  mu_str_to_cstr(&inspector_str, s_cstr_buf, len);
  printf("%s",s_cstr_buf);  
}

static void parse_info() {
  mu_strbuf_t info_string, prev_info_string;
  mu_str_t reader, prev_reader;
  usb_dev_t *usb_dev;
  bool keepGoing = true;
  // Initialize the mu_strbuf and the reader mu_str objects.
  mu_strbuf_init_from_cstr(&info_string, readbuf);
  mu_str_init_for_read(&reader, &info_string);
  mu_strbuf_init_from_cstr(&prev_info_string, readbuf_prev);
  mu_str_init_for_read(&prev_reader, &prev_info_string);

  usb_dev_cnt = 0;
  
  while(keepGoing) {
    usb_dev = &s_usb_devs[usb_dev_cnt];
    if(os_type == OS_TYPE_DARWIN)
      keepGoing = extract_next_dev_darwin(&reader, usb_dev);
    else
      keepGoing = extract_next_dev_linux(&reader, usb_dev);
    if(!keepGoing) break;
    // make a cstr of the location, so we can search for it in the prev readbuf
    mu_str_to_cstr(&usb_dev->location_id, s_cstr_buf, (usb_dev->location_id.e - usb_dev->location_id.s));
    print_dev(usb_dev, mu_str_find(&prev_reader, s_cstr_buf) < 0);
    usb_dev_cnt++;
  }
}

static void print_dev(usb_dev_t *usb_dev, bool isNew) {

  if(!verbose_flag) {
    if(mu_str_find(&usb_dev->name,"Hub") >= 0)
      return;
    if(mu_str_find(&usb_dev->name,"USB Host") >= 0)
      return;
  }

  mu_ansi_term_set_colors(isNew ? MU_ANSI_TERM_BRIGHT_GREEN : MU_ANSI_TERM_YELLOW, MU_ANSI_TERM_DEFAULT_COLOR);
  print_string(&usb_dev->name,0,0);
  mu_ansi_term_set_colors(MU_ANSI_TERM_GRAY, MU_ANSI_TERM_DEFAULT_COLOR);
  printf(" (");
  print_string(&usb_dev->product_id,0,0);
  mu_ansi_term_set_colors(MU_ANSI_TERM_GRAY, MU_ANSI_TERM_DEFAULT_COLOR);
  printf(")");
  mu_ansi_term_set_colors(MU_ANSI_TERM_DEFAULT_COLOR, MU_ANSI_TERM_DEFAULT_COLOR);
  printf(" [");
  print_string(&usb_dev->location_id,0,0);
  printf("]\n");
}

static bool extract_next_dev_darwin(mu_str_t *str, usb_dev_t *usb_dev) {
  mu_str_t str_cp;
  char *keystr;
  char separator = '^';
  int prodindex_a, prodindex_b, key_index, separator_index;
  mu_str_copy(&str_cp, str);

  // find first product_id
  keystr = "Product ID:";
  prodindex_a = mu_str_find(&str_cp, keystr) + str_cp.s; // absolute index
  if(prodindex_a < str_cp.s) // no records found
    return false;
  
  // incr .s then find terminal product_id
  str_cp.s = prodindex_a + strlen(keystr);
  prodindex_b = mu_str_find(&str_cp, keystr) + str_cp.s;
  if(prodindex_b < str_cp.s) // must be tge last record
    prodindex_b = mu_str_read_available(&str_cp) + str_cp.s;
  
  // find end of the product_id
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; // mu_str_index confusin since offset from ->s

  // store product_id
  usb_dev->product_id.buf = str->buf;
  usb_dev->product_id.s = str_cp.s + 3; // skipping "0x" prefix
  usb_dev->product_id.e = separator_index;

  // find location_id
  keystr = "Location ID:";
  key_index = mu_str_find(&str_cp, keystr) + str_cp.s;
  if(key_index < str_cp.s) // must have returned -1
    return false;
  str_cp.s = key_index + strlen(keystr);
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; // mu_str_index confusin since offset from ->s

  usb_dev->location_id.buf = str->buf;
  usb_dev->location_id.s = key_index + strlen(keystr) + 1;
  usb_dev->location_id.e = separator_index;

  // to left of prod is name
  str_cp.s = prodindex_a;
  separator_index = mu_str_index_reverse(&str_cp,':') + str_cp.s; 

  usb_dev->name.buf = str->buf;
  usb_dev->name.e = separator_index;

  str_cp.s = separator_index;
  separator_index = mu_str_index_reverse(&str_cp,separator) + str_cp.s; 

  if(separator_index + 10 < usb_dev->name.e)
    usb_dev->name.s = separator_index + 1 + 8; // skip 8 spaces of formatted indentation

  if(!verbose_flag)
    mu_str_trim_char(&usb_dev->name,' ');

  str->s = prodindex_b; // bump up the .s pointer in the source

  return true;
}

static bool extract_next_dev_linux(mu_str_t *str, usb_dev_t *usb_dev) {
  mu_str_t str_cp;
  char *keystr;
  char separator = '^';
  int prodindex_a, prodindex_b, key_index, separator_index;

  mu_str_copy(&str_cp, str);

  keystr = ": ID "; // robust?

  // find first product_id
  prodindex_a = mu_str_find(&str_cp, keystr) + str_cp.s; // absolute index
  
  if(prodindex_a < str_cp.s) 
    return false;
  
  // incr .s then find terminal product_id
  str_cp.s = prodindex_a + 8;
  prodindex_b = mu_str_find(&str_cp, keystr) + str_cp.s; // -1 never happens because we add...
  if(prodindex_b < str_cp.s)
    prodindex_b = mu_str_read_available(&str_cp) + str_cp.s;

  // store product_id
  str_cp.s = prodindex_a + strlen(keystr) + 5;
  usb_dev->product_id.buf = str->buf;
  usb_dev->product_id.s = str_cp.s + 5;
  usb_dev->product_id.e = usb_dev->product_id.s + 4;

  str_cp.s = prodindex_a;
  keystr = "Location ID:";
  key_index = mu_str_index_reverse(&str_cp, 'B') + str_cp.s + 4; // skip back to Bus, then omit it

  if(key_index < 0) // TODO -- will never be < 0 because + str_cp.s
    return false;

  usb_dev->location_id.buf = str->buf;
  usb_dev->location_id.s = key_index; // Bus 
  usb_dev->location_id.e = prodindex_a;

  // name
  str_cp.s = prodindex_a;
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; 

  usb_dev->name.buf = str->buf;
  usb_dev->name.s = prodindex_a + 15;
  usb_dev->name.e = separator_index;

  str->s = prodindex_b; // bump up the .s pointer in the source

  return true;
}

