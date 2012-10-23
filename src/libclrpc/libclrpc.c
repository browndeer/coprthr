
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

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include <CL/cl.h>

#include "clrpc.h"
//#include "util.h"
#include "printcl.h"
#include "clrpc_common.h"
#include "clrpc.gen.h"
//#include "oclcall.h"

/* why the hell do i have to define this? -DAR */
#define min(a,b) ((a<b)?a:b)

//#define __alias(name) __attribute__((alias(#name)))
#define __alias(name) 


extern void** __clrpc_call_vector;


typedef struct {
   void* _reserved;
   clrpc_dptr* object;
   struct evrpc_pool* rpc_pool;
	void* ext;
} _xobject_t;

#define _xobject_rpc_pool(xobj) ((_xobject_t*)xobj)->rpc_pool

#define _xobject_create(xobj,obj,pool) \
	_xobject_t* xobj = (_xobject_t*)malloc(sizeof(_xobject_t)); \
	do { \
	xobj->object = (clrpc_dptr*)obj; \
	xobj->rpc_pool = pool; \
	xobj->ext = 0; \
	xobj->_reserved = (void*)__clrpc_call_vector; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	obj = (void*)xobj; \
	} while (0)


typedef struct {
	void* _reserved;
	clrpc_dptr* object;
	struct evrpc_pool* rpc_pool;
   void* buf_ptr;
   size_t buf_sz;
} _xevent;

#define _xevent_rpc_pool(xobj) (((_xevent*)xobj)->rpc_pool)

#define _xevent_create(xobj,obj,pool) \
	_xevent* xobj = (_xevent*)malloc(sizeof(_xevent)); \
	do { \
	xobj->object = (clrpc_dptr*)obj; \
	xobj->rpc_pool = pool; \
	xobj->_reserved = (void*)__clrpc_call_vector; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)


