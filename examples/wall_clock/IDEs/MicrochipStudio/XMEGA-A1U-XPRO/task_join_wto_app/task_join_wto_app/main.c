#include "task_join_wto.h"
#include <atmel_start.h>
#include <stdio.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	task_join_wto_init();

	/* Replace with your application code */
	while (1) {
		task_join_wto_step();
	}
}
