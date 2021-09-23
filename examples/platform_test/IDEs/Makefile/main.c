#include "platform_test.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    platform_test_init(); // this will hang until a key press ir button press appens
    while(1) {
       platform_test_step();
  }
}
