
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <ifaddrs.h>

#define min(a,b) ((a<b)?a:b)

#include <CL/cl.h>

//#include "util.h"
#include "printcl.h"
#include "clrpc_common.h"
#include "clrpc.gen.h"

/* XXX identical to release! -DAR */
#define CLRPC_GENERIC_RETAIN_SVRCB(name,type,arg) \
static void _clrpc_##name##_svrcb( \
	EVRPC_STRUCT(_clrpc_##name)* rpc, void* parg) \
{ \
	printcl( CL_DEBUG "_clrpc_" #name "_svrcb"); \
	CLRPC_SVRCB_INIT(name); \
	type arg; \
	struct dual_ptr* d; \
   EVTAG_GET(request,arg,&d); \
   EVTAG_GET(d,remote,(void*)&arg); \
	cl_int retval = name(arg); \
	CLRPC_ASSIGN(reply, int, retval, retval ); \
   EVRPC_REQUEST_DONE(rpc); \
}

#define CLRPC_GENERIC_RELEASE_SVRCB(name,type,arg) \
static void _clrpc_##name##_svrcb( \
	EVRPC_STRUCT(_clrpc_##name)* rpc, void* parg) \
{ \
	printcl( CL_DEBUG "_clrpc_" #name "_svrcb"); \
	CLRPC_SVRCB_INIT(name); \
	type arg; \
	struct dual_ptr* d; \
   EVTAG_GET(request,arg,&d); \
   EVTAG_GET(d,remote,(void*)&arg); \
	cl_int retval = name(arg); \
	CLRPC_ASSIGN(reply, int, retval, retval ); \
   EVRPC_REQUEST_DONE(rpc); \
}

#define CLRPC_GENERIC_GETINFO_SVRCB(name,type,arg,infotype) \
static void _clrpc_##name##_svrcb( \
	EVRPC_STRUCT(_clrpc_##name)* rpc, void* parg) \
{ \
	printcl( CL_DEBUG "_clrpc_" #name "_svrcb"); \
	CLRPC_SVRCB_INIT(name); \
	cl_##type arg; \
	cl_##infotype param_name; \
	size_t param_sz; \
	void* param_val = 0; \
	size_t param_sz_ret; \
	CLRPC_GET_DPTR_REMOTE(request,type,arg,&arg); \
	CLRPC_GET(request,infotype,param_name,&param_name); \
	CLRPC_GET(request,uint,param_sz,&param_sz); \
	param_val = (param_sz)? calloc(param_sz,1) : 0; \
	printcl( CL_DEBUG "param_sz = %ld", param_sz ); \
	printcl( CL_DEBUG "calling '%s' ( %p, %d, %ld, %p, ...)", #name,arg,param_name,param_sz,param_val ); \
	cl_int retval = name(arg,param_name,param_sz,param_val,&param_sz_ret); \
	printcl( CL_DEBUG "retval %d", retval ); \
	CLRPC_ASSIGN(reply, int, retval, retval ); \
	CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); \
	unsigned int len = min(param_sz,param_sz_ret); \
	EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); \
	if (len) printcl( CL_DEBUG "%d:%s",len,(char*)param_val); \
	if (param_val) free(param_val); \
   EVRPC_REQUEST_DONE(rpc); \
}

#define HIDE(stmt)

#define CLRPC_GENERIC_GETINFO_SVRCB4(name,type,arg,infotype) \
static void _clrpc_##name##_svrcb( \
	EVRPC_STRUCT(_clrpc_clGetGENERICInfo)* rpc, void* parg) \
{ \
	printcl( CL_DEBUG "_clrpc_" #name "_svrcb"); \
	CLRPC_SVRCB_INIT(clGetGENERICInfo); \
	cl_##type arg; \
	cl_##infotype param_name; \
	size_t param_sz; \
	void* param_val = 0; \
	size_t param_sz_ret; \
	clrpc_ptr obj; \
	CLRPC_GET_DPTR_REMOTE(request,type,obj,&obj); \
	arg = (cl_##type)obj; \
	CLRPC_GET(request,infotype,param_name,&param_name); \
	CLRPC_GET(request,uint,param_sz,&param_sz); \
	param_val = calloc(param_sz,1); \
	cl_int retval = name(arg,param_name,param_sz,param_val,&param_sz_ret); \
	CLRPC_ASSIGN(reply, int, retval, retval ); \
	CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); \
	unsigned int len = min(param_sz,param_sz_ret); \
	EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); \
	if (len) printcl( CL_DEBUG "%d:%s",len,(char*)param_val); \
	if (param_val) free(param_val); \
   EVRPC_REQUEST_DONE(rpc); \
}

struct xevent_struct {
	cl_event event;
	void* buf_ptr;
	size_t buf_sz;
	int buf_flag;
};
#define XEVENT_BUF_FREE	0x1


#define CLRPC_XEVENT_CREATE(xevent,event) \
	struct xevent_struct* xevent \
		= (struct xevent_struct*)calloc(1,sizeof(struct xevent_struct)); \
	do { \
		xevent->event = event; \
		printcl( CL_DEBUG "xevent at creation is %p",xevent); \
	} while(0)

#define __xevent_set_buf(xevent,ptr,sz,flag) do { \
	xevent->buf_ptr = ptr; \
	xevent->buf_sz = sz; \
	xevent->buf_flag = flag; \
	} while(0)


struct event_base *global_base;
evutil_socket_t global_spair[2] = { -1, -1 };

static struct evhttp *
http_setup(const char* address, ev_uint16_t port)
{
	struct evhttp *myhttp;
	struct evhttp_bound_socket *sock;

	myhttp = evhttp_new(NULL);
	if (!myhttp) {
		printcl( CL_ERR "Could not start web server");
		exit(-1);
	}

	sock = evhttp_bind_socket_with_handle(myhttp, address, port);
	if (!sock) {
		printcl( CL_ERR "Couldn't open web port");
		exit(-1);
	}

	printcl( CL_INFO "http_setup: address=%s port=%d",address,port);

	return (myhttp);
}

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
CLRPC_HEADER(clRetainCommandQueue)
CLRPC_HEADER(clReleaseCommandQueue)
CLRPC_HEADER(clCreateBuffer)
CLRPC_HEADER(clGetMemObjectInfo)
CLRPC_HEADER(clRetainMemObject)
CLRPC_HEADER(clReleaseMemObject)
CLRPC_HEADER(clEnqueueReadBuffer)
CLRPC_HEADER(clEnqueueWriteBuffer)
CLRPC_HEADER(clEnqueueCopyBuffer)
CLRPC_HEADER(clEnqueueMapBuffer)
CLRPC_HEADER(clEnqueueUnmapMemObject)
CLRPC_HEADER(clGetEventInfo)
CLRPC_HEADER(clGetEventProfilingInfo)
CLRPC_HEADER(clRetainEvent)
CLRPC_HEADER(clReleaseEvent)
CLRPC_HEADER(clCreateProgramWithSource)
CLRPC_HEADER(clCreateProgramWithBinary)
CLRPC_HEADER(clBuildProgram)
CLRPC_HEADER(clGetProgramInfo)
CLRPC_HEADER(clGetProgramBuildInfo)
CLRPC_HEADER(clRetainProgram)
CLRPC_HEADER(clReleaseProgram)
CLRPC_HEADER(clCreateKernel)
CLRPC_HEADER(clCreateKernelsInProgram)
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
CLRPC_GENERATE(clEnqueueUnmapMemObject)
CLRPC_GENERATE(clGetEventInfo)
CLRPC_GENERATE(clGetEventProfilingInfo)
CLRPC_GENERATE(clRetainEvent)
CLRPC_GENERATE(clReleaseEvent)
CLRPC_GENERATE(clCreateProgramWithSource)
CLRPC_GENERATE(clCreateProgramWithBinary)
CLRPC_GENERATE(clBuildProgram)
CLRPC_GENERATE(clGetProgramInfo)
CLRPC_GENERATE(clGetProgramBuildInfo)
CLRPC_GENERATE(clRetainProgram)
CLRPC_GENERATE(clReleaseProgram)
CLRPC_GENERATE(clCreateKernel)
CLRPC_GENERATE(clCreateKernelsInProgram)
CLRPC_GENERATE(clGetKernelInfo)
CLRPC_GENERATE(clRetainKernel)
CLRPC_GENERATE(clReleaseKernel)
CLRPC_GENERATE(clSetKernelArg)
CLRPC_GENERATE(clEnqueueNDRangeKernel)
CLRPC_GENERATE(clFlush)
CLRPC_GENERATE(clFinish)
CLRPC_GENERATE(clWaitForEvents)


static void 
_clrpc_conn_close_cb( 
	struct evhttp_connection* http_conn, void* cbarg 
)
{
	printcl( CL_DEBUG "_clrpc_conn_close_cb");
}


struct _dptr_struct {
   union {
      struct {
         LIST_ENTRY(_dptr_struct) dptr_list;
			clrpc_ptr local;
			clrpc_ptr remote;
      };
      char __pad[32];
   };
};


LIST_HEAD(_dptr_listhead_struct, _dptr_struct) dptr_listhead;

static void
_clrpc_clGetPlatformIDs_svrcb(
	EVRPC_STRUCT(_clrpc_clGetPlatformIDs)* rpc, void* arg)
{
	printcl( CL_DEBUG "_clrpc_clGetPlatformIDs_svrcb");

	CLRPC_SVRCB_INIT(clGetPlatformIDs);

	int i;

// XXX this code makes more sense on context create -DAR
	/* initialization for THIS connection */
   struct evhttp_request* http_req = EVRPC_REQUEST_HTTP(rpc);
   struct evhttp_connection* http_conn
      = evhttp_request_get_connection(http_req);
   evhttp_connection_set_closecb(http_conn,_clrpc_conn_close_cb,0);

	LIST_INIT(&dptr_listhead);

   printcl( CL_DEBUG "_clrpc_clGetPlatformIDs_svrcb %p",http_conn);

	cl_uint nplatforms = 0;
	cl_platform_id* platforms = 0;
	cl_uint nplatforms_ret;

	CLRPC_GET(request,uint,nplatforms,&nplatforms);

	if (nplatforms)
		platforms = (cl_platform_id*)calloc(nplatforms,sizeof(cl_platform_id));

	cl_int retval = clGetPlatformIDs(nplatforms,platforms,&nplatforms_ret);

	printcl( CL_DEBUG "*nplatforms_ret = %d",nplatforms_ret);

	CLRPC_ASSIGN(reply, int, retval, retval );
	CLRPC_ASSIGN(reply, uint, nplatforms_ret, nplatforms_ret );

	cl_uint n = min(nplatforms,nplatforms_ret);

	printcl( CL_DEBUG "n nplatforms nplatforms_ret %d %d %d",
		n,nplatforms,nplatforms_ret);

	for(i=0;i<n;i++)
		printcl( CL_DEBUG "real platforms[%d] = %p",i,platforms[i]);

	CLRPC_DPTR_ARRAY_COPY(request,reply,n,platforms);
	CLRPC_DPTR_ARRAY_SET_REMOTE(reply,n,platforms,platforms);

	if (platforms) 
		free(platforms);

   EVRPC_REQUEST_DONE(rpc);

}


CLRPC_GENERIC_GETINFO_SVRCB(clGetPlatformInfo,platform_id,platform,
	platform_info)


static void
_clrpc_clGetDeviceIDs_svrcb(
	EVRPC_STRUCT(_clrpc_clGetDeviceIDs)* rpc, void* arg)
{
	printcl( CL_DEBUG "_clrpc_clGetDeviceIDs_svrcb");

	CLRPC_SVRCB_INIT(clGetDeviceIDs);

	int i;

	cl_platform_id platform;
	cl_device_type devtype;
	cl_uint ndevices = 0;
	cl_device_id* devices = 0;
	cl_uint ndevices_ret;

	struct dual_ptr* d;
   EVTAG_GET(request,platform,&d);
	EVTAG_GET(d,remote,(void*)&platform);

	CLRPC_GET(request,uint,devtype,&devtype);
	CLRPC_GET(request,uint,ndevices,&ndevices);

	if (ndevices) 
		devices = (cl_device_id*)calloc(ndevices,sizeof(cl_device_id));

	printcl( CL_DEBUG " platform = %p",platform);
	printcl( CL_DEBUG " devtype = %ld",devtype);

	cl_int retval = clGetDeviceIDs(platform,devtype,
		ndevices,devices,&ndevices_ret);

	printcl( CL_DEBUG " retval = %d",retval);
	printcl( CL_DEBUG " ndevices_ret = %d",ndevices_ret);

	CLRPC_ASSIGN(reply, int, retval, retval );
	CLRPC_ASSIGN(reply, uint, ndevices_ret, ndevices_ret );

	cl_uint n = min(ndevices,ndevices_ret);

	for(i=0;i<n;i++)
		printcl( CL_DEBUG "real devices[%d] = %p",i,devices[i]);

	printcl( CL_DEBUG "n devices %d %p",n,devices);

	CLRPC_DPTR_ARRAY_COPY(request,reply,n,devices);
	CLRPC_DPTR_ARRAY_SET_REMOTE(reply,n,devices,devices);

	if (devices) 
		free(devices);

   EVRPC_REQUEST_DONE(rpc);

}


CLRPC_GENERIC_GETINFO_SVRCB(clGetDeviceInfo,device_id,device,device_info)
//CLRPC_GENERIC_GETINFO_SVRCB4(clGetDeviceInfo,device_id,device,device_info)


static void
_clrpc_clCreateContext_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateContext)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateContext_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateContext);

	int i,j;

	cl_context_properties* prop;
	cl_uint ndev;
	cl_device_id* devices;
	void* pfn_notify = 0;
	void* user_Data = 0;
	cl_int err_ret;

	
	size_t xprop_len = EVTAG_ARRAY_LEN(request,xprop);

	if (xprop_len%3 != 1) 
		printcl( CL_ERR "xprop_len incorrect");

	int nprop = xprop_len/3;
	prop = (cl_context_properties*)malloc(nprop*2+1);
	for(i=0,j=0;i<2*nprop;i+=2,j+=3) {
		clrpc_ptr tmp;
		EVTAG_ARRAY_GET(request,xprop,j,&tmp);
		prop[i] = (cl_context_properties)tmp;
		EVTAG_ARRAY_GET(request,xprop,j+2,&tmp);
		prop[i+1] = (cl_context_properties)tmp;
		printcl( CL_DEBUG "prop[] %d 0x%x",(int)prop[i],(int)prop[i+1]);
	}
	prop[i] = 0;

	CLRPC_GET(request,uint,ndev,&ndev);

	size_t devices_len = EVTAG_ARRAY_LEN(request,devices);
	if (devices_len != ndev) 
		printcl( CL_ERR "devices_len not equal to ndev");
	devices = (cl_device_id*)calloc(ndev,sizeof(cl_device_id));
	for(i=0;i<ndev;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, devices, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		devices[i] = (cl_device_id)remote;
		printcl( CL_DEBUG "devices[] %p",devices[i]);
	}

	cl_context context = clCreateContext(prop,ndev,devices,pfn_notify,
		user_Data,&err_ret);

	printcl( CL_DEBUG "remote context = %p",context);

	clrpc_dptr retval;
	struct dual_ptr* d;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)context;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

	if (prop) free(prop);
	if (devices) free(devices);
	
   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clCreateContextFromType_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateContextFromType)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateContextFromType_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateContextFromType);

	int i,j;

	cl_context_properties* prop;
