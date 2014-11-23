/* ocl_device.c 
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
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


#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "device.h"
#include "coprthr_arch.h"

void __do_discover_devices( unsigned int* p_ndevices, 
	struct _cl_device_id** p_dtab, int flag);

void __do_get_ndevices( cl_platform_id platformid, cl_device_type devtype, 
	cl_uint* ndev );

void __do_get_devices( cl_platform_id platformid, cl_device_type devtype, 
	cl_uint ndev, cl_device_id* devices);

void __do_discover_opencl_device_info_x86_64(
	struct coprthr_device* dev,
	struct opencl_device_info* ocldevinfo );

void __do_discover_opencl_device_info_e32(
	struct coprthr_device* dev,
	struct opencl_device_info* ocldevinfo );

#define min(a,b) ((a<b)?a:b)

// Device API Calls

cl_int 
_clGetDeviceIDs(
	cl_platform_id platformid,
	cl_device_type devtype,
	cl_uint ndev,  
	cl_device_id* devices,
	cl_uint* ndev_ret
)
{
	if (__invalid_platform_id(platformid)) return(CL_INVALID_PLATFORM);

	if (ndev == 0 && devices) return(CL_INVALID_VALUE);

	if (!devices && !ndev_ret) return(CL_INVALID_VALUE);

	
	cl_uint n;

	__do_get_ndevices(platformid,devtype,&n);

	if (ndev_ret) *ndev_ret = n;

	if (n == 0) return(CL_DEVICE_NOT_FOUND);

//	if (ndev_ret) *ndev_ret = n;

	n = min(n,ndev);

	__do_get_devices(platformid,devtype,n,devices);

	return(CL_SUCCESS);
}


cl_int 
_clGetDeviceInfo(
	cl_device_id devid,
	cl_device_info param_name,
	size_t param_sz, 
	void* param_val,
	size_t* param_sz_ret
) 
{
	if (__invalid_device_id(devid)) return(CL_INVALID_DEVICE);

	size_t sz;

	switch (param_name) {

		case CL_DEVICE_TYPE:

			__case_get_param(sizeof(cl_device_type),
				&__resolve_devid_ocldevinfo(devid,devtype));

			break;

		case CL_DEVICE_VENDOR_ID:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,vendorid));

			break;

		case CL_DEVICE_MAX_COMPUTE_UNITS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,max_compute_units));

			break;
		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,max_wi_dim));

			break;

		case CL_DEVICE_MAX_WORK_ITEM_SIZES:

			__case_get_param(
				__resolve_devid_ocldevinfo(devid,max_wi_dim)*sizeof(size_t*),
				&__resolve_devid_ocldevinfo(devid,max_wi_sz));

			break;

		case CL_DEVICE_MAX_WORK_GROUP_SIZE:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,max_wg_sz));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,pref_charn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,pref_shortn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,pref_intn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,pref_longn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,pref_floatn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,pref_doublen));

			break;

		case CL_DEVICE_MAX_CLOCK_FREQUENCY:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,max_freq));

			break;

		case CL_DEVICE_ADDRESS_BITS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,addr_bits));

			break;

		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid_ocldevinfo(devid,max_mem_alloc_sz));

			break;

		case CL_DEVICE_IMAGE_SUPPORT:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid_ocldevinfo(devid,supp_img));

			break;

		case CL_DEVICE_MAX_READ_IMAGE_ARGS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,img_max_narg_r));

			break;

		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,img_max_narg_w));

			break;

		case CL_DEVICE_IMAGE2D_MAX_WIDTH:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,img2d_max_width));

			break;

		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,img2d_max_height));

			break;

		case CL_DEVICE_IMAGE3D_MAX_WIDTH:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,img3d_max_width));

			break;

		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:


			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,img3d_max_height));

			break;

		case CL_DEVICE_IMAGE3D_MAX_DEPTH:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,img3d_max_depth));

			break;

		case CL_DEVICE_MAX_SAMPLERS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,max_samplers));

			break;

		case CL_DEVICE_MAX_PARAMETER_SIZE:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,max_param_sz));

			break;

		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:	

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,mem_align_bits)); 

			break;


		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,datatype_align_sz));

			break;

		case CL_DEVICE_SINGLE_FP_CONFIG:

			__case_get_param(sizeof(cl_device_fp_config),
				&__resolve_devid_ocldevinfo(devid,single_fp_config));

			break;

		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:

			__case_get_param(sizeof(cl_device_mem_cache_type),
				&__resolve_devid_ocldevinfo(devid,global_mem_cache_type));

			break;

		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,global_mem_cacheline_sz));

			break;

		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid_ocldevinfo(devid,global_mem_cache_sz));

			break;

		case CL_DEVICE_GLOBAL_MEM_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid_ocldevinfo(devid,global_mem_sz));

			break;

		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid_ocldevinfo(devid,max_const_buffer_sz));

			break;

		case CL_DEVICE_MAX_CONSTANT_ARGS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid_ocldevinfo(devid,max_const_narg));

			break;

		case CL_DEVICE_LOCAL_MEM_TYPE:

			__case_get_param(sizeof(cl_device_local_mem_type),
				&__resolve_devid_ocldevinfo(devid,local_mem_type));

			break;

		case CL_DEVICE_LOCAL_MEM_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid_ocldevinfo(devid,local_mem_sz));

			break;

		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid_ocldevinfo(devid,supp_ec));

			break;

		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:

			__case_get_param(sizeof(size_t),
				&__resolve_devid_ocldevinfo(devid,prof_timer_res));

			break;

		case CL_DEVICE_ENDIAN_LITTLE:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid_ocldevinfo(devid,endian_little));

			break;

		case CL_DEVICE_AVAILABLE:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid_ocldevinfo(devid,avail));

			break;

		case CL_DEVICE_COMPILER_AVAILABLE:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid_ocldevinfo(devid,compiler_avail));

			break;

		case CL_DEVICE_EXECUTION_CAPABILITIES:

			__case_get_param(sizeof(cl_device_exec_capabilities),
				&__resolve_devid_ocldevinfo(devid,supp_exec_cap));

			break;

		case CL_DEVICE_QUEUE_PROPERTIES:

			__case_get_param(sizeof(cl_command_queue_properties),
				&__resolve_devid_ocldevinfo(devid,cmdq_prop));

			break;

		case CL_DEVICE_PLATFORM:

			__case_get_param(sizeof(cl_platform_id),
				&__resolve_devid_ocldevinfo(devid,platformid));

			break;

		case CL_DEVICE_NAME:

			__case_get_param(
				1+strnlen(__resolve_devid_devinfo(devid,name),__CLMAXSTR_BUFSZ),
				__resolve_devid_devinfo(devid,name));

			break;

		case CL_DEVICE_VENDOR:

			__case_get_param(
				1+strnlen(__resolve_devid_devinfo(devid,vendor),__CLMAXSTR_BUFSZ),
				__resolve_devid_devinfo(devid,vendor));

			break;

		case CL_DRIVER_VERSION:

			__case_get_param(
				1+strnlen(__resolve_devid_devinfo(devid,drv_version),__CLMAXSTR_BUFSZ),
				__resolve_devid_devinfo(devid,drv_version));

			break;

		case CL_DEVICE_PROFILE:

			__case_get_param(
				1+strnlen(__resolve_devid_devinfo(devid,profile),__CLMAXSTR_BUFSZ),
				__resolve_devid_devinfo(devid,profile));

			break;

		case CL_DEVICE_VERSION:

			__case_get_param(
				1+strnlen(__resolve_devid_devinfo(devid,version),__CLMAXSTR_BUFSZ),
				__resolve_devid_devinfo(devid,version));

			break;

		case CL_DEVICE_EXTENSIONS:

			__case_get_param(
				1+strnlen(__resolve_devid_ocldevinfo(devid,extensions),__CLMAXSTR_BUFSZ),
				__resolve_devid_ocldevinfo(devid,extensions));

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


// Aliased Device APi Calls

cl_int
clGetDeviceIDs( cl_platform_id platformid, cl_device_type devtype, 
	cl_uint ndev, cl_device_id* devices, cl_uint* ndev_ret)
	__attribute__((alias("_clGetDeviceIDs")));


cl_int
clGetDeviceInfo( cl_device_id devid, cl_device_info param_name,
   size_t param_sz, void* param_val, size_t* param_sz_ret)
	__attribute__((alias("_clGetDeviceInfo")));



/*
 * Internal device implementation calls
 */

