#include "blink_1.h"
#include <atmel_start.h>
#include <stdio.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	blink_1_init();

	/* Replace with your application code */
	while (1) {
		blink_1_step();
	}
}
