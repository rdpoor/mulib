#include "parse_http.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    parse_http_init(); // this will hang until a key press ir button press appens
   while(1) {
       parse_http_step();
  }
}
