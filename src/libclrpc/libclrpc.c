
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

#include <CL/cl.h>

#include "clrpc.h"
#include "util.h"
#include "clrpc_common.h"
#include "clrpc.gen.h"


/* why the hell do i have to define this? -DAR */
#define min(a,b) ((a<b)?a:b)

#define __alias(name) __attribute__((alias(#name)))

typedef struct {
   void* _reserved;
   clrpc_dptr* object;
   struct evrpc_pool* rpc_pool;
} _xobject_t;

#define _xobject_rpc_pool(xobj) ((_xobject_t*)xobj)->rpc_pool

#define _xobject_create(xobj,obj,pool) \
	_xobject_t* xobj = (_xobject_t*)malloc(sizeof(_xobject_t)); \
	do { \
	xobj->object = (clrpc_dptr*)obj; \
	xobj->rpc_pool = pool; \
	xobj->_reserved = (void*)0xabcdabcd; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	obj = (void*)xobj; \
	} while (0)

/*
typedef struct {
	void* _reserved;
	cl_platform_id platform;
	struct evrpc_pool* rpc_pool;
} _xplatform_id;
#define _xplatform_id_rpc_pool(obj) \
	((_xplatform_id*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xplatform_id_create(xobj,obj,pool) \
	_xplatform_id* xobj = (_xplatform_id*)malloc(sizeof(_xplatform_id)); \
	do { \
	xobj->platform = (cl_platform_id)obj; \
	xobj->rpc_pool = pool; \
	xobj->_reserved = 0xabcdabcd; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

/*
typedef struct {
	cl_device_id device;
	struct evrpc_pool* rpc_pool;
} _xdevice_id;
#define _xdevice_id_rpc_pool(obj) \
	((_xdevice_id*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xdevice_id_create(xobj,obj,pool) \
	_xdevice_id* xobj = (_xdevice_id*)malloc(sizeof(_xdevice_id)); \
	do { \
	xobj->device = (cl_device_id)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

/*
typedef struct {
	cl_context context;
	struct evrpc_pool* rpc_pool;
} _xcontext;
#define _xcontext_rpc_pool(obj) \
	((_xcontext*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xcontext_create(xobj,obj,pool) \
	_xcontext* xobj = (_xcontext*)malloc(sizeof(_xcontext)); \
	do { \
	xobj->context = (cl_context)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

/*
typedef struct { 
	cl_command_queue command_queue; 
	struct evrpc_pool* rpc_pool;
} _xcommand_queue;
#define _xcommand_queue_rpc_pool(obj) \
	((_xcommand_queue*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xcommand_queue_create(xobj,obj,pool) \
	_xcommand_queue* xobj = (_xcommand_queue*)malloc(sizeof(_xcommand_queue)); \
	do { \
	xobj->command_queue = (cl_command_queue)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

/*
typedef struct { 
	cl_mem memobj; 
	struct evrpc_pool* rpc_pool; 
} _xmem;
#define _xmem_rpc_pool(obj) ((_xmem*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xmem_create(xobj,obj,pool) \
	_xmem* xobj = (_xmem*)malloc(sizeof(_xmem)); \
	do { \
	xobj->memobj = (cl_mem)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

/*
typedef struct {
	cl_program program;
	struct evrpc_pool* rpc_pool;
} _xprogram;
#define _xprogram_rpc_pool(obj) \
	((_xprogram*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xprogram_create(xobj,obj,pool) \
	_xprogram* xobj = (_xprogram*)malloc(sizeof(_xprogram)); \
	do { \
	xobj->program = (cl_program)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

/*
typedef struct {
	cl_kernel kernel;
	struct evrpc_pool* rpc_pool;
} _xkernel;
#define _xkernel_rpc_pool(obj) \
	((_xkernel*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xkernel_create(xobj,obj,pool) \
	_xkernel* xobj = (_xkernel*)malloc(sizeof(_xkernel)); \
	do { \
	xobj->kernel = (cl_kernel)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)
*/

typedef struct {
	void* _reserved;
//   cl_event event;
	clrpc_dptr* object;
	struct evrpc_pool* rpc_pool;
   void* buf_ptr;
   size_t buf_sz;
} _xevent;

