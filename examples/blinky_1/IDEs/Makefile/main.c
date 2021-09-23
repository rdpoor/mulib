#include "blink_1.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    blink_1_init(); // this will hang until a key press ir button press appens
   while(1) {
       blink_1_step();
  }
}
