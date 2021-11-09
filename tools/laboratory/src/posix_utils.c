/**
 * MIT License
 *
 * Copyright (c) 2021 R. D. Poor <rdpoor@gmail.com>
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

#include "template.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include "mu_drunken_bishop.h"
#include "mu_ansi_term.h"

// =============================================================================
// Private types and definitions

#define OUTPUT_BUFFER_SIZE 80

// =============================================================================
// Private (static) storage

// =============================================================================
// Private (forward) declarations

static int read_output_from_shell_command(char *command, char *output_buffer);

// =============================================================================
// Public code

// =============================================================================
// Private (static) code

static void art_from_src() {



   char readbuf[OUTPUT_BUFFER_SIZE];

  output an identicon derived from hashing the source code
 int err = read_output_from_shell_command("md5sum ../../mulib/core/*.c ../../mulib/extras/*.c | md5sum", readbuf);
 if(err) {
    if the shell md5sum failed, we (lamely) use the mu_version string
    TODO: actually traverse the source tree and do a md5sum thing ourselves.  in extras?
    sprintf(readbuf, "%s", mu_version());
 }
 printf("source fingerprint:\n");
 mu_print_random_art_from_string(readbuf, 17);
}


static int read_output_from_shell_command(char *command, char *output_buffer) {
  FILE *input;
  input = popen (command, "r");
  if (!input)
    {
      fprintf (stderr, "incorrect parameters.\n");
      return -1;
    }
  while(fgets(output_buffer, OUTPUT_BUFFER_SIZE, input)) {}
  
  if (pclose (input) != 0)
    {
      fprintf (stderr, "Could not run shell command or other error.\n");
      return -1;
    }
    return 0;
}