/*
int getenv_token( const char* name, const char* token, char* value, size_t n );

void* dlh_compiler = 0;


static char* strnlen_ws( char* p, char* s, size_t maxlen)
{
	size_t sz = strnlen(s,maxlen);
	char* p1 = s;
	char* p2 = s + sz;
	while(p1 < p2 && (*p1==' ' || *p1=='\t' || *p1=='\n')) ++p1;
	while(--p2 > s && (*p2==' ' || *p2=='\t' || *p2=='\n')) *p2='\0';
	return(p1);
}

static char* truncate_ws(char* buf)
{
	size_t sz = strnlen(buf,__CLMAXSTR_LEN);
	char* p = buf + sz - 1;
	while( p>buf && (*p==' '||*p=='\t'||*p=='\n')) *(p--) = '\0'; 
	p = buf;
	while( p<buf+sz && (*p==' '||*p=='\t'||*p=='\n')) ++p; 
	return(p);
}

#define COPRTHR_DEVICE_NSUPP_MAX 2
*/

//struct coprthr_device* __coprthr_do_discover_device_x86_64(void);
//struct coprthr_device* __coprthr_do_discover_device_i386(void);

void __do_discover_devices(
	unsigned int* p_ndevices, 
	struct _cl_device_id** p_dtab, 
	int flag
)
{
	int i;

	printcl( CL_DEBUG "__do_discover_devices %p",*p_dtab);

	if (*p_dtab) return;

	struct coprthr_device** devtab = 0;

	__do_discover_devices_1(p_ndevices,&devtab,flag);

/* XXX HACK */
//*p_ndevices = 1;

	struct _cl_device_id* dtab = *p_dtab = (struct _cl_device_id*)
      malloc(*p_ndevices*sizeof(struct _cl_device_id));

	for(i = 0; i<(*p_ndevices); i++) {
		__init_device_id(dtab+i);
		dtab[i].codev = devtab[i];
	}

	for(i = 0; i<(*p_ndevices); i++) {

		dtab[i].ocldevinfo = (struct opencl_device_info*)
			malloc(sizeof(struct opencl_device_info));

		switch(dtab[i].codev->devinfo->arch_id) {

			case COPRTHR_ARCH_ID_X86_64:
				printcl( CL_DEBUG "DEVICE %d is arch x86_64",i);
				__do_discover_opencl_device_info_x86_64(dtab[i].codev,
					dtab[i].ocldevinfo);
				break;

			case COPRTHR_ARCH_ID_E32:
				printcl( CL_DEBUG "DEVICE %d is arch e32",i);
				__do_discover_opencl_device_info_e32(dtab[i].codev,
					dtab[i].ocldevinfo);
				break;

			default:
				break;
		}
	}

	printcl( CL_DEBUG "__do_discover_devices ndevices %d",*p_ndevices);

//	int i;
//
//	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);
//
//	if (getenv("COPRTHR_MAX_NUM_ENGINES"))
//		ncore = min(ncore,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));
//
//#ifdef ENABLE_NCPU
//
//	printcl( CL_DEBUG "checking for force_ncpu");
//
//	if (!getenv_token("COPRTHR_OCL","force_ncpu",buf,256)) ncpu = atoi(buf);
//
//	if (ncore%ncpu) {
//		printcl( CL_WARNING "force_ncpu ignored, must be multiple of ncore");
//		ncpu = 1;
//	}
//
//	if (ncpu > 1) {
//
//		printcl( CL_WARNING "force_ncpu %d",ncpu);
//
//		printcl( CL_DEBUG "force_ncpu = %d",ncpu);
//
//		*p_dtab = (struct _cl_device_id*)
//			realloc(*p_dtab,(ncpu-1)*sizeof(struct _cl_device_id));
//
//		for(devnum=1;devnum<ncpu;devnum++) 
//			memcpy((*p_dtab)+devnum,(*p_dtab),sizeof(struct _cl_device_id));
//
//		int cpd = ncore/ncpu;
//		for(devnum=0;devnum<ncpu;devnum++) {
//			CPU_ZERO(&dtab[devnum].imp->cpumask);
//			for(i=devnum*cpd;i<(devnum+1)*cpd;i++) 
//				CPU_SET(i,&dtab[devnum].imp->cpumask);
//			dtab[devnum].imp->cpu.veid_base = devnum*cpd;
//			dtab[devnum].imp->cpu.nve = cpd;
//			
//			printcl( CL_DEBUG "devnum base nve %d %d %d",
//				devnum,dtab[devnum].imp->cpu.veid_base,dtab[devnum].imp->cpu.nve);
//		}
//
//		*p_ndevices = ncpu;
//
//	} else {
//		CPU_ZERO(&dtab[0].imp->cpumask);
//		for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp->cpumask);
//		dtab[0].imp->cpu.veid_base = 0;
//		dtab[0].imp->cpu.nve = ncore;
//	}
//#else
//	CPU_ZERO(&dtab[0].imp->cpumask);
//	for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp->cpumask);
//	dtab[0].imp->cpu.veid_base = 0;
//	dtab[0].imp->cpu.nve = ncore;
//#endif
//

//	dtab[0].codev = __coprthr_do_discover_device_x86_64();

}


