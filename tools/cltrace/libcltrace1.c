/* libcltrace1.c
 *
 * Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define __USE_GNU
#include <dlfcn.h>

#include <CL/cl.h>

#define CALLID_clGetPlatformIDs					1
#define CALLID_clGetPlatformInfo				2
#define CALLID_clGetDeviceIDs					3
#define CALLID_clGetDeviceInfo					4	
#define CALLID_clCreateContext					5
#define CALLID_clCreateContextFromType		6	
#define CALLID_clRetainContext					7
#define CALLID_clReleaseContext					8
#define CALLID_clGetContextInfo					9
#define CALLID_clCreateCommandQueue			10	
#define CALLID_clRetainCommandQueue			11	
#define CALLID_clReleaseCommandQueue			12	
#define CALLID_clGetCommandQueueInfo			13	
#define CALLID_clSetCommandQueueProperty		14		
#define CALLID_clCreateBuffer					15	
#define CALLID_clCreateImage2D					16	
#define CALLID_clCreateImage3D					17	
#define CALLID_clRetainMemObject				18		
#define CALLID_clReleaseMemObject				19		
#define CALLID_clGetSupportedImageFormats	20		
#define CALLID_clGetMemObjectInfo				21	
#define CALLID_clGetImageInfo					22	
#define CALLID_clCreateSampler					23	
#define CALLID_clRetainSampler					24	
#define CALLID_clReleaseSampler					25	
#define CALLID_clGetSamplerInfo					26	
#define CALLID_clCreateProgramWithSource		27	
#define CALLID_clCreateProgramWithBinary		28
#define CALLID_clRetainProgram					29	
#define CALLID_clReleaseProgram					30	
#define CALLID_clBuildProgram					31	
#define CALLID_clUnloadCompiler					32
#define CALLID_clGetProgramInfo					33	
#define CALLID_clGetProgramBuildInfo			34			
#define CALLID_clCreateKernel					35	
#define CALLID_clCreateKernelsInProgram		36			
#define CALLID_clRetainKernel					37	
#define CALLID_clReleaseKernel					38	
#define CALLID_clSetKernelArg					39	
#define CALLID_clGetKernelInfo					40	
#define CALLID_clGetKernelWorkGroupInfo		41		
#define CALLID_clWaitForEvents					42	
#define CALLID_clGetEventInfo					43	
#define CALLID_clRetainEvent						44	
#define CALLID_clReleaseEvent					45		
#define CALLID_clGetEventProfilingInfo		46			
#define CALLID_clFlush								47	
#define CALLID_clFinish							48	
#define CALLID_clEnqueueReadBuffer				49	
#define CALLID_clEnqueueWriteBuffer			50		
#define CALLID_clEnqueueCopyBuffer				51	
#define CALLID_clEnqueueReadImage				52	
#define CALLID_clEnqueueWriteImage				53	
#define CALLID_clEnqueueCopyImage				54	
#define CALLID_clEnqueueCopyImageToBuffer	55			
#define CALLID_clEnqueueCopyBufferToImage	56			
#define CALLID_clEnqueueMapBuffer				57	
#define CALLID_clEnqueueMapImage				58	
#define CALLID_clEnqueueUnmapMemObject		59			
#define CALLID_clEnqueueNDRangeKernel			60		
#define CALLID_clEnqueueTask						61	
#define CALLID_clEnqueueNativeKernel			62				
#define CALLID_clEnqueueMarker					63	
#define CALLID_clEnqueueWaitForEvents			64		
#define CALLID_clEnqueueBarrier					65								

struct calltab_entry { 
	int id, nc, nerr; 
	unsigned long long tusec; 
	char cname[64];
}; 

#define __entry(cname) { CALLID_##cname, 0,0,0, #cname }

static struct calltab_entry calltab[] = {
	{ 0,0,0,0,0 }, /* dummy entry */
	__entry(clGetPlatformIDs),
	__entry(clGetPlatformInfo),
	__entry(clGetDeviceIDs),
	__entry(clGetDeviceInfo),
	__entry(clCreateContext),
	__entry(clCreateContextFromType),
	__entry(clRetainContext),
	__entry(clReleaseContext),
	__entry(clGetContextInfo),
	__entry(clCreateCommandQueue),
	__entry(clRetainCommandQueue),
	__entry(clReleaseCommandQueue),
	__entry(clGetCommandQueueInfo),
	__entry(clSetCommandQueueProperty),
	__entry(clCreateBuffer),
	__entry(clCreateImage2D),
	__entry(clCreateImage3D),
	__entry(clRetainMemObject),
	__entry(clReleaseMemObject),
	__entry(clGetSupportedImageFormats),
	__entry(clGetMemObjectInfo),
	__entry(clGetImageInfo),
	__entry(clCreateSampler),
	__entry(clRetainSampler),
	__entry(clReleaseSampler),
	__entry(clGetSamplerInfo),
	__entry(clCreateProgramWithSource),
	__entry(clCreateProgramWithBinary),
	__entry(clRetainProgram),
	__entry(clReleaseProgram),
	__entry(clBuildProgram),
	__entry(clUnloadCompiler),
	__entry(clGetProgramInfo),
	__entry(clGetProgramBuildInfo),
	__entry(clCreateKernel),
	__entry(clCreateKernelsInProgram),
	__entry(clRetainKernel),
	__entry(clReleaseKernel),
	__entry(clSetKernelArg),
	__entry(clGetKernelInfo),
	__entry(clGetKernelWorkGroupInfo),
	__entry(clWaitForEvents),
	__entry(clGetEventInfo),
	__entry(clRetainEvent),
	__entry(clReleaseEvent),
	__entry(clGetEventProfilingInfo),
	__entry(clFlush),
	__entry(clFinish),
	__entry(clEnqueueReadBuffer),
	__entry(clEnqueueWriteBuffer),
	__entry(clEnqueueCopyBuffer),
	__entry(clEnqueueReadImage),
	__entry(clEnqueueWriteImage),
	__entry(clEnqueueCopyImage),
	__entry(clEnqueueCopyImageToBuffer),
	__entry(clEnqueueCopyBufferToImage),
	__entry(clEnqueueMapBuffer),
	__entry(clEnqueueMapImage),
	__entry(clEnqueueUnmapMemObject),
	__entry(clEnqueueNDRangeKernel),
	__entry(clEnqueueTask),
	__entry(clEnqueueNativeKernel),
	__entry(clEnqueueMarker),
	__entry(clEnqueueWaitForEvents),
	__entry(clEnqueueBarrier),
	{ 0,0,0,0,0 }  /* dummy entry */
};

