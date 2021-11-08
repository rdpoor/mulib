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
#include <mu_ansi_term.h>
#include "mu_platform.h"

// =============================================================================
// Local types and definitions

#define VERSION "0.1"

#define POLL_INTERVAL_MS 5000

// Define the context for a usb_mon task.  When task_fn is called, this
// context is passed in as an argument and gives task_fn all of the
// information it needs to run the task.
typedef struct {
  mu_task_t task;
  char *usb_state_string;
} ctx_t;

// A struct to hold an HTTP header key / value pair.
typedef struct {
  mu_str_t key;
  mu_str_t value;
} kv_pair_t;


typedef struct {
  mu_str_t name;
  mu_str_t product_id;
  mu_str_t location_id;
  mu_str_t manufacturer;
} usb_dev_t;



// copious string storage for info from lsusb or system_profiler -- on a busy osx this can easily exceed 20kb
// presumably this is running on a dev machine where ram isn't precious
#define OUTPUT_BUFFER_SIZE (1024 * 50)
#define LINE_BUFFER_SIZE 181
#define MAX_CSTR_LENGTH (1024 * 50)

// =============================================================================
// Local (forward) declarations

static void task_fn(void *ctx, void *arg);
static int read_output_from_shell_command(char *command, char *output_buffer);
static void parse_info();
//static void skip_eol(mu_str_t *reader);
//static void skip_whitespace(mu_str_t *reader);
//static bool extract_next_key(mu_str_t *reader, kv_pair_t *kv_pair, char *key);
static bool extract_next_dev(mu_str_t *str, usb_dev_t *usb_dev);
static bool extract_next_dev_linux(mu_str_t *str, usb_dev_t *usb_dev);
//static void print_current_line(mu_str_t *reader);
//static void print_line(mu_str_t *line);
//static void print_reader(mu_str_t *reader, int how_many);
static void print_string(mu_str_t *reader, int a, int b);
int mu_str_index_reverse(mu_str_t *str, uint8_t byte);

// =============================================================================
// Local storage

static ctx_t s_ctx;

static os_type_t os_type; 

static char readbuf[OUTPUT_BUFFER_SIZE];
//static char linebuf[LINE_BUFFER_SIZE];
// temp buffer for printing c-style strings
static char s_cstr_buf[MAX_CSTR_LENGTH];

#define MAX_DEVS 200

static int usb_dev_cnt = 0;
static usb_dev_t s_usb_devs[MAX_DEVS];


// provide storage for a key-value pair
//static kv_pair_t temp_kv_pair;


// static bool on_darwin = false;

// #if (ON_DARWIN)
// on_darwin = true;
// #endif

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
  mu_platform_init();
  mu_ansi_term_init();

  //printf("\r\nusb_mon v%s\n", VERSION);

  // initialize the mu_task to associate function (task_fn) with context (s_ctx)
  mu_task_init(&s_ctx.task, task_fn, &s_ctx, "USB_MON");

  // Initialize the context's initial state
  s_ctx.usb_state_string = readbuf;

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
  // Recast the void * argument to a usb_mon ctx_t * argument.
  ctx_t *self = (ctx_t *)ctx;
  (void)arg;  // unused

  //int err = read_output_from_shell_command("system_profiler SPUSBDataType 2>/dev/null", readbuf);
  //int err = read_output_from_shell_command("system_profiler SPUSBDataType -detailLevel mini 2>/dev/null | tr \"\\n\" \",\" | tr -d \"\\t\\n\"", readbuf);
  int err = read_output_from_shell_command("system_profiler SPUSBDataType -detailLevel mini 2>/dev/null | tr \"\\n\" \"^\" | tr -d \"\\t\"", readbuf);

  //printf("readbuf \n %s\n-----------------------------",readbuf);

  //int err = read_output_from_shell_command("ls -l | tr \"\\n\" \"-\" | tr -d \" \\t\\n\"", readbuf);
  if(err) {
    printf("read_output_from_shell_command err %d\n", err);
  } else {
      self->usb_state_string = readbuf;
      mu_ansi_term_clear_screen();
      parse_info();
  }

  mu_sched_reschedule_in(POLL_INTERVAL_MS); // POLL_INTERVAL_MS
}


