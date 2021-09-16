#include "parse_http.h"
#include <atmel_start.h>
#include <stdio.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	parse_http_init();

	/* Replace with your application code */
	while (1) {
		parse_http_step();
	}
}