//	cl_uint ndev;
//	cl_device_id* devices;
	cl_device_type devtype;
	void* pfn_notify = 0;
	void* user_Data = 0;
	cl_int err_ret;

	
	size_t xprop_len = EVTAG_ARRAY_LEN(request,xprop);

	if (xprop_len%3 != 1) 
		printcl( CL_ERR "xprop_len incorrect");

	int nprop = xprop_len/3;
	prop = (cl_context_properties*)malloc(nprop*2+1);
	for(i=0,j=0;i<2*nprop;i+=2,j+=3) {
		clrpc_ptr tmp;
		EVTAG_ARRAY_GET(request,xprop,j,&tmp);
		prop[i] = (cl_context_properties)tmp;
		EVTAG_ARRAY_GET(request,xprop,j+2,&tmp);
		prop[i+1] = (cl_context_properties)tmp;
		printcl( CL_DEBUG "prop[] %d 0x%x",(int)prop[i],(int)prop[i+1]);
	}
	prop[i] = 0;

//	CLRPC_GET(request,uint,ndev,&ndev);
//
//	size_t devices_len = EVTAG_ARRAY_LEN(request,devices);
//	if (devices_len != ndev) 
//		printcl( CL_ERR "devices_len not equal to ndev");
//	devices = (cl_device_id*)calloc(ndev,sizeof(cl_device_id));
//	for(i=0;i<ndev;i++) {
//		struct dual_ptr* d;
//		clrpc_ptr local,remote;
//		EVTAG_ARRAY_GET(request, devices, i, &d);
//		EVTAG_GET(d,local,&local);
//		EVTAG_GET(d,remote,&remote);
//		devices[i] = (cl_device_id)remote;
//		printcl( CL_DEBUG "devices[] %p",devices[i]);
//	}
	CLRPC_GET(request,uint,devtype,&devtype);

	cl_context context = clCreateContextFromType(prop,devtype,pfn_notify,
		user_Data,&err_ret);

	printcl( CL_DEBUG "remote context = %p",context);

	clrpc_dptr retval;
	struct dual_ptr* d;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)context;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

	if (prop) free(prop);