#define CLRPC_UNBLOCK_CLICB(fname) \
static void \
_clrpc_##fname##_clicb(struct evrpc_status* status, \
   struct _clrpc_##fname##_request* request, \
	struct _clrpc_##fname##_reply* reply, void* parg) \
{ \
   printcl( CL_DEBUG "_clrpc_" #fname "_clicb"); \
	CLRPC_UNBLOCK(parg); \
} 


/* XXX retain is identical to release! -DAR */
#define CLRPC_GENERIC_RETAIN(name,type,arg) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int name( cl_##type arg ) __alias(clrpc_##name); \
cl_int clrpc_##name( cl_##type x##arg ) \
{ \
	CLRPC_INIT(name); \
	clrpc_dptr* arg = ((_xobject_t*)x##arg)->object; \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	printcl( CL_DEBUG "retain rpc_pool %p",_xobject_rpc_pool(x##arg)); \
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(x##arg),name); \
	cl_int retval; \
	CLRPC_GET(reply,int,retval,&retval); \
	printcl( CL_DEBUG "clrpc_" #name ": retval = %d",retval); \
	return(retval); \
}

#define CLRPC_GENERIC_RELEASE3(name,type,arg) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int name( cl_##type arg ) __alias(clrpc_##name); \
cl_int clrpc_##name( cl_##type x##arg ) \
{ \
	CLRPC_INIT(name); \
	clrpc_dptr* arg = ((_xobject_t*)x##arg)->object; \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	printcl( CL_DEBUG "release rpc_pool %p",_xobject_rpc_pool(x##arg)); \
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(x##arg),name); \
	cl_int retval; \
	CLRPC_GET(reply,int,retval,&retval); \
	printcl( CL_DEBUG "clrpc_" #name ": retval = %d",retval); \
	return(retval); \
}


#define CLRPC_GENERIC_GETINFO3(name,type,arg,infotype) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int name(cl_##type arg, cl_##infotype param_name, \
   size_t param_sz, void* param_val, size_t *param_sz_ret) \
	__alias(clrpc_##name); \
cl_int clrpc_##name(cl_##type x##arg, cl_##infotype param_name, \
	size_t param_sz, void* param_val, size_t *param_sz_ret) \
{ \
	cl_int retval = 0; \
	size_t tmp_param_sz_ret; \
	CLRPC_INIT(name); \
	clrpc_dptr* arg = (clrpc_dptr*) (((_xobject_t*)x##arg)->object); \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	CLRPC_ASSIGN(request,infotype,param_name,param_name); \
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); \
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(x##arg),name); \
	CLRPC_GET(reply,int,retval,&retval); \
	CLRPC_GET(reply,uint,param_sz_ret,&tmp_param_sz_ret); \
	printcl( CL_DEBUG "clrpc_" #name ": *param_sz_ret %ld",tmp_param_sz_ret); \
	param_sz = min(param_sz,tmp_param_sz_ret); \
	void* tmp_param_val = 0; \
	unsigned int tmplen = 0; \
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);\
	memcpy(param_val,tmp_param_val,param_sz); \
	if (param_sz_ret) *param_sz_ret = tmp_param_sz_ret; \
	return(retval); \
}

#define CLRPC_GENERIC_GETINFO4(name,type,arg,infotype) \
cl_int name(cl_##type arg, cl_##infotype param_name, \
   size_t param_sz, void* param_val, size_t *param_sz_ret) \
	__alias(clrpc_##name); \
cl_int clrpc_##name(cl_##type x##arg, cl_##infotype param_name, \
	size_t param_sz, void* param_val, size_t *param_sz_ret) \
{ \
	cl_int retval = 0; \
	size_t tmp_param_sz_ret; \
	CLRPC_INIT(clGetGENERICInfo); \
	clrpc_dptr* arg = (clrpc_dptr*) (((_xobject_t*)x##arg)->object); \
	CLRPC_ASSIGN_DPTR(request,obj,arg); \
	CLRPC_ASSIGN(request,int64,param_name,param_name); \
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); \
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(x##arg),clGetGENERICInfo); \
	CLRPC_GET(reply,int,retval,&retval); \
	CLRPC_GET(reply,uint,param_sz_ret,&tmp_param_sz_ret); \
	printcl( CL_DEBUG "clrpc_clGetGENERICInfo: *param_sz_ret %ld",\
		tmp_param_sz_ret); \
	param_sz = min(param_sz,tmp_param_sz_ret); \
	void* tmp_param_val = 0; \
	unsigned int tmplen = 0; \
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);\
	memcpy(param_val,tmp_param_val,param_sz); \
	if (param_sz_ret) *param_sz_ret = tmp_param_sz_ret; \
	return(retval); \
}


struct xevent_struct {
   clrpc_ptr event;
   void* buf_ptr;
   size_t buf_sz;
};


static struct evrpc_pool*
rpc_pool_with_connection( const char* address, ev_uint16_t port)
{
   struct evhttp_connection *evcon;
   struct evrpc_pool *pool;

   pool = evrpc_pool_new(NULL);
   assert(pool != NULL);

   evcon = evhttp_connection_new(address, port);
   assert(evcon != NULL);

   evrpc_pool_add_connection(pool, evcon);

   return (pool);
}

struct event_base* global_base;
evutil_socket_t global_spair[2] = { -1, -1 };

CLRPC_HEADER(clGetGENERICInfo)
CLRPC_HEADER(clGetPlatformIDs)
CLRPC_HEADER(clGetPlatformInfo)
CLRPC_HEADER(clGetDeviceIDs)
CLRPC_HEADER(clGetDeviceInfo)
CLRPC_HEADER(clCreateContext)
CLRPC_HEADER(clCreateContextFromType)
CLRPC_HEADER(clGetContextInfo)
CLRPC_HEADER(clRetainContext)
CLRPC_HEADER(clReleaseContext)
CLRPC_HEADER(clCreateCommandQueue)
CLRPC_HEADER(clGetCommandQueueInfo)
CLRPC_HEADER(clSetCommandQueueProperty)
CLRPC_HEADER(clRetainCommandqueue)
CLRPC_HEADER(clReleaseCommandqueue)
CLRPC_HEADER(clCreateBuffer)
CLRPC_HEADER(clGetMemObjectInfo)
CLRPC_HEADER(clRetainMemObject)
CLRPC_HEADER(clReleaseMemObject)
CLRPC_HEADER(clEnqueueReadBuffer)
CLRPC_HEADER(clEnqueueWriteBuffer)
CLRPC_HEADER(clEnqueueCopyBuffer)
CLRPC_HEADER(clEnqueueMapBuffer)
CLRPC_HEADER(clGetEventInfo)
CLRPC_HEADER(clGetEventProfilingInfo)
CLRPC_HEADER(clRetainEvent)
CLRPC_HEADER(clReleaseEvent)
CLRPC_HEADER(clCreateProgramWithSource)
CLRPC_HEADER(clCreateProgramWithBinary)
CLRPC_HEADER(clBuildProgram)
CLRPC_HEADER(clGetProgramInfo)
CLRPC_HEADER(clRetainProgram)
CLRPC_HEADER(clReleaseProgram)
CLRPC_HEADER(clCreateKernel)
CLRPC_HEADER(clGetKernelInfo)
CLRPC_HEADER(clRetainKernel)
CLRPC_HEADER(clReleaseKernel)
CLRPC_HEADER(clSetKernelArg)
CLRPC_HEADER(clEnqueueNDRangeKernel)
CLRPC_HEADER(clFlush)
CLRPC_HEADER(clFinish)
CLRPC_HEADER(clWaitForEvents)

CLRPC_GENERATE(clGetGENERICInfo)
CLRPC_GENERATE(clGetPlatformIDs)
CLRPC_GENERATE(clGetPlatformInfo)
CLRPC_GENERATE(clGetDeviceIDs)
CLRPC_GENERATE(clGetDeviceInfo)
CLRPC_GENERATE(clCreateContext)
CLRPC_GENERATE(clCreateContextFromType)
CLRPC_GENERATE(clGetContextInfo)
CLRPC_GENERATE(clRetainContext)
CLRPC_GENERATE(clReleaseContext)
CLRPC_GENERATE(clCreateCommandQueue)
CLRPC_GENERATE(clGetCommandQueueInfo)
CLRPC_GENERATE(clSetCommandQueueProperty)
CLRPC_GENERATE(clRetainCommandQueue)
CLRPC_GENERATE(clReleaseCommandQueue)
CLRPC_GENERATE(clCreateBuffer)
CLRPC_GENERATE(clGetMemObjectInfo)
CLRPC_GENERATE(clRetainMemObject)
CLRPC_GENERATE(clReleaseMemObject)
CLRPC_GENERATE(clEnqueueReadBuffer)
CLRPC_GENERATE(clEnqueueWriteBuffer)
CLRPC_GENERATE(clEnqueueCopyBuffer)
CLRPC_GENERATE(clEnqueueMapBuffer)
CLRPC_GENERATE(clGetEventInfo)
CLRPC_GENERATE(clGetEventProfilingInfo)
CLRPC_GENERATE(clRetainEvent)
CLRPC_GENERATE(clReleaseEvent)
CLRPC_GENERATE(clCreateProgramWithSource)
CLRPC_GENERATE(clCreateProgramWithBinary)
CLRPC_GENERATE(clBuildProgram)
CLRPC_GENERATE(clGetProgramInfo)
CLRPC_GENERATE(clRetainProgram)
CLRPC_GENERATE(clReleaseProgram)
CLRPC_GENERATE(clCreateKernel)
CLRPC_GENERATE(clGetKernelInfo)
CLRPC_GENERATE(clRetainKernel)
CLRPC_GENERATE(clReleaseKernel)
CLRPC_GENERATE(clSetKernelArg)
CLRPC_GENERATE(clEnqueueNDRangeKernel)
CLRPC_GENERATE(clFlush)
CLRPC_GENERATE(clFinish)
CLRPC_GENERATE(clWaitForEvents)

struct cb_struct {
	pthread_mutex_t mtx;
	pthread_cond_t sig;
};

static void* loop( void* parg ) 
{
	printf("i am a loop\n");
   while(1) {
      event_loop(EVLOOP_NONBLOCK);
//      sleep(1);
//      printf("loop\n");
   };
	return ((void*)0);
}

int _clrpc_init = 0;
ev_uint16_t clrpc_port = 0;
struct evhttp* clrpc_http = 0;
struct evrpc_pool* clrpc_pool = 0;
pthread_t clrpc_td;
pthread_attr_t clrpc_td_attr;

struct evrpc_pool** rpc_pools = 0;

unsigned int clrpc_nservers = 0;

//int clrpc_init( unsigned int nservers, clrpc_server_info* servers )
int clrpc_init()
{
	int i;

	int err = evthread_use_pthreads();

	printcl( CL_DEBUG "evthread_use_pthreads returned %d", err);

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

   pthread_attr_init(&clrpc_td_attr);
   pthread_attr_setdetachstate(&clrpc_td_attr,PTHREAD_CREATE_JOINABLE);
	pthread_create(&clrpc_td,&clrpc_td_attr,loop,(void*)0);

	_clrpc_init = 1;

	return(0);
}

int clrpc_connect( unsigned int nservers, clrpc_server_info* servers )
{
	int i,j,n;

	if (!_clrpc_init) clrpc_init();

	n = clrpc_nservers;
	clrpc_nservers += nservers;

	if (n==0) {
		rpc_pools = (struct evrpc_pool**)
			malloc(clrpc_nservers*sizeof(struct evrpc_pool*));
	} else {
		rpc_pools = (struct evrpc_pool**)
			realloc(rpc_pools,clrpc_nservers*sizeof(struct evrpc_pool*));
	}

	for(i=0,j=n;i<nservers;i++,j++) {	
		clrpc_port = servers[i].port;
		clrpc_pool = rpc_pool_with_connection(servers[i].address,servers[i].port);
		rpc_pools[j] = clrpc_pool;
	}

printcl( CL_DEBUG "clrpc_nservers %d",clrpc_nservers);

	return(0);

}

/* XXX this sometimes hangs if no servers running -DAR */
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

cl_int clGetPlatformIDs( cl_uint nplatforms, cl_platform_id* platforms, 
	cl_uint* nplatforms_ret) __alias(clrpc_clGetPlatformIDs);

cl_int
clrpc_clGetPlatformIDs( cl_uint nplatforms, 
	cl_platform_id* platforms, cl_uint* nplatforms_ret)
{
	int n,i;

	cl_int retval = 0;

	CLRPC_ALLOC_DPTR_ARRAY(nplatforms,platforms);

	cl_uint tmp_nplatforms;
	cl_uint total = 0;

	for(n=0;n<clrpc_nservers;n++) {

		printcl( CL_DEBUG "rpc_pools[%d] %p",n,rpc_pools[n]);

		struct _clrpc_clGetPlatformIDs_request* request 
			= _clrpc_clGetPlatformIDs_request_new();

   	struct _clrpc_clGetPlatformIDs_reply* reply 
			= _clrpc_clGetPlatformIDs_reply_new();

		cl_uint tmp_nplatforms_ret;

		CLRPC_ASSIGN(request,uint,nplatforms,nplatforms);

		CLRPC_ASSIGN_DPTR_ARRAY(request,nplatforms,platforms);

		CLRPC_MAKE_REQUEST_WAIT2(rpc_pools[n],clGetPlatformIDs);

		CLRPC_GET(reply,int,retval,&retval);

		CLRPC_GET(reply,uint,nplatforms_ret,&tmp_nplatforms_ret);

		printcl( CL_DEBUG "total %d",total);

		total += tmp_nplatforms_ret;

		tmp_nplatforms = min(nplatforms,tmp_nplatforms_ret);

		size_t len = EVTAG_ARRAY_LEN(reply,platforms);

		printcl( CL_DEBUG "getting array %d %p %d",
			tmp_nplatforms,platforms,len);

		CLRPC_GET_DPTR_ARRAY(reply,tmp_nplatforms,platforms);

		for(i=0;i<tmp_nplatforms;i++) {

			_xobject_create(xplatform,platforms[i],rpc_pools[n]);	

			printcl( CL_DEBUG "at creation platform[%d] is %p",i,platforms[i]);
		}

		if (platforms) {
			nplatforms -= tmp_nplatforms_ret;
			platforms += tmp_nplatforms_ret;
			if (nplatforms < 1) {
				nplatforms = 0;
				platforms = 0;
			}
		}

		printcl( CL_DEBUG 
			"clrpc_clGetPlatformIDs: [%p] *nplatforms_ret = %d\n",
			rpc_pools[n], tmp_nplatforms_ret);

		_clrpc_clGetPlatformIDs_request_free(request);
		_clrpc_clGetPlatformIDs_reply_free(reply);

	}

	if (nplatforms_ret) *nplatforms_ret = total;

	return(retval);
}


/*
 * clGetPlatformInfo
 */

/* XXX why not generic? -DAR */
//CLRPC_GENERIC_GETINFO3(clGetPlatformInfo,platform_id,platform,platform_info)
CLRPC_UNBLOCK_CLICB(clGetPlatformInfo) 
cl_int name(cl_platform_id arg, cl_platform_info param_name, 
   size_t param_sz, void* param_val, size_t *param_sz_ret) 
	__alias(clrpc_clGetPlatformInfo); 
cl_int clrpc_clGetPlatformInfo(cl_platform_id xplatform, 
	cl_platform_info param_name, size_t param_sz, void* param_val, 
	size_t *param_sz_ret) 
{ 
	cl_int retval = 0;
	if (param_name == 999) {
		*(cl_uint*)param_val = 1;
		return(retval);
	}
	CLRPC_INIT(clGetPlatformInfo); 
	clrpc_dptr* platform = (clrpc_dptr*) (((_xobject_t*)xplatform)->object); 
	CLRPC_ASSIGN_DPTR(request,platform,platform); 
	CLRPC_ASSIGN(request,platform_info,param_name,param_name); 
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); 
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xplatform),clGetPlatformInfo); 
	CLRPC_GET(reply,int,retval,&retval); 
	size_t tmp_param_sz;
	CLRPC_GET(reply,uint,param_sz_ret,&tmp_param_sz);
	printcl( CL_DEBUG "clrpc_clGetPlatformInfo: required param_sz%ld",
		tmp_param_sz); 
	param_sz = min(param_sz,tmp_param_sz); 
	void* tmp_param_val = 0; 
	unsigned int tmplen = 0; 
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);
	memcpy(param_val,tmp_param_val,param_sz); 
	printf("%ld:%s\n",param_sz,(char*)param_val); 
	if (param_sz_ret) *param_sz_ret = tmp_param_sz;
	return(retval); 
}



/*
 * clGetDeviceIDs
 */

CLRPC_UNBLOCK_CLICB(clGetDeviceIDs)

cl_int clGetDeviceIDs( cl_platform_id xplatform, cl_device_type devtype,
   cl_uint ndevices, cl_device_id* devices, cl_uint* ndevices_ret)
	__alias(clrpc_clGetDeviceIDs);

cl_int
clrpc_clGetDeviceIDs( cl_platform_id xplatform, cl_device_type devtype,
	cl_uint ndevices, cl_device_id* devices, cl_uint* ndevices_ret)
{
	int i;

	cl_int retval = 0;
	cl_uint tmp_ndevices_ret;

	CLRPC_INIT(clGetDeviceIDs);

	clrpc_dptr* platform = ((_xobject_t*)xplatform)->object;

	CLRPC_ASSIGN_DPTR(request,platform,platform);
	CLRPC_ASSIGN(request,uint,devtype,devtype);

	CLRPC_ASSIGN(request,uint,ndevices,ndevices);

	CLRPC_ALLOC_DPTR_ARRAY(ndevices,devices);
	
	CLRPC_ASSIGN_DPTR_ARRAY(request,ndevices,devices);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xplatform),clGetDeviceIDs);

	CLRPC_GET(reply,int,retval,&retval);

	CLRPC_GET(reply,uint,ndevices_ret,&tmp_ndevices_ret);
	printcl( CL_DEBUG "clrpc_clGetDeviceIDs: *ndevices_ret = %d\n",
		tmp_ndevices_ret);

	printcl( CL_DEBUG 
		"clrpc_clGetDeviceIDs: compare ndevices"
		" *ndevices_ret ARRAY_LEN %d %d %d\n",
		ndevices,tmp_ndevices_ret,EVTAG_ARRAY_LEN(reply, devices));

	ndevices = min(ndevices,tmp_ndevices_ret);

	CLRPC_GET_DPTR_ARRAY(reply,ndevices,devices);

	for(i=0;i<ndevices;i++) {

		_xobject_create(xdevice,devices[i],_xobject_rpc_pool(xplatform));

	}

	for(i=0;i<ndevices;i++) {
		printcl( CL_DEBUG "device pool values %d %p",i,
			((_xobject_t*)devices[i])->rpc_pool);
	}

	CLRPC_FINAL(clGetDeviceIDs);

	if (ndevices_ret) *ndevices_ret = tmp_ndevices_ret;

	return(retval);
}


/*
 * clGetDeviceInfo
 */

CLRPC_GENERIC_GETINFO3(clGetDeviceInfo,device_id,device,device_info)
//CLRPC_UNBLOCK_CLICB(clGetGENERICInfo)
//CLRPC_GENERIC_GETINFO4(clGetDeviceInfo,device_id,device,device_info)


/*
 * clCreateContext
 */

CLRPC_UNBLOCK_CLICB(clCreateContext)

cl_context clCreateContext( const cl_context_properties* prop, cl_uint ndev,
   const cl_device_id* devices, 
	void (*pfn_notify) (const char*, const void*, size_t, void*),
   void* user_data, cl_int* err_ret)
	__alias(clrpc_clCreateContext);

cl_context
clrpc_clCreateContext(
   const cl_context_properties* prop,
   cl_uint ndev,
   const cl_device_id* xdevices, 
   void (*pfn_notify) (const char*, const void*, size_t, void*),
   void* user_data,
   cl_int* err_ret 
)
{
	if (!prop) { *err_ret = CL_INVALID_PLATFORM; return(0); }

	if (ndev==0 || !xdevices) { *err_ret = CL_INVALID_VALUE; return(0); }

	clrpc_dptr* context = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xobject_create(xcontext,context,_xobject_rpc_pool(xdevices[0]));

	CLRPC_INIT(clCreateContext);

	int i,j;

	const cl_context_properties* p = prop;
	while(*p) p += 2;
	int nprop = ((intptr_t)p-(intptr_t)prop)/sizeof(cl_context_properties)/2;
	printcl( CL_DEBUG "nprop %d",nprop);
	clrpc_ptr* xprop = calloc(nprop*3+1,sizeof(clrpc_ptr));
	j=0;
	for(i=0;i<2*nprop;i+=2) {
		printcl( CL_DEBUG "checking prop %d",prop[i]);
		if (prop[i] == CL_CONTEXT_PLATFORM) {
			xprop[j] = (clrpc_ptr)prop[i];
			clrpc_dptr* obj = ((_xobject_t*)prop[i+1])->object;
			xprop[j+1] = obj->local;
			xprop[j+2] = obj->remote;
			printcl( CL_DEBUG "set xprop value %p %p",xprop[j+1],xprop[j+2]);
			j+=3;
		} else {
			printcl( CL_WARNING "skipping unrecognized context property");
		}
	}
	xprop[j] = 0;
	for(i=0;i<j+1;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,xprop,xprop[i]);

	CLRPC_ASSIGN(request,uint,ndev,ndev);

	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,ndev,devices,xdevices);

	_xobject_t* retval = (_xobject_t*)context;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	printcl( CL_DEBUG "here");
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xdevices[0]),clCreateContext);
	printcl( CL_DEBUG "here");

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "context local remote %p %p",
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	printcl( CL_DEBUG "clrpc_clCreateContext: *err_ret = %d\n", *err_ret);