static int read_output_from_shell_command(char *command, char *output_buffer) {
  FILE *input;
  input = popen (command, "r");
  if (!input)
    {
      fprintf (stderr, "incorrect parameters.\n");
      return -1;
    }
  while(fgets(output_buffer, OUTPUT_BUFFER_SIZE, input))
  
  if (pclose (input) != 0)
    {
      fprintf (stderr, "Could not run shell command or other error.\n");
      return -1;
    }
    return 0;
}

int mu_str_index_reverse(mu_str_t *str, uint8_t byte) {
  for (int i=str->s; i>=0; i--) {
    uint8_t b = mu_strbuf_rdata(str->buf)[i];
    if (b == byte) {
      return i - str->s;
    }
  }
  // not found
  return -1;
}

static void print_string(mu_str_t *str, int a, int b) {
  int st = a > 0 ? a : str->s;
  int len = b > 0 ? b : str->e;
  mu_str_t inspector_str;
  inspector_str.buf = str->buf;
  inspector_str.s = st;
  inspector_str.e = len;
  mu_str_to_cstr(&inspector_str, s_cstr_buf, len);
  //printf("inspector_str %d %d: %s \n",a,len,s_cstr_buf);  
  printf("%s",s_cstr_buf);  
}


// static void parse_info1() {
//   mu_strbuf_t info_string;
//   mu_str_t reader;

//   //strcpy(readbuf,"Test orf: 123^orf:345^asdlkjasd asdaa^orf:678^asd orf:3333^sdfsdf");

//   // Initialize the mu_strbuf and the reader mu_str objects.
//   mu_strbuf_init_from_cstr(&info_string, readbuf);
//   mu_str_init_for_read(&reader, &info_string);

//   int ndev = 0;
//   bool keepGoing = true;
//   while(keepGoing) {
//     // can we rely on things always being in this order?
//     // might be safer to: 1) get all the Product IDs -- friendly name precedes these. 
//     // store these indexes, and check BETWEEN them for Manufacturer and other keys

//     keepGoing = extract_next_key(&reader, &temp_kv_pair, "Manufacturer:");
//     print_string(&temp_kv_pair.key,0,0);
//     printf(" -> ");
//     print_string(&temp_kv_pair.value,0,0);

//     if(keepGoing) {
//       keepGoing = extract_next_key(&reader, &temp_kv_pair, "Location ID:");
//       printf(", ");
//       print_string(&temp_kv_pair.key,0,0);
//       printf(" -> ");
//       print_string(&temp_kv_pair.value,0,0);
//     }

//     printf("\n");
//     ndev++;
//   }
// }

static void parse_info() {
  mu_strbuf_t info_string;
  mu_str_t reader;

  //strcpy(readbuf,"Test orf: 123^orf:345^asdlkjasd asdaa^orf:678^asd orf:3333^sdfsdf");
  //strcpy(readbuf,"Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub^Bus 001 Device 003: ID 05ac:0307 Apple, Inc. Apple Optical USB Mouse^Bus 001 Device 004: ID 05ac:024f Apple, Inc. ^Bus 001 Device 002: ID 05ac:1006 Apple, Inc. Hub in Aluminum Keyboard^Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub");

  // Initialize the mu_strbuf and the reader mu_str objects.
  mu_strbuf_init_from_cstr(&info_string, readbuf);
  mu_str_init_for_read(&reader, &info_string);

  //print_string(&reader,0,0);

  usb_dev_cnt = 0;
  usb_dev_t *usb_dev;
  bool keepGoing = true;
  // first we find all the product IDs
  while(keepGoing) {
    // can we rely on things always being in this order?
    // might be safer to: 1) get all the Product IDs -- friendly name precedes these. 
    // store these indexes, and check BETWEEN them for Manufacturer and other keys

    usb_dev = &s_usb_devs[usb_dev_cnt];
    
    if(os_type == OS_TYPE_DARWIN)
      keepGoing = extract_next_dev(&reader, usb_dev);
    else
      keepGoing = extract_next_dev_linux(&reader, usb_dev);

    if(!keepGoing) break;

    //print_string(&usb_dev->name,0,0);
    //printf(" ");
    print_string(&usb_dev->name,0,0);
    printf(" ");
    //print_string(&usb_dev->manufacturer,0,0);
    //printf(" ");
    print_string(&usb_dev->product_id,0,0);
    printf(" ");
    print_string(&usb_dev->location_id,0,0);
    printf("\n");

    usb_dev_cnt++;

    // if(usb_dev_cnt > 26) // TODO solve eof problems...
    //   break;
  }
}