//	if (devices) free(devices);
	
   EVRPC_REQUEST_DONE(rpc);
}


CLRPC_GENERIC_GETINFO_SVRCB(clGetContextInfo,context,context,context_info)


CLRPC_GENERIC_RETAIN_SVRCB(clRetainContext,cl_context,context)


CLRPC_GENERIC_RELEASE_SVRCB(clReleaseContext,cl_context,context)


static void
_clrpc_clCreateCommandQueue_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateCommandQueue)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateCommandQueue_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateCommandQueue);

	cl_context context;
	cl_device_id device;
	cl_context_properties properties;
	cl_int err_ret;

	CLRPC_GET(request,command_queue_properties,properties,&properties);

	struct dual_ptr* d;

   EVTAG_GET(request,context,&d);
   EVTAG_GET(d,remote,(void*)&context);
   EVTAG_GET(request,device,&d);
   EVTAG_GET(d,remote,(void*)&device);

	printcl( CL_DEBUG "context device properties %p %p %d",
		context,device,(int)properties);

	cl_command_queue command_queue 
		= clCreateCommandQueue(context,device,properties,&err_ret);

	printcl( CL_DEBUG "remote command_queue = %p",command_queue);

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)command_queue;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

   EVRPC_REQUEST_DONE(rpc);
}


CLRPC_GENERIC_GETINFO_SVRCB(clGetCommandQueueInfo,command_queue,command_queue,
	command_queue_info)


static void _clrpc_clSetCommandQueueProperty_svrcb(
	EVRPC_STRUCT(_clrpc_clSetCommandQueueProperty)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clSetCommandQueueProperty_svrcb");

	CLRPC_SVRCB_INIT(clSetCommandQueueProperty);

	cl_command_queue command_queue;

	cl_command_queue_properties properties;
	cl_bool enable;
	cl_command_queue_properties old_properties;

	CLRPC_GET_DPTR_REMOTE(request,command_queue,command_queue,&command_queue);

	CLRPC_GET(request,command_queue_properties,properties,&properties);
	CLRPC_GET(request,bool,enable,&enable);

	cl_int retval = clSetCommandQueueProperty(command_queue,properties,
		enable,&old_properties);

	CLRPC_ASSIGN(reply, int, retval, retval );
	CLRPC_ASSIGN(reply, uint64, old_properties, old_properties );

   EVRPC_REQUEST_DONE(rpc);

}


CLRPC_GENERIC_RETAIN_SVRCB(clRetainCommandQueue,cl_command_queue,
	command_queue)


CLRPC_GENERIC_RELEASE_SVRCB(clReleaseCommandQueue,cl_command_queue,
	command_queue)


static void
_clrpc_clCreateBuffer_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateBuffer)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateBuffer_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateBuffer);

	cl_context context;
	cl_mem_flags flags;
	size_t size;
	cl_int err_ret;

	struct dual_ptr* d;

   EVTAG_GET(request,context,&d);
   EVTAG_GET(d,remote,(void*)&context);

	CLRPC_GET(request,mem_flags,flags,&flags);

	EVTAG_GET(request,size,&size);

	printcl( CL_DEBUG "context flags size %p %ld %ld",
		context,flags,size);

	cl_mem buffer
		= clCreateBuffer(context,flags,size,0,&err_ret);

	printcl( CL_DEBUG "remote membuf = %p",buffer);

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)buffer;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

	struct _dptr_struct* dptr 
		= (struct _dptr_struct*)malloc(sizeof(struct _dptr_struct));
	dptr->local = retval.local;
	dptr->remote = retval.remote;
	LIST_INSERT_HEAD(&dptr_listhead, dptr, dptr_list);

   EVRPC_REQUEST_DONE(rpc);
}


CLRPC_GENERIC_GETINFO_SVRCB(clGetMemObjectInfo,mem,memobj,mem_info)


CLRPC_GENERIC_RETAIN_SVRCB(clRetainMemObject,cl_mem,memobj)


CLRPC_GENERIC_RELEASE_SVRCB(clReleaseMemObject,cl_mem,memobj)


static void
_clrpc_clEnqueueReadBuffer_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueReadBuffer)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clEnqueueReadBuffer_svrcb");
	
	CLRPC_SVRCB_INIT(clEnqueueReadBuffer);

	cl_command_queue command_queue;
	cl_mem buffer;
	cl_bool blocking_read;
	size_t offset;
	size_t cb;
	void* ptr = 0;
	cl_uint num_events_in_wait_list;
	cl_event* event_wait_list = 0;
	cl_event event;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

   EVTAG_GET(request,buffer,&d);
   EVTAG_GET(d,remote,(void*)&buffer);

	CLRPC_GET(request,bool,blocking_read,&blocking_read);
	EVTAG_GET(request,offset,&offset);
	EVTAG_GET(request,cb,&cb);

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		printcl( CL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		printcl( CL_DEBUG "clEnqueueReadBuffer xevent remote = %p",remote);
		event_wait_list[i] = (cl_event)((struct xevent_struct*)remote)->event;
		printcl( CL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	printcl( CL_DEBUG "clEnqueueReadBuffer %p %p %d %ld %ld %p %d %p %p",
		command_queue,buffer,blocking_read,offset,cb,
		ptr,num_events_in_wait_list,event_wait_list,&event);

	ptr = malloc(cb);

	cl_int retval = clEnqueueReadBuffer(command_queue,buffer,blocking_read,
		offset,cb,ptr,num_events_in_wait_list,event_wait_list,&event);

	printcl( CL_DEBUG "remote membuf = %p",buffer);

	printcl( CL_DEBUG "retval %d", retval);

	CLRPC_XEVENT_CREATE(xevent,event);
	__xevent_set_buf(xevent,ptr,cb,XEVENT_BUF_FREE);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)xevent;
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, retval, retval );

	if ( blocking_read == CL_TRUE ) {
		EVTAG_ASSIGN_WITH_LEN(reply,_bytes,ptr,cb);
	} else {
	}

   EVRPC_REQUEST_DONE(rpc);
}