	if (xprop) free(xprop);

	CLRPC_FINAL(clCreateContext);

	return((cl_context)context);
}


/*
 * clCreateContextFromType
 */

CLRPC_UNBLOCK_CLICB(clCreateContextFromType)

cl_context clCreateContextFromType( const cl_context_properties* prop, 
	cl_device_type devtype, 
	void (*pfn_notify) (const char*, const void*, size_t, void*),
   void* user_data, cl_int* err_ret)
	__alias(clrpc_clCreateContextFromType);

cl_context
clrpc_clCreateContextFromType(
   const cl_context_properties* prop,
   cl_device_type devtype,
   void (*pfn_notify) (const char*, const void*, size_t, void*),
   void* user_data,
   cl_int* err_ret 
)
{
	if (!prop) { *err_ret = CL_INVALID_PLATFORM; return(0); }

	clrpc_dptr* context = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	// XXX defer xobject_create until we find the platform 

	CLRPC_INIT(clCreateContextFromType);

	int i,j;

	const cl_context_properties* p = prop;
	while(*p) p += 2;
	int nprop = ((intptr_t)p-(intptr_t)prop)/sizeof(cl_context_properties)/2;
	printcl( CL_DEBUG "nprop %d",nprop);
	clrpc_ptr* xprop = calloc(nprop*3+1,sizeof(clrpc_ptr));
	j=0;
	cl_platform_id xplatform = 0;
	for(i=0;i<2*nprop;i+=2) {
		printcl( CL_DEBUG "checking prop %d",prop[i]);
		if (prop[i] == CL_CONTEXT_PLATFORM) {
			xprop[j] = (clrpc_ptr)prop[i];
			xplatform = (cl_platform_id)prop[i+1];
			clrpc_dptr* obj = ((_xobject_t*)prop[i+1])->object;
			xprop[j+1] = obj->local;
			xprop[j+2] = obj->remote;
			printcl( CL_DEBUG "set xprop value %p %p",xprop[j+1],xprop[j+2]);
			j+=3;
		} else {
			printcl( CL_WARNING "skipping unrecognized context property");
		}
	}
	xprop[j] = 0;
	for(i=0;i<j+1;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,xprop,xprop[i]);

	if (!xplatform) { 
		if (xprop) free(xprop);
		*err_ret = CL_INVALID_PLATFORM; 
		return(0); 
	}

	_xobject_create(xcontext,context,_xobject_rpc_pool(xplatform));

	CLRPC_ASSIGN(request,device_type,devtype,devtype);

	_xobject_t* retval = (_xobject_t*)context;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xplatform),
		clCreateContextFromType);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "context local remote %p %p",
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	printcl( CL_DEBUG "clrpc_clCreateContextFromType: *err_ret = %d\n", 
		*err_ret);

	if (xprop) free(xprop);

	CLRPC_FINAL(clCreateContextFromType);

	return((cl_context)context);
}