#define NCALLS (sizeof(calltab)/sizeof(struct calltab_entry))

static char* cltrace_report = 0;
static char* cltrace_timestamp = 0;
static char* cltrace_timer = 0;

static int init_skip = 0;
static int fini_defer = 0;

//void __attribute__((__constructor__(101))) _libcltrace1_init(void)
void __attribute__((__constructor__)) _libcltrace1_init(void)
{
	if (init_skip==1) return;

	cltrace_report = getenv("CLTRACE_REPORT");
	cltrace_timestamp = getenv("CLTRACE_TIMESTAMP");
	cltrace_timer = getenv("CLTRACE_TIMER");
//	fprintf(stderr,"_libcltrace1_init: %p %p %p\n",cltrace_report,cltrace_timestamp,cltrace_timer);

	init_skip = 1;
}

//void __attribute__((__destructor__(101))) _libcltrace1_fini(void)
void __attribute__((__destructor__)) _libcltrace1_fini(void)
{
	if (fini_defer==1) { fini_defer=2; return; }

	int i,j;
	int sort[NCALLS];
	unsigned long long sum = 0;

	for(i=0;i<NCALLS;i++) { sort[i]=i; sum += calltab[i].tusec; }

	for(j=0;j<NCALLS;j++) for(i=0;i<NCALLS-1;i++) {
		int k1 = sort[i]; 
		int k2 = sort[i+1];
		if (calltab[k1].tusec < calltab[k2].tusec) { sort[i]=k2; sort[i+1]=k1; }
	}

	float s = 100.0f/(float)sum;

	int count = 0;
	for(i=0;i<NCALLS;i++) count += calltab[sort[i]].nc;

	if (cltrace_report && count) {

		fflush(stdout);
		printf("\n");
		
		printf("%% time     seconds  usecs/call     calls    errors opencl_call\n");
		printf("------ ----------- ----------- --------- --------- ----------------------------\n");

		for(i=0;i<NCALLS;i++) {
			struct calltab_entry e = calltab[sort[i]];
			unsigned long long t = e.tusec;
			if (e.nc>0) {
				printf("%6.2f  %10.6f %11d %9d",
					(float)t*s,1.0e-6*(float)t,t/e.nc,e.nc);
				if (e.nerr>0) printf(" %9d",e.nerr); else printf("          ");
				printf(" %s\n",e.cname);
			}
		
		}	

		printf("------ ----------- ----------- --------- --------- ----------------------------\n");

	}

}