static void
_clrpc_clEnqueueWriteBuffer_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueWriteBuffer)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clEnqueueWriteBuffer_svrcb");
	
	CLRPC_SVRCB_INIT(clEnqueueWriteBuffer);

	cl_command_queue command_queue;
	cl_mem buffer;
	cl_bool blocking_write;
	size_t offset;
	size_t cb;
	void* ptr;
	cl_uint num_events_in_wait_list;
	cl_event* event_wait_list = 0;
	cl_event event;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

   EVTAG_GET(request,buffer,&d);
   EVTAG_GET(d,remote,(void*)&buffer);

	CLRPC_GET(request,bool,blocking_write,&blocking_write);
	EVTAG_GET(request,offset,&offset);
	EVTAG_GET(request,cb,&cb);

	void* tmp_ptr = 0;
   unsigned int tmp_cb = 0;
   EVTAG_GET_WITH_LEN(request,_bytes,(unsigned char**)&tmp_ptr,&tmp_cb);
	printcl( CL_DEBUG "COMPARE cb %ld %d",cb,tmp_cb);


//	ptr = tmp_ptr; /* XXX simplify later -DAR */
	ptr = malloc(tmp_cb);
	memcpy(ptr,tmp_ptr,tmp_cb);

	printcl( CL_DEBUG "memcpy %p %p %d",ptr,tmp_ptr,tmp_cb);

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		printcl( CL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		event_wait_list[i] = (cl_event)((struct xevent_struct*)remote)->event;
		printcl( CL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	printcl( CL_DEBUG "clEnqueueWriteBuffer %p %p %d %ld %ld %p %d %p %p",
		command_queue,buffer,blocking_write,offset,cb,
		ptr,num_events_in_wait_list,event_wait_list,&event);

//	int* iptr = (int*)ptr;
//	for(i=0;i<32;i++) printf("%d/",iptr[i]); printf("\n");

	cl_int retval = clEnqueueWriteBuffer(command_queue,buffer,blocking_write,
		offset,cb,ptr,num_events_in_wait_list,event_wait_list,&event);

	printcl( CL_DEBUG "remote membuf = %p",buffer);

	printcl( CL_DEBUG "retval %d", retval);

//	CLRPC_XEVENT_CREATE(xevent,event,0,0);
	CLRPC_XEVENT_CREATE(xevent,event);
	__xevent_set_buf(xevent,ptr,0,XEVENT_BUF_FREE);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)xevent;
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clEnqueueCopyBuffer_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueCopyBuffer)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clEnqueueCopyBuffer_svrcb");
	
	CLRPC_SVRCB_INIT(clEnqueueCopyBuffer);

	cl_command_queue command_queue;
	cl_mem src_buffer;
	cl_mem dst_buffer;
	size_t src_offset;
	size_t dst_offset;
	size_t cb;
	cl_uint num_events_in_wait_list;
	cl_event* event_wait_list = 0;
	cl_event event;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

   EVTAG_GET(request,src_buffer,&d);
   EVTAG_GET(d,remote,(void*)&src_buffer);

   EVTAG_GET(request,dst_buffer,&d);
   EVTAG_GET(d,remote,(void*)&dst_buffer);

	EVTAG_GET(request,src_offset,&src_offset);
	EVTAG_GET(request,dst_offset,&dst_offset);
	EVTAG_GET(request,cb,&cb);

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		printcl( CL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		event_wait_list[i] = (cl_event)((struct xevent_struct*)remote)->event;
		printcl( CL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	cl_int retval = clEnqueueCopyBuffer(command_queue,src_buffer,dst_buffer,
		src_offset,dst_offset,cb,num_events_in_wait_list,event_wait_list,&event);

	printcl( CL_DEBUG "remote src membuf = %p",src_buffer);
	printcl( CL_DEBUG "remote dst membuf = %p",dst_buffer);

	printcl( CL_DEBUG "retval %d", retval);

	CLRPC_XEVENT_CREATE(xevent,event);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)xevent;
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clEnqueueMapBuffer_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueMapBuffer)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clEnqueueMapBuffer_svrcb");
	
	CLRPC_SVRCB_INIT(clEnqueueMapBuffer);

	cl_command_queue command_queue;
	cl_mem buffer;
	cl_bool blocking_map;
	cl_map_flags map_flags;
	size_t offset;
	size_t cb;
//	void* ptr = 0;
	cl_uint num_events_in_wait_list;
	cl_event* event_wait_list = 0;
	cl_event event;
	cl_int err_ret;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

   EVTAG_GET(request,buffer,&d);
   EVTAG_GET(d,remote,(void*)&buffer);

	CLRPC_GET(request,bool,blocking_map,&blocking_map);

	CLRPC_GET(request,map_flags,map_flags,&map_flags);

	EVTAG_GET(request,offset,&offset);

	EVTAG_GET(request,cb,&cb);

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		printcl( CL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		printcl( CL_DEBUG "clEnqueueMapBuffer xevent remote = %p",remote);
		event_wait_list[i] = (cl_event)((struct xevent_struct*)remote)->event;
		printcl( CL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	printcl( CL_DEBUG "clEnqueueMapBuffer %p %p %d %d %ld %ld %d %p %p",
		command_queue,buffer,blocking_map,map_flags,offset,cb,
		num_events_in_wait_list,event_wait_list,&event);

//	ptr = malloc(cb);

	void* ptr = clEnqueueMapBuffer(command_queue,buffer,blocking_map,map_flags,
		offset,cb,num_events_in_wait_list,event_wait_list,&event,&err_ret);

	printcl( CL_DEBUG "remote membuf = %p",buffer);

	printcl( CL_DEBUG "err_ret %d", err_ret);

	CLRPC_XEVENT_CREATE(xevent,event);
	if ( blocking_map == CL_FALSE ) 
		__xevent_set_buf(xevent,ptr,cb,0);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)xevent;
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)ptr;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	if ( blocking_map == CL_TRUE ) {
		EVTAG_ASSIGN_WITH_LEN(reply,_bytes,ptr,cb);
	}

   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clEnqueueUnmapMemObject_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueUnmapMemObject)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clEnqueueUnmapMemObject_svrcb");
	
	CLRPC_SVRCB_INIT(clEnqueueUnmapMemObject);

	cl_command_queue command_queue;
	cl_mem memobj;
	void* ptr = 0;
	cl_uint num_events_in_wait_list;
	cl_event* event_wait_list = 0;
	cl_event event;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

   EVTAG_GET(request,memobj,&d);
   EVTAG_GET(d,remote,(void*)&memobj);

   EVTAG_GET(request,mapped_ptr,&d);
   EVTAG_GET(d,remote,(void*)&ptr);

	void* tmp_ptr = 0;
   unsigned int tmp_cb = 0;
   EVTAG_GET_WITH_LEN(request,_bytes,(unsigned char**)&tmp_ptr,&tmp_cb);
	printcl( CL_DEBUG "writing back %d bytes",tmp_cb);
	memcpy(ptr,tmp_ptr,tmp_cb);
	printcl( CL_DEBUG "memcpy %p %p %d",ptr,tmp_ptr,tmp_cb);

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		printcl( CL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		printcl( CL_DEBUG "clEnqueueUnmapMemObject xevent remote = %p",remote);
		event_wait_list[i] = (cl_event)((struct xevent_struct*)remote)->event;
		printcl( CL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	printcl( CL_DEBUG "clEnqueueUnmapMemObject %p %p %p %d %p %p",
		command_queue,memobj,ptr,
		num_events_in_wait_list,event_wait_list,&event);

	cl_int retval = clEnqueueUnmapMemObject(command_queue,memobj,ptr,
		num_events_in_wait_list,event_wait_list,&event);

	printcl( CL_DEBUG "remote memobj = %p",memobj);

	printcl( CL_DEBUG "retval %d", retval );

	CLRPC_XEVENT_CREATE(xevent,event);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)xevent;
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}


