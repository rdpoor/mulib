#include "task_join.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    task_join_init(); // this will hang until a key press ir button press appens
   while(1) {
       task_join_step();
  }
}