#include "_interceptor.h"

static void* h = 0;


_d_dpe( 1,libOpenCL.so, cl_int ,
clGetPlatformIDs,cl_uint ,
                 cl_platform_id* ,
                 cl_uint* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetPlatformInfo,cl_platform_id ,
                  cl_platform_info ,
                  size_t ,
                  void* ,
                  size_t* ) 


_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetDeviceIDs,cl_platform_id ,
               cl_device_type ,
               cl_uint ,
               cl_device_id* ,
               cl_uint* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetDeviceInfo,cl_device_id ,
                cl_device_info ,
                size_t ,
                void* ,
                size_t* ) 


typedef void (*pfn_notify_t)(const char*, const void*, size_t, void*);

_p_pdpppe( 1,libOpenCL.so, cl_context ,
clCreateContext,const cl_context_properties* ,
                cl_uint ,
                const cl_device_id* ,
                pfn_notify_t,
                void* ,
                cl_int* ) 

_p_ppppe( 1,libOpenCL.so, cl_context ,
clCreateContextFromType,const cl_context_properties* ,
                        cl_device_type ,
                        pfn_notify_t,
                        void* ,
                        cl_int* ) 


_d_p( 1,libOpenCL.so, cl_int ,
clRetainContext,cl_context ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseContext,cl_context ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetContextInfo,cl_context ,
                 cl_context_info ,
                 size_t ,
                 void* ,
                 size_t* ) 


_p_pppe( 1,libOpenCL.so, cl_command_queue ,
clCreateCommandQueue,cl_context ,
                     cl_device_id ,
                     cl_command_queue_properties ,
                     cl_int* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clRetainCommandQueue,cl_command_queue ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseCommandQueue,cl_command_queue ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetCommandQueueInfo,cl_command_queue ,
                      cl_command_queue_info ,
                      size_t ,
                      void* ,
                      size_t* ) 

_d_ppdp( 1,libOpenCL.so, cl_int ,
clSetCommandQueueProperty,cl_command_queue ,
                          cl_command_queue_properties ,
                          cl_bool ,
                          cl_command_queue_properties* ) 


_p_pddpe( 1,libOpenCL.so, cl_mem ,
clCreateBuffer,cl_context ,
               cl_mem_flags ,
               size_t ,
               void* ,
               cl_int* ) 

_p_pdpdddpp( 1,libOpenCL.so, cl_mem ,
clCreateImage2D,cl_context ,
                cl_mem_flags ,
                const cl_image_format* ,
                size_t ,
                size_t ,
                size_t ,
                void* ,
                cl_int* ) 

