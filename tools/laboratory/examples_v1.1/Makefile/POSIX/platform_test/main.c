#include "kbd_read.h"
#include "platform_test.h"
#include <mulib.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * main.c
 */

int main(void) {
    // printf("FR\n");
    // start_kbd_reader_thread();
    platform_test_init();
    while (1) {
        platform_test_step();
    }
}
