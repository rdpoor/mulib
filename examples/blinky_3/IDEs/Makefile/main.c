#include "blink_3.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    blink_3_init(); // this will hang until a key press ir button press appens
   while(1) {
       blink_3_step();
  }
}