_p_pdpdddddpp( 1,libOpenCL.so, cl_mem ,
clCreateImage3D,cl_context ,
                cl_mem_flags ,
                const cl_image_format* ,
                size_t ,
                size_t ,
                size_t ,
                size_t ,
                size_t ,
                void* ,
                cl_int* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clRetainMemObject,cl_mem ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseMemObject,cl_mem ) 

_d_pdpdpp( 1,libOpenCL.so, cl_int ,
clGetSupportedImageFormats,cl_context ,
                           cl_mem_flags ,
                           cl_mem_object_type ,
                           cl_uint ,
                           cl_image_format* ,
                           cl_uint* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetMemObjectInfo,cl_mem ,
                   cl_mem_info ,
                   size_t ,
                   void* ,
                   size_t* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetImageInfo,cl_mem ,
               cl_image_info ,
               size_t ,
               void* ,
               size_t* ) 


_p_pdppp( 1,libOpenCL.so, cl_sampler ,
clCreateSampler,cl_context ,
                cl_bool ,
                cl_addressing_mode ,
                cl_filter_mode ,
                cl_int* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clRetainSampler,cl_sampler ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseSampler,cl_sampler ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetSamplerInfo,cl_sampler ,
                 cl_sampler_info ,
                 size_t ,
                 void* ,
                 size_t* ) 


_p_pdppe( 1,libOpenCL.so, cl_program ,
clCreateProgramWithSource,cl_context ,
                          cl_uint ,
                          const char** ,
                          const size_t* ,
                          cl_int* ) 

_p_pdppppe( 1,libOpenCL.so, cl_program ,
clCreateProgramWithBinary,cl_context ,
                          cl_uint ,
                          const cl_device_id* ,
                          const size_t* ,
                          const unsigned char** ,
                          cl_int* ,
                          cl_int* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clRetainProgram,cl_program ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseProgram,cl_program ) 

typedef void (*pfn_notify2_t)(cl_program , void* );
_d_pdpppp( 1,libOpenCL.so, cl_int ,
clBuildProgram,cl_program ,
               cl_uint ,
               const cl_device_id* ,
               const char*,
               pfn_notify2_t,
               void* ) 

_d( 1, libOpenCL.so, cl_int ,
clUnloadCompiler) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetProgramInfo,cl_program ,
                 cl_program_info ,
                 size_t ,
                 void* ,
                 size_t* ) 

_d_pppdpp( 1,libOpenCL.so, cl_int ,
clGetProgramBuildInfo,cl_program ,
                      cl_device_id ,
                      cl_program_build_info ,
                      size_t ,
                      void* ,
                      size_t* ) 


_p_ppe( 1,libOpenCL.so, cl_kernel ,
clCreateKernel,cl_program ,
               const char* ,
               cl_int* ) 

_d_pdpp( 1,libOpenCL.so, cl_int ,
clCreateKernelsInProgram,cl_program ,
                         cl_uint ,
                         cl_kernel* ,
                         cl_uint* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clRetainKernel,cl_kernel ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseKernel,cl_kernel ) 

_d_pddp( 1,libOpenCL.so, cl_int ,
clSetKernelArg,cl_kernel ,
               cl_uint ,
               size_t ,
               const void* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetKernelInfo,cl_kernel ,
                cl_kernel_info ,
                size_t ,
                void* ,
                size_t* ) 

_d_pppdpp( 1,libOpenCL.so, cl_int ,
clGetKernelWorkGroupInfo,cl_kernel ,
                         cl_device_id ,
                         cl_kernel_work_group_info ,
                         size_t ,
                         void* ,
                         size_t* ) 


_d_dp( 1,libOpenCL.so, cl_int ,
clWaitForEvents,cl_uint ,
                const cl_event* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetEventInfo,cl_event ,
               cl_event_info ,
               size_t ,
               void* ,
               size_t* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clRetainEvent,cl_event ) 

