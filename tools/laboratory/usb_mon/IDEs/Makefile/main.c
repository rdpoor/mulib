#include "usb_mon.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    usb_mon_init(); // this will hang until a key press ir button press appens
   while(1) {
       usb_mon_step();
  }
}