/*
 * clGetContextInfo
 */

CLRPC_GENERIC_GETINFO3(clGetContextInfo,context,context,context_info)


/*
 * clRetainContext
 */

CLRPC_GENERIC_RETAIN(clRetainContext,context,context)


/*
 * clReleaseContext
 */

CLRPC_GENERIC_RELEASE3(clReleaseContext,context,context)


/*
 * clCreateCommandQueue
 */

CLRPC_UNBLOCK_CLICB(clCreateCommandQueue)

cl_command_queue clCreateCommandQueue( cl_context context, cl_device_id device,
   cl_command_queue_properties properties, cl_int *err_ret)
	__alias(clrpc_clCreateCommandQueue);

cl_command_queue 
clrpc_clCreateCommandQueue(
	cl_context xcontext,
	cl_device_id xdevice,
	cl_command_queue_properties properties,
	cl_int *err_ret
)
{
	clrpc_dptr* command_queue = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xobject_create(xcommand_queue,command_queue,
		_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateCommandQueue);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,context,xcontext);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,device,xdevice);
	CLRPC_ASSIGN(request,command_queue_properties,properties,properties);

	_xobject_t* retval = (_xobject_t*)command_queue;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),clCreateCommandQueue);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "command_queue local remote %p %p",
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	printcl( CL_DEBUG "clrpc_clCreateCommandQueue: *err_ret = %d\n",*err_ret);

	CLRPC_FINAL(clCreateCommandQueue);

	return((cl_command_queue)command_queue);
}


/*
 * clGetCommandQueueInfo
 */

CLRPC_GENERIC_GETINFO3(clGetCommandQueueInfo,command_queue,command_queue,
	command_queue_info)


/*
 * clSetCommandQueueProperty
 */

CLRPC_UNBLOCK_CLICB(clSetCommandQueueProperty)

cl_int clSetCommandQueueProperty(cl_command_queue command_queue, 
	cl_command_queue_properties properties, cl_bool enable,
	cl_command_queue_properties* old_properties )
	__alias(clrpc_clSetCommandQueueProperty);

