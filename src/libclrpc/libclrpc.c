
#include <sys/queue.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <CL/cl.h>

#include "clrpc.h"
#include "util.h"
#include "clrpc_common.h"
#include "clrpc.gen.h"


/* why the hell do i have to define this? -DAR */
#define min(a,b) ((a<b)?a:b)


#define CLRPC_UNBLOCK_CLICB(fname) \
static void \
_clrpc_##fname##_clicb(struct evrpc_status* status, \
   struct _clrpc_##fname##_request* request, \
	struct _clrpc_##fname##_reply* reply, void* parg) \
{ \
   xclreport( XCL_DEBUG "_clrpc_" #fname "_clicb"); \
	CLRPC_UNBLOCK(parg); \
} 


struct event_base* global_base;
evutil_socket_t global_spair[2] = { -1, -1 };

CLRPC_HEADER(clGetPlatformIDs)
CLRPC_HEADER(clGetPlatformInfo)
CLRPC_HEADER(clGetDeviceIDs)
CLRPC_HEADER(clGetDeviceInfo)
CLRPC_HEADER(clCreateContext)

CLRPC_GENERATE(clGetPlatformIDs)
CLRPC_GENERATE(clGetPlatformInfo)
CLRPC_GENERATE(clGetDeviceIDs)
CLRPC_GENERATE(clGetDeviceInfo)
CLRPC_GENERATE(clCreateContext)

struct cb_struct {
	pthread_mutex_t mtx;
	pthread_cond_t sig;
};

static void* loop( void* parg ) 
{
	printf("i am a loop\n");
   while(1) {
      event_loop(EVLOOP_NONBLOCK);
      sleep(1);
      printf("loop\n");
   };
	return ((void*)0);
}

ev_uint16_t clrpc_port = 0;
struct evhttp* clrpc_http = 0;
struct evrpc_pool* clrpc_pool = 0;
pthread_t clrpc_td;
pthread_attr_t clrpc_td_attr;

int clrpc_init( void )
{

	      if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, global_spair) == -1) {
         fprintf(stderr, "%s: socketpair\n", __func__);
         exit(1);
      }

      if (evutil_make_socket_nonblocking(global_spair[0]) == -1) {
         fprintf(stderr, "fcntl(O_NONBLOCK)");
         exit(1);
      }

      if (evutil_make_socket_nonblocking(global_spair[1]) == -1) {
         fprintf(stderr, "fcntl(O_NONBLOCK)");
         exit(1);
      }

   global_base = event_init();

	clrpc_port = 8080;

	clrpc_pool = rpc_pool_with_connection(clrpc_port);

   pthread_attr_init(&clrpc_td_attr);
   pthread_attr_setdetachstate(&clrpc_td_attr,PTHREAD_CREATE_JOINABLE);
	
	pthread_create(&clrpc_td,&clrpc_td_attr,loop,(void*)0);

	return(0);
}


int clrpc_final( void )
{
	pthread_cancel(clrpc_td);

	void* status;
	pthread_join(clrpc_td,&status);

	if (clrpc_pool)
		evrpc_pool_free(clrpc_pool);
	if (clrpc_http)
		evhttp_free(clrpc_http);

     if (global_base)
         event_base_free(global_base);

   global_spair[0] = global_spair[1] = -1;
   global_base = 0;


	return(0);
}


/*
 * clGetPlatformIDs
 */

CLRPC_UNBLOCK_CLICB(clGetPlatformIDs)

cl_int
clrpc_clGetPlatformIDs( cl_uint nplatforms, 
	cl_platform_id* platforms, cl_uint* nplatforms_ret)
{
	cl_int retval = 0;

	CLRPC_INIT(clGetPlatformIDs);

	CLRPC_ASSIGN(request,uint,nplatforms,nplatforms);

	CLRPC_ALLOC_DPTR_ARRAY(nplatforms,platforms);
	
	CLRPC_ASSIGN_DPTR_ARRAY(request,nplatforms,platforms);

	CLRPC_MAKE_REQUEST_WAIT(clGetPlatformIDs);

	CLRPC_GET(reply,int,retval,&retval);

	CLRPC_GET(reply,uint,nplatforms_ret,nplatforms_ret);
	xclreport( XCL_DEBUG "clrpc_clGetPlatformIDs: *nplatforms_ret = %d\n",
		*nplatforms_ret);

	xclreport( XCL_DEBUG 
		"clrpc_clGetPlatformIDs: compare nplatforms"
		" *nplatforms_ret ARRAY_LEN %d %d %d\n",
		nplatforms,*nplatforms_ret,EVTAG_ARRAY_LEN(reply, platforms));

	nplatforms = min(nplatforms,*nplatforms_ret);

	CLRPC_GET_DPTR_ARRAY(reply,nplatforms,platforms);

	return(retval);
}


/*
 * clGetPlatformInfo
 */

CLRPC_UNBLOCK_CLICB(clGetPlatformInfo)

cl_int
clrpc_clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name,
	size_t param_sz, void* param_val, size_t *param_sz_ret)
{
	cl_int retval = 0;

	CLRPC_INIT(clGetPlatformInfo);

	CLRPC_ASSIGN_DPTR(request,platform,platform);

	CLRPC_ASSIGN(request,platform_info,param_name,param_name);
	CLRPC_ASSIGN(request,uint,param_sz,param_sz);

	CLRPC_MAKE_REQUEST_WAIT(clGetPlatformInfo);

	CLRPC_GET(reply,int,retval,&retval);

	CLRPC_GET(reply,uint,param_sz_ret,param_sz_ret);
	xclreport( XCL_DEBUG "clrpc_clGetPlatformInfo: *param_sz_ret = %ld\n",
		*param_sz_ret);

	param_sz = min(param_sz,*param_sz_ret);

	void* tmp_param_val = 0;
	unsigned int tmplen = 0;
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);
	memcpy(param_val,tmp_param_val,param_sz);
