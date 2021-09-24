#include "oblique.h"

/**
 * main.c
 */

#include <stdio.h>

int main(void)
{
    oblique_init(); // this will hang until a key press ir button press appens
    while(1) {
       oblique_step();
    }
}