//#define _xevent_rpc_pool(obj) ((_xevent*)((clrpc_dptr*)obj)->local)->rpc_pool
#define _xevent_rpc_pool(xobj) (((_xevent*)xobj)->rpc_pool)

#define _xevent_create(xobj,obj,pool) \
	_xevent* xobj = (_xevent*)malloc(sizeof(_xevent)); \
	do { \
	xobj->object = (clrpc_dptr*)obj; \
	xobj->rpc_pool = pool; \
	((clrpc_dptr*)obj)->local = (clrpc_ptr)xobj; \
	} while (0)


#define CLRPC_UNBLOCK_CLICB(fname) \
static void \
_clrpc_##fname##_clicb(struct evrpc_status* status, \
   struct _clrpc_##fname##_request* request, \
	struct _clrpc_##fname##_reply* reply, void* parg) \
{ \
   xclreport( XCL_DEBUG "_clrpc_" #fname "_clicb"); \
	CLRPC_UNBLOCK(parg); \
} 

#define CLRPC_GENERIC_RELEASE(name,type,arg) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int name( cl_##type arg ) __alias(clrpc_##name); \
cl_int clrpc_##name( cl_##type arg ) \
{ \
	CLRPC_INIT(name); \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	xclreport( XCL_DEBUG "release rpc_pool %p",_x##type##_rpc_pool(arg)); \
	CLRPC_MAKE_REQUEST_WAIT2(_x##type##_rpc_pool(arg),name); \
	cl_int retval; \
	CLRPC_GET(reply,int,retval,&retval); \
	xclreport( XCL_DEBUG "clrpc_" #name ": retval = %d",retval); \
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
	xclreport( XCL_DEBUG "release rpc_pool %p",_xobject_rpc_pool(x##arg)); \
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(x##arg),name); \
	cl_int retval; \
	CLRPC_GET(reply,int,retval,&retval); \
	xclreport( XCL_DEBUG "clrpc_" #name ": retval = %d",retval); \
	return(retval); \
}

#if(0)
#define CLRPC_GENERIC_RELEASE2(name,type,arg) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int clrpc_##name( cl_##type arg ) \
{ \
	CLRPC_INIT(name); \
	_x##type* xarg = (_x##type*)arg; \
	CLRPC_ASSIGN_DPTR(request,arg,xarg->arg); \
	CLRPC_MAKE_REQUEST_WAIT2(xarg->rpc_pool,name); \
	cl_int retval; \
	CLRPC_GET(reply,int,retval,&retval); \
	xclreport( XCL_DEBUG "clrpc_" #name ": retval = %d",retval); \
	CLRPC_FINAL(name); \
	return(retval); \
}
#endif

#define CLRPC_GENERIC_GETINFO(name,type,arg,infotype) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int name(cl_##type arg, cl_##infotype param_name, \
   size_t param_sz, void* param_val, size_t *param_sz_ret) \
	__alias(clrpc_##name); \
cl_int clrpc_##name(cl_##type arg, cl_##infotype param_name, \
	size_t param_sz, void* param_val, size_t *param_sz_ret) \
{ \
	cl_int retval = 0; \
	CLRPC_INIT(name); \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	CLRPC_ASSIGN(request,infotype,param_name,param_name); \
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); \
	CLRPC_MAKE_REQUEST_WAIT2(_x##type##_rpc_pool(arg),name); \
	CLRPC_GET(reply,int,retval,&retval); \
	CLRPC_GET(reply,uint,param_sz_ret,param_sz_ret); \
	xclreport( XCL_DEBUG "clrpc_" #name ": *param_sz_ret %ld",*param_sz_ret); \
	param_sz = min(param_sz,*param_sz_ret); \
	void* tmp_param_val = 0; \
	unsigned int tmplen = 0; \
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);\
	memcpy(param_val,tmp_param_val,param_sz); \
	printf("%ld:%s\n",param_sz,(char*)param_val); \
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
	CLRPC_INIT(name); \
	xclreport( XCL_DEBUG "here"); \
	clrpc_dptr* arg = (clrpc_dptr*) (((_xobject_t*)x##arg)->object); \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	CLRPC_ASSIGN(request,infotype,param_name,param_name); \
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); \
	xclreport( XCL_DEBUG "here %p %p ",arg->local,arg->remote); \
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(x##arg),name); \
	xclreport( XCL_DEBUG "here"); \
	CLRPC_GET(reply,int,retval,&retval); \
	xclreport( XCL_DEBUG "here"); \
	CLRPC_GET(reply,uint,param_sz_ret,param_sz_ret); \
	xclreport( XCL_DEBUG "clrpc_" #name ": *param_sz_ret %ld",*param_sz_ret); \
	param_sz = min(param_sz,*param_sz_ret); \
	void* tmp_param_val = 0; \
	unsigned int tmplen = 0; \
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);\
	memcpy(param_val,tmp_param_val,param_sz); \
	printf("%ld:%s\n",param_sz,(char*)param_val); \
	return(retval); \
}

#if(0)
#define CLRPC_GENERIC_GETINFO2(name,type,arg,infotype) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int clrpc_##name(cl_##type arg, cl_##infotype param_name, \
	size_t param_sz, void* param_val, size_t *param_sz_ret) \
{ \
	cl_int retval = 0; \
	CLRPC_INIT(name); \
	cl_##type arg2 = *(cl_##type*)arg; \
	do { \
   struct dual_ptr* d; \
   EVTAG_GET(request,arg,&d); \
   EVTAG_ASSIGN(d,local,((clrpc_dptr*)arg2)->local); \
   EVTAG_ASSIGN(d,remote,((clrpc_dptr*)arg2)->remote); \
   } while(0); \
	CLRPC_ASSIGN(request,infotype,param_name,param_name); \
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); \
	CLRPC_MAKE_REQUEST_WAIT2(((_x##type*)arg)->rpc_pool,name); \
	CLRPC_GET(reply,int,retval,&retval); \
	CLRPC_GET(reply,uint,param_sz_ret,param_sz_ret); \
	xclreport( XCL_DEBUG "clrpc_" #name ": *param_sz_ret %ld",*param_sz_ret); \
	param_sz = min(param_sz,*param_sz_ret); \
	void* tmp_param_val = 0; \
	unsigned int tmplen = 0; \
	EVTAG_GET_WITH_LEN(reply,param_val,(unsigned char**)&tmp_param_val,&tmplen);\
	memcpy(param_val,tmp_param_val,param_sz); \
	printf("%ld:%s\n",param_sz,(char*)param_val); \
	CLRPC_FINAL(name); \
	return(retval); \
}
#endif


struct xevent_struct {
   clrpc_ptr event;
   void* buf_ptr;
   size_t buf_sz;
};


struct event_base* global_base;
evutil_socket_t global_spair[2] = { -1, -1 };

CLRPC_HEADER(clGetPlatformIDs)
CLRPC_HEADER(clGetPlatformInfo)
CLRPC_HEADER(clGetDeviceIDs)
CLRPC_HEADER(clGetDeviceInfo)
CLRPC_HEADER(clCreateContext)
CLRPC_HEADER(clGetContextInfo)
CLRPC_HEADER(clReleaseContext)
CLRPC_HEADER(clCreateCommandQueue)
CLRPC_HEADER(clReleaseCommandqueue)
CLRPC_HEADER(clCreateBuffer)
CLRPC_HEADER(clReleaseMemObject)
CLRPC_HEADER(clEnqueueReadBuffer)
CLRPC_HEADER(clEnqueueWriteBuffer)
CLRPC_HEADER(clGetEventInfo)
CLRPC_HEADER(clReleaseEvent)
CLRPC_HEADER(clCreateProgramWithSource)
CLRPC_HEADER(clBuildProgram)
CLRPC_HEADER(clGetProgramInfo)
CLRPC_HEADER(clReleaseProgram)
CLRPC_HEADER(clCreateKernel)
CLRPC_HEADER(clGetKernelInfo)
CLRPC_HEADER(clReleaseKernel)
CLRPC_HEADER(clSetKernelArg)
CLRPC_HEADER(clEnqueueNDRangeKernel)
CLRPC_HEADER(clFlush)
CLRPC_HEADER(clWaitForEvents)

CLRPC_GENERATE(clGetPlatformIDs)
CLRPC_GENERATE(clGetPlatformInfo)
CLRPC_GENERATE(clGetDeviceIDs)
CLRPC_GENERATE(clGetDeviceInfo)
CLRPC_GENERATE(clCreateContext)
CLRPC_GENERATE(clGetContextInfo)
CLRPC_GENERATE(clReleaseContext)
CLRPC_GENERATE(clCreateCommandQueue)
CLRPC_GENERATE(clReleaseCommandQueue)
CLRPC_GENERATE(clCreateBuffer)
CLRPC_GENERATE(clReleaseMemObject)
CLRPC_GENERATE(clEnqueueReadBuffer)
CLRPC_GENERATE(clEnqueueWriteBuffer)
CLRPC_GENERATE(clGetEventInfo)
CLRPC_GENERATE(clReleaseEvent)
CLRPC_GENERATE(clCreateProgramWithSource)
CLRPC_GENERATE(clBuildProgram)
CLRPC_GENERATE(clGetProgramInfo)
CLRPC_GENERATE(clReleaseProgram)
CLRPC_GENERATE(clCreateKernel)
CLRPC_GENERATE(clGetKernelInfo)
CLRPC_GENERATE(clReleaseKernel)
CLRPC_GENERATE(clSetKernelArg)
CLRPC_GENERATE(clEnqueueNDRangeKernel)
CLRPC_GENERATE(clFlush)
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

ev_uint16_t clrpc_port = 0;
struct evhttp* clrpc_http = 0;
struct evrpc_pool* clrpc_pool = 0;
pthread_t clrpc_td;
pthread_attr_t clrpc_td_attr;

struct evrpc_pool** rpc_pools = 0;

unsigned int clrpc_nservers = 0;

int clrpc_init( unsigned int nservers, clrpc_server_info* servers )
{
	int i;

	int err = evthread_use_pthreads();

	xclreport( XCL_DEBUG "evthread_use_pthreads returned %d", err);

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

	rpc_pools = (struct evrpc_pool**)malloc(nservers*sizeof(struct evrpc_pool*));

	clrpc_nservers = nservers;

	for(i=0;i<nservers;i++) {	
		clrpc_port = servers[i].port;
		clrpc_pool = rpc_pool_with_connection(servers[i].address,servers[i].port);
		rpc_pools[i] = clrpc_pool;
	}

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

		xclreport( XCL_DEBUG "rpc_pools[%d] %p",n,rpc_pools[n]);

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

		total += tmp_nplatforms_ret;

		tmp_nplatforms = min(nplatforms,tmp_nplatforms_ret);

		size_t len = EVTAG_ARRAY_LEN(reply,platforms);

		xclreport( XCL_DEBUG "getting array %d %p %d",
			tmp_nplatforms,platforms,len);

		CLRPC_GET_DPTR_ARRAY(reply,tmp_nplatforms,platforms);

		for(i=0;i<tmp_nplatforms;i++) {

//			_xplatform_id_create(xplatform,platforms[i],rpc_pools[n]);	
			_xobject_create(xplatform,platforms[i],rpc_pools[n]);	
//			platforms[i] = ((clrpc_dptr*)platforms[i])->local;

			xclreport( XCL_DEBUG "at creation platform[%d] is %p",i,platforms[i]);
		}

		if (platforms) {
			nplatforms -= tmp_nplatforms_ret;
			platforms += tmp_nplatforms_ret;
			if (nplatforms < 1) {
				nplatforms = 0;
				platforms = 0;
			}
		}

		xclreport( XCL_DEBUG 
			"clrpc_clGetPlatformIDs: [%p] *nplatforms_ret = %d\n",
			rpc_pools[n], tmp_nplatforms_ret);

		_clrpc_clGetPlatformIDs_request_free(request);
		_clrpc_clGetPlatformIDs_reply_free(reply);

	}

	*nplatforms_ret = total;

	return(retval);
}


/*
 * clGetPlatformInfo
 */

CLRPC_GENERIC_GETINFO3(clGetPlatformInfo,platform_id,platform,platform_info)


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

	CLRPC_INIT(clGetDeviceIDs);

	clrpc_dptr* platform = ((_xobject_t*)xplatform)->object;

	CLRPC_ASSIGN_DPTR(request,platform,platform);
	CLRPC_ASSIGN(request,uint,devtype,devtype);

	CLRPC_ASSIGN(request,uint,ndevices,ndevices);

	CLRPC_ALLOC_DPTR_ARRAY(ndevices,devices);
	
	CLRPC_ASSIGN_DPTR_ARRAY(request,ndevices,devices);

//	CLRPC_MAKE_REQUEST_WAIT2(_xplatform_id_rpc_pool(platform),clGetDeviceIDs);
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xplatform),clGetDeviceIDs);

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

	for(i=0;i<ndevices;i++) {

//		_xdevice_id_create(xdevice,devices[i],_xplatform_id_rpc_pool(platform));
		_xobject_create(xdevice,devices[i],_xobject_rpc_pool(xplatform));

	}

	for(i=0;i<ndevices;i++) {
		xclreport( XCL_DEBUG "device pool values %d %p",i,
//			((_xdevice_id*)devices[i])->rpc_pool);
			((_xobject_t*)devices[i])->rpc_pool);
	}

	CLRPC_FINAL(clGetDeviceIDs);

	return(retval);
}


/*
 * clGetDeviceInfo
 */

CLRPC_GENERIC_GETINFO3(clGetDeviceInfo,device_id,device,device_info)


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
//	_xcontext_create(xcontext,context,_xobject_rpc_pool(xdevices[0]));
	_xobject_create(xcontext,context,_xobject_rpc_pool(xdevices[0]));

	CLRPC_INIT(clCreateContext);

	int i,j;

	const cl_context_properties* p = prop;
	while(*p) p += 2;
	int nprop = ((intptr_t)p-(intptr_t)prop)/sizeof(cl_context_properties)/2;
	xclreport( XCL_DEBUG "nprop %d",nprop);
	clrpc_ptr* xprop = calloc(nprop*3+1,sizeof(clrpc_ptr));
	j=0;
	for(i=0;i<2*nprop;i+=2) {
		xclreport( XCL_DEBUG "checking prop %d",prop[i]);
		if (prop[i] == CL_CONTEXT_PLATFORM) {
			xprop[j] = (clrpc_ptr)prop[i];
//			xprop[j+1] = ((clrpc_dptr*)prop[i+1])->local;
//			xprop[j+2] = ((clrpc_dptr*)prop[i+1])->remote;
			clrpc_dptr* obj = ((_xobject_t*)prop[i+1])->object;
			xprop[j+1] = obj->local;
			xprop[j+2] = obj->remote;
			xclreport( XCL_DEBUG "set xprop value %p %p",xprop[j+1],xprop[j+2]);
			j+=3;
		} else {
			xclreport( XCL_WARNING "skipping unrecognized context property");
		}
	}
	xprop[j] = 0;
	for(i=0;i<j+1;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,xprop,xprop[i]);

	CLRPC_ASSIGN(request,uint,ndev,ndev);

	CLRPC_ASSIGN_DPTR_ARRAY_FROM_OBJECT(request,ndev,devices,xdevices);

//	clrpc_dptr* retval = context;
	_xobject_t* retval = (_xobject_t*)context;
//	CLRPC_ASSIGN_DPTR(request,retval,retval);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	xclreport( XCL_DEBUG "here");
	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xdevices[0]),clCreateContext);
	xclreport( XCL_DEBUG "here");

//	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&context->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	xclreport( XCL_DEBUG "context local remote %p %p",
//		context->local,context->remote);
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateContext: *err_ret = %d\n", *err_ret);

	if (xprop) free(xprop);

	CLRPC_FINAL(clCreateContext);

	return((cl_context)context);
}


/*
 * clGetContextInfo
 */

CLRPC_GENERIC_GETINFO3(clGetContextInfo,context,context,context_info)


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
//	_xcommand_queue_create(xcommand_queue,command_queue,
	_xobject_create(xcommand_queue,command_queue,
		_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateCommandQueue);

//	clrpc_dptr* context = ((_xobject_t*)xcontext)->object;	
//	clrpc_dptr* device = ((_xobject_t*)xdevice)->object;

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,context,xcontext);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,device,xdevice);
	CLRPC_ASSIGN(request,command_queue_properties,properties,properties);

//	clrpc_dptr* retval = command_queue;
	_xobject_t* retval = (_xobject_t*)command_queue;
//	CLRPC_ASSIGN_DPTR(request,retval,retval);
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),clCreateCommandQueue);

//	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&command_queue->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	xclreport( XCL_DEBUG "command_queue local remote %p %p",
//		command_queue->local,command_queue->remote);
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateCommandQueue: *err_ret = %d\n",*err_ret);

