
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "libocl.h"


int main()
{
	int i,j;

	unsigned int nplatforms;

	int err = clGetPlatformIDs( 0,0,&nplatforms );

	cl_platform_id* platforms 
		= (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));

	err = clGetPlatformIDs( nplatforms,platforms,0 );

	printf("nplatforms=%d\n",nplatforms);

	int n;

	for(n=0;n<nplatforms;n++) {

		cl_platform_id platformid = platforms[n];

		void** cv = *(void***)platformid;

		printf("platformid=%p oclent=%p\n",platformid,cv);

		char* buf[1024];

		clGetPlatformInfo(platformid,CL_PLATFORM_VENDOR,1024,buf,0);

		printf("%s\n",buf);


#if(0)

		//// get devices first 

		unsigned int ndev;
		clGetDeviceIDs(platformid,CL_DEVICE_TYPE_CPU,0,0,&ndev);
		cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));
		clGetDeviceIDs(platformid,CL_DEVICE_TYPE_ALL,ndev,devices,0);

		printf("ndev=%d\n",ndev);

		for(j=0;j<ndev;j++) {
			clGetDeviceInfo(devices[j],CL_DEVICE_NAME,1024,buf,0);
			printf("%s\n",buf);
		}

		cl_context_properties ctxprop[] = { 
			(cl_context_properties)CL_CONTEXT_PLATFORM, (cl_context_properties)platformid, 0, 0 };
		cl_context ctx = clCreateContext(ctxprop,ndev,devices,0,0,&err);

#else

		////  get context first

		cl_context_properties ctxprop[] = { 
			(cl_context_properties)CL_CONTEXT_PLATFORM, (cl_context_properties)platformid, 0, 0 };
		cl_context ctx = clCreateContextFromType(ctxprop,CL_DEVICE_TYPE_CPU,0,0,&err);

		size_t sz;
		clGetContextInfo(ctx,CL_CONTEXT_DEVICES,0,0,&sz);
		unsigned int ndev = sz/sizeof(cl_device_id);
		cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));
		clGetContextInfo(ctx,CL_CONTEXT_DEVICES,sz,devices,0);

		for(j=0;j<ndev;j++) {
			clGetDeviceInfo(devices[j],CL_DEVICE_NAME,1024,buf,0);
			printf("%s\n",buf);
		}

		printf("sz %d ndev %d\n",sz,ndev);

#endif


		//// continue 

		cl_command_queue* cmdq = (cl_command_queue*)malloc(ndev*sizeof(cl_command_queue));

		for(j=0;j<ndev;j++) {
			cmdq[j] = clCreateCommandQueue(ctx,devices[j],0,&err);
		}

		sleep(1);

		for(j=0;j<ndev;j++) {
			clReleaseCommandQueue(cmdq[j]);
		}

		clReleaseContext(ctx);

		free(cmdq);
		free(devices);

	}

}