cl_int clrpc_clSetCommandQueueProperty(
	cl_command_queue xcommand_queue,
	cl_command_queue_properties properties,
	cl_bool enable,
	cl_command_queue_properties* old_properties
)
{
	cl_int retval = 0;
	cl_command_queue_properties tmp_old_properties;

	CLRPC_INIT(clSetCommandQueueProperty);

	clrpc_dptr* command_queue = (clrpc_dptr*)
		(((_xobject_t*)xcommand_queue)->object);

	CLRPC_ASSIGN_DPTR(request,command_queue,command_queue);
	CLRPC_ASSIGN(request,command_queue_properties,properties,properties);
	CLRPC_ASSIGN(request,bool,enable,enable);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcommand_queue),
		clSetCommandQueueProperty);

	CLRPC_GET(reply,int,retval,&retval);
	CLRPC_GET(reply,uint,old_properties,&tmp_old_properties);

	if (old_properties) *old_properties = tmp_old_properties;

	return(retval);
}


/*
 * clRetainCommandQueue
 */

CLRPC_GENERIC_RETAIN(clRetainCommandQueue,command_queue,command_queue)


/*
 * clReleaseCommandQueue
 */

CLRPC_GENERIC_RELEASE3(clReleaseCommandQueue,command_queue,command_queue)


/*
 * clCreateBuffer
 */

CLRPC_UNBLOCK_CLICB(clCreateBuffer)

cl_mem clCreateBuffer( cl_context context, cl_mem_flags flags, size_t size,
   void *host_ptr, cl_int* err_ret)
	__alias(clrpc_clCreateBuffer);

cl_mem 
clrpc_clCreateBuffer(
	cl_context xcontext,
	cl_mem_flags flags,
	size_t size,
	void *host_ptr,
	cl_int* err_ret
)
{
	clrpc_dptr* buffer = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xobject_create(xbuffer,buffer,_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateBuffer);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,context,xcontext);
	CLRPC_ASSIGN(request,mem_flags,flags,flags);
	EVTAG_ASSIGN(request,size,size);

	if (host_ptr)
		printcl( CL_WARNING "host_ptr not supported, forced to null");

	_xobject_t* retval = (_xobject_t*)buffer;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),clCreateBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "buffer %p local remote %p %p",
		buffer,
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	printcl( CL_DEBUG "clrpc_clCreateBuffer: *err_ret = %d\n",*err_ret);

	CLRPC_FINAL(clCreateBuffer);

	return((cl_mem)buffer);
}


/*
 * clGetMemObjectInfo
 */

CLRPC_GENERIC_GETINFO3(clGetMemObjectInfo,mem,memobj,mem_info)


/*
 * clRetainMemObject
 */

CLRPC_GENERIC_RETAIN(clRetainMemObject,mem,memobj)


/*
 * clReleaseMemObject
 */

CLRPC_GENERIC_RELEASE3(clReleaseMemObject,mem,memobj)


/*
 * clEnqueueReadBuffer
 */

CLRPC_UNBLOCK_CLICB(clEnqueueReadBuffer)

cl_int clEnqueueReadBuffer( cl_command_queue command_queue, cl_mem buffer,
   cl_bool blocking_read, size_t offset, size_t cb, void* ptr,
   cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
   cl_event* pevent)
	__alias(clrpc_clEnqueueReadBuffer);

cl_int
clrpc_clEnqueueReadBuffer (
	cl_command_queue xcommand_queue, 
	cl_mem xbuffer,
   cl_bool blocking_read, 
	size_t offset, 
	size_t cb, 
	void* ptr,
   cl_uint num_events_in_wait_list, 
	const cl_event* xevent_wait_list,
   cl_event* pevent 
)
{
	int i;

	CLRPC_INIT(clEnqueueReadBuffer);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,buffer,xbuffer);
	EVTAG_ASSIGN(request,blocking_read,blocking_read);
	EVTAG_ASSIGN(request,offset,offset);
	EVTAG_ASSIGN(request,cb,cb);
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,num_events_in_wait_list,
		event_wait_list,xevent_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xevent_create(xevent,event,_xobject_rpc_pool(xbuffer));

	if ( blocking_read == CL_FALSE ) {
		xevent->buf_ptr = ptr;
		xevent->buf_sz = cb;
	} else {
		xevent->buf_ptr = 0;
		xevent->buf_sz = 0;
	}

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xbuffer),clEnqueueReadBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	printcl( CL_DEBUG "event local remote %p %p",
		xevent->object->local,xevent->object->remote);

	*pevent = (cl_event)xevent;

	if ( EVTAG_HAS(reply,_bytes) ) {
		printcl( CL_DEBUG "bytes sent back");
	} else {
		printcl( CL_DEBUG "bytes not sent back");
	}

	/* XXX later make this depend on confirmed receipt in event -DAR */
	if ( blocking_read == CL_TRUE ) {
		void* tmp_ptr;
		unsigned int tmp_sz;
		EVTAG_GET_WITH_LEN(reply,_bytes,(unsigned char**)&tmp_ptr,&tmp_sz);
		if (tmp_sz > 0) 
			memcpy(ptr,tmp_ptr,cb);
		int* iptr = (int*)ptr;
		for(i=0;i<32;i++) printf("/%d",iptr[i]); printf("\n");
	}

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clEnqueueReadBuffer: retval = %d",retval);

	CLRPC_FINAL(clEnqueueReadBuffer);

	return(retval);
}


/*
 * clEnqueueWriteBuffer
 */

CLRPC_UNBLOCK_CLICB(clEnqueueWriteBuffer)

cl_int clEnqueueWriteBuffer( cl_command_queue command_queue, cl_mem buffer,
   cl_bool blocking_write, size_t offset, size_t cb, const void *ptr,
   cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
   cl_event* pevent)
	__alias(clrpc_clEnqueueWriteBuffer);

cl_int
clrpc_clEnqueueWriteBuffer (
	cl_command_queue xcommand_queue, 
	cl_mem xbuffer,
   cl_bool blocking_write, 
	size_t offset, 
	size_t cb, 
	const void *ptr,
   cl_uint num_events_in_wait_list, 
	const cl_event* xevent_wait_list,
   cl_event* pevent 
)
{
	CLRPC_INIT(clEnqueueWriteBuffer);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,buffer,xbuffer);
	EVTAG_ASSIGN(request,blocking_write,blocking_write);
	EVTAG_ASSIGN(request,offset,offset);
	EVTAG_ASSIGN(request,cb,cb);
	EVTAG_ASSIGN_WITH_LEN(request,_bytes,ptr,cb);
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,num_events_in_wait_list,
		event_wait_list,xevent_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xevent_create(xevent,event,_xobject_rpc_pool(xbuffer));
	xevent->buf_ptr = 0;
	xevent->buf_sz = 0;

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);

	printcl( CL_DEBUG "_xobject_rpc_pool %p",_xobject_rpc_pool(xbuffer));

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xbuffer),clEnqueueWriteBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	printcl( CL_DEBUG "event local remote %p %p",
		xevent->object->local,xevent->object->remote);

	*pevent = (cl_event)xevent;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clEnqueueWriteBuffer: retval = %d",retval);

	CLRPC_FINAL(clEnqueueWriteBuffer);

	return(retval);
}


/*
 * clEnqueueCopyBuffer
 */

CLRPC_UNBLOCK_CLICB(clEnqueueCopyBuffer)

