
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
cl_int clrpc_##name( type arg ) \
{ \
	CLRPC_INIT(name); \
	CLRPC_ASSIGN_DPTR(request,arg,dummy); \
	CLRPC_MAKE_REQUEST_WAIT(name); \
	cl_int retval; \
	CLRPC_GET(reply,int,retval,&retval); \
	xclreport( XCL_DEBUG "clrpc_" #name ": retval = %d",retval); \
	return(retval); \
}

#define CLRPC_GENERIC_GETINFO(name,type,arg,infotype) \
CLRPC_UNBLOCK_CLICB(name) \
cl_int clrpc_##name(cl_##type arg, cl_##infotype param_name, \
	size_t param_sz, void* param_val, size_t *param_sz_ret) \
{ \
	cl_int retval = 0; \
	CLRPC_INIT(name); \
	CLRPC_ASSIGN_DPTR(request,arg,arg); \
	CLRPC_ASSIGN(request,infotype,param_name,param_name); \
	CLRPC_ASSIGN(request,uint,param_sz,param_sz); \
	CLRPC_MAKE_REQUEST_WAIT(name); \
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

#if(0)
static void
_clrpc_Send_svrcb(
   EVRPC_STRUCT(_clrpc_Send)* rpc, void* parg)
{
	xclreport( XCL_DEBUG "_clrpc_Send_svrcb");
}

static struct evhttp *
http_setup(ev_uint16_t *pport)
{
   struct evhttp *myhttp;
   ev_uint16_t port;
   struct evhttp_bound_socket *sock;

   myhttp = evhttp_new(NULL);
   if (!myhttp) {
      xclreport( XCL_ERR "Could not start web server");
      exit(-1);
   }

   /* Try a few different ports */
   sock = evhttp_bind_socket_with_handle(myhttp, "127.0.0.1", 8081);
   if (!sock) {
      xclreport( XCL_ERR "Couldn't open web port");
      exit(-1);
   }

   port = regress_get_socket_port(evhttp_bound_socket_get_fd(sock));

   *pport = port;

   xclreport( XCL_INFO "http_setup: port=%d",port);

   return (myhttp);
}
#endif


int clrpc_init( void )
{

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

	clrpc_port = 8080;

	clrpc_pool = rpc_pool_with_connection(clrpc_port);

   pthread_attr_init(&clrpc_td_attr);
   pthread_attr_setdetachstate(&clrpc_td_attr,PTHREAD_CREATE_JOINABLE);
	
	pthread_create(&clrpc_td,&clrpc_td_attr,loop,(void*)0);

//	ev_uint16_t port;
//   struct evhttp *http = http_setup(&port);
//   struct evrpc_base *base = evrpc_init(http);

//	CLRPC_REGISTER(Send);

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
CLRPC_GENERIC_GETINFO(clGetPlatformInfo,platform_id,platform,platform_info)


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
CLRPC_GENERIC_GETINFO(clGetDeviceInfo,device_id,device,device_info)


/*
 * clCreateContext
 */
CLRPC_UNBLOCK_CLICB(clCreateContext)
cl_context
clrpc_clCreateContext(
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
	xclreport( XCL_DEBUG "nprop %d",nprop);
	clrpc_ptr* xprop = calloc(nprop*3+1,sizeof(clrpc_ptr));
	for(i=0,j=0;i<2*nprop;i+=2,j+=3) {
		xprop[j] = (clrpc_ptr)prop[i];
		xprop[j+1] = (clrpc_ptr)((clrpc_dptr*)prop[i+1])->local;
		xprop[j+2] = (clrpc_ptr)((clrpc_dptr*)prop[i+1])->remote;
		xclreport( XCL_DEBUG "xprop[] %ld %p %p",xprop[j],xprop[j+1],xprop[j+2]);
	}
	xprop[j] = 0;
	for(i=0;i<nprop*3+1;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,xprop,xprop[i]);

	CLRPC_ASSIGN(request,uint,ndev,ndev);

	CLRPC_ASSIGN_DPTR_ARRAY(request,ndev,devices);

	clrpc_dptr* retval = context;
	CLRPC_ASSIGN_DPTR(request,retval,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clCreateContext);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&context->remote);
	xclreport( XCL_DEBUG "context local remote %p %p",
		context->local,context->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateContext: *err_ret = %d\n", *err_ret);

	if (xprop) free(xprop);

	return((cl_context)context);
}


/*
 * clGetContextInfo
 */
CLRPC_GENERIC_GETINFO(clGetContextInfo,context,context,context_info)


/*
 * clReleaseContext
 */
CLRPC_GENERIC_RELEASE(clReleaseContext,cl_context,context)


/*
 * clCreateCommandQueue
 */
CLRPC_UNBLOCK_CLICB(clCreateCommandQueue)
cl_command_queue 
clrpc_clCreateCommandQueue(
	cl_context context,
	cl_device_id device,
	cl_command_queue_properties properties,
	cl_int *err_ret
)
{
	clrpc_dptr* command_queue = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	command_queue->local = (clrpc_ptr)command_queue;

	CLRPC_INIT(clCreateCommandQueue);

	xclreport( XCL_DEBUG "context local remote %p %p",
		((clrpc_dptr*)context)->local,((clrpc_dptr*)context)->remote);

	CLRPC_ASSIGN_DPTR(request,context,dummy);
	CLRPC_ASSIGN_DPTR(request,device,dummy);
	CLRPC_ASSIGN(request,command_queue_properties,properties,properties);

	clrpc_dptr* retval = command_queue;
	CLRPC_ASSIGN_DPTR(request,retval,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clCreateCommandQueue);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&command_queue->remote);
	xclreport( XCL_DEBUG "command_queue local remote %p %p",
		command_queue->local,command_queue->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateCommandQueue: *err_ret = %d\n",*err_ret);

	return((cl_command_queue)command_queue);
}


/*
 * clReleaseCommandQueue
 */
CLRPC_GENERIC_RELEASE(clReleaseCommandQueue,cl_command_queue,command_queue)


/*
 * clCreateBuffer
 */
CLRPC_UNBLOCK_CLICB(clCreateBuffer)
cl_mem 
clrpc_clCreateBuffer(
	cl_context context,
	cl_mem_flags flags,
	size_t size,
	void *host_ptr,
	cl_int* err_ret
)
{
	clrpc_dptr* buffer = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	buffer->local = (clrpc_ptr)buffer;

	CLRPC_INIT(clCreateBuffer);

	CLRPC_ASSIGN_DPTR(request,context,dummy);
	CLRPC_ASSIGN(request,mem_flags,flags,flags);
	EVTAG_ASSIGN(request,size,size);

	if (host_ptr)
		xclreport( XCL_WARNING "host_ptr not supported, forced to null");

	clrpc_dptr* retval = buffer;
	CLRPC_ASSIGN_DPTR(request,retval,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clCreateBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&buffer->remote);
	xclreport( XCL_DEBUG "buffer local remote %p %p",
		buffer->local,buffer->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateBuffer: *err_ret = %d\n",*err_ret);

	return((cl_mem)buffer);
}


/*
 * clReleaseMemObject
 */
CLRPC_GENERIC_RELEASE(clReleaseMemObject,cl_mem,memobj)


/*
 * clEnqueueReadBuffer
 */
CLRPC_UNBLOCK_CLICB(clEnqueueReadBuffer)
cl_int
clrpc_clEnqueueReadBuffer (
	cl_command_queue command_queue, 
	cl_mem buffer,
   cl_bool blocking_read, 
	size_t offset, 
	size_t cb, 
	void* ptr,
   cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list,
   cl_event* pevent 
)
{
	int i;

	CLRPC_INIT(clEnqueueReadBuffer);

	CLRPC_ASSIGN_DPTR(request,command_queue,dummy);
	CLRPC_ASSIGN_DPTR(request,buffer,dummy);
	EVTAG_ASSIGN(request,blocking_read,blocking_read);
	EVTAG_ASSIGN(request,offset,offset);
	EVTAG_ASSIGN(request,cb,cb);
//	EVTAG_ASSIGN_WITH_LEN(request,_bytes,ptr,cb);
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY(request,num_events_in_wait_list,event_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
//	event->local = (clrpc_ptr)event;
	struct xevent_struct* xevent 
		= (struct xevent_struct*)malloc(sizeof(struct xevent_struct));
	xevent->event = (clrpc_ptr)event;
	if ( blocking_read == CL_FALSE ) {
		xevent->buf_ptr = ptr;
		xevent->buf_sz = cb;
	} else {
		xevent->buf_ptr = 0;
		xevent->buf_sz = 0;
	}
	event->local = (clrpc_ptr)xevent;
	CLRPC_ASSIGN_DPTR(request,event,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clEnqueueReadBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&(event)->remote);
	xclreport( XCL_DEBUG "event local remote %p %p",
		(event)->local,(event)->remote);
	*pevent = (cl_event)event;

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

	return(retval);
}


/*
 * clEnqueueWriteBuffer
 */
CLRPC_UNBLOCK_CLICB(clEnqueueWriteBuffer)
cl_int
clrpc_clEnqueueWriteBuffer (
	cl_command_queue command_queue, 
	cl_mem buffer,
   cl_bool blocking_write, 
	size_t offset, 
	size_t cb, 
	const void *ptr,
   cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list,
   cl_event* pevent 
)
{
	CLRPC_INIT(clEnqueueWriteBuffer);

	CLRPC_ASSIGN_DPTR(request,command_queue,dummy);
	CLRPC_ASSIGN_DPTR(request,buffer,dummy);
	EVTAG_ASSIGN(request,blocking_write,blocking_write);
	EVTAG_ASSIGN(request,offset,offset);
	EVTAG_ASSIGN(request,cb,cb);
	EVTAG_ASSIGN_WITH_LEN(request,_bytes,ptr,cb);
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY(request,num_events_in_wait_list,event_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
//	event->local = (clrpc_ptr)event;
//	event->buf_local = (clrpc_ptr)0;
	struct xevent_struct* xevent 
		= (struct xevent_struct*)malloc(sizeof(struct xevent_struct));
	xevent->event = (clrpc_ptr)event;
	xevent->buf_ptr = 0;
	xevent->buf_sz = 0;
	event->local = (clrpc_ptr)xevent;
	CLRPC_ASSIGN_DPTR(request,event,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clEnqueueWriteBuffer);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&(event)->remote);
	xclreport( XCL_DEBUG "event local remote %p %p",
		(event)->local,(event)->remote);
	*pevent = (cl_event)event;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clEnqueueWriteBuffer: retval = %d",retval);

	return(retval);
}


/*
 * clGetEventInfo
 */
CLRPC_GENERIC_GETINFO(clGetEventInfo,event,event,event_info)


/*
 * clReleaseEvent
 */
//CLRPC_GENERIC_RELEASE(clReleaseEvent,cl_event,event)
CLRPC_UNBLOCK_CLICB(clReleaseEvent)
cl_int clrpc_clReleaseEvent( cl_event event ) 
{ 
   CLRPC_INIT(clReleaseEvent);
   CLRPC_ASSIGN_DPTR(request,event,dummy);
   CLRPC_MAKE_REQUEST_WAIT(clReleaseEvent);
	struct xevent_struct* xevent
		= (struct xevent_struct*)((clrpc_dptr*)event)->local;
	free(xevent);
	free(event);
   cl_int retval;
   CLRPC_GET(reply,int,retval,&retval);
   xclreport( XCL_DEBUG "clrpc_" "clReleaseEvent" ": retval = %d",retval);
   return(retval);
}



/*
 * clCreateProgramWithSource
 */
CLRPC_UNBLOCK_CLICB(clCreateProgramWithSource)
cl_program
clrpc_clCreateProgramWithSource(
	cl_context context, 
	cl_uint count,
   const char** strings, 
	const size_t* lengths, 
	cl_int* err_ret
)
{
	int i;

	clrpc_dptr* program = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	program->local = (clrpc_ptr)program;

	CLRPC_INIT(clCreateProgramWithSource);

	CLRPC_ASSIGN_DPTR(request,context,dummy);
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

	clrpc_dptr* retval = program;
	CLRPC_ASSIGN_DPTR(request,retval,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clCreateProgramWithSource);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&program->remote);
	xclreport( XCL_DEBUG "program local remote %p %p",
		program->local,program->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateProgramWithSource: *err_ret = %d", 
		*err_ret);

	if (buf) free(buf);

	return((cl_program)program);
}


/*
 * clBuildProgram
 */
CLRPC_UNBLOCK_CLICB(clBuildProgram)
cl_int
clrpc_clBuildProgram(
	cl_program program, 
	cl_uint ndevices,
	const cl_device_id* devices,
	const char* options,
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
	void *user_data
)
{
	CLRPC_INIT(clBuildProgram);

	CLRPC_ASSIGN_DPTR(request,program,dummy);
	CLRPC_ASSIGN(request,uint,ndevices,ndevices);
	CLRPC_ASSIGN_DPTR_ARRAY(request,ndevices,devices);
	size_t options_sz = (options)? strnlen(options,4096) : 0;
	EVTAG_ASSIGN_WITH_LEN(request,options,(unsigned char*)options,options_sz);
	
	CLRPC_MAKE_REQUEST_WAIT(clBuildProgram);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clBuildProgram: retval = %d", retval);

	return(retval);
}


/*
 * clGetProgramInfo
 */
CLRPC_GENERIC_GETINFO(clGetProgramInfo,program,program,program_info)


/*
 * clReleaseProgram
 */
CLRPC_GENERIC_RELEASE(clReleaseProgram,cl_program,program)


/*
 * clCreateKernel
 */
CLRPC_UNBLOCK_CLICB(clCreateKernel)
cl_kernel
clrpc_clCreateKernel(
	cl_program program, 
	const char* kernel_name,
   cl_int* err_ret
)
{
	clrpc_dptr* kernel = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
	kernel->local = (clrpc_ptr)kernel;

	CLRPC_INIT(clCreateKernel);

	CLRPC_ASSIGN_DPTR(request,program,dummy);
	EVTAG_ASSIGN(request,kernel_name,kernel_name);

	clrpc_dptr* retval = kernel;
	CLRPC_ASSIGN_DPTR(request,retval,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clCreateKernel);

	CLRPC_GET_DPTR_REMOTE(reply,uint,retval,&kernel->remote);
	xclreport( XCL_DEBUG "kernel local remote %p %p",
		kernel->local,kernel->remote);

	CLRPC_GET(reply,int,err_ret,err_ret);

	xclreport( XCL_DEBUG "clrpc_clCreateKernel: *err_ret = %d\n", *err_ret);

	return((cl_kernel)kernel);
}


/*
 * clGetKernelInfo
 */
CLRPC_GENERIC_GETINFO(clGetKernelInfo,kernel,kernel,kernel_info)


/*
 * clReleaseKernel
 */
CLRPC_GENERIC_RELEASE(clReleaseKernel,cl_kernel,kernel)


/*
 * clSetKernelArg
 */
CLRPC_UNBLOCK_CLICB(clSetKernelArg)
cl_int
clrpc_clSetKernelArg(
	cl_kernel kernel, 
	cl_uint arg_index,
	size_t arg_size,
   const void* arg_value
)
{
	CLRPC_INIT(clSetKernelArg);

	CLRPC_ASSIGN_DPTR(request,kernel,dummy);
	EVTAG_ASSIGN(request,arg_index,arg_index);
	EVTAG_ASSIGN(request,arg_size,arg_size);
	if (arg_value)
		 EVTAG_ASSIGN_WITH_LEN(request,arg_value,(unsigned char*)arg_value,
			arg_size);

	CLRPC_MAKE_REQUEST_WAIT(clSetKernelArg);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clSetKernelArg: retval = %d\n", retval);

	return(retval);
}


/*
 * clEnqueueNDRangeKernel
 */
CLRPC_UNBLOCK_CLICB(clEnqueueNDRangeKernel)
cl_int
clrpc_clEnqueueNDRangeKernel (
	cl_command_queue command_queue, 
	cl_kernel kernel,
	cl_uint work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size,
   cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list,
   cl_event* pevent 
)
{
	int i;

	CLRPC_INIT(clEnqueueNDRangeKernel);

	CLRPC_ASSIGN_DPTR(request,command_queue,dummy);
	CLRPC_ASSIGN_DPTR(request,kernel,dummy);
	CLRPC_ASSIGN(request,uint,work_dim,work_dim);

	if (global_work_offset) for(i=0;i<work_dim;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,global_work_offset,global_work_offset[i]);

	if (global_work_size) for(i=0;i<work_dim;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,global_work_size,global_work_size[i]);

	if (local_work_size) for(i=0;i<work_dim;i++) 
		EVTAG_ARRAY_ADD_VALUE(request,local_work_size,local_work_size[i]);
	
	CLRPC_ASSIGN(request,uint,num_events_in_wait_list,num_events_in_wait_list);
	CLRPC_ASSIGN_DPTR_ARRAY(request,num_events_in_wait_list,event_wait_list);

	clrpc_dptr* event = (clrpc_dptr*)malloc(sizeof(clrpc_dptr));
//	event->local = (clrpc_ptr)event;
//	event->buf_local = (clrpc_ptr)0;
	struct xevent_struct* xevent 
		= (struct xevent_struct*)malloc(sizeof(struct xevent_struct));
	xevent->event = (clrpc_ptr)event;
	xevent->buf_ptr = 0;
	xevent->buf_sz = 0;
	event->local = (clrpc_ptr)xevent;
	CLRPC_ASSIGN_DPTR(request,event,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clEnqueueNDRangeKernel);

	CLRPC_GET_DPTR_REMOTE(reply,uint,event,&(event)->remote);
	xclreport( XCL_DEBUG "event local remote %p %p",
		(event)->local,(event)->remote);
	*pevent = (cl_event)event;

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clEnqueueNDRangeKernel: retval = %d",retval);

	return(retval);
}


/*
 * clFlush
 */
CLRPC_UNBLOCK_CLICB(clFlush)
cl_int
clrpc_clFlush(
	cl_command_queue command_queue
)
{
	CLRPC_INIT(clFlush);

	CLRPC_ASSIGN_DPTR(request,command_queue,dummy);

	CLRPC_MAKE_REQUEST_WAIT(clFlush);

	cl_int retval;
	CLRPC_GET(reply,int,retval,&retval);

	xclreport( XCL_DEBUG "clrpc_clFlush: retval = %d\n", retval);

	return(retval);
}


/*
 * clWaitForEvents
 */
CLRPC_UNBLOCK_CLICB(clWaitForEvents)
cl_int
clrpc_clWaitForEvents(
	cl_uint nevents, 
	const cl_event* events
)
{
	int i;

	CLRPC_INIT(clWaitForEvents);

	CLRPC_ASSIGN(request,uint,nevents,nevents);

	CLRPC_ASSIGN_DPTR_ARRAY(request,nevents,events);

	CLRPC_MAKE_REQUEST_WAIT(clWaitForEvents);

	if (EVTAG_HAS(reply,_bytes)) {
		xclreport( XCL_DEBUG "bytes sent back");
   	void* tmp_buf = 0;
   	unsigned int tmp_len = 0;
   	EVTAG_GET_WITH_LEN(reply,_bytes,(unsigned char**)&tmp_buf,&tmp_len);
		void* tmp_ptr = tmp_buf;
		for(i=0;i<nevents;i++) {
			struct xevent_struct* xevent 
				= (struct xevent_struct*)((clrpc_dptr*)events[i])->local;
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

	return(retval);
}