static void _clrpc_clGetEventInfo_svrcb( 
	EVRPC_STRUCT(_clrpc_clGetEventInfo)* rpc, void* parg) 
{ 
	printcl( CL_DEBUG "_clrpc_" "clGetEventInfo" "_svrcb"); 
	CLRPC_SVRCB_INIT(clGetEventInfo);
	struct xevent_struct* xevent;
	cl_event_info param_name; 
	size_t param_sz; 
	void* param_val = 0; 
	size_t param_sz_ret; 

	struct dual_ptr* d;
	EVTAG_GET(request,event,&d);
	clrpc_ptr tmp;
	EVTAG_GET(d,remote,(void*)&tmp);
	xevent = (struct xevent_struct*)tmp;

	CLRPC_GET(request,event_info,param_name,&param_name); 
	CLRPC_GET(request,uint,param_sz,&param_sz); 
	param_val = calloc(param_sz,1); 
	cl_int retval = clGetEventInfo(xevent->event,param_name,param_sz,param_val,
		&param_sz_ret); 
	CLRPC_ASSIGN(reply, int, retval, retval ); 
	CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); 
	unsigned int len = min(param_sz,param_sz_ret); 
	EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); 
	if (len) printcl( CL_DEBUG "%d:%s",len,(char*)param_val); 
	if (param_val) free(param_val); 
   EVRPC_REQUEST_DONE(rpc); 
}


static void _clrpc_clGetEventProfilingInfo_svrcb( 
	EVRPC_STRUCT(_clrpc_clGetEventProfilingInfo)* rpc, void* parg) 
{ 
	printcl( CL_DEBUG "_clrpc_" "clGetEventProfilingInfo" "_svrcb"); 
	CLRPC_SVRCB_INIT(clGetEventProfilingInfo);
	struct xevent_struct* xevent;
	cl_profiling_info param_name; 
	size_t param_sz; 
	void* param_val = 0; 
	size_t param_sz_ret; 

	struct dual_ptr* d;
	EVTAG_GET(request,event,&d);
	clrpc_ptr tmp;
	EVTAG_GET(d,remote,(void*)&tmp);
	xevent = (struct xevent_struct*)tmp;

	CLRPC_GET(request,event_info,param_name,&param_name); 
	CLRPC_GET(request,uint,param_sz,&param_sz); 
	param_val = calloc(param_sz,1); 
	cl_int retval = clGetEventProfilingInfo(xevent->event,param_name,param_sz,
		param_val, &param_sz_ret); 
	CLRPC_ASSIGN(reply, int, retval, retval ); 
	CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); 
	unsigned int len = min(param_sz,param_sz_ret); 
	EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); 
	if (len) printcl( CL_DEBUG "%d:%s",len,(char*)param_val); 
	if (param_val) free(param_val); 
   EVRPC_REQUEST_DONE(rpc); 
}


static void _clrpc_clRetainEvent_svrcb( 
	EVRPC_STRUCT(_clrpc_clRetainEvent)* rpc, void* parg) 
{ 
	printcl( CL_DEBUG "_clrpc_" "clRetainEvent" "_svrcb");
	CLRPC_SVRCB_INIT(clRetainEvent);
	struct xevent_struct* xevent;
	struct dual_ptr* d; 
   EVTAG_GET(request,event,&d); 
   EVTAG_GET(d,remote,(void*)&xevent); 
	cl_int retval = clRetainEvent(xevent->event);
	if (xevent->buf_ptr) free(xevent->buf_ptr);
	free(xevent);
	CLRPC_ASSIGN(reply, int, retval, retval ); 
   EVRPC_REQUEST_DONE(rpc); 
}


static void _clrpc_clReleaseEvent_svrcb( 
	EVRPC_STRUCT(_clrpc_clReleaseEvent)* rpc, void* parg) 
{ 
	printcl( CL_DEBUG "_clrpc_" "clReleaseEvent" "_svrcb");
	CLRPC_SVRCB_INIT(clReleaseEvent);
	struct xevent_struct* xevent;
	struct dual_ptr* d; 
   EVTAG_GET(request,event,&d); 
   EVTAG_GET(d,remote,(void*)&xevent); 
	cl_int retval = clReleaseEvent(xevent->event);
	if (xevent->buf_ptr && ((xevent->buf_flag)&XEVENT_BUF_FREE) ) 
		free(xevent->buf_ptr);
	free(xevent);
	CLRPC_ASSIGN(reply, int, retval, retval ); 
   EVRPC_REQUEST_DONE(rpc); 
}


static void
_clrpc_clCreateProgramWithSource_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateProgramWithSource)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateProgramWithSource_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateProgramWithSource);

	int i;

	cl_context context;
	cl_uint count;
	char** strings;
	size_t* lengths;
	cl_int err_ret;

	struct dual_ptr* d;

   EVTAG_GET(request,context,&d);
   EVTAG_GET(d,remote,(void*)&context);

	CLRPC_GET(request,uint,count,&count);
	strings = (char**)calloc(count,sizeof(char*));
	lengths = (size_t*)calloc(count,sizeof(size_t));

	size_t lengths_len = EVTAG_ARRAY_LEN(request,lengths);
	if (lengths_len != count) 
		printcl( CL_ERR "lengths_len not equal to count");

	size_t sz = 0;
	for(i=0;i<count;i++) {
		clrpc_uint tmp;
		EVTAG_ARRAY_GET(request,lengths,i,&tmp);
		lengths[i] = tmp;
		sz += tmp;
	}

	char* tmp_buf = 0;
   unsigned int tmp_sz = 0;
   EVTAG_GET_WITH_LEN(request,_bytes,(unsigned char**)&tmp_buf,&tmp_sz);
	printcl( CL_DEBUG "COMPARE sz %ld %d",sz,tmp_sz);
	char* p = tmp_buf;
	printcl( CL_DEBUG "|%s|", p);
	for(i=0;i<count;i++) {
		strings[i] = p; 
		p += lengths[i];
	}

	cl_program program = clCreateProgramWithSource(context,count,
		(const char**)strings, lengths,&err_ret);

	printcl( CL_DEBUG "remote program = %p",program);

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)program;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

	if (strings) free(strings);
	if (lengths) free(lengths);
	
   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clCreateProgramWithBinary_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateProgramWithBinary)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateProgramWithBinary_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateProgramWithBinary);

	int i;

	cl_context context;
	cl_uint ndev;
	cl_device_id* devices;
	size_t* lengths;
	unsigned char** binaries;
	cl_int* status;
	cl_int err_ret;

	struct dual_ptr* d;

   EVTAG_GET(request,context,&d);
   EVTAG_GET(d,remote,(void*)&context);

	CLRPC_GET(request,uint,ndev,&ndev);
	size_t devices_len = EVTAG_ARRAY_LEN(request,devices);
	if (devices_len != ndev) 
		printcl( CL_ERR "devices_len not equal to ndev");
	devices = (cl_device_id*)calloc(ndev,sizeof(cl_device_id));
	for(i=0;i<ndev;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, devices, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		devices[i] = (cl_device_id)remote;
		printcl( CL_DEBUG "devices[] %p",devices[i]);
	}

	binaries = (unsigned char**)calloc(ndev,sizeof(char*));
	lengths = (size_t*)calloc(ndev,sizeof(size_t));
	status = (cl_int*)calloc(ndev,sizeof(cl_int));

	size_t lengths_len = EVTAG_ARRAY_LEN(request,lengths);
	if (lengths_len != ndev) 
		printcl( CL_ERR "lengths_len not equal to count");

	size_t sz = 0;
	for(i=0;i<ndev;i++) {
		clrpc_uint tmp;
		EVTAG_ARRAY_GET(request,lengths,i,&tmp);
		lengths[i] = tmp;
		sz += tmp;
	}

	char* tmp_buf = 0;
   unsigned int tmp_sz = 0;
   EVTAG_GET_WITH_LEN(request,_bytes,(unsigned char**)&tmp_buf,&tmp_sz);
	printcl( CL_DEBUG "COMPARE sz %ld %d",sz,tmp_sz);
	char* p = tmp_buf;
	printcl( CL_DEBUG "|%s|", p);
	for(i=0;i<ndev;i++) {
		binaries[i] = p; 
		p += lengths[i];
	}

	cl_program program = clCreateProgramWithBinary(context,ndev,devices,
		lengths,(const unsigned char**)binaries,status,&err_ret);

	printcl( CL_DEBUG "remote program = %p",program);

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)program;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

   for(i=0;i<ndev;i++)
      EVTAG_ARRAY_ADD_VALUE(reply,status,status[i]);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

	if (binaries) free(binaries);
	if (lengths) free(lengths);
	if (status) free(status);
	
   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clBuildProgram_svrcb(
	EVRPC_STRUCT(_clrpc_clBuildProgram)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clBuildProgram_svrcb");
	
	CLRPC_SVRCB_INIT(clBuildProgram);

	int i;

	cl_program program;
	cl_uint ndevices;
	cl_device_id* devices = 0;
	char* options;
	void* pfn_notify = 0;
	void* user_data = 0;
	
	struct dual_ptr* d;

   EVTAG_GET(request,program,&d);
   EVTAG_GET(d,remote,(void*)&program);

	CLRPC_GET(request,uint,ndevices,&ndevices);

	size_t devices_len = EVTAG_ARRAY_LEN(request,devices);
	printcl( CL_DEBUG "compare ndevices devices_len %d %d",devices,devices_len);

	if (devices_len != ndevices) 
		printcl( CL_ERR "devices_len not equal to ndevices");

	if (ndevices) {
		devices = (cl_device_id*)calloc(ndevices,sizeof(cl_device_id));
		for(i=0;i<ndevices;i++) {
			struct dual_ptr* d;
			clrpc_ptr local,remote;
			EVTAG_ARRAY_GET(request, devices, i, &d);
			EVTAG_GET(d,local,&local);
			EVTAG_GET(d,remote,&remote);
			devices[i] = (cl_device_id)remote;
			printcl( CL_DEBUG "devices[] %p",devices[i]);
		}
	}

   unsigned int options_sz = 0;
   EVTAG_GET_WITH_LEN(request,options,(unsigned char**)&options,&options_sz);
	if (options_sz == 0) options = 0;
	printcl( CL_DEBUG "%d:|%s|",options_sz,options);

   printcl( CL_DEBUG "program %p",program);
   printcl( CL_DEBUG "ndevices %d",ndevices);
   printcl( CL_DEBUG "devices %p",devices);
   printcl( CL_DEBUG "options %p",options);
   printcl( CL_DEBUG "pfn_notify %p",pfn_notify);
   printcl( CL_DEBUG "user_data %p",user_data);

	cl_int retval = clBuildProgram(program,ndevices,devices,options,
		pfn_notify,user_data);

	printcl( CL_DEBUG "retval %d",retval);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}