cl_int clEnqueueCopyBuffer( cl_command_queue command_queue, 
	cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, 
	size_t cb, cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
   cl_event* pevent)
	__alias(clrpc_clEnqueueCopyBuffer);

cl_int
clrpc_clEnqueueCopyBuffer (
	cl_command_queue xcommand_queue, 
	cl_mem xsrc_buffer,
	cl_mem xdst_buffer,
	size_t src_offset, 
	size_t dst_offset, 
	size_t cb, 
   cl_uint num_events_in_wait_list, 
	const cl_event* xevent_wait_list,
   cl_event* pevent 
)
{
	CLRPC_INIT(clEnqueueCopyBuffer);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,src_buffer,xsrc_buffer);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,dst_buffer,xdst_buffer);
	EVTAG_ASSIGN(request,src_offset,src_offset);
	EVTAG_ASSIGN(request,dst_offset,dst_offset);
	EVTAG_ASSIGN(request,cb,cb);
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,num_events_in_wait_list,
		event_wait_list,xevent_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xevent_create(xevent,event,_xobject_rpc_pool(xcommand_queue));
	xevent->buf_ptr = 0;
	xevent->buf_sz = 0;

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);

	printcl( CL_DEBUG "_xobject_rpc_pool %p",_xobject_rpc_pool(xcommand_queue));

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcommand_queue),
		clEnqueueCopyBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	printcl( CL_DEBUG "event local remote %p %p",
		xevent->object->local,xevent->object->remote);

	*pevent = (cl_event)xevent;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clEnqueueCopyBuffer: retval = %d",retval);

	CLRPC_FINAL(clEnqueueCopyBuffer);

	return(retval);
}


/*
 * clEnqueueMapBuffer
 */

CLRPC_UNBLOCK_CLICB(clEnqueueMapBuffer)

void* clEnqueueMapBuffer( cl_command_queue command_queue, 
	cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t cb, cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
   cl_event* pevent, cl_int* err_ret )
	__alias(clrpc_clEnqueueMapBuffer);

void*
clrpc_clEnqueueMapBuffer (
	cl_command_queue xcommand_queue, 
	cl_mem xbuffer,
	cl_bool blocking_map, 
	cl_map_flags map_flags,
	size_t offset, 
	size_t cb, 
   cl_uint num_events_in_wait_list, 
	const cl_event* xevent_wait_list,
   cl_event* pevent,
	cl_int* err_ret
)
{

	int i;

	void* ptr0 = malloc(cb + sizeof(void*) + sizeof(clrpc_dptr));
	void* ptr = ptr0 + sizeof(void*) + sizeof(clrpc_dptr);

	CLRPC_INIT(clEnqueueMapBuffer);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,buffer,xbuffer);

	EVTAG_ASSIGN(request,blocking_map,blocking_map);

	EVTAG_ASSIGN(request,map_flags,map_flags);

	EVTAG_ASSIGN(request,offset,offset);

	EVTAG_ASSIGN(request,cb,cb);

	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);

	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,num_events_in_wait_list,
		event_wait_list,xevent_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xevent_create(xevent,event,_xobject_rpc_pool(xbuffer));

/*XXX still need to rework this conditional */
	if ( blocking_map == CL_FALSE ) {
		xevent->buf_ptr = ptr;
		xevent->buf_sz = cb;
	} else {
		xevent->buf_ptr = 0;
		xevent->buf_sz = 0;
	}

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);

	clrpc_dptr* retval = (clrpc_dptr*)(ptr0 + sizeof(void*));
	CLRPC_ASSIGN_DPTR(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xbuffer),clEnqueueMapBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	printcl( CL_DEBUG "event local remote %p %p",
		xevent->object->local,xevent->object->remote);

	if (pevent) *pevent = (cl_event)xevent;

	cl_int tmp_err_ret;
	CLRPC_GET(reply,int,err_ret,&tmp_err_ret);
	if (err_ret) *err_ret = tmp_err_ret;
	printcl( CL_DEBUG "clrpc_clEnqueueMapBuffer: *err_ret = %d", tmp_err_ret);

	if ( EVTAG_HAS(reply,_bytes) ) {
		printcl( CL_DEBUG "bytes sent back");
	} else {
		printcl( CL_DEBUG "bytes not sent back");
	}

	/* XXX later make this depend on confirmed receipt in event -DAR */
	if ( blocking_map == CL_TRUE ) {
		void* tmp_ptr;
		unsigned int tmp_sz;
		EVTAG_GET_WITH_LEN(reply,_bytes,(unsigned char**)&tmp_ptr,&tmp_sz);
		if (tmp_sz > 0) 
			memcpy(ptr,tmp_ptr,cb);
		int* iptr = (int*)ptr;
		for(i=0;i<32;i++) printf("/%d",iptr[i]); printf("\n");
	}

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->remote);
	
	CLRPC_FINAL(clEnqueueMapBuffer);

	return(ptr);
}



/*
 * clGetEventInfo
 */

CLRPC_GENERIC_GETINFO3(clGetEventInfo,event,event,event_info)


/*
 * clGetEventProfilingInfo
 */

CLRPC_GENERIC_GETINFO3(clGetEventProfilingInfo,event,event,profiling_info)


/*
 * clRetainEvent
 */

CLRPC_GENERIC_RETAIN(clRetainEvent,event,event)


/*
 * clReleaseEvent
 */

CLRPC_UNBLOCK_CLICB(clReleaseEvent)

cl_int clReleaseEvent( cl_event event ) __alias(clrpc_clReleaseEvent);

cl_int clrpc_clReleaseEvent( cl_event xevent ) 
{ 
   CLRPC_INIT(clReleaseEvent);
   CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);
   CLRPC_MAKE_REQUEST_WAIT2(_xevent_rpc_pool(xevent),clReleaseEvent);
	free(((_xevent*)xevent)->object);
	free(xevent);
   cl_int retval;
   CLRPC_GET(reply,int,retval,&retval);
   printcl( CL_DEBUG "clrpc_" "clReleaseEvent" ": retval = %d",retval);
   return(retval);
}



/*
 * clCreateProgramWithSource
 */

CLRPC_UNBLOCK_CLICB(clCreateProgramWithSource)

cl_program clCreateProgramWithSource( cl_context context, cl_uint count,
   const char** strings, const size_t* lengths, cl_int* err_ret)
	__alias(clrpc_clCreateProgramWithSource);

cl_program
clrpc_clCreateProgramWithSource(
	cl_context xcontext, 
	cl_uint count,
   const char** strings, 
	const size_t* lengths, 
	cl_int* err_ret
)
{
	int i;

	clrpc_dptr* program = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xobject_create(xprogram,program,_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateProgramWithSource);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,context,xcontext);
	CLRPC_ASSIGN(request,uint,count,count);
	size_t sz = 0;
	for(i=0;i<count;i++) sz += lengths[i];
	char* buf = (char*)malloc(sz);
	char* p = buf;
	for(i=0;i<count;i++) {
		memcpy(p,strings[i],lengths[i]);
		p += lengths[i];
	}
	EVTAG_ASSIGN_WITH_LEN(request,_bytes,(unsigned char*)buf,sz);

	for(i=0;i<count;i++)
		EVTAG_ARRAY_ADD_VALUE(request,lengths,lengths[i]);

	_xobject_t* retval = (_xobject_t*)program;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),
		clCreateProgramWithSource);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "program local remote %p %p",
		retval->object->local,retval->object->remote);

	cl_int tmp_err_ret;
	CLRPC_GET(reply,int,err_ret,&tmp_err_ret);

	printcl( CL_DEBUG "clrpc_clCreateProgramWithSource: *err_ret = %d", 
		tmp_err_ret);

	if (err_ret) *err_ret = tmp_err_ret;

	if (buf) free(buf);

	CLRPC_FINAL(clCreateProgramWithSource);

	return((cl_program)program);
}


