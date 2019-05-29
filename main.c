#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "aal.h"


int main(int argc, char** argv)
{
	struct aal_config config;
	if (aal_init(&config) != 0)
		return -1;

	aal_run(&config);

	aal_clean(&config);

	return 0;
}

