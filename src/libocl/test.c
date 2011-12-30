
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "libocl.h"

int main()
{
	unsigned int nplatforms;

	int err = clGetPlatformIDs( 0,0,&nplatforms );

	cl_platform_id* platforms 
		= (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));

	err = clGetPlatformIDs( nplatforms,platforms,0 );

}