/*
 * clCreateProgramWithBinary
 */

CLRPC_UNBLOCK_CLICB(clCreateProgramWithBinary)

cl_program clCreateProgramWithBinary( cl_context context, cl_uint ndev,
	const cl_device_id* xdevices, const size_t* lengths,
	const unsigned char** binaries, cl_int* status, cl_int* err_ret)
	__alias(clrpc_clCreateProgramWithBinary);

cl_program
clrpc_clCreateProgramWithBinary(
	cl_context xcontext, 
	cl_uint ndev,
	const cl_device_id* xdevices,
	const size_t* lengths, 
   const unsigned char** binaries, 
	cl_int* status,
	cl_int* err_ret
)
{
	int i;

	clrpc_dptr* program = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xobject_create(xprogram,program,_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateProgramWithBinary);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,context,xcontext);

	CLRPC_ASSIGN(request,uint,ndev,ndev);

	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,ndev,devices,xdevices);

	size_t sz = 0;
	for(i=0;i<ndev;i++) sz += lengths[i];
	char* buf = (char*)malloc(sz);
	char* p = buf;
	for(i=0;i<ndev;i++) {
		memcpy(p,binaries[i],lengths[i]);
		p += lengths[i];
	}
	EVTAG_ASSIGN_WITH_LEN(request,_bytes,(unsigned char*)buf,sz);

	for(i=0;i<ndev;i++)
		EVTAG_ARRAY_ADD_VALUE(request,lengths,lengths[i]);

	_xobject_t* retval = (_xobject_t*)program;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),
		clCreateProgramWithBinary);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "program local remote %p %p",
		retval->object->local,retval->object->remote);

	for(i=0;i<ndev;i++) {
		clrpc_int tmp;
		EVTAG_ARRAY_GET(reply,status,i,&tmp);
		if (status) status[i] = tmp;
	}

	cl_int tmp_err_ret;
	CLRPC_GET(reply,int,err_ret,&tmp_err_ret);
	if (err_ret) *err_ret = tmp_err_ret;

	printcl( CL_DEBUG "clrpc_clCreateProgramWithSource: *err_ret = %d", 
		tmp_err_ret);

	if (buf) free(buf);

	CLRPC_FINAL(clCreateProgramWithBinary);

	return((cl_program)program);
}


/*
 * clBuildProgram
 */

CLRPC_UNBLOCK_CLICB(clBuildProgram)

cl_int clBuildProgram( cl_program program, cl_uint ndevices, 
	const cl_device_id* devices, const char* options,
   void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
   void *user_data)
	__alias(clrpc_clBuildProgram);

cl_int
clrpc_clBuildProgram(
	cl_program xprogram, 
	cl_uint ndevices,
	const cl_device_id* xdevices,
	const char* options,
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
	void *user_data
)
{
	CLRPC_INIT(clBuildProgram);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,program,xprogram);
	CLRPC_ASSIGN(request,uint,ndevices,ndevices);
	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,ndevices,devices,xdevices);
	size_t options_sz = (options)? strnlen(options,4096) : 0;
	EVTAG_ASSIGN_WITH_LEN(request,options,(unsigned char*)options,options_sz);
	
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xprogram),clBuildProgram);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clBuildProgram: retval = %d", retval);

	CLRPC_FINAL(clBuildProgram);

	return(retval);
}


/*
 * clGetProgramInfo
 */

CLRPC_GENERIC_GETINFO3(clGetProgramInfo,program,program,program_info)


/*
 * clRetainProgram
 */

CLRPC_GENERIC_RETAIN(clRetainProgram,program,program)


/*
 * clReleaseProgram
 */

CLRPC_GENERIC_RELEASE3(clReleaseProgram,program,program)


/*
 * clUnloadCompiler 
 * note: this call does nothing since it never had a purpose -DAR 
 */

cl_int clUnloadCompiler( void ) __alias(clrpc_clUnloadCompiler);

cl_int
clrpc_clUnloadCompiler( void )
{ return(CL_SUCCESS); }


/*
 * clCreateKernel
 */

CLRPC_UNBLOCK_CLICB(clCreateKernel)

cl_kernel clCreateKernel( cl_program program, const char* kernel_name,
   cl_int* err_ret)
	__alias(clrpc_clCreateKernel);

cl_kernel
clrpc_clCreateKernel(
	cl_program xprogram, 
	const char* kernel_name,
   cl_int* err_ret
)
{
	clrpc_dptr* kernel = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xobject_create(xkernel,kernel,_xobject_rpc_pool(xprogram));

	CLRPC_INIT(clCreateKernel);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,program,xprogram);
	EVTAG_ASSIGN(request,kernel_name,kernel_name);

	_xobject_t* retval = (_xobject_t*)kernel;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xprogram),clCreateKernel);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	printcl( CL_DEBUG "kernel local remote %p %p",
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	printcl( CL_DEBUG "clrpc_clCreateKernel: *err_ret = %d\n", *err_ret);

	CLRPC_FINAL(clCreateKernel);

	return((cl_kernel)kernel);
}


/*
 * clGetKernelInfo
 */

CLRPC_GENERIC_GETINFO3(clGetKernelInfo,kernel,kernel,kernel_info)


/*
 * clRetainKernel
 */

CLRPC_GENERIC_RETAIN(clRetainKernel,kernel,kernel)


/*
 * clReleaseKernel
 */

CLRPC_GENERIC_RELEASE3(clReleaseKernel,kernel,kernel)


/*
 * clSetKernelArg
 */

CLRPC_UNBLOCK_CLICB(clSetKernelArg)

cl_int clSetKernelArg( cl_kernel kernel, cl_uint arg_index, size_t arg_size,
   const void* arg_value)
	__alias(clrpc_clSetKernelArg);

cl_int
clrpc_clSetKernelArg(
	cl_kernel xkernel, 
	cl_uint arg_index,
	size_t arg_size,
   const void* arg_value
)
{
	CLRPC_INIT(clSetKernelArg);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,kernel,xkernel);
	EVTAG_ASSIGN(request,arg_index,arg_index);
	EVTAG_ASSIGN(request,arg_size,arg_size);
	if (arg_value) {
		printcl( CL_DEBUG "clrpc_clSetKernelArg: arg_value %p %p",
			arg_value,*(void**)arg_value);
		 EVTAG_ASSIGN_WITH_LEN(request,arg_value,(unsigned char*)arg_value,
			arg_size);
	}

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xkernel),clSetKernelArg);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clSetKernelArg: retval = %d\n", retval);

	CLRPC_FINAL(clSetKernelArg);

	return(retval);
}


/*
 * clEnqueueNDRangeKernel
 */

CLRPC_UNBLOCK_CLICB(clEnqueueNDRangeKernel)

