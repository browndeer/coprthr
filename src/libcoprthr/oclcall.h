
#ifndef _OCLCALL_H
#define _OCLCALL_H


#include <CL/cl.h>


#include <CL/cl_gl.h>


struct oclent_struct {
	unsigned int ocl_narg;
	void* ocl_call;
	struct ocltrace_struct* ocl_trace;
};

#define oclent_sz sizeof(struct oclent_struct)

typedef void (*cl_pfn_notify_t)(const char*, const void*, size_t, void*);
typedef void (*cl_pfn_notify2_t)(cl_program , void* );
typedef void (*cl_user_func_t)(void*);

#define OCLCALL_clGetPlatformIDs	0
#define OCLCALL_clGetPlatformInfo	1
#define OCLCALL_clGetDeviceIDs	2
#define OCLCALL_clGetDeviceInfo	3
#define OCLCALL_clCreateContext	4
#define OCLCALL_clCreateContextFromType	5
#define OCLCALL_clRetainContext	6
#define OCLCALL_clReleaseContext	7
#define OCLCALL_clGetContextInfo	8
#define OCLCALL_clCreateCommandQueue	9
#define OCLCALL_clRetainCommandQueue	10
#define OCLCALL_clReleaseCommandQueue	11
#define OCLCALL_clGetCommandQueueInfo	12
#define OCLCALL_clSetCommandQueueProperty	13
#define OCLCALL_clCreateBuffer	14
#define OCLCALL_clCreateImage2D	15
#define OCLCALL_clCreateImage3D	16
#define OCLCALL_clRetainMemObject	17
#define OCLCALL_clReleaseMemObject	18
#define OCLCALL_clGetSupportedImageFormats	19
#define OCLCALL_clGetMemObjectInfo	20
#define OCLCALL_clGetImageInfo	21
#define OCLCALL_clCreateSampler	22
#define OCLCALL_clRetainSampler	23
#define OCLCALL_clReleaseSampler	24
#define OCLCALL_clGetSamplerInfo	25
#define OCLCALL_clCreateProgramWithSource	26
#define OCLCALL_clCreateProgramWithBinary	27
#define OCLCALL_clRetainProgram	28
#define OCLCALL_clReleaseProgram	29
#define OCLCALL_clBuildProgram	30
#define OCLCALL_clUnloadCompiler	31
#define OCLCALL_clGetProgramInfo	32
#define OCLCALL_clGetProgramBuildInfo	33
#define OCLCALL_clCreateKernel	34
#define OCLCALL_clCreateKernelsInProgram	35
#define OCLCALL_clRetainKernel	36
#define OCLCALL_clReleaseKernel	37
#define OCLCALL_clSetKernelArg	38
#define OCLCALL_clGetKernelInfo	39
#define OCLCALL_clGetKernelWorkGroupInfo	40
#define OCLCALL_clWaitForEvents	41
#define OCLCALL_clGetEventInfo	42
#define OCLCALL_clRetainEvent	43
#define OCLCALL_clReleaseEvent	44
#define OCLCALL_clGetEventProfilingInfo	45
#define OCLCALL_clFlush	46
#define OCLCALL_clFinish	47
#define OCLCALL_clEnqueueReadBuffer	48
#define OCLCALL_clEnqueueWriteBuffer	49
#define OCLCALL_clEnqueueCopyBuffer	50
#define OCLCALL_clEnqueueReadImage	51
#define OCLCALL_clEnqueueWriteImage	52
#define OCLCALL_clEnqueueCopyImage	53
#define OCLCALL_clEnqueueCopyImageToBuffer	54
#define OCLCALL_clEnqueueCopyBufferToImage	55
#define OCLCALL_clEnqueueMapBuffer	56
#define OCLCALL_clEnqueueMapImage	57
#define OCLCALL_clEnqueueUnmapMemObject	58
#define OCLCALL_clEnqueueNDRangeKernel	59
#define OCLCALL_clEnqueueTask	60
#define OCLCALL_clEnqueueNativeKernel	61
#define OCLCALL_clEnqueueMarker	62
#define OCLCALL_clEnqueueWaitForEvents	63
#define OCLCALL_clEnqueueBarrier	64
#define OCLCALL_clCreateFromGLBuffer	65
#define OCLCALL_clCreateFromGLTexture2D	66
#define OCLCALL_clCreateFromGLTexture3D	67
#define OCLCALL_clCreateFromGLRenderbuffer	68
#define OCLCALL_clGetGLObjectInfo	69
#define OCLCALL_clGetGLTextureInfo	70
#define OCLCALL_clEnqueueAcquireGLObjects	71
#define OCLCALL_clEnqueueReleaseGLObjects	72