void __do_get_ndevices(
	cl_platform_id platformid, cl_device_type devtype, cl_uint* ndev 
)
{
	unsigned int ndevices = __resolve_platformid(platformid,ndevices);
	struct _cl_device_id* dtab = __resolve_platformid(platformid,dtab);

	printcl( CL_DEBUG "ndevices = %d",ndevices);

	int devnum;
	unsigned int n = 0;

	for(devnum=0;devnum<ndevices;devnum++) {
		printcl( CL_DEBUG "match devtype %d %d",
			dtab[devnum].ocldevinfo->devtype,devtype);
		if (dtab[devnum].ocldevinfo->devtype & devtype) n++;
	}

	printcl( CL_DEBUG "n = %d",n);

	*ndev = n;
}



void __do_get_devices(
	cl_platform_id platformid, cl_device_type devtype, 
	cl_uint ndev, cl_device_id* devices)
{
	unsigned int ndevices = __resolve_platformid(platformid,ndevices);
	struct _cl_device_id* dtab = __resolve_platformid(platformid,dtab);

	int devnum;
	int n = 0;

	for(devnum=0;devnum<ndevices;devnum++) 
		if (n<ndev && dtab[devnum].ocldevinfo->devtype & devtype) 
			devices[n++] = &__resolve_platformid(platformid,dtab[devnum]);

}


