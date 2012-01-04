
/*
#include <stdio.h>
#include <unistd.h>
#define __USE_GNU
#include <sched.h>
*/

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <signal.h>

//#define __USE_GNU
#include <sched.h>


int main()
{

	printf("%d\n",sysconf(_SC_NPROCESSORS_ONLN));

	cpu_set_t mask;
	sched_getaffinity(0,sizeof(cpu_set_t),&mask);
	CPU_ZERO(&mask);
//	CPU_SET(0,&mask);
//	CPU_SET(1,&mask);
//	CPU_SET(2,&mask);
	CPU_SET(3,&mask);

	printf("%d\n",mask);

}