	CLRPC_FINAL(clCreateCommandQueue);

	return((cl_command_queue)command_queue);
}


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
//	_xmem_create(xbuffer,buffer,_xobject_rpc_pool(xcontext));
	_xobject_create(xbuffer,buffer,_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateBuffer);

//	clrpc_dptr* context = ((_xobject_t*)xcontext)->object;

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,context,xcontext);
	CLRPC_ASSIGN(request,mem_flags,flags,flags);
	EVTAG_ASSIGN(request,size,size);

	if (host_ptr)
		xclreport( XCL_WARNING "host_ptr not supported, forced to null");

//	clrpc_dptr* retval = buffer;
	_xobject_t* retval = (_xobject_t*)buffer;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),clCreateBuffer);

//	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&buffer->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	xclreport( XCL_DEBUG "buffer %p local remote %p %p",
		buffer,
//		buffer->local,buffer->remote);
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateBuffer: *err_ret = %d\n",*err_ret);

	CLRPC_FINAL(clCreateBuffer);

	return((cl_mem)buffer);
}


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

//	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&(event)->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	xclreport( XCL_DEBUG "event local remote %p %p",
//		(event)->local,(event)->remote);
		xevent->object->local,xevent->object->remote);

//	*pevent = (cl_event)event;
	*pevent = (cl_event)xevent;

	if ( EVTAG_HAS(reply,_bytes) ) {
		xclreport( XCL_DEBUG "bytes sent back");
	} else {
		xclreport( XCL_DEBUG "bytes not sent back");
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

	xclreport( XCL_DEBUG "clrpc_clEnqueueReadBuffer: retval = %d",retval);

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
//	event->local = (clrpc_ptr)xevent;

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,event,xevent);

	xclreport( XCL_DEBUG "_xobject_rpc_pool %p",_xobject_rpc_pool(xbuffer));

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xbuffer),clEnqueueWriteBuffer);