static bool extract_next_dev(mu_str_t *str, usb_dev_t *usb_dev) {
  mu_str_t str_cp;
  char *keystr;
  char separator = '^';
  int prodindex_a, prodindex_b, loc_index, separator_index;

  mu_str_copy(&str_cp, str);

  keystr = "Product ID:";

  // find first product_id
  prodindex_a = mu_str_find(&str_cp, keystr) + str_cp.s; // absolute index
  
  //printf("prodindex_a %d\n",prodindex_a);

  if(prodindex_a < str_cp.s) {
    printf("prodindex_a fail\n");
    return false;
  }
  // incr .s then find terminal product_id
  //mu_str_read_increment(str_cp, prodindex_a + 10);
  str_cp.s = prodindex_a + strlen(keystr);
  prodindex_b = mu_str_find(&str_cp, keystr) + str_cp.s; // -1 never happens because we add...
  if(prodindex_b < str_cp.s) {
    //printf("aha!\n");
    prodindex_b = mu_str_read_available(&str_cp) + str_cp.s;
  }

  //printf("prodindex_b %d\n",prodindex_b);

  // store product_id
  str_cp.s = prodindex_a + strlen(keystr);
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; // mu_str_index confusin since offset from ->s

  usb_dev->product_id.buf = str->buf;
  usb_dev->product_id.s = prodindex_a + strlen(keystr) + 1;
  usb_dev->product_id.e = separator_index;

  //printf("usb_dev->product_id.s e %zu %zu\n",usb_dev->product_id.s,usb_dev->product_id.e);


  str_cp.s = prodindex_a + strlen(keystr);
  keystr = "Location ID:";
  loc_index = mu_str_find(&str_cp, keystr) + str_cp.s;
  str_cp.s = loc_index + strlen(keystr);
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; // mu_str_index confusin since offset from ->s

  //printf("loc_index %d %d\n",loc_index,separator_index);

  if(loc_index < 0) { // TODO -- will never be < 0 because + str_cp.s
    printf("loc_index fail\n");
    return false;
  }

  usb_dev->location_id.buf = str->buf;
  usb_dev->location_id.s = loc_index + strlen(keystr) + 1;
  usb_dev->location_id.e = separator_index;


  str_cp.s = prodindex_a + strlen(keystr);
  keystr = "Manufacturer:";
  loc_index = mu_str_find(&str_cp, keystr) + str_cp.s;
  str_cp.s = loc_index + strlen(keystr);
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; // mu_str_index confusin since offset from ->s

  //printf("loc_index %d %d\n",loc_index,separator_index);

  if(loc_index < 0) { // TODO rename loc_index
    printf("loc_index fail\n");
    return false;
  }

  usb_dev->manufacturer.buf = str->buf;
  usb_dev->manufacturer.s = loc_index + strlen(keystr) + 1;
  usb_dev->manufacturer.e = separator_index;


  // to left of prod is name

  str_cp.s = prodindex_a;
  separator_index = mu_str_index_reverse(&str_cp,':') + str_cp.s; 

  usb_dev->name.buf = str->buf;
  usb_dev->name.e = separator_index;

  str_cp.s = separator_index;
  separator_index = mu_str_index_reverse(&str_cp,separator) + str_cp.s; 

  if(separator_index + 10 < usb_dev->name.e)
    usb_dev->name.s = separator_index + 1 + 8; // skip 8 spaces of formatted indentation

   // bump up the .s pointer in the source
  str->s = prodindex_b;

  return true;
}