void __do_discover_opencl_device_info_x86_64(
	struct coprthr_device* dev, struct opencl_device_info* ocldevinfo )
{
	*ocldevinfo = (struct opencl_device_info){
		.devtype = CL_DEVICE_TYPE_CPU,
		.vendorid = 0,
      .max_compute_units = 1,          /* max_compute_units */
      .max_wi_dim = 3,
      .max_wi_sz = {1,1,1,0}, /* max_wi_dim,max_wi_sz[] */
      .max_wg_sz = 64,           /* max_wg_sz */
      .pref_charn = 8,
      .pref_shortn = 4,
      .pref_intn = 2,
      .pref_longn = 1,
      .pref_floatn = 2,
      .pref_doublen = 1,   /* pref_char/short/int/long/float/double/n */
      .max_freq = 0,          /* max_freq */
      .addr_bits = 64,        /* bits */
      .max_mem_alloc_sz = 1024*1024*1024,    /* max_mem_alloc_sz */
      .supp_img = CL_FALSE,   /* supp_img */
      .img_max_narg_r = 0,
      .img_max_narg_w = 0,          /* img_max_narg_r, img_max_narg_w */
		.img2d_max_width = 0,
      .img2d_max_height = 0,        /* img2d_max_width, img2d_max_height */
      .img3d_max_width = 0,
      .img3d_max_height = 0,
      .img3d_max_depth = 0,/* img3d_max_width,img3d_max_height,img3d_max_depth*/
      .max_samplers = 0,         /* max_samplers */
      .max_param_sz = 256,          /* max_param_sz */
      .mem_align_bits = 64,         /* mem_align_bits */
      .datatype_align_sz = 8,          /* datatype_align_sz */
      .single_fp_config = CL_FP_ROUND_TO_NEAREST|CL_FP_INF_NAN, /* single_fp_config */
      .global_mem_cache_type = CL_NONE,   /* global_mem_cache_type */
      .global_mem_cacheline_sz = 0,          /* global_mem_cacheline_sz */
      .global_mem_cache_sz = 0,        /* global_mem_cache_sz */
      .global_mem_sz = dev->devinfo->global_mem_sz,        /* global_mem_sz */
      .max_const_buffer_sz = 65536,       /* cl_ulong max_const_buffer_sz */
      .max_const_narg = 8,          /* max_const_narg */
      .local_mem_type = CL_GLOBAL,  /* local_mem_type */
      .local_mem_sz = 0,         /* local_mem_sz */
      .supp_ec = CL_FALSE, /* supp_ec */
      .prof_timer_res = 0,          /* prof_timer_res */
      .endian_little = CL_TRUE,     /* endian_little */
      .supp_exec_cap = CL_EXEC_KERNEL, /* supp_exec_cap */
      .cmdq_prop = CL_QUEUE_PROFILING_ENABLE, /* cmdq_prop */
      .platformid = (cl_platform_id)(-1), /* platformid */
//      .extensions = "cl_khr_icd"      /* extensions */
	};