cl_int clEnqueueNDRangeKernel( cl_command_queue command_queue, cl_kernel kernel,
   cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size,
   cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
   cl_event* pevent)
	__alias(clrpc_clEnqueueNDRangeKernel);

cl_int
clrpc_clEnqueueNDRangeKernel (
	cl_command_queue xcommand_queue, 
	cl_kernel xkernel,
	cl_uint work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size,
   cl_uint num_events_in_wait_list, 
	const cl_event* xevent_wait_list,
   cl_event* pevent 
)
{
	int i;

	CLRPC_INIT(clEnqueueNDRangeKernel);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,kernel,xkernel);
	CLRPC_ASSIGN(request,uint,work_dim,work_dim);

	if (global_work_offset) for(i=0;i<work_dim;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,global_work_offset,global_work_offset[i]);

	if (global_work_size) for(i=0;i<work_dim;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,global_work_size,global_work_size[i]);

	if (local_work_size) for(i=0;i<work_dim;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,local_work_size,local_work_size[i]);
	
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,num_events_in_wait_list,
		event_wait_list,xevent_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	_xevent_create(xevent,event,_xobject_rpc_pool(xkernel));
	xevent->buf_ptr = 0;
	xevent->buf_sz = 0;

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xkernel),clEnqueueNDRangeKernel);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	printcl( CL_DEBUG "event local remote %p %p",
		xevent->object->local,xevent->object->remote);

	*pevent = (cl_event)xevent;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clEnqueueNDRangeKernel: retval = %d",retval);

	CLRPC_FINAL(clEnqueueNDRangeKernel);

	return(retval);
}


/*
 * clFlush
 */

CLRPC_UNBLOCK_CLICB(clFlush)

cl_int clFlush( cl_command_queue command_queue) __alias(clrpc_clFlush);

cl_int
clrpc_clFlush(
	cl_command_queue xcommand_queue
)
{
	CLRPC_INIT(clFlush);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcommand_queue),clFlush);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clFlush: retval = %d\n", retval);

	return(retval);
}


/*
 * clFinish
 */

CLRPC_UNBLOCK_CLICB(clFinish)

cl_int clFinish( cl_command_queue command_queue) __alias(clrpc_clFinish);

cl_int
clrpc_clFinish(
	cl_command_queue xcommand_queue
)
{
	CLRPC_INIT(clFinish);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,command_queue,xcommand_queue);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcommand_queue),clFinish);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clFinish: retval = %d\n", retval);

	printcl( CL_WARNING "clrpc_clFinish: buffer transfers not yet supported!");

	return(retval);
}


/*
 * clWaitForEvents
 */

CLRPC_UNBLOCK_CLICB(clWaitForEvents)

cl_int clWaitForEvents( cl_uint nevents, const cl_event* events)
	__alias(clrpc_clWaitForEvents);

cl_int
clrpc_clWaitForEvents(
	cl_uint nevents, 
	const cl_event* xevents
)
{
	int i;

	if (nevents == 0 || !xevents) return(CL_INVALID_VALUE);

	CLRPC_INIT(clWaitForEvents);

	CLRPC_ASSIGN(request,uint,nevents,nevents);

	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,nevents,events,xevents);

	CLRPC_MAKE_REQUEST_WAIT2(_xevent_rpc_pool(xevents[0]),clWaitForEvents);

	if (EVTAG_HAS(reply,_bytes)) {
		printcl( CL_DEBUG "bytes sent back");
   	void* tmp_buf = 0;
   	unsigned int tmp_len = 0;
   	EVTAG_GET_WITH_LEN(reply,_bytes,(unsigned char**)&tmp_buf,&tmp_len);
		void* tmp_ptr = tmp_buf;
		for(i=0;i<nevents;i++) {

			_xevent* xevent = (_xevent*)xevents[i];

			printcl( CL_DEBUG "[%d] xevent registered: %p %p %ld",
				i,xevent,xevent->buf_ptr,xevent->buf_sz);
			if (xevent->buf_ptr && xevent->buf_sz > 0) {
				memcpy(xevent->buf_ptr,tmp_ptr,xevent->buf_sz);
				tmp_ptr += xevent->buf_sz;
			}
		}
	}

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	printcl( CL_DEBUG "clrpc_clWaitForEvents: retval = %d\n", retval);

	CLRPC_FINAL(clWaitForEvents);

	return(retval);
}



/* XXX these calls are not yet implemented so we make them null for now -DAR */
//void* clrpc_clCreateContextFromType = 0;
//void* clrpc_clRetainContext = 0;
//void* clrpc_clRetainCommandQueue = 0;
//void* clrpc_clGetCommandQueueInfo = 0;
//void* clrpc_clSetCommandQueueProperty = 0;
void* clrpc_clCreateImage2D = 0;
void* clrpc_clCreateImage3D = 0;
//void* clrpc_clRetainMemObject = 0;
void* clrpc_clGetSupportedImageFormats = 0;
//void* clrpc_clGetMemObjectInfo = 0;
void* clrpc_clGetImageInfo = 0;
void* clrpc_clCreateSampler = 0;
void* clrpc_clRetainSampler = 0;
void* clrpc_clReleaseSampler = 0;
void* clrpc_clGetSamplerInfo = 0;
//void* clrpc_clCreateProgramWithBinary = 0;
//void* clrpc_clRetainProgram = 0;
//void* clrpc_clUnloadCompiler = 0;
void* clrpc_clGetProgramBuildInfo = 0;
void* clrpc_clCreateKernelsInProgram = 0;
//void* clrpc_clRetainKernel = 0;
void* clrpc_clGetKernelWorkGroupInfo = 0;
//void* clrpc_clRetainEvent = 0;
//void* clrpc_clGetEventProfilingInfo = 0;
//void* clrpc_clFinish = 0;
//void* clrpc_clEnqueueCopyBuffer = 0;
void* clrpc_clEnqueueReadImage = 0;
void* clrpc_clEnqueueWriteImage = 0;
void* clrpc_clEnqueueCopyImage = 0;
void* clrpc_clEnqueueCopyImageToBuffer = 0;
void* clrpc_clEnqueueCopyBufferToImage = 0;
//void* clrpc_clEnqueueMapBuffer = 0;
void* clrpc_clEnqueueMapImage = 0;
void* clrpc_clEnqueueUnmapMemObject = 0;
void* clrpc_clEnqueueTask = 0;
void* clrpc_clEnqueueNativeKernel = 0;
void* clrpc_clEnqueueMarker = 0;
void* clrpc_clEnqueueWaitForEvents = 0;
void* clrpc_clEnqueueBarrier = 0;
void* clrpc_clCreateFromGLBuffer = 0;
void* clrpc_clCreateFromGLTexture2D = 0;
void* clrpc_clCreateFromGLTexture3D = 0;
void* clrpc_clCreateFromGLRenderbuffer = 0;
void* clrpc_clGetGLObjectInfo = 0;
void* clrpc_clGetGLTextureInfo = 0;
void* clrpc_clEnqueueAcquireGLObjects = 0;
void* clrpc_clEnqueueReleaseGLObjects = 0;



//static void* _rpc_icd_call_vector[] = __set_icd_call_vector(clrpc_,);
//void** __clrpc_call_vector = (void**)_rpc_icd_call_vector;


