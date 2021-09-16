#include "task_join.h"
#include <atmel_start.h>
#include <stdio.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	task_join_init();

	/* Replace with your application code */
	while (1) {
		task_join_step();
	}
}