/* XXX this call is a mess as is the original -DAR */
static void _clrpc_clGetProgramInfo_svrcb( 
	EVRPC_STRUCT(_clrpc_clGetProgramInfo)* rpc, void* parg
) 
{ 
	printcl( CL_DEBUG "_clrpc_clGetProgramInfo_svrcb"); 
	CLRPC_SVRCB_INIT(clGetProgramInfo); 

	int err;

	cl_program program; 
	cl_program_info param_name; 
	size_t param_sz; 
	void* param_val = 0; 
	size_t param_sz_ret; 
	CLRPC_GET_DPTR_REMOTE(request,program,program,&program); 
	CLRPC_GET(request,program_info,param_name,&param_name); 

//	if (param_name == CL_PROGRAM_NUM_DEVICES) 
//		printcl( CL_DEBUG "CL_PROGRAM_NUM_DEVICES"); 
//	else if (param_name == CL_PROGRAM_BINARY_SIZES) 
//		printcl( CL_DEBUG "CL_PROGRAM_BINARY_SIZES"); 
//	else if (param_name == CL_PROGRAM_BINARIES) 
//		printcl( CL_DEBUG "CL_PROGRAM_BINARIES"); 

	CLRPC_GET(request,uint,param_sz,&param_sz); 
	printcl( CL_DEBUG "param_sz = %ld", param_sz ); 

	if (param_name == CL_PROGRAM_BINARIES) {

		param_val = (param_sz)? calloc(param_sz,1) : 0; 

		printcl( CL_DEBUG "CL_PROGRAM_BINARIES requires special steps");
	
		cl_uint __ndev;
		err = clGetProgramInfo( program, CL_PROGRAM_NUM_DEVICES,
         sizeof(cl_uint), &__ndev, 0 );

		size_t* __bin_sizes = (size_t*)malloc( sizeof(size_t)*__ndev );
		err = clGetProgramInfo(program,CL_PROGRAM_BINARY_SIZES,
			sizeof(size_t)*__ndev,__bin_sizes,0);

		size_t __sz = 0;
		int __i;
		for(__i=0;__i<__ndev;__i++) __sz += __bin_sizes[__i];
		void* __bin = calloc(__sz,1);
		char** __bins = (char**)malloc( sizeof(char*)*__ndev );
		__bins[0] = __bin;
		for(__i=1;__i<__ndev;__i++) __bins[__i] = __bin + __bin_sizes[__i-1];

		cl_int retval = clGetProgramInfo(program,CL_PROGRAM_BINARIES,param_sz,
			__bins, &param_sz_ret);
	
		printcl( CL_DEBUG "retval %d", retval ); 
		CLRPC_ASSIGN(reply, int, retval, retval ); 
		CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); 
		unsigned int len = min(param_sz,param_sz_ret); 

//		EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); 
		printcl( CL_DEBUG "param_val assigned" ); 
		EVTAG_ASSIGN_WITH_LEN(reply,param_val,((void*)__bin_sizes),len); 
		printcl( CL_DEBUG "assign %p %d",__bin,__sz ); 
		EVTAG_ASSIGN_WITH_LEN(reply,bin,__bin,(unsigned int)__sz);

		printcl( CL_DEBUG "sending back %d bytes", __sz);

		char* pc = (char*)__bins[0];
		printcl( CL_DEBUG "tag: %d %d %d %d",(int)pc[0],(int)pc[1],(int)pc[2],(int)pc[3]);

		free(__bins);
		free(__bin);
		free(__bin_sizes);

	} else {

		param_val = (param_sz)? calloc(param_sz,1) : 0; 

		printcl( CL_DEBUG "calling '%s' ( %p, %d, %ld, %p, ...)", 
		"clGetProgramInfo",program,param_name,param_sz,param_val ); 

		cl_int retval = clGetProgramInfo(program,param_name,param_sz,param_val,
			&param_sz_ret);

		printcl( CL_DEBUG "retval %d", retval ); 
		CLRPC_ASSIGN(reply, int, retval, retval ); 
		CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); 
		unsigned int len = min(param_sz,param_sz_ret); 
		EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); 
		if (len) printcl( CL_DEBUG "%d:%s",len,(char*)param_val); 
		if (param_val) free(param_val); 

	}

   EVRPC_REQUEST_DONE(rpc); 
}


static void _clrpc_clGetProgramBuildInfo_svrcb( 
	EVRPC_STRUCT(_clrpc_clGetProgramBuildInfo)* rpc, void* parg) 
{ 
	printcl( CL_DEBUG "_clrpc_clGetProgramBuildInfo_svrcb"); 
	CLRPC_SVRCB_INIT(clGetProgramBuildInfo);
	cl_program program; 
	cl_device_id device; 
	cl_program_build_info param_name; 
	size_t param_sz; 
	void* param_val = 0; 
	size_t param_sz_ret; 
	CLRPC_GET_DPTR_REMOTE(request,program,program,&program); 
	CLRPC_GET_DPTR_REMOTE(request,device,device,&device); 
	CLRPC_GET(request,program_build_info,param_name,&param_name); 
	CLRPC_GET(request,uint,param_sz,&param_sz); 
	param_val = (param_sz)? calloc(param_sz,1) : 0; 
	printcl( CL_DEBUG "param_sz = %ld", param_sz ); 
	printcl( CL_DEBUG "calling '%s' ( %p, %d, %ld, %p, ...)", 
		"clGetProgramBuildInfo",program,param_name,param_sz,param_val ); 
	cl_int retval = clGetProgramBuildInfo(program,device,param_name,param_sz,
		param_val,&param_sz_ret); 
	printcl( CL_DEBUG "retval %d", retval ); 
	CLRPC_ASSIGN(reply, int, retval, retval ); 
	CLRPC_ASSIGN(reply, uint64, param_sz_ret, param_sz_ret ); 
	unsigned int len = min(param_sz,param_sz_ret); 
	EVTAG_ASSIGN_WITH_LEN(reply,param_val,param_val,len); 
	if (len) printcl( CL_DEBUG "%d:%s",len,(char*)param_val); 
	if (param_val) free(param_val); 
   EVRPC_REQUEST_DONE(rpc); 
}


