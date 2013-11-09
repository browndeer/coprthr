
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coprthr.h"
#include "coprthr_thread.h"

char src[] = \
	"__kernel void\n" \
	"my_thread( void* p) {\n" \
	"	long long* p_data = (long long*)p;\n" \
	"	*p_data = 44332211;\n" \
	"}\n";

#define __check( call ) do { \
	int err = call; \
	if (err) printf("error: %d %s: " #call "\n",err,strerror(err)); \
	} while(0)

int main()
{
	int dd = coprthr_dopen(0,0);

	printf("dd=%d\n",dd);

	coprthr_program_t prg = coprthr_compile(dd,src,sizeof(src),"",0);
	coprthr_sym_t thr = coprthr_link(dd,prg,"my_thread");

	printf("%p %p\n",prg,thr);

	coprthr_attr_t attr;
	coprthr_td_t td;
	void* status;

	__check( coprthr_attr_init( &attr ) );
	__check( coprthr_attr_setdetachstate(&attr,COPRTHR_CREATE_JOINABLE) );
	__check( coprthr_attr_setdevice(&attr,dd) );

	coprthr_mem_t mem = coprthr_dmalloc(dd,8,0);
	long long data = 9988776655;
	coprthr_event_t ev1 = coprthr_dmwrite(dd,mem,&data,8,0);
	coprthr_dwaitev(dd,ev1);
	
	__check( coprthr_create( &td, &attr, thr, (void*)&mem ) );

	__check( coprthr_join(td,&status) );

	printf("status %d\n",(int)status);

	coprthr_event_t ev2 = coprthr_dmread(dd,mem,&data,8,0);
	coprthr_dwaitev(dd,ev2);

	printf("data %ld\n",data);

	__check( coprthr_attr_destroy( &attr) );
	
	coprthr_dclose(dd);
}