//	CLRPC_GET_WITH_LEN(reply,param_val,&tmp_param_val,&tmplen);

	printf("%ld:%s\n",param_sz,(char*)param_val);

	return(retval);
}


/*
 * clGetDeviceIDs
 */

CLRPC_UNBLOCK_CLICB(clGetDeviceIDs)

cl_int
clrpc_clGetDeviceIDs( cl_platform_id platform, cl_device_type devtype,
	cl_uint ndevices, cl_device_id* devices, cl_uint* ndevices_ret)
{
	cl_int retval = 0;

	CLRPC_INIT(clGetDeviceIDs);

	CLRPC_ASSIGN_DPTR(request,platform,dummy);
	CLRPC_ASSIGN(request,uint,devtype,devtype);

	CLRPC_ASSIGN(request,uint,ndevices,ndevices);

	CLRPC_ALLOC_DPTR_ARRAY(ndevices,devices);
	
	CLRPC_ASSIGN_DPTR_ARRAY(request,ndevices,devices);

	CLRPC_MAKE_REQUEST_WAIT(clGetDeviceIDs);

	CLRPC_GET(reply,int,retval,&retval);

	CLRPC_GET(reply,uint,ndevices_ret,ndevices_ret);
	xclreport( XCL_DEBUG "clrpc_clGetDeviceIDs: *ndevices_ret = %d\n",
		*ndevices_ret);

	xclreport( XCL_DEBUG 
		"clrpc_clGetDeviceIDs: compare ndevices"
		" *ndevices_ret ARRAY_LEN %d %d %d\n",
		ndevices,*ndevices_ret,EVTAG_ARRAY_LEN(reply, devices));

	ndevices = min(ndevices,*ndevices_ret);

	CLRPC_GET_DPTR_ARRAY(reply,ndevices,devices);

	return(retval);
}


/*
 * clGetDeviceInfo
 */

CLRPC_UNBLOCK_CLICB(clGetDeviceInfo)

cl_int
clrpc_clGetDeviceInfo(cl_device_id device, cl_device_info param_name,
	size_t param_sz, void* param_val, size_t *param_sz_ret)
{
	cl_int retval = 0;

	CLRPC_INIT(clGetDeviceInfo);

	CLRPC_ASSIGN_DPTR(request,device,device);

	CLRPC_ASSIGN(request,device_info,param_name,param_name);
	CLRPC_ASSIGN(request,uint,param_sz,param_sz);

	CLRPC_MAKE_REQUEST_WAIT(clGetDeviceInfo);

	CLRPC_GET(reply,int,retval,&retval);

	CLRPC_GET(reply,uint,param_sz_ret,param_sz_ret);
	xclreport( XCL_DEBUG "clrpc_clGetDeviceInfo: *param_sz_ret = %ld\n",
		*param_sz_ret);

	param_sz = min(param_sz,*param_sz_ret);

	void* tmp_param_val = 0;
	unsigned int tmplen = 0;
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);
	memcpy(param_val,tmp_param_val,param_sz);
//	CLRPC_GET_WITH_LEN(reply,param_val,&tmp_param_val,&tmplen);

	printf("%ld:%s\n",param_sz,(char*)param_val);

	return(retval);
}


/*
 * clCreateContext
 */

CLRPC_UNBLOCK_CLICB(clCreateContext)

cl_context
clCreateContext(
   const cl_context_properties* prop,
   cl_uint ndev,
   const cl_device_id* devices, 
   void (*pfn_notify) (const char*, const void*, size_t, void*),
   void* user_data,
   cl_int* err_ret 
)
{
	clrpc_dptr* context = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	context->local = (clrpc_ptr)context;

	CLRPC_INIT(clCreateContext);

	int i,j;

	const cl_context_properties* p = prop;
	while(*p) p += 2;
	int nprop = ((intptr_t)p-(intptr_t)prop)/sizeof(cl_context_properties)/2;
	clrpc_ptr* xprop = calloc(nprop*3+1,sizeof(clrpc_ptr));
	for(i=0,j=0;i<2*nprop;i+=2,j+=3) {
		xprop[j] = (clrpc_ptr)prop[i];
		xprop[j+1] = (clrpc_ptr)((clrpc_dptr*)prop[i+1])->local;
		xprop[j+2] = (clrpc_ptr)((clrpc_dptr*)prop[i+1])->remote;
	}
	xprop[j] = 0;
//	CLRPC_ASSIGN_DPTR_ARRAY(request,nprop,xprop);
	for(i=0;i<nprop*3+1;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,xprop,xprop[i]);

	CLRPC_ASSIGN(request,uint,ndev,ndev);

	CLRPC_ASSIGN_DPTR_ARRAY(request,ndev,devices);

	clrpc_dptr* retval = context;
	CLRPC_ASSIGN_DPTR(request,retval,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clCreateContext);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&context->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateContext: *err_ret = %d\n", *err_ret);

	if (xprop) free(xprop);

	return((cl_context)context);
}

