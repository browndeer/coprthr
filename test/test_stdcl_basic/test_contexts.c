
#include <stdlib.h>
#include <stdio.h>

#include "stdcl.h"

int test_context( CONTEXT* cp );

#define __error(n) do { \
	fprintf(stderr,"ERROR: %s:%d\n",__FILE__,n); \
	exit(n); \
	} while (0)

int main( int argc, char** argv )
{

	if (argc != 4) {
		fprintf(stderr,"usage: ./test_contexts.x ndev_cpu ndev_gpu ndev_rpu\n");
		__error(__LINE__);
	}

	int a1 = atoi(argv[1]);
	int a2 = atoi(argv[2]);
	int a3 = atoi(argv[3]);

	int err = 0;


	/* test stddev */

//	if (!stddev) {
//		fprintf(stderr,"ERROR: no stddev\n");
//		exit(__LINE__);
//	}
//
//	err = test_context(stddev,a1+a2+a3);
//	if (err) exit(__LINE__);

	
	/*** test stdcpu ***/	

	if (!stdcpu) __error(__LINE__);

	if (clgetndev(stdcpu) != a1) __error(__LINE__);

	if (test_context(stdcpu)) __error(__LINE__);
	
	
	/*** test stdgpu ***/	

	if (!stdgpu) __error(__LINE__);

	if (clgetndev(stdgpu) != a2) __error(__LINE__);
	
	if (test_context(stdgpu)) __error(__LINE__);

}


int test_context( CONTEXT* cp ) 
{
	int err = 0;

	struct clstat_info stat_info;

	clstat(cp,&stat_info);

	int ndev = clgetndev(cp);

	struct cldev_info* dev_info 
		= (struct cldev_info*)malloc(ndev*sizeof(struct cldev_info));

	clgetdevinfo(cp,dev_info);

	clfreport_devinfo(stdout,ndev,dev_info);

	free(dev_info);

	return(0);
}

