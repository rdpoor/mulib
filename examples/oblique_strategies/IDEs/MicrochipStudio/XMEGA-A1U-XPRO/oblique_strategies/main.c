#include "oblique.h"
#include <atmel_start.h>
#include <stdio.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	oblique_init();

	/* Replace with your application code */
	while (1) {
		oblique_step();
	}
}
