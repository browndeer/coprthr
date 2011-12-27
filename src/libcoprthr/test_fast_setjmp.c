
#include <stdio.h>
#include <stdlib.h>

#include <fast_setjmp.h>


#define __callsp(sp,pf,arg) __asm ( \
   "movq %0,%%rdi\n\t"  \
   "movq %1,%%rsp\n\t"  \
   "call *%2\n"   \
   : : "m" (arg), "r" (sp), "m" (pf)  \
   );


fast_jmp_buf c0;
fast_jmp_buf cx[16];

char stack_storage[16*16384];
void* stack[16];

int count = 0;

int foo_r(int id )
{
	int rc;

	printf("A %d foo_r id=%d\n",count++,id);

	if (!(rc=fast_setjmp(cx[id-1]))) 
		{ printf("NORTH %d\n",id); fflush(stdout); fast_longjmp(c0,id); 
			printf("SOUTH\n"); fflush(stdout); }

	id = rc;

	printf("B %d foo_r id=%d\n",count++,id);

	if (!(id=fast_setjmp(cx[id-1]))) fast_longjmp(c0,id);

	printf("NEVER GET HERE\n");

	return(0);
}


int main()
{
	int i;
	int id;	
	int rc = 0;

	for(i=0;i<16;i++) stack[i] = stack_storage+(i+1)*16384;

	int(*pf)(int) = foo_r;

	/* try this, setjmp(0), then setjmp(1) next, then call func(0) */

	for(i=0;i<10;i++) {
//		if (!fast_setjmp(c0)) foo_r(i+1);
		void* sp = stack[i];
		int arg = i+1;
		if (!(id=fast_setjmp(c0))) 
			{ printf("LEFT%d\n",id); __callsp(sp,pf,arg); printf("RIGHT\n"); }
		printf("ELSE %d\n",id);
	}

//exit(0);

	for(i=0;i<10;i++) {
		if (!fast_setjmp(c0)) fast_longjmp(cx[i],i+1);
	}

}