#define OCLCALL_NARG_reserved 0

#define OCLCALL_NARG_clGetPlatformIDs	3
#define OCLCALL_NARG_clGetPlatformInfo	5
#define OCLCALL_NARG_clGetDeviceIDs	5
#define OCLCALL_NARG_clGetDeviceInfo	5
#define OCLCALL_NARG_clCreateContext	6
#define OCLCALL_NARG_clCreateContextFromType	5
#define OCLCALL_NARG_clRetainContext	1
#define OCLCALL_NARG_clReleaseContext	1
#define OCLCALL_NARG_clGetContextInfo	5
#define OCLCALL_NARG_clCreateCommandQueue	4
#define OCLCALL_NARG_clRetainCommandQueue	1
#define OCLCALL_NARG_clReleaseCommandQueue	1
#define OCLCALL_NARG_clGetCommandQueueInfo	5
#define OCLCALL_NARG_clSetCommandQueueProperty	4
#define OCLCALL_NARG_clCreateBuffer	5
#define OCLCALL_NARG_clCreateImage2D	8
#define OCLCALL_NARG_clCreateImage3D	10
#define OCLCALL_NARG_clRetainMemObject	1
#define OCLCALL_NARG_clReleaseMemObject	1
#define OCLCALL_NARG_clGetSupportedImageFormats	6
#define OCLCALL_NARG_clGetMemObjectInfo	5
#define OCLCALL_NARG_clGetImageInfo	5
#define OCLCALL_NARG_clCreateSampler	5
#define OCLCALL_NARG_clRetainSampler	1
#define OCLCALL_NARG_clReleaseSampler	1
#define OCLCALL_NARG_clGetSamplerInfo	5
#define OCLCALL_NARG_clCreateProgramWithSource	5
#define OCLCALL_NARG_clCreateProgramWithBinary	7
#define OCLCALL_NARG_clRetainProgram	1
#define OCLCALL_NARG_clReleaseProgram	1
#define OCLCALL_NARG_clBuildProgram	6
#define OCLCALL_NARG_clUnloadCompiler	0
#define OCLCALL_NARG_clGetProgramInfo	5
#define OCLCALL_NARG_clGetProgramBuildInfo	6
#define OCLCALL_NARG_clCreateKernel	3
#define OCLCALL_NARG_clCreateKernelsInProgram	4
#define OCLCALL_NARG_clRetainKernel	1
#define OCLCALL_NARG_clReleaseKernel	1
#define OCLCALL_NARG_clSetKernelArg	4
#define OCLCALL_NARG_clGetKernelInfo	5
#define OCLCALL_NARG_clGetKernelWorkGroupInfo	6
#define OCLCALL_NARG_clWaitForEvents	2
#define OCLCALL_NARG_clGetEventInfo	5
#define OCLCALL_NARG_clRetainEvent	1
#define OCLCALL_NARG_clReleaseEvent	1
#define OCLCALL_NARG_clGetEventProfilingInfo	5
#define OCLCALL_NARG_clFlush	1
#define OCLCALL_NARG_clFinish	1
#define OCLCALL_NARG_clEnqueueReadBuffer	9
#define OCLCALL_NARG_clEnqueueWriteBuffer	9
#define OCLCALL_NARG_clEnqueueCopyBuffer	9
#define OCLCALL_NARG_clEnqueueReadImage	11
#define OCLCALL_NARG_clEnqueueWriteImage	11
#define OCLCALL_NARG_clEnqueueCopyImage	9
#define OCLCALL_NARG_clEnqueueCopyImageToBuffer	9
#define OCLCALL_NARG_clEnqueueCopyBufferToImage	9
#define OCLCALL_NARG_clEnqueueMapBuffer	10
#define OCLCALL_NARG_clEnqueueMapImage	12
#define OCLCALL_NARG_clEnqueueUnmapMemObject	6
#define OCLCALL_NARG_clEnqueueNDRangeKernel	9
#define OCLCALL_NARG_clEnqueueTask	5
#define OCLCALL_NARG_clEnqueueNativeKernel	10
#define OCLCALL_NARG_clEnqueueMarker	2
#define OCLCALL_NARG_clEnqueueWaitForEvents	3
#define OCLCALL_NARG_clEnqueueBarrier	1
#define OCLCALL_NARG_clCreateFromGLBuffer	4
#define OCLCALL_NARG_clCreateFromGLTexture2D	6
#define OCLCALL_NARG_clCreateFromGLTexture3D	6
#define OCLCALL_NARG_clCreateFromGLRenderbuffer	4
#define OCLCALL_NARG_clGetGLObjectInfo	3
#define OCLCALL_NARG_clGetGLTextureInfo	5
#define OCLCALL_NARG_clEnqueueAcquireGLObjects	6
#define OCLCALL_NARG_clEnqueueReleaseGLObjects	6