//	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&(event)->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	xclreport( XCL_DEBUG "event local remote %p %p",
//		(event)->local,(event)->remote);
		xevent->object->local,xevent->object->remote);

//	*pevent = (cl_event)event;
	*pevent = (cl_event)xevent;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clEnqueueWriteBuffer: retval = %d",retval);

	CLRPC_FINAL(clEnqueueWriteBuffer);

	return(retval);
}


/*
 * clGetEventInfo
 */

CLRPC_GENERIC_GETINFO3(clGetEventInfo,event,event,event_info)


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
//	struct xevent_struct* xevent
//		= (struct xevent_struct*)((clrpc_dptr*)event)->local;
	free(((_xevent*)xevent)->object);
	free(xevent);
//	free(event);
   cl_int retval;
   CLRPC_GET(reply,int,retval,&retval);
   xclreport( XCL_DEBUG "clrpc_" "clReleaseEvent" ": retval = %d",retval);
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
//	_xprogram_create(xprogram,program,_xobject_rpc_pool(xcontext));
	_xobject_create(xprogram,program,_xobject_rpc_pool(xcontext));

	CLRPC_INIT(clCreateProgramWithSource);

//	clrpc_dptr* context = ((_xobject_t*)xcontext)->object;

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

//	clrpc_dptr* retval = program;
	_xobject_t* retval = (_xobject_t*)program;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xcontext),
		clCreateProgramWithSource);