CLRPC_GENERIC_RETAIN_SVRCB(clRetainProgram,cl_program,program)

CLRPC_GENERIC_RELEASE_SVRCB(clReleaseProgram,cl_program,program)



static void
_clrpc_clCreateKernel_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateKernel)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clCreateKernel_svrcb");
	
	CLRPC_SVRCB_INIT(clCreateKernel);

	cl_program program;
	char* kernel_name;
	cl_int err_ret;

	struct dual_ptr* d;

   EVTAG_GET(request,program,&d);
   EVTAG_GET(d,remote,(void*)&program);

	EVTAG_GET(request,kernel_name,&kernel_name);

	cl_kernel kernel = clCreateKernel(program,kernel_name,&err_ret);

	printcl( CL_DEBUG "remote kernel = %p",kernel);

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)kernel;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

   EVRPC_REQUEST_DONE(rpc);
}

static void
_clrpc_clCreateKernelsInProgram_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateKernelsInProgram)* rpc, void* arg)
{
	printcl( CL_DEBUG "_clrpc_clCreateKernelsInProgram_svrcb");

	CLRPC_SVRCB_INIT(clCreateKernelsInProgram);

	int i;

	cl_program program;
	cl_uint nkernels = 0;
	cl_kernel* kernels = 0;
	cl_uint nkernels_ret;

	struct dual_ptr* d;
   EVTAG_GET(request,program,&d);
	EVTAG_GET(d,remote,(void*)&program);

	CLRPC_GET(request,uint,nkernels,&nkernels);

	if (nkernels) 
		kernels = (cl_kernel*)calloc(nkernels,sizeof(cl_kernel));

	printcl( CL_DEBUG " program = %p",program);

	cl_int retval = clCreateKernelsInProgram(program,
		nkernels,kernels,&nkernels_ret);

	printcl( CL_DEBUG " retval = %d",retval);
	printcl( CL_DEBUG " nkernels_ret = %d",nkernels_ret);

	CLRPC_ASSIGN(reply, int, retval, retval );
	CLRPC_ASSIGN(reply, uint, nkernels_ret, nkernels_ret );

	cl_uint n = min(nkernels,nkernels_ret);

	for(i=0;i<n;i++)
		printcl( CL_DEBUG "real kernels[%d] = %p",i,kernels[i]);

	printcl( CL_DEBUG "n kernels %d %p",n,kernels);

	CLRPC_DPTR_ARRAY_COPY(request,reply,n,kernels);
	CLRPC_DPTR_ARRAY_SET_REMOTE(reply,n,kernels,kernels);

	if (kernels) 
		free(kernels);

   EVRPC_REQUEST_DONE(rpc);

}


CLRPC_GENERIC_GETINFO_SVRCB(clGetKernelInfo,kernel,kernel,kernel_info)

CLRPC_GENERIC_RETAIN_SVRCB(clRetainKernel,cl_kernel,kernel)

CLRPC_GENERIC_RELEASE_SVRCB(clReleaseKernel,cl_kernel,kernel)

static void
_clrpc_clSetKernelArg_svrcb(
	EVRPC_STRUCT(_clrpc_clSetKernelArg)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clSetKernelArg_svrcb");
	
	CLRPC_SVRCB_INIT(clSetKernelArg);

	cl_kernel kernel;
	cl_uint arg_index;
	size_t arg_size;
	void* arg_value = 0;

	struct dual_ptr* d;

   EVTAG_GET(request,kernel,&d);
   EVTAG_GET(d,remote,(void*)&kernel);

	CLRPC_GET(request,uint,arg_index,&arg_index);
	EVTAG_GET(request,arg_size,&arg_size);

	printcl( CL_DEBUG "index size %d %ld", arg_index, arg_size);

	if (arg_size > 0) {
   	unsigned int tmp_sz = 0;
   	EVTAG_GET_WITH_LEN(request,arg_value,(unsigned char**)&arg_value,&tmp_sz);
		if (tmp_sz != arg_size) 
			printcl( CL_ERR "array len not equal to arg_size");
	}

	void* local_ptr = *(void**)arg_value;
	printcl( CL_DEBUG "local_ptr %p",local_ptr);

	struct _dptr_struct* dptr;
	for (
      dptr = dptr_listhead.lh_first; dptr != 0;
      dptr = dptr->dptr_list.le_next
   ) {
		if (dptr->local == (clrpc_ptr)local_ptr) {
			printcl( CL_DEBUG "found local, remap remote %p -> %p",
				(void*)dptr->local,(void*)dptr->remote);
			*(void**)arg_value = (void*)dptr->remote;
		}
	}

	cl_int retval = clSetKernelArg(kernel,arg_index,arg_size,arg_value);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}

static void
_clrpc_clEnqueueNDRangeKernel_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueNDRangeKernel)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clEnqueueNDRangeKernel_svrcb");
	
	CLRPC_SVRCB_INIT(clEnqueueNDRangeKernel);

	cl_command_queue command_queue;
	cl_kernel kernel;
	cl_uint work_dim;
	size_t* global_work_offset = 0;
	size_t global_work_size[3];
	size_t local_work_size[3];
	cl_uint num_events_in_wait_list;
	cl_event* event_wait_list = 0;
	cl_event event;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

   EVTAG_GET(request,kernel,&d);
   EVTAG_GET(d,remote,(void*)&kernel);

	CLRPC_GET(request,uint,work_dim,&work_dim);
	clrpc_uint64 tmp;
	tmp = EVTAG_ARRAY_LEN(request,global_work_offset);
	if (tmp == work_dim) {
		global_work_offset = (size_t*)malloc(work_dim*sizeof(size_t));
		for(i=0;i<work_dim;i++) {
			EVTAG_ARRAY_GET(request,global_work_offset,i,&tmp);
			global_work_offset[i] = (size_t)tmp;
		}
	} else if (tmp != 0) {
		printcl( CL_ERR "invalid array len for global_work_offset");
	}
	tmp = EVTAG_ARRAY_LEN(request,global_work_size);
	if (tmp == work_dim) for(i=0;i<work_dim;i++) {
		EVTAG_ARRAY_GET(request,global_work_size,i,&tmp);
		global_work_size[i] = (size_t)tmp;
	} else {
		printcl( CL_ERR "array len != work_dim");
	}
	tmp = EVTAG_ARRAY_LEN(request,local_work_size);
	if (tmp == work_dim) for(i=0;i<work_dim;i++) {
		EVTAG_ARRAY_GET(request,local_work_size,i,&tmp);
		local_work_size[i] = (size_t)tmp;
	} else {
		printcl( CL_ERR "array len != work_dim");
	}

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		printcl( CL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		event_wait_list[i] = (cl_event)((struct xevent_struct*)remote)->event;
		printcl( CL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	cl_int retval = clEnqueueNDRangeKernel(command_queue,kernel,
		work_dim,global_work_offset,global_work_size,local_work_size,
		num_events_in_wait_list,event_wait_list,&event);

	printcl( CL_DEBUG "remote kernel = %p",kernel);

	printcl( CL_DEBUG "retval %d", retval);

	CLRPC_XEVENT_CREATE(xevent,event);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)xevent;
	printcl( CL_DEBUG "remote xevent = %p",xevent);
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, retval, retval );

	if (global_work_offset) free(global_work_offset);

   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clFlush_svrcb(
	EVRPC_STRUCT(_clrpc_clFlush)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clFlush_svrcb");
	
	CLRPC_SVRCB_INIT(clFlush);

	cl_command_queue command_queue;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

	cl_int retval = clFlush(command_queue);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}