	ocldevinfo->extensions = strdup("cl_khr_icd");

	ocldevinfo->avail = (dev->devstate->avail==0)? CL_FALSE : CL_TRUE;

	ocldevinfo->compiler_avail 
		= (dev->devstate->compiler_avail==0)? CL_FALSE : CL_TRUE;
}


void __do_discover_opencl_device_info_e32(
	struct coprthr_device* dev, struct opencl_device_info* ocldevinfo )
{
	*ocldevinfo = (struct opencl_device_info){
		.devtype = CL_DEVICE_TYPE_ACCELERATOR,
		.vendorid = 0,
      .max_compute_units = 1,          /* max_compute_units */
      .max_wi_dim = 3,
      .max_wi_sz = {1,1,1,0}, /* max_wi_dim,max_wi_sz[] */
      .max_wg_sz = 16,           /* max_wg_sz */
      .pref_charn = 4,
      .pref_shortn = 2,
      .pref_intn = 1,
      .pref_longn = 1,
      .pref_floatn = 1,
      .pref_doublen = 1,   /* pref_char/short/int/long/float/double/n */
      .max_freq = 0,          /* max_freq */
      .addr_bits = 32,        /* bits */
      .max_mem_alloc_sz = 1024*1024*1024,    /* max_mem_alloc_sz */
      .supp_img = CL_FALSE,   /* supp_img */
      .img_max_narg_r = 0,
      .img_max_narg_w = 0,          /* img_max_narg_r, img_max_narg_w */
		.img2d_max_width = 0,
      .img2d_max_height = 0,        /* img2d_max_width, img2d_max_height */
      .img3d_max_width = 0,
      .img3d_max_height = 0,
      .img3d_max_depth = 0,/* img3d_max_width,img3d_max_height,img3d_max_depth*/
      .max_samplers = 0,         /* max_samplers */
      .max_param_sz = 256,          /* max_param_sz */
      .mem_align_bits = 32,         /* mem_align_bits */
      .datatype_align_sz = 8,          /* datatype_align_sz */
      .single_fp_config = CL_FP_ROUND_TO_NEAREST|CL_FP_INF_NAN, /* single_fp_config */
      .global_mem_cache_type = CL_NONE,   /* global_mem_cache_type */
      .global_mem_cacheline_sz = 0,          /* global_mem_cacheline_sz */
      .global_mem_cache_sz = 0,        /* global_mem_cache_sz */
      .global_mem_sz = dev->devinfo->global_mem_sz,  /* global_mem_sz */
      .max_const_buffer_sz = 0,       /* cl_ulong max_const_buffer_sz */
      .max_const_narg = 0,          /* max_const_narg */
      .local_mem_type = CL_GLOBAL,  /* local_mem_type */
      .local_mem_sz = dev->devinfo->local_mem_sz,     /* local_mem_sz */
      .supp_ec = CL_FALSE, /* supp_ec */
      .prof_timer_res = 0,          /* prof_timer_res */
      .endian_little = CL_TRUE,     /* endian_little */
      .supp_exec_cap = CL_EXEC_KERNEL, /* supp_exec_cap */
      .cmdq_prop = CL_QUEUE_PROFILING_ENABLE, /* cmdq_prop */
      .platformid = (cl_platform_id)(-1), /* platformid */
//      .extensions = "cl_khr_icd"      /* extensions */
	};

	ocldevinfo->extensions = strdup("cl_khr_icd");

	ocldevinfo->avail = (dev->devstate->avail==0)? CL_FALSE : CL_TRUE;

	ocldevinfo->compiler_avail 
		= (dev->devstate->compiler_avail==0)? CL_FALSE : CL_TRUE;

	ocldevinfo->max_compute_units = dev->devinfo->max_compute_units;

}