extern char* oclcallnames[];

extern struct oclent_struct empty_oclent[];
extern unsigned int oclncalls;

#define __DECL_API_CALLS( prefix, suffix ) \
	cl_int prefix##clGetPlatformIDs##suffix(cl_uint a0,cl_platform_id* a1,cl_uint* a2);\
	cl_int prefix##clGetPlatformInfo##suffix(cl_platform_id a0,cl_platform_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clGetDeviceIDs##suffix(cl_platform_id a0,cl_device_type a1,cl_uint a2,cl_device_id* a3,cl_uint* a4);\
	cl_int prefix##clGetDeviceInfo##suffix(cl_device_id a0,cl_device_info a1,size_t a2,void* a3,size_t* a4);\
	cl_context prefix##clCreateContext##suffix(const cl_context_properties* a0,cl_uint a1,const cl_device_id* a2,cl_pfn_notify_t a3,void* a4,cl_int* a5);\
	cl_context prefix##clCreateContextFromType##suffix(const cl_context_properties* a0,cl_device_type a1,cl_pfn_notify_t a2,void* a3,cl_int* a4);\
	cl_int prefix##clRetainContext##suffix(cl_context a0);\
	cl_int prefix##clReleaseContext##suffix(cl_context a0);\
	cl_int prefix##clGetContextInfo##suffix(cl_context a0,cl_context_info a1,size_t a2,void* a3,size_t* a4);\
	cl_command_queue prefix##clCreateCommandQueue##suffix(cl_context a0,cl_device_id a1,cl_command_queue_properties a2,cl_int* a3);\
	cl_int prefix##clRetainCommandQueue##suffix(cl_command_queue a0);\
	cl_int prefix##clReleaseCommandQueue##suffix(cl_command_queue a0);\
	cl_int prefix##clGetCommandQueueInfo##suffix(cl_command_queue a0,cl_command_queue_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clSetCommandQueueProperty##suffix(cl_command_queue a0,cl_command_queue_properties a1,cl_bool a2,cl_command_queue_properties* a3);\
	cl_mem prefix##clCreateBuffer##suffix(cl_context a0,cl_mem_flags a1,size_t a2,void* a3,cl_int* a4);\
	cl_mem prefix##clCreateImage2D##suffix(cl_context a0,cl_mem_flags a1,const cl_image_format* a2,size_t a3,size_t a4,size_t a5,void* a6,cl_int* a7);\
	cl_mem prefix##clCreateImage3D##suffix(cl_context a0,cl_mem_flags a1,const cl_image_format* a2,size_t a3,size_t a4,size_t a5,size_t a6,size_t a7,void* a8,cl_int* a9);\
	cl_int prefix##clRetainMemObject##suffix(cl_mem a0);\
	cl_int prefix##clReleaseMemObject##suffix(cl_mem a0);\
	cl_int prefix##clGetSupportedImageFormats##suffix(cl_context a0,cl_mem_flags a1,cl_mem_object_type a2,cl_uint a3,cl_image_format* a4,cl_uint* a5);\
	cl_int prefix##clGetMemObjectInfo##suffix(cl_mem a0,cl_mem_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clGetImageInfo##suffix(cl_mem a0,cl_image_info a1,size_t a2,void* a3,size_t* a4);\
	cl_sampler prefix##clCreateSampler##suffix(cl_context a0,cl_bool a1,cl_addressing_mode a2,cl_filter_mode a3,cl_int* a4);\
	cl_int prefix##clRetainSampler##suffix(cl_sampler a0);\
	cl_int prefix##clReleaseSampler##suffix(cl_sampler a0);\
	cl_int prefix##clGetSamplerInfo##suffix(cl_sampler a0,cl_sampler_info a1,size_t a2,void* a3,size_t* a4);\
	cl_program prefix##clCreateProgramWithSource##suffix(cl_context a0,cl_uint a1,const char** a2,const size_t* a3,cl_int* a4);\
	cl_program prefix##clCreateProgramWithBinary##suffix(cl_context a0,cl_uint a1,const cl_device_id* a2,const size_t* a3,const unsigned char** a4,cl_int* a5,cl_int* a6);\
	cl_int prefix##clRetainProgram##suffix(cl_program a0);\
	cl_int prefix##clReleaseProgram##suffix(cl_program a0);\
	cl_int prefix##clBuildProgram##suffix(cl_program a0,cl_uint a1,const cl_device_id* a2,const char* a3,cl_pfn_notify2_t a4,void* a5);\
	cl_int prefix##clUnloadCompiler##suffix(void a0);\
	cl_int prefix##clGetProgramInfo##suffix(cl_program a0,cl_program_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clGetProgramBuildInfo##suffix(cl_program a0,cl_device_id a1,cl_program_build_info a2,size_t a3,void* a4,size_t* a5);\
	cl_kernel prefix##clCreateKernel##suffix(cl_program a0,const char* a1,cl_int* a2);\
	cl_int prefix##clCreateKernelsInProgram##suffix(cl_program a0,cl_uint a1,cl_kernel* a2,cl_uint* a3);\
	cl_int prefix##clRetainKernel##suffix(cl_kernel a0);\
	cl_int prefix##clReleaseKernel##suffix(cl_kernel a0);\
	cl_int prefix##clSetKernelArg##suffix(cl_kernel a0,cl_uint a1,size_t a2,const void* a3);\
	cl_int prefix##clGetKernelInfo##suffix(cl_kernel a0,cl_kernel_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clGetKernelWorkGroupInfo##suffix(cl_kernel a0,cl_device_id a1,cl_kernel_work_group_info a2,size_t a3,void* a4,size_t* a5);\
	cl_int prefix##clWaitForEvents##suffix(cl_uint a0,const cl_event* a1);\
	cl_int prefix##clGetEventInfo##suffix(cl_event a0,cl_event_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clRetainEvent##suffix(cl_event a0);\
	cl_int prefix##clReleaseEvent##suffix(cl_event a0);\
	cl_int prefix##clGetEventProfilingInfo##suffix(cl_event a0,cl_profiling_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clFlush##suffix(cl_command_queue a0);\
	cl_int prefix##clFinish##suffix(cl_command_queue a0);\
	cl_int prefix##clEnqueueReadBuffer##suffix(cl_command_queue a0,cl_mem a1,cl_bool a2,size_t a3,size_t a4,void* a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	cl_int prefix##clEnqueueWriteBuffer##suffix(cl_command_queue a0,cl_mem a1,cl_bool a2,size_t a3,size_t a4,const void* a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	cl_int prefix##clEnqueueCopyBuffer##suffix(cl_command_queue a0,cl_mem a1,cl_mem a2,size_t a3,size_t a4,size_t a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	cl_int prefix##clEnqueueReadImage##suffix(cl_command_queue a0,cl_mem a1,cl_bool a2,const size_t* a3,const size_t* a4,size_t a5,size_t a6,void* a7,cl_uint a8,const cl_event* a9,cl_event* a10);\
	cl_int prefix##clEnqueueWriteImage##suffix(cl_command_queue a0,cl_mem a1,cl_bool a2,const size_t* a3,const size_t* a4,size_t a5,size_t a6,const void* a7,cl_uint a8,const cl_event* a9,cl_event* a10);\
	cl_int prefix##clEnqueueCopyImage##suffix(cl_command_queue a0,cl_mem a1,cl_mem a2,const size_t* a3,const size_t* a4,const size_t* a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	cl_int prefix##clEnqueueCopyImageToBuffer##suffix(cl_command_queue a0,cl_mem a1,cl_mem a2,const size_t* a3,const size_t* a4,size_t a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	cl_int prefix##clEnqueueCopyBufferToImage##suffix(cl_command_queue a0,cl_mem a1,cl_mem a2,size_t a3,const size_t* a4,const size_t* a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	void* prefix##clEnqueueMapBuffer##suffix(cl_command_queue a0,cl_mem a1,cl_bool a2,cl_map_flags a3,size_t a4,size_t a5,cl_uint a6,const cl_event* a7,cl_event* a8,cl_int* a9);\
	void* prefix##clEnqueueMapImage##suffix(cl_command_queue a0,cl_mem a1,cl_bool a2,cl_map_flags a3,const size_t* a4,const size_t* a5,size_t* a6,size_t* a7,cl_uint a8,const cl_event* a9,cl_event* a10,cl_int* a11);\
	cl_int prefix##clEnqueueUnmapMemObject##suffix(cl_command_queue a0,cl_mem a1,void* a2,cl_uint a3,const cl_event* a4,cl_event* a5);\
	cl_int prefix##clEnqueueNDRangeKernel##suffix(cl_command_queue a0,cl_kernel a1,cl_uint a2,const size_t* a3,const size_t* a4,const size_t* a5,cl_uint a6,const cl_event* a7,cl_event* a8);\
	cl_int prefix##clEnqueueTask##suffix(cl_command_queue a0,cl_kernel a1,cl_uint a2,const cl_event* a3,cl_event* a4);\
	cl_int prefix##clEnqueueNativeKernel##suffix(cl_command_queue a0,cl_user_func_t a1,void* a2,size_t a3,cl_uint a4,const cl_mem* a5,const void** a6,cl_uint a7,const cl_event* a8,cl_event* a9);\
	cl_int prefix##clEnqueueMarker##suffix(cl_command_queue a0,cl_event* a1);\
	cl_int prefix##clEnqueueWaitForEvents##suffix(cl_command_queue a0,cl_uint a1,const cl_event* a2);\
	cl_int prefix##clEnqueueBarrier##suffix(cl_command_queue a0);\
	cl_mem prefix##clCreateFromGLBuffer##suffix(cl_context a0,cl_mem_flags a1,cl_GLuint a2,int* a3);\
	cl_mem prefix##clCreateFromGLTexture2D##suffix(cl_context a0,cl_mem_flags a1,cl_GLenum a2,cl_GLint a3,cl_GLuint a4,cl_int* a5);\
	cl_mem prefix##clCreateFromGLTexture3D##suffix(cl_context a0,cl_mem_flags a1,cl_GLenum a2,cl_GLint a3,cl_GLuint a4,cl_int* a5);\
	cl_mem prefix##clCreateFromGLRenderbuffer##suffix(cl_context a0,cl_mem_flags a1,cl_GLuint a2,cl_int* a3);\
	cl_int prefix##clGetGLObjectInfo##suffix(cl_mem a0,cl_gl_object_type* a1,cl_GLuint* a2);\
	cl_int prefix##clGetGLTextureInfo##suffix(cl_mem a0,cl_gl_texture_info a1,size_t a2,void* a3,size_t* a4);\
	cl_int prefix##clEnqueueAcquireGLObjects##suffix(cl_command_queue a0,cl_uint a1,const cl_mem* a2,cl_uint a3,const cl_event* a4,cl_event* a5);\
	cl_int prefix##clEnqueueReleaseGLObjects##suffix(cl_command_queue a0,cl_uint a1,const cl_mem* a2,cl_uint a3,const cl_event* a4,cl_event* a5);\