//	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&program->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	xclreport( XCL_DEBUG "program local remote %p %p",
//		program->local,program->remote);
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateProgramWithSource: *err_ret = %d", 
		*err_ret);

	if (buf) free(buf);

	CLRPC_FINAL(clCreateProgramWithSource);

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

	xclreport( XCL_DEBUG "clrpc_clBuildProgram: retval = %d", retval);

	CLRPC_FINAL(clBuildProgram);

	return(retval);
}


/*
 * clGetProgramInfo
 */

CLRPC_GENERIC_GETINFO3(clGetProgramInfo,program,program,program_info)


/*
 * clReleaseProgram
 */

CLRPC_GENERIC_RELEASE3(clReleaseProgram,program,program)


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
//	_xkernel_create(xkernel,kernel,_xobject_rpc_pool(xprogram));
	_xobject_create(xkernel,kernel,_xobject_rpc_pool(xprogram));

	CLRPC_INIT(clCreateKernel);

	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,program,xprogram);
	EVTAG_ASSIGN(request,kernel_name,kernel_name);

//	clrpc_dptr* retval = kernel;
	_xobject_t* retval = (_xobject_t*)kernel;
	CLRPC_ASSIGN_DPTR_FROM_OBJECT(request,retval,retval);

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xprogram),clCreateKernel);

