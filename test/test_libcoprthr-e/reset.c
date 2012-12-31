
#include <stdio.h>
#include <stdlib.h>

#include "e-host.h"
#include "e_loader.h"

int main()
{

	Epiphany_t e;
	e_open(&e);

	sleep(2);

	printf( "send reset\n");
	e_send_reset(&e,E_RESET_CORES);


	e_close(&e);

}

