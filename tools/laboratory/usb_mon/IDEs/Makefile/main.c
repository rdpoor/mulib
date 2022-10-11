#include "usb_mon.h"

/**
 * main.c
 */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

bool verbose_flag = false;

int main(int argc, char *argv[]) {
    int opt = 0;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
        case 'v':
            verbose_flag = true;
            break;
        }
    }

    usb_mon_init(); // this will hang until a key press ir button press appens
    while (1) {
        usb_mon_step();
    }
}