//	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&kernel->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&retval->object->remote);
	xclreport( XCL_DEBUG "kernel local remote %p %p",
//		kernel->local,kernel->remote);
		retval->object->local,retval->object->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateKernel: *err_ret = %d\n", *err_ret);

	CLRPC_FINAL(clCreateKernel);

	return((cl_kernel)kernel);
}


/*
 * clGetKernelInfo
 */

CLRPC_GENERIC_GETINFO3(clGetKernelInfo,kernel,kernel,kernel_info)


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
		xclreport( XCL_DEBUG "clrpc_clSetKernelArg: arg_value %p %p",arg_value,*(void**)arg_value);
		 EVTAG_ASSIGN_WITH_LEN(request,arg_value,(unsigned char*)arg_value,
			arg_size);
	}

	CLRPC_MAKE_REQUEST_WAIT2(_xobject_rpc_pool(xkernel),clSetKernelArg);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clSetKernelArg: retval = %d\n", retval);

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

//	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&(event)->remote);
	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&xevent->object->remote);
	xclreport( XCL_DEBUG "event local remote %p %p",
//		(event)->local,(event)->remote);
		xevent->object->local,xevent->object->remote);

//	*pevent = (cl_event)event;
	*pevent = (cl_event)xevent;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clEnqueueNDRangeKernel: retval = %d",retval);

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

	xclreport( XCL_DEBUG "clrpc_clFlush: retval = %d\n", retval);

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
		xclreport( XCL_DEBUG "bytes sent back");
   	void* tmp_buf = 0;
   	unsigned int tmp_len = 0;
   	EVTAG_GET_WITH_LEN(reply,_bytes,(unsigned char**)&tmp_buf,&tmp_len);
		void* tmp_ptr = tmp_buf;
		for(i=0;i<nevents;i++) {

//			_xevent* xevent = (_xevent*)((clrpc_dptr*)events[i])->local;
			_xevent* xevent = (_xevent*)xevents[i];

			xclreport( XCL_DEBUG "[%d] xevent registered: %p %p %ld",
				i,xevent,xevent->buf_ptr,xevent->buf_sz);
			if (xevent->buf_ptr && xevent->buf_sz > 0) {
				memcpy(xevent->buf_ptr,tmp_ptr,xevent->buf_sz);
				tmp_ptr += xevent->buf_sz;
			}
		}
	}

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clWaitForEvents: retval = %d\n", retval);

	CLRPC_FINAL(clWaitForEvents);

	return(retval);
}