static bool extract_next_dev_linux(mu_str_t *str, usb_dev_t *usb_dev) {
  mu_str_t str_cp;
  char *keystr;
  char separator = '^';
  int prodindex_a, prodindex_b, loc_index, separator_index;

  mu_str_copy(&str_cp, str);

  keystr = ": ID "; // robust?

  // find first product_id
  prodindex_a = mu_str_find(&str_cp, keystr) + str_cp.s; // absolute index
  
  //printf("prodindex_a %d %zu\n",prodindex_a, str_cp.s);

  if(prodindex_a < str_cp.s) {
    printf("prodindex_a fail\n");
    return false;
  }
  // incr .s then find terminal product_id
  //mu_str_read_increment(str_cp, prodindex_a + 10);
  str_cp.s = prodindex_a + 8;
  prodindex_b = mu_str_find(&str_cp, keystr) + str_cp.s; // -1 never happens because we add...
  if(prodindex_b < str_cp.s) {
    //printf("aha!\n");
    prodindex_b = mu_str_read_available(&str_cp) + str_cp.s;
  }

  //printf("prodindex_b %d\n",prodindex_b);

  // store product_id
  str_cp.s = prodindex_a + strlen(keystr) + 5;
  usb_dev->product_id.buf = str->buf;
  usb_dev->product_id.s = prodindex_a + strlen(keystr) + 5;
  usb_dev->product_id.e = prodindex_a + strlen(keystr) + 9;

  //printf("usb_dev->product_id.s e %zu %zu\n",usb_dev->product_id.s,usb_dev->product_id.e);

  str_cp.s = prodindex_a;
  keystr = "Location ID:";
  loc_index = mu_str_index_reverse(&str_cp, 'B') + str_cp.s;

  //printf("loc_index %d %d\n",loc_index,separator_index);

  if(loc_index < 0) { // TODO -- will never be < 0 because + str_cp.s
    printf("loc_index fail\n");
    return false;
  }

  usb_dev->location_id.buf = str->buf;
  usb_dev->location_id.s = loc_index; // Bus 
  usb_dev->location_id.e = prodindex_a + 1;

  // name

  str_cp.s = prodindex_a;
  separator_index = mu_str_index(&str_cp,separator) + str_cp.s; 

  usb_dev->name.buf = str->buf;
  usb_dev->name.s = prodindex_a + 15;
  usb_dev->name.e = separator_index;

   // bump up the .s pointer in the source
  str->s = prodindex_b;

  return true;
}

/* 

lsusb output:

Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 003: ID 05ac:0307 Apple, Inc. Apple Optical USB Mouse
Bus 001 Device 004: ID 05ac:024f Apple, Inc. 
Bus 001 Device 002: ID 05ac:1006 Apple, Inc. Hub in Aluminum Keyboard
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub

system_profiler SPUSBDataType -detailLevel mini 2>/dev/null | tr -d "\t"
EXAMPLE:


      AudioBox USB 96:

          Product ID: 0x0303
          Vendor ID: 0x194f
          Version: 1.12
          Serial Number: 000000000000
          Speed: Up to 480 Mb/s
          Manufacturer: PreSonus
          Location ID: 0x00100000 / 3
          Current Available (mA): 500
          Current Required (mA): 500
          Extra Operating Current (mA): 0

*/




/*

format with colors

If name contains HUB, dont print manufacturer, location (#define SHOW_HUBS)

monitor deltas
  (make birthdates, keyed by Location ID)

Test on systems with no hubs

Test on linux

Pass in strings, which are used as keys for addl display info


*/