_d_p( 1,libOpenCL.so, cl_int ,
clReleaseEvent,cl_event ) 


_d_ppdpp( 1,libOpenCL.so, cl_int ,
clGetEventProfilingInfo,cl_event ,
                        cl_profiling_info ,
                        size_t ,
                        void* ,
                        size_t* ) 


_d_p( 1,libOpenCL.so, cl_int ,
clFlush,cl_command_queue ) 

_d_p( 1,libOpenCL.so, cl_int ,
clFinish,cl_command_queue ) 


_d_ppdddpdpp( 1,libOpenCL.so, cl_int ,
clEnqueueReadBuffer,cl_command_queue ,
                    cl_mem ,
                    cl_bool ,
                    size_t ,
                    size_t ,
                    void* ,
                    cl_uint ,
                    const cl_event* ,
                    cl_event* ) 

_d_ppdddpdpp( 1,libOpenCL.so, cl_int ,
clEnqueueWriteBuffer,cl_command_queue ,
                     cl_mem ,
                     cl_bool ,
                     size_t ,
                     size_t ,
                     const void* ,
                     cl_uint ,
                     const cl_event* ,
                     cl_event* ) 

_d_pppddddpp( 1,libOpenCL.so, cl_int ,
clEnqueueCopyBuffer,cl_command_queue ,
                    cl_mem ,
                    cl_mem ,
                    size_t ,
                    size_t ,
                    size_t ,
                    cl_uint ,
                    const cl_event* ,
                    cl_event* ) 

_d_ppdppddpdpp( 1,libOpenCL.so, cl_int ,
clEnqueueReadImage,cl_command_queue ,
                   cl_mem ,
                   cl_bool ,
                   const size_t* ,
                   const size_t* ,
                   size_t ,
                   size_t ,
                   void* ,
                   cl_uint ,
                   const cl_event* ,
                   cl_event* ) 

_d_ppdppddpdpp( 1,libOpenCL.so, cl_int ,
clEnqueueWriteImage,cl_command_queue ,
                    cl_mem ,
                    cl_bool ,
                    const size_t* ,
                    const size_t* ,
                    size_t ,
                    size_t ,
                    const void* ,
                    cl_uint ,
                    const cl_event* ,
                    cl_event* ) 

_d_ppppppdpp( 1,libOpenCL.so, cl_int ,
clEnqueueCopyImage,cl_command_queue ,
                   cl_mem ,
                   cl_mem ,
                   const size_t* ,
                   const size_t* ,
                   const size_t* ,
                   cl_uint ,
                   const cl_event* ,
                   cl_event* ) 

_d_pppppddpp( 1,libOpenCL.so, cl_int ,
clEnqueueCopyImageToBuffer,cl_command_queue ,
                           cl_mem ,
                           cl_mem ,
                           const size_t* ,
                           const size_t* ,
                           size_t ,
                           cl_uint ,
                           const cl_event* ,
                           cl_event* ) 

_d_pppdppdpp( 1,libOpenCL.so, cl_int ,
clEnqueueCopyBufferToImage,cl_command_queue ,
                           cl_mem ,
                           cl_mem ,
                           size_t ,
                           const size_t* ,
                           const size_t* ,
                           cl_uint ,
                           const cl_event* ,
                           cl_event* ) 

_d_ppdddddppe( 1,libOpenCL.so, void* ,
clEnqueueMapBuffer,cl_command_queue ,
                   cl_mem ,
                   cl_bool ,
                   cl_map_flags ,
                   size_t ,
                   size_t ,
                   cl_uint ,
                   const cl_event* ,
                   cl_event* ,
                   cl_int* ) 

