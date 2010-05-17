#define _GNU_SOURCE
#include <sched.h>

int main()
{
	cpu_set_t mask;
	CPU_ISSET(0,&mask);
}
