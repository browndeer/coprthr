#ifndef _OCLCALL_HOOKS_H
#define _OCLCALL_HOOKS_H

#include "clproc.h"

extern struct clproc_state_struct* _libocl_clproc_state;

void stopped_interface();


/* error accounting */

#undef __oclcall_test_error_rv
#define __oclcall_test_error_rv(rv) do { \
	if (rv) {++_libocl_clproc_state->errs; _libocl_clproc_state->_errno = rv;} \
	} while(0)

#undef __oclcall_test_error_parg
#define __oclcall_test_error_parg(err_ret) do { \
	if (err_ret && *err_ret) \
		{++_libocl_clproc_state->errs; _libocl_clproc_state->_errno = *err_ret;} \
	} while(0)



/* other accounting */

#undef __oclcall_post_clCreateBuffer
#define __oclcall_post_clCreateBuffer() do { \
	printcl( CL_DEBUG "*** I AM A HOOK - __oclcall_pre_clCreateBuffer" ); \
	if (rv) _libocl_clproc_state->gmem += a2; \
	} while(0)

#undef __oclcall_pre_clReleaseMemObject
#define __oclcall_pre_clReleaseMemObject() do { \
	printcl( CL_DEBUG "*** I AM A HOOK - __oclcall_pre_clReleaseMemObject" ); \
	typedef cl_int (*pf_info_t) (cl_mem,cl_mem_info,size_t,void*,size_t*); \
	size_t sz_; \
   cl_int info_rv_ \
      = ((pf_info_t)(*(((void**)oclent)+OCLCALL_clGetMemObjectInfo))) \
			(a0,CL_MEM_SIZE,sizeof(size_t),&sz_,0); \
	_libocl_clproc_state->gmem -= sz_; \
	} while(0)


#undef __oclcall_post_clCreateContext
#define __oclcall_post_clCreateContext() do { \
	printcl( CL_DEBUG "*** I AM A HOOK - __oclcall_post_clCreateContext" ); \
	if (rv) _libocl_clproc_state->ndev += a1; \
	} while(0)

#undef __oclcall_post_clCreateContextFromType
#define __oclcall_post_clCreateContextFromType() do { if (rv) { \
	typedef cl_int (*pf_info_t) (cl_context,cl_context_info,size_t,void*,size_t*); \
	cl_uint ndev_; \
   cl_int info_rv_ \
      = ((pf_info_t)(*(((void**)oclent)+OCLCALL_clGetContextInfo))) \
			(rv,CL_CONTEXT_NUM_DEVICES,sizeof(cl_uint),&ndev_,0); \
	_libocl_clproc_state->ndev += ndev_; \
	} } while(0)


#undef __oclcall_pre_clEnqueueNDRangeKernel
#define __oclcall_pre_clEnqueueNDRangeKernel() do { \
	_libocl_clproc_state->cmds += 1; \
	_libocl_clproc_state->krns += 1; \
	} while(0)

#undef __oclcall_post_clEnqueueNDRangeKernel
#define __oclcall_post_clEnqueueNDRangeKernel() do { if (rv) { \
	} } while(0)


#undef __oclcall_pre_clEnqueueReadBuffer
#define __oclcall_pre_clEnqueueReadBuffer() do { \
	_libocl_clproc_state->cmds += 1; \
	} while(0)

#undef __oclcall_post_clEnqueueReadBuffer
#define __oclcall_post_clEnqueueReadBuffer() do { if (rv) { \
	} } while(0)


#undef __oclcall_pre_clEnqueueWriteBuffer
#define __oclcall_pre_clEnqueueWriteBuffer() do { \
	_libocl_clproc_state->cmds += 1; \
	} while(0)

#undef __oclcall_post_clEnqueueWriteBuffer
#define __oclcall_post_clEnqueueWriteBuffer() do { if (rv) { \
	} } while(0)


#undef __oclcall_pre_clWaitForEvents
#undef __oclcall_pre_clWaitForEvents
#define __oclcall_pre_clWaitForEvents() do { \
	gettimeofday(&(_libocl_clproc_state->tstart),0); \
	printcl( CL_DEBUG "__oclcall_pre_clWaitForEvents" ); \
	_libocl_clproc_state->status = CLPROC_STATUS_WAITING; \
	} while(0)

#undef __oclcall_post_clWaitForEvents
#define __oclcall_post_clWaitForEvents() do { \
	struct timeval t; \
	gettimeofday(&t,0); \
	printcl( CL_DEBUG "gettimeofday %d %d",t.tv_sec,t.tv_usec); \
	if ( (t.tv_usec -= _libocl_clproc_state->tstart.tv_usec) < 0 ) \
		{t.tv_usec += 1000000; --t.tv_sec;} \
	t.tv_sec -= _libocl_clproc_state->tstart.tv_sec; \
	_libocl_clproc_state->twait.tv_sec += t.tv_sec; \
	_libocl_clproc_state->twait.tv_usec += t.tv_usec; \
	_libocl_clproc_state->status = CLPROC_STATUS_RUNNING; \
	} while(0)
/*
#undef __oclcall_post_clWaitForEvents
#define __oclcall_post_clWaitForEvents() do { \
	struct timeval t; \
	gettimeofday(&t,0); \
	printcl( CL_DEBUG "gettimeofday %d %d",t.tv_sec,t.tv_usec); \
	if ( (t.tv_usec -= _libocl_clproc_state->tstart.tv_usec) < 0 ) \
		{t.tv_usec += 1000000; --t.tv_sec;} \
	t.tv_sec -= _libocl_clproc_state->tstart.tv_sec; \
	_libocl_clproc_state->twait.tv_sec += t.tv_sec; \
	_libocl_clproc_state->twait.tv_usec += t.tv_usec; \
	_libocl_clproc_state->status = CLPROC_STATUS_STOPPED; \
	printf(">>> stopped @ post clWaitForEvents ( [c]ontinue )\n"); \
  	char key = 0; while ( (key = getchar()) != 'c' ); \
	_libocl_clproc_state->status = CLPROC_STATUS_RUNNING; \
	} while(0)
*/

#endif