_p_ppddppppdppp( 1,libOpenCL.so, void* ,
clEnqueueMapImage,cl_command_queue ,
                  cl_mem ,
                  cl_bool ,
                  cl_map_flags ,
                  const size_t* ,
                  const size_t* ,
                  size_t* ,
                  size_t* ,
                  cl_uint ,
                  const cl_event* ,
                  cl_event* ,
                  cl_int* ) 

_d_pppdpp( 1,libOpenCL.so, cl_int ,
clEnqueueUnmapMemObject,cl_command_queue ,
                        cl_mem ,
                        void* ,
                        cl_uint ,
                        const cl_event* ,
                        cl_event* ) 

_d_ppdpppdpp( 1,libOpenCL.so, cl_int ,
clEnqueueNDRangeKernel,cl_command_queue ,
                       cl_kernel ,
                       cl_uint ,
                       const size_t* ,
                       const size_t* ,
                       const size_t* ,
                       cl_uint ,
                       const cl_event* ,
                       cl_event* ) 

_d_ppdpp( 1,libOpenCL.so, cl_int ,
clEnqueueTask,cl_command_queue ,
              cl_kernel ,
              cl_uint ,
              const cl_event* ,
              cl_event* ) 

typedef void (*user_func_t)(void*);

_d_pppddppdpp( 1,libOpenCL.so, cl_int ,
clEnqueueNativeKernel,cl_command_queue ,
       user_func_t,
                      void* ,
                      size_t ,
                      cl_uint ,
                      const cl_mem* ,
                      const void** ,
                      cl_uint ,
                      const cl_event* ,
                      cl_event* ) 

_d_pp( 1,libOpenCL.so, cl_int ,
clEnqueueMarker,cl_command_queue ,
                cl_event* ) 

_d_pdp( 1,libOpenCL.so, cl_int ,
clEnqueueWaitForEvents,cl_command_queue ,
                       cl_uint ,
                       const cl_event* ) 

_d_p( 1,libOpenCL.so, cl_int ,
clEnqueueBarrier,cl_command_queue ) 



/* special intercepts for libstdcl compat */

void _libstdcl_init(void) { 
	typedef void (*pfunc_t)(void); 
	if (!h) h = dlopen("libstdcl.so", RTLD_LAZY); 
	if (!h) { fprintf(stderr,"libcltrace1: open libstdcl failed\n"); exit(-1); } 
	pfunc_t pf = (pfunc_t)dlsym(h,"_libstdcl_init"); 
	if (!pf) { 
		fprintf(stderr,"libcltrace1: get symbol " "_libstdcl_init" " failed\n"); 
		exit(-1); 
	} 
//	struct timeval time0,time1; 
//	gettimeofday(&time0,0); 

		fini_defer = 1;
		_libcltrace1_init();
		pf(); 
//	gettimeofday(&time1,0); 
//	printf("cltrace1(%d): ""clEnqueueBarrier""(%p) = 0\n",
//		(time1.tv_sec-time0.tv_sec),a0,retval); 
	fflush(stdout); 
	return; 
}


void _libstdcl_fini(void) { 
	typedef void (*pfunc_t)(void); 
	if (!h) h = dlopen("libstdcl.so", RTLD_LAZY); 
	if (!h) { fprintf(stderr,"libcltrace1: open libstdcl failed\n"); exit(-1); } 
	pfunc_t pf = (pfunc_t)dlsym(h,"_libstdcl_fini"); 
	if (!pf) { 
		fprintf(stderr,"libcltrace1: get symbol " "_libstdcl_fini" " failed\n"); 
		exit(-1); 
	} 
//	struct timeval time0,time1; 
//	gettimeofday(&time0,0); 

		pf(); 

		if (fini_defer==2) { fini_defer=0; _libcltrace1_fini(); }
		else fini_defer = 0;

//	gettimeofday(&time1,0); 
//	printf("cltrace1(%d): ""clEnqueueBarrier""(%p) = 0\n",
//		(time1.tv_sec-time0.tv_sec),a0,retval); 
	fflush(stdout); 
	return; 
}


