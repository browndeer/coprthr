
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define min(a,b) ((a<b)?a:b)

#include <CL/cl.h>

#include "util.h"
#include "clrpc_common.h"
#include "clrpc.gen.h"

#define CLRPC_GENERIC_RELEASE_SVRCB(name,type,arg) \
static void _clrpc_##name##_svrcb( \
	EVRPC_STRUCT(_clrpc_##name)* rpc, void* parg) \
{ \
	xclreport( XCL_DEBUG "_clrpc_" #name "_svrcb"); \
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
	xclreport( XCL_DEBUG "_clrpc_" #name "_svrcb"); \
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
	if (len) xclreport( XCL_DEBUG "%d:%s",len,(char*)param_val); \
	if (param_val) free(param_val); \
   EVRPC_REQUEST_DONE(rpc); \
}


struct event_base *global_base;
evutil_socket_t global_spair[2] = { -1, -1 };

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
	sock = evhttp_bind_socket_with_handle(myhttp, "127.0.0.1", 8080);
	if (!sock) {
		xclreport( XCL_ERR "Couldn't open web port");
		exit(-1);
	}

	port = regress_get_socket_port(evhttp_bound_socket_get_fd(sock));

	*pport = port;

	xclreport( XCL_INFO "http_setup: port=%d",port);

	return (myhttp);
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
CLRPC_HEADER(clEnqueueWriteBuffer)
CLRPC_HEADER(clGetEventInfo)
CLRPC_HEADER(clReleaseEvent)
CLRPC_HEADER(clCreateProgramWithSource)

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
CLRPC_GENERATE(clEnqueueWriteBuffer)
CLRPC_GENERATE(clGetEventInfo)
CLRPC_GENERATE(clReleaseEvent)
CLRPC_GENERATE(clCreateProgramWithSource)

static void 
_clrpc_conn_close_cb( 
	struct evhttp_connection* http_conn, void* cbarg 
)
{
	xclreport( XCL_DEBUG "_clrpc_conn_close_cb");
}


static void
_clrpc_clGetPlatformIDs_svrcb(
	EVRPC_STRUCT(_clrpc_clGetPlatformIDs)* rpc, void* arg)
{
	xclreport( XCL_DEBUG "_clrpc_clGetPlatformIDs_svrcb");

	CLRPC_SVRCB_INIT(clGetPlatformIDs);

	int i;

// XXX this code makes more sense on context create -DAR
   struct evhttp_request* http_req = EVRPC_REQUEST_HTTP(rpc);
   struct evhttp_connection* http_conn
      = evhttp_request_get_connection(http_req);
   evhttp_connection_set_closecb(http_conn,_clrpc_conn_close_cb,0);
   xclreport( XCL_DEBUG "_clrpc_clGetPlatformIDs_svrcb %p",http_conn);

	cl_uint nplatforms = 0;
	cl_platform_id* platforms = 0;
	cl_uint nplatforms_ret;

	CLRPC_GET(request,uint,nplatforms,&nplatforms);

	if (nplatforms)
		platforms = (cl_platform_id*)calloc(nplatforms,sizeof(cl_platform_id));

	cl_int retval = clGetPlatformIDs(nplatforms,platforms,&nplatforms_ret);

	xclreport( XCL_DEBUG "*nplatforms_ret = %d",nplatforms_ret);

	CLRPC_ASSIGN(reply, int, retval, retval );
	CLRPC_ASSIGN(reply, uint, nplatforms_ret, nplatforms_ret );

	cl_uint n = min(nplatforms,nplatforms_ret);

	for(i=0;i<n;i++)
		xclreport( XCL_DEBUG "real platforms[%d] = %p",i,platforms[i]);

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
	xclreport( XCL_DEBUG "_clrpc_clGetDeviceIDs_svrcb");

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

	xclreport( XCL_DEBUG " platform = %p",platform);
	xclreport( XCL_DEBUG " devtype = %ld",devtype);

	cl_int retval = clGetDeviceIDs(platform,devtype,
		ndevices,devices,&ndevices_ret);

	xclreport( XCL_DEBUG " retval = %d",retval);
	xclreport( XCL_DEBUG " ndevices_ret = %d",ndevices_ret);

	CLRPC_ASSIGN(reply, int, retval, retval );
	CLRPC_ASSIGN(reply, uint, ndevices_ret, ndevices_ret );

	cl_uint n = min(ndevices,ndevices_ret);

	for(i=0;i<n;i++)
		xclreport( XCL_DEBUG "real devices[%d] = %p",i,devices[i]);

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
	xclreport( XCL_DEBUG "_clrpc_clCreateContext_svrcb");
	
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
		xclreport( XCL_ERR "xprop_len incorrect");

	int nprop = xprop_len/3;
	prop = (cl_context_properties*)malloc(nprop*2+1);
	for(i=0,j=0;i<2*nprop;i+=2,j+=3) {
		clrpc_ptr tmp;
		EVTAG_ARRAY_GET(request,xprop,j,&tmp);
		prop[i] = (cl_context_properties)tmp;
		EVTAG_ARRAY_GET(request,xprop,j+2,&tmp);
		prop[i+1] = (cl_context_properties)tmp;
		xclreport( XCL_DEBUG "prop[] %d 0x%x",(int)prop[i],(int)prop[i+1]);
	}
	prop[i] = 0;

	CLRPC_GET(request,uint,ndev,&ndev);

	size_t devices_len = EVTAG_ARRAY_LEN(request,devices);
	if (devices_len != ndev) 
		xclreport( XCL_ERR "devices_len not equal to ndev");
	devices = (cl_device_id*)calloc(ndev,sizeof(cl_device_id));
	for(i=0;i<ndev;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, devices, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		devices[i] = (cl_device_id)remote;
		xclreport( XCL_DEBUG "devices[] %p",devices[i]);
	}

	cl_context context = clCreateContext(prop,ndev,devices,pfn_notify,
		user_Data,&err_ret);

	xclreport( XCL_DEBUG "remote context = %p",context);

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
	xclreport( XCL_DEBUG "_clrpc_clCreateCommandQueue_svrcb");
	
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

	xclreport( XCL_DEBUG "context device properties %p %p %d",
		context,device,(int)properties);

	cl_command_queue command_queue 
		= clCreateCommandQueue(context,device,properties,&err_ret);

	xclreport( XCL_DEBUG "remote command_queue = %p",command_queue);

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
	xclreport( XCL_DEBUG "_clrpc_clCreateBuffer_svrcb");
	
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

	xclreport( XCL_DEBUG "context flags size %p %ld %ld",
		context,flags,size);

	cl_mem buffer
		= clCreateBuffer(context,flags,size,0,&err_ret);

	xclreport( XCL_DEBUG "remote membuf = %p",buffer);

	clrpc_dptr retval;
	EVTAG_GET(request,retval,&d);
	EVTAG_GET(d,local,&retval.local);
	retval.remote = (clrpc_ptr)buffer;
	EVTAG_GET(reply,retval,&d);
	EVTAG_ASSIGN(d,local,retval.local);
	EVTAG_ASSIGN(d,remote,retval.remote);

	CLRPC_ASSIGN(reply, int64, err_ret, err_ret );

   EVRPC_REQUEST_DONE(rpc);
}


CLRPC_GENERIC_RELEASE_SVRCB(clReleaseMemObject,cl_mem,memobj)


static void
_clrpc_clEnqueueWriteBuffer_svrcb(
	EVRPC_STRUCT(_clrpc_clEnqueueWriteBuffer)* rpc, void* parg)
{
	int i;

	xclreport( XCL_DEBUG "_clrpc_clCreateBuffer_svrcb");
	
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
	xclreport( XCL_DEBUG "COMPARE cb %ld %d",cb,tmp_cb);
	ptr = tmp_ptr; /* XXX simplify later -DAR */

	CLRPC_GET(request,uint,num_events_in_wait_list,&num_events_in_wait_list);

	size_t len = EVTAG_ARRAY_LEN(request,event_wait_list);
	if (len != num_events_in_wait_list) 
		xclreport( XCL_ERR "array len not equal to num_events_in_wait_list");
	if (len)
		event_wait_list = (cl_event*)calloc(len,sizeof(cl_event));
	for(i=0;i<len;i++) {
		struct dual_ptr* d;
		clrpc_ptr local,remote;
		EVTAG_ARRAY_GET(request, event_wait_list, i, &d);
		EVTAG_GET(d,local,&local);
		EVTAG_GET(d,remote,&remote);
		event_wait_list[i] = (cl_event)remote;
		xclreport( XCL_DEBUG "event_wait_list[] %p",event_wait_list[i]);
	}

	xclreport( XCL_DEBUG "clEnqueueWriteBuffer %p %p %d %ld %ld %p %d %p %p",
		command_queue,buffer,blocking_write,offset,cb,
		ptr,num_events_in_wait_list,event_wait_list,&event);

	int* iptr = (int*)ptr;
	for(i=0;i<32;i++) printf("%d/",iptr[i]); printf("\n");

	cl_int retval = clEnqueueWriteBuffer(command_queue,buffer,blocking_write,
		offset,cb,ptr,num_events_in_wait_list,event_wait_list,&event);

	xclreport( XCL_DEBUG "remote membuf = %p",buffer);

	xclreport( XCL_DEBUG "retval %d", retval);

	clrpc_dptr retevent;
	EVTAG_GET(request,event,&d);
	EVTAG_GET(d,local,&retevent.local);
	retevent.remote = (clrpc_ptr)event;
	EVTAG_GET(reply,event,&d);
	EVTAG_ASSIGN(d,local,retevent.local);
	EVTAG_ASSIGN(d,remote,retevent.remote);

	CLRPC_ASSIGN(reply, int64, retval, retval );

   EVRPC_REQUEST_DONE(rpc);
}

CLRPC_GENERIC_GETINFO_SVRCB(clGetEventInfo,event,event,event_info)

CLRPC_GENERIC_RELEASE_SVRCB(clReleaseEvent,cl_event,event)


static void
_clrpc_clCreateProgramWithSource_svrcb(
	EVRPC_STRUCT(_clrpc_clCreateProgramWithSource)* rpc, void* parg)
{
	xclreport( XCL_DEBUG "_clrpc_clCreateProgramWithSource_svrcb");
	
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
		xclreport( XCL_ERR "lengths_len not equal to count");

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
	xclreport( XCL_DEBUG "COMPARE sz %ld %d",sz,tmp_sz);
	char* p = tmp_buf;
	xclreport( XCL_DEBUG "|%s|", p);
	for(i=0;i<count;i++) {
		strings[i] = p; 
		p += lengths[i];
	}

	cl_program program = clCreateProgramWithSource(context,count,
		(const char**)strings, lengths,&err_ret);

	xclreport( XCL_DEBUG "remote program = %p",program);

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


void _clrpc_buf_eventcb(struct bufferevent *bev, short events, void *ptr)
{
	xclreport( XCL_DEBUG "_clrpc_buf_eventcb");

    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
            something here, like start reading or writing. */
    } else if (events & BEV_EVENT_ERROR) {
         /* An error occured while connecting. */
    }
}

static void
clrpc_server(void)
{
	ev_uint16_t port;
	struct evhttp *http = http_setup(&port);
	struct evrpc_base *base = evrpc_init(http);
	struct evrpc_pool *pool = 0;

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
	CLRPC_REGISTER(clEnqueueWriteBuffer);
	CLRPC_REGISTER(clGetEventInfo);
	CLRPC_REGISTER(clReleaseEvent);
	CLRPC_REGISTER(clCreateProgramWithSource);

	struct bufferevent *bev;
   bev = bufferevent_socket_new(global_base, -1, BEV_OPT_CLOSE_ON_FREE);
   bufferevent_setcb(bev, NULL, NULL, _clrpc_buf_eventcb, NULL);
	bufferevent_enable(bev, EV_READ|EV_WRITE);

	pool = rpc_pool_with_connection(port);

	event_dispatch();

}


int
main(int argc, const char **argv)
{
	cl_uint n;
	int rc = clGetPlatformIDs(0,0,&n);

	printf("hello world %d %d\n",rc,n);

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

	clrpc_server();

      if (global_base)
         event_base_free(global_base);
   global_spair[0] = global_spair[1] = -1;
   global_base = 0;

   return 0;
}

