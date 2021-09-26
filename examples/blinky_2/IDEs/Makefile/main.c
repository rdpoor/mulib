#include "blink_2.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    blink_2_init(); // this will hang until a key press ir button press appens
   while(1) {
       blink_2_step();
  }
}
