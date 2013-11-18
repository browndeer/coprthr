
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coprthr.h"
#include "coprthr_thread.h"

#include "test.h"

struct my_args {
	void* mtx;
	long long data;
};

char src[] = \
	"#include <pthread.h>\n" \
	"typedef struct { void* mtx; long long data; } my_args_t;\n" \
	"__kernel void\n" \
	"my_thread( void* p) {\n" \
	"  my_args_t* pargs = (my_args_t*)p;\n" \
	"	long long data = pargs->data;\n" \
	"  pthread_mutex_lock((pthread_mutex_t*)pargs->mtx);\n" \
	"	pargs->data = pargs->data - 44332211;\n" \
	"  pthread_mutex_unlock((pthread_mutex_t*)pargs->mtx);\n" \
	"}\n";

int main()
{
	/* open device for threads */

	int dd = coprthr_dopen(COPRTHR_DEVICE_X86_64,COPRTHR_O_THREAD);

	printf("dd=%d\n",dd);


	/* compile thread function */

	char* log;
	coprthr_program_t prg = coprthr_compile(dd,src,sizeof(src),"",&log);
	printf("%s",log);
	coprthr_sym_t thr = coprthr_link(dd,prg,"my_thread");

	printf("%p %p\n",prg,thr);


	/* mutex */

	coprthr_mutex_attr_t mtxattr;
	coprthr_mutex_attr_init(&mtxattr);
	coprthr_mutex_attr_setdevice(&mtxattr,dd);

	coprthr_mutex_t mtx;
	__check( coprthr_mutex_init(&mtx,&mtxattr) );

	__check( coprthr_mutex_attr_destroy(&mtxattr) );


	/* allocate memory on device and write a value */

	struct my_args args;
	args.data = 9988776655;
	args.mtx = coprthr_devmemptr( (coprthr_mem_t)mtx); 
	coprthr_mem_t mem = coprthr_dmalloc(dd,sizeof(struct my_args),0);
	coprthr_dwrite(dd,mem,&args,sizeof(struct my_args),COPRTHR_E_NOW);


	/* lock the mutex on the device */

	coprthr_mutex_lock(&mtx);
	

	/* create thread */

	coprthr_attr_t attr;
	coprthr_td_t td,td2;
	void* status;

	__check( coprthr_attr_init( &attr ) );
	__check( coprthr_attr_setdetachstate(&attr,COPRTHR_CREATE_JOINABLE) );
	__check( coprthr_attr_setdevice(&attr,dd) );

	__check( coprthr_create( &td, &attr, thr, (void*)&mem ) );
	__check( coprthr_create( &td2, &attr, thr, (void*)&mem ) );

	__check( coprthr_attr_destroy( &attr) );


	/* wait 3 seconds to give the thread time to work */

	sleep(3);


	/* change the value stored in memory */

	args.data = 8877665544;
	coprthr_dwrite(dd,mem,&args,sizeof(struct my_args),COPRTHR_E_NOW);
	args.data = -1;

	/* unlock the mutex on the device */

	coprthr_mutex_unlock(&mtx);

	
	/* join the thread */

	__check( coprthr_join(td,&status) );
	__check( coprthr_join(td2,&status) );

	printf("status %d\n",(int)status);


	/* read back value from memory on device */

	coprthr_dread(dd,mem,&args,sizeof(struct my_args),COPRTHR_E_NOW);

	printf("data %ld\n",args.data);


	/* clean up */

	__check( coprthr_mutex_destroy(&mtx) );

	coprthr_dclose(dd);
}

