
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "clrpc.h"


/* why the hell do i have to define this? -DAR */
#define min(a,b) ((a<b)?a:b)

ev_uint16_t clrpc_port = 0;
struct evhttp* clrpc_http = 0;
struct evrpc_pool* clrpc_pool = 0;
pthread_t clrpc_td;
pthread_attr_t clrpc_td_attr;


static void
clrpc_client_test(void)
{

	clrpc_init();

	cl_uint nplatforms = 0;
	cl_platform_id* platforms = 0;
	cl_uint nplatforms_ret;

	clrpc_clGetPlatformIDs(nplatforms,platforms,&nplatforms_ret);	

	xclreport( XCL_DEBUG "after call one i get nplatforms_ret = %d",
		nplatforms_ret);

	nplatforms = nplatforms_ret;
	platforms = (cl_platform_id*)calloc(nplatforms,sizeof(cl_platform_id));

	clrpc_clGetPlatformIDs(nplatforms,platforms,&nplatforms_ret);

	int i;
	for(i=0;i<nplatforms;i++) {
		xclreport( XCL_DEBUG "platforms[%d] local=%p remote=%p\n",
			i,(void*)((clrpc_dptr*)platforms[i])->local,
			(void*)((clrpc_dptr*)platforms[i])->remote);
	}

	char buffer[1024];
	size_t sz;
	clrpc_clGetPlatformInfo(platforms[0],CL_PLATFORM_NAME,1023,buffer,&sz);

	printf("CL_PLATFORM_NAME|%ld:%s|\n",sz,buffer);

	cl_uint ndevices = 0;
	cl_device_id* devices = 0;
	cl_uint ndevices_ret;

	clrpc_clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_GPU,
		ndevices,devices,&ndevices_ret);

	xclreport( XCL_DEBUG "after call one i get ndevices_ret = %d",
      ndevices_ret);

	ndevices = ndevices_ret;
	devices = (cl_device_id*)calloc(ndevices,sizeof(cl_device_id));

	clrpc_clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_GPU,
		ndevices,devices,&ndevices_ret);

	for(i=0;i<ndevices;i++) {
		xclreport( XCL_DEBUG "devices[%d] local=%p remote=%p\n",
			i,(void*)((clrpc_dptr*)devices[i])->local,
			(void*)((clrpc_dptr*)devices[i])->remote);
		clrpc_clGetDeviceInfo(devices[i],CL_DEVICE_NAME,1023,buffer,&sz);
		xclreport( XCL_DEBUG "CL_DEVICE_NAME |%s|",buffer);
	}

	sleep(5);

	clrpc_final();

}


int
main(int argc, const char **argv)
{

	printf("hello world\n");

   clrpc_client_test();

   return 0;
}

