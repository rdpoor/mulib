#include "blink_3.h"
#include <atmel_start.h>
#include <stdio.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	blink_3_init();

	/* Replace with your application code */
	while (1) {
		blink_3_step();
	}
}
