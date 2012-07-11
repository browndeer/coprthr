
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
	int err;

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

	cl_context_properties ctxprop[] = { 
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 0 };

	cl_context ctx = clrpc_clCreateContext(ctxprop,ndevices,devices, 0,0,&err);

	cl_command_queue* cmdq 
		= (cl_command_queue*) calloc(ndevices,sizeof(cl_command_queue));

	for(i=0;i<ndevices;i++) {
		cmdq[i] = clrpc_clCreateCommandQueue(ctx,devices[i],0,&err);
		xclreport( XCL_DEBUG	 "cmdq %d %p",i,cmdq[i]);
	}

	cl_mem a_buf = clrpc_clCreateBuffer(ctx,CL_MEM_READ_WRITE,1024*sizeof(int),
		0,&err);
	cl_mem b_buf = clrpc_clCreateBuffer(ctx,CL_MEM_READ_WRITE,1024*sizeof(int),
		0,&err);

	int* a = (int*)malloc(1024*sizeof(int));
	int* b = (int*)malloc(1024*sizeof(int));
	for(i=0;i<1024;i++) a[i] = i*10;
	for(i=0;i<1024;i++) b[i] = i*100;

	cl_event ev;

	for(i=0;i<32;i++) printf("%d/",a[i]); printf("\n");
	for(i=0;i<32;i++) printf("%d/",b[i]); printf("\n");

	clrpc_clEnqueueWriteBuffer(cmdq[0],a_buf,CL_TRUE,0,1024*sizeof(int),a,
		0,0,&ev);
	clrpc_clReleaseEvent(ev);

	clrpc_clEnqueueWriteBuffer(cmdq[0],b_buf,CL_TRUE,0,1024*sizeof(int),b,
		0,0,&ev);
	clrpc_clReleaseEvent(ev);

	char* prgsrc[] = { 
		"__kernel void my_kern( __global int* a, __global int* b )\n"
		" { int i = get_global_id(0); b[i] = a[i]; }\n" 
	};
	size_t prgsrc_sz = strlen(prgsrc[0]) + 1;

	cl_program prg = clrpc_clCreateProgramWithSource(ctx,1,
		(const char**)prgsrc,&prgsrc_sz,&err);

	clrpc_clReleaseMemObject(a_buf);
	clrpc_clReleaseMemObject(b_buf);

	clrpc_clReleaseCommandQueue(cmdq[0]);
	clrpc_clReleaseContext(ctx);

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