#define __set_icd_call_vector( prefix, suffix ) { \
	prefix##clGetPlatformIDs##suffix, \
	prefix##clGetPlatformInfo##suffix, \
	prefix##clGetDeviceIDs##suffix, \
	prefix##clGetDeviceInfo##suffix, \
	prefix##clCreateContext##suffix, \
	prefix##clCreateContextFromType##suffix, \
	prefix##clRetainContext##suffix, \
	prefix##clReleaseContext##suffix, \
	prefix##clGetContextInfo##suffix, \
	prefix##clCreateCommandQueue##suffix, \
	prefix##clRetainCommandQueue##suffix, \
	prefix##clReleaseCommandQueue##suffix, \
	prefix##clGetCommandQueueInfo##suffix, \
	prefix##clSetCommandQueueProperty##suffix, \
	prefix##clCreateBuffer##suffix, \
	prefix##clCreateImage2D##suffix, \
	prefix##clCreateImage3D##suffix, \
	prefix##clRetainMemObject##suffix, \
	prefix##clReleaseMemObject##suffix, \
	prefix##clGetSupportedImageFormats##suffix, \
	prefix##clGetMemObjectInfo##suffix, \
	prefix##clGetImageInfo##suffix, \
	prefix##clCreateSampler##suffix, \
	prefix##clRetainSampler##suffix, \
	prefix##clReleaseSampler##suffix, \
	prefix##clGetSamplerInfo##suffix, \
	prefix##clCreateProgramWithSource##suffix, \
	prefix##clCreateProgramWithBinary##suffix, \
	prefix##clRetainProgram##suffix, \
	prefix##clReleaseProgram##suffix, \
	prefix##clBuildProgram##suffix, \
	prefix##clUnloadCompiler##suffix, \
	prefix##clGetProgramInfo##suffix, \
	prefix##clGetProgramBuildInfo##suffix, \
	prefix##clCreateKernel##suffix, \
	prefix##clCreateKernelsInProgram##suffix, \
	prefix##clRetainKernel##suffix, \
	prefix##clReleaseKernel##suffix, \
	prefix##clSetKernelArg##suffix, \
	prefix##clGetKernelInfo##suffix, \
	prefix##clGetKernelWorkGroupInfo##suffix, \
	prefix##clWaitForEvents##suffix, \
	prefix##clGetEventInfo##suffix, \
	prefix##clRetainEvent##suffix, \
	prefix##clReleaseEvent##suffix, \
	prefix##clGetEventProfilingInfo##suffix, \
	prefix##clFlush##suffix, \
	prefix##clFinish##suffix, \
	prefix##clEnqueueReadBuffer##suffix, \
	prefix##clEnqueueWriteBuffer##suffix, \
	prefix##clEnqueueCopyBuffer##suffix, \
	prefix##clEnqueueReadImage##suffix, \
	prefix##clEnqueueWriteImage##suffix, \
	prefix##clEnqueueCopyImage##suffix, \
	prefix##clEnqueueCopyImageToBuffer##suffix, \
	prefix##clEnqueueCopyBufferToImage##suffix, \
	prefix##clEnqueueMapBuffer##suffix, \
	prefix##clEnqueueMapImage##suffix, \
	prefix##clEnqueueUnmapMemObject##suffix, \
	prefix##clEnqueueNDRangeKernel##suffix, \
	prefix##clEnqueueTask##suffix, \
	prefix##clEnqueueNativeKernel##suffix, \
	prefix##clEnqueueMarker##suffix, \
	prefix##clEnqueueWaitForEvents##suffix, \
	prefix##clEnqueueBarrier##suffix, \
	prefix##clCreateFromGLBuffer##suffix, \
	prefix##clCreateFromGLTexture2D##suffix, \
	prefix##clCreateFromGLTexture3D##suffix, \
	prefix##clCreateFromGLRenderbuffer##suffix, \
	prefix##clGetGLObjectInfo##suffix, \
	prefix##clGetGLTextureInfo##suffix, \
	prefix##clEnqueueAcquireGLObjects##suffix, \
	prefix##clEnqueueReleaseGLObjects##suffix, \
	}

#endif
