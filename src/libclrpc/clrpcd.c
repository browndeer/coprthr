
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define min(a,b) ((a<b)?a:b)

#include <CL/cl.h>

//#include "util.h"
#include "printcl.h"
#include "clrpc_common.h"
#include "clrpc.gen.h"

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
};


#define CLRPC_XEVENT_CREATE(xevent,event,ptr,sz) \
	struct xevent_struct* xevent \
		= (struct xevent_struct*)malloc(sizeof(struct xevent_struct)); \
	printcl( CL_DEBUG "xevent at creation is %p",xevent); \
	do { xevent->event = event; xevent->buf_ptr = ptr; xevent->buf_sz = sz; \
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


CLRPC_HEADER(clGetPlatformIDs)
CLRPC_HEADER(clGetPlatformInfo)
CLRPC_HEADER(clGetDeviceIDs)
CLRPC_HEADER(clGetDeviceInfo)
CLRPC_HEADER(clCreateContext)
CLRPC_HEADER(clGetContextInfo)
CLRPC_HEADER(clReleaseContext)
CLRPC_HEADER(clCreateCommandQueue)
CLRPC_HEADER(clReleaseCommandQueue)
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

	CLRPC_DPTR_ARRAY_COPY(request,reply,n,devices);
	CLRPC_DPTR_ARRAY_SET_REMOTE(reply,n,devices,devices);

	if (devices) 
		free(devices);

   EVRPC_REQUEST_DONE(rpc);

}


CLRPC_GENERIC_GETINFO_SVRCB(clGetDeviceInfo,device_id,device,device_info)


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


CLRPC_GENERIC_GETINFO_SVRCB(clGetContextInfo,context,context,context_info)


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

	CLRPC_XEVENT_CREATE(xevent,event,ptr,cb);

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

	int* iptr = (int*)ptr;
	for(i=0;i<32;i++) printf("%d/",iptr[i]); printf("\n");

	cl_int retval = clEnqueueWriteBuffer(command_queue,buffer,blocking_write,
		offset,cb,ptr,num_events_in_wait_list,event_wait_list,&event);

	printcl( CL_DEBUG "remote membuf = %p",buffer);

	printcl( CL_DEBUG "retval %d", retval);

//	CLRPC_XEVENT_CREATE(xevent,event,0,0);
	CLRPC_XEVENT_CREATE(xevent,event,ptr,0);

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
	if (xevent->buf_ptr) free(xevent->buf_ptr);
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
_clrpc_clBuildProgram_svrcb(
	EVRPC_STRUCT(_clrpc_clBuildProgram)* rpc, void* parg)
{
	printcl( CL_DEBUG "_clrpc_clBuildProgram_svrcb");
	
	CLRPC_SVRCB_INIT(clBuildProgram);

	int i;

	cl_program program;
	cl_uint ndevices;
	cl_device_id* devices;
	char* options;
	void* pfn_notify = 0;
	void* user_data = 0;
	
	struct dual_ptr* d;

   EVTAG_GET(request,program,&d);
   EVTAG_GET(d,remote,(void*)&program);

	CLRPC_GET(request,uint,ndevices,&ndevices);

	size_t devices_len = EVTAG_ARRAY_LEN(request,devices);
	if (devices_len != ndevices) 
		printcl( CL_ERR "devices_len not equal to ndevices");
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

   unsigned int options_sz = 0;
   EVTAG_GET_WITH_LEN(request,options,(unsigned char**)&options,&options_sz);
	if (options_sz == 0) options = 0;
	printcl( CL_DEBUG "%d:|%s|",options_sz,options);

	cl_int retval = clBuildProgram(program,ndevices,devices,options,
		pfn_notify,user_data);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}

CLRPC_GENERIC_GETINFO_SVRCB(clGetProgramInfo,program,program,program_info)

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

CLRPC_GENERIC_GETINFO_SVRCB(clGetKernelInfo,kernel,kernel,kernel_info)

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
	} else {
		printcl( CL_ERR "array len != work_dim");
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

	CLRPC_XEVENT_CREATE(xevent,event,0,0);

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
				printf("EVENT %d:",i);
				int ii; for(ii=0;ii<10;ii++) printf("*%d",ptmp[ii]); printf("\n"); fflush(stdout);
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
	CLRPC_REGISTER(clGetContextInfo);
	CLRPC_REGISTER(clReleaseContext);
	CLRPC_REGISTER(clCreateCommandQueue);
	CLRPC_REGISTER(clReleaseCommandQueue);
	CLRPC_REGISTER(clCreateBuffer);
	CLRPC_REGISTER(clReleaseMemObject);
	CLRPC_REGISTER(clEnqueueReadBuffer);
	CLRPC_REGISTER(clEnqueueWriteBuffer);
	CLRPC_REGISTER(clGetEventInfo);
	CLRPC_REGISTER(clReleaseEvent);
	CLRPC_REGISTER(clCreateProgramWithSource);
	CLRPC_REGISTER(clBuildProgram);
	CLRPC_REGISTER(clGetProgramInfo);
	CLRPC_REGISTER(clReleaseProgram);
	CLRPC_REGISTER(clCreateKernel);
	CLRPC_REGISTER(clGetKernelInfo);
	CLRPC_REGISTER(clReleaseKernel);
	CLRPC_REGISTER(clSetKernelArg);
	CLRPC_REGISTER(clEnqueueNDRangeKernel);
	CLRPC_REGISTER(clFlush);
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

	char default_address[] = "127.0.0.1";
	const char* address = default_address;
	ev_uint16_t port = 8091;

	n = 1;
   while (n < argc) {

		const char* arg = argv[n++];
		
		if (!strcmp(arg,"-a")) {
			address = argv[n++];
		} else if (!strcmp(arg,"-p")) {
			port = atoi(argv[n++]);
		} else {
			printcl( CL_ERR "unrecognized option '%s'\n",arg);
			exit(1);
		}

	}

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