static void
_clrpc_clFinish_svrcb(
	EVRPC_STRUCT(_clrpc_clFinish)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clFinish_svrcb");
	
	CLRPC_SVRCB_INIT(clFinish);

	cl_command_queue command_queue;

	struct dual_ptr* d;

   EVTAG_GET(request,command_queue,&d);
   EVTAG_GET(d,remote,(void*)&command_queue);

	cl_int retval = clFinish(command_queue);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}



static void
_clrpc_clWaitForEvents_svrcb(
	EVRPC_STRUCT(_clrpc_clWaitForEvents)* rpc, void* parg)
{
	int i;

	printcl( CL_DEBUG "_clrpc_clWaitForEvents_svrcb");
	
	CLRPC_SVRCB_INIT(clWaitForEvents);

	cl_uint nevents;
	cl_event* events = 0;

   CLRPC_GET(request,uint,nevents,&nevents);

	size_t events_len = EVTAG_ARRAY_LEN(request,events);
	if (events_len != nevents) 
		printcl( CL_ERR "events_len not equal to nevents");
	events = (cl_event*)calloc(nevents,sizeof(cl_event));
	struct xevent_struct** xevents 
		= (struct xevent_struct**)calloc(nevents,sizeof(struct xevent_struct*));
	size_t sz = 0;
	for(i=0;i<nevents;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, events, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		xevents[i] = (struct xevent_struct*)remote;
		events[i] = xevents[i]->event;
		sz += xevents[i]->buf_sz;
	}

	cl_int retval = clWaitForEvents(nevents,events);

	printcl( CL_DEBUG "post-event buffer size %ld", sz );
	if (sz > 0) {
		void* tmp_buf = malloc(sz);
		void* tmp_ptr = tmp_buf;
		for(i=0;i<nevents;i++) {
			if (xevents[i]->buf_sz > 0 && xevents[i]->buf_ptr) {
				memcpy(tmp_ptr,xevents[i]->buf_ptr,xevents[i]->buf_sz);
				int* ptmp = (int*)tmp_ptr;
//				printf("EVENT %d:",i);
				int ii; for(ii=0;ii<10;ii++) 
//				printf("*%d",ptmp[ii]); printf("\n"); fflush(stdout);
				tmp_ptr += xevents[i]->buf_sz;
			}
		}
		EVTAG_ASSIGN_WITH_LEN(reply,_bytes,tmp_buf,sz);
		if (tmp_buf) free(tmp_buf);
	}

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}


/**********
 **********
 **********/

void _clrpc_buf_eventcb(struct bufferevent *bev, short events, void *ptr)
{
	printcl( CL_DEBUG "_clrpc_buf_eventcb");

    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
            something here, like start reading or writing. */
    } else if (events & BEV_EVENT_ERROR) {
         /* An error occured while connecting. */
    }
}

static void
clrpc_server( const char* address, ev_uint16_t port )
{
	struct evhttp* http = http_setup(address,port);
	struct evrpc_base* base = evrpc_init(http);
	struct evrpc_pool* pool = 0;

	CLRPC_REGISTER(clGetPlatformIDs);
	CLRPC_REGISTER(clGetPlatformInfo);
	CLRPC_REGISTER(clGetDeviceIDs);
	CLRPC_REGISTER(clGetDeviceInfo);
	CLRPC_REGISTER(clCreateContext);
	CLRPC_REGISTER(clCreateContextFromType);
	CLRPC_REGISTER(clGetContextInfo);
	CLRPC_REGISTER(clRetainContext);
	CLRPC_REGISTER(clReleaseContext);
	CLRPC_REGISTER(clCreateCommandQueue);
	CLRPC_REGISTER(clGetCommandQueueInfo);
	CLRPC_REGISTER(clSetCommandQueueProperty);
	CLRPC_REGISTER(clRetainCommandQueue);
	CLRPC_REGISTER(clReleaseCommandQueue);
	CLRPC_REGISTER(clCreateBuffer);
	CLRPC_REGISTER(clGetMemObjectInfo);
	CLRPC_REGISTER(clRetainMemObject);
	CLRPC_REGISTER(clReleaseMemObject);
	CLRPC_REGISTER(clEnqueueReadBuffer);
	CLRPC_REGISTER(clEnqueueWriteBuffer);
	CLRPC_REGISTER(clEnqueueCopyBuffer);
	CLRPC_REGISTER(clEnqueueMapBuffer);
	CLRPC_REGISTER(clGetEventInfo);
	CLRPC_REGISTER(clGetEventProfilingInfo);
	CLRPC_REGISTER(clRetainEvent);
	CLRPC_REGISTER(clReleaseEvent);
	CLRPC_REGISTER(clCreateProgramWithSource);
	CLRPC_REGISTER(clCreateProgramWithBinary);
	CLRPC_REGISTER(clBuildProgram);
	CLRPC_REGISTER(clGetProgramInfo);
	CLRPC_REGISTER(clGetProgramBuildInfo);
	CLRPC_REGISTER(clRetainProgram);
	CLRPC_REGISTER(clReleaseProgram);
	CLRPC_REGISTER(clCreateKernel);
	CLRPC_REGISTER(clCreateKernelsInProgram);
	CLRPC_REGISTER(clGetKernelInfo);
	CLRPC_REGISTER(clReleaseKernel);
	CLRPC_REGISTER(clRetainKernel);
	CLRPC_REGISTER(clSetKernelArg);
	CLRPC_REGISTER(clEnqueueNDRangeKernel);
	CLRPC_REGISTER(clFlush);
	CLRPC_REGISTER(clFinish);
	CLRPC_REGISTER(clWaitForEvents);

	struct bufferevent *bev;
   bev = bufferevent_socket_new(global_base, -1, BEV_OPT_CLOSE_ON_FREE);
   bufferevent_setcb(bev, NULL, NULL, _clrpc_buf_eventcb, NULL);
	bufferevent_enable(bev, EV_READ|EV_WRITE);

	pool = rpc_pool_with_connection(address,port);

	event_dispatch();

}


int
main(int argc, const char **argv)
{
	cl_uint n;

	char default_address[NI_MAXHOST] = "127.0.0.1";
	char if_address[NI_MAXHOST];
//	const char* address = default_address;
	const char* address = 0;
	const char* ifname = 0;
	ev_uint16_t port = 2112;

	n = 1;
   while (n < argc) {

		const char* arg = argv[n++];
		
		if (!strcmp(arg,"-a")) {
			address = argv[n++];
		} else if (!strcmp(arg,"-i")) {
			ifname = argv[n++];
		} else if (!strcmp(arg,"-p")) {
			port = atoi(argv[n++]);
		} else if (!strcmp(arg,"-d")) {
			setenv("COPRTHR_CLMESG_LEVEL","7",1);
		} else {
			printcl( CL_ERR "unrecognized option '%s'\n",arg);
			exit(-1);
		}

	}

	if (ifname) {

		if (address) {
			printcl( CL_ERR "usage: cannot specify both -a and -i");
			exit(-1);
		}

		struct ifaddrs* ifaddr;
		struct ifaddrs* ifa;

		if (getifaddrs(&ifaddr) == -1) {

			printcl( CL_WARNING "getifaddrs() failed");

		} else {

			for ( ifa = ifaddr; ifa != 0; ifa = ifa->ifa_next) {

				if (ifa->ifa_addr != 0
					&& ifa->ifa_addr->sa_family == AF_INET 
					&& !strncmp(ifa->ifa_name, ifname, NI_MAXHOST)
				) {

#if defined(__FreeBSD__)
               int s = getnameinfo( ifa->ifa_addr,sizeof(struct sockaddr), 
						if_address, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
#else
               int s = getnameinfo( ifa->ifa_addr,sizeof(struct sockaddr_in), 
						if_address, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
#endif

               if (s != 0) {

                  printcl( CL_WARNING "getnameinfo() failed with error %s\n", 
							gai_strerror(s));

					} else {

						address = if_address;

					}

					break;
				
				}

				++n;
			}
		}
	}

	if (!address) address = default_address;

	evthread_use_pthreads();

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

	clrpc_server( address, port);

      if (global_base)
         event_base_free(global_base);
   global_spair[0] = global_spair[1] = -1;
   global_base = 0;

   return 0;
}

