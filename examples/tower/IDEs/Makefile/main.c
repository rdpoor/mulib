#include "tower.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    tower_init(); // this will hang until a key press ir button press appens
   while(1) {
       tower_step();
  }
}
