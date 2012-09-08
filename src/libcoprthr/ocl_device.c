/* ocl_device.c 
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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
#include "device.h"

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

	if (n == 0) return(CL_DEVICE_NOT_FOUND);

	if (ndev_ret) *ndev_ret = n;

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
				&__resolve_devid(devid,devtype));

			break;

		case CL_DEVICE_VENDOR_ID:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,vendorid));

			break;

		case CL_DEVICE_MAX_COMPUTE_UNITS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,max_compute_units));

			break;
		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,max_wi_dim));

			break;

		case CL_DEVICE_MAX_WORK_ITEM_SIZES:

			__case_get_param(__resolve_devid(devid,max_wi_dim)*sizeof(size_t*),
				&__resolve_devid(devid,max_wi_sz));

			break;

		case CL_DEVICE_MAX_WORK_GROUP_SIZE:

			__case_get_param(sizeof(size_t),&__resolve_devid(devid,max_wg_sz));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,pref_charn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,pref_shortn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,pref_intn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,pref_longn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,pref_floatn));

			break;

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,pref_doublen));

			break;

		case CL_DEVICE_MAX_CLOCK_FREQUENCY:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,max_freq));

			break;

		case CL_DEVICE_ADDRESS_BITS:

			__case_get_param(sizeof(cl_uint),&__resolve_devid(devid,addr_bits));

			break;

		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid(devid,max_mem_alloc_sz));

			break;

		case CL_DEVICE_IMAGE_SUPPORT:

			__case_get_param(sizeof(cl_bool),&__resolve_devid(devid,supp_img));

			break;

		case CL_DEVICE_MAX_READ_IMAGE_ARGS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,img_max_narg_r));

			break;

		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,img_max_narg_w));

			break;

		case CL_DEVICE_IMAGE2D_MAX_WIDTH:

			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,img2d_max_width));

			break;

		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:

			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,img2d_max_height));

			break;

		case CL_DEVICE_IMAGE3D_MAX_WIDTH:

			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,img3d_max_width));

			break;

		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:


			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,img3d_max_height));

			break;

		case CL_DEVICE_IMAGE3D_MAX_DEPTH:

			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,img3d_max_depth));

			break;

		case CL_DEVICE_MAX_SAMPLERS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,max_samplers));

			break;

		case CL_DEVICE_MAX_PARAMETER_SIZE:

			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,max_param_sz));

			break;

		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:	

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,mem_align_bits)); 

			break;


		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,datatype_align_sz));

			break;

		case CL_DEVICE_SINGLE_FP_CONFIG:

			__case_get_param(sizeof(cl_device_fp_config),
				&__resolve_devid(devid,single_fp_config));

			break;

		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:

			__case_get_param(sizeof(cl_device_mem_cache_type),
				&__resolve_devid(devid,global_mem_cache_type));

			break;

		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,global_mem_cacheline_sz));

			break;

		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid(devid,global_mem_cache_sz));

			break;

		case CL_DEVICE_GLOBAL_MEM_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid(devid,global_mem_sz));

			break;

		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid(devid,max_const_buffer_sz));

			break;

		case CL_DEVICE_MAX_CONSTANT_ARGS:

			__case_get_param(sizeof(cl_uint),
				&__resolve_devid(devid,max_const_narg));

			break;

		case CL_DEVICE_LOCAL_MEM_TYPE:

			__case_get_param(sizeof(cl_device_local_mem_type),
				&__resolve_devid(devid,local_mem_type));

			break;

		case CL_DEVICE_LOCAL_MEM_SIZE:

			__case_get_param(sizeof(cl_ulong),
				&__resolve_devid(devid,local_mem_sz));

			break;

		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid(devid,supp_ec));

			break;

		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:

			__case_get_param(sizeof(size_t),
				&__resolve_devid(devid,prof_timer_res));

			break;

		case CL_DEVICE_ENDIAN_LITTLE:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid(devid,endian_little));

			break;

		case CL_DEVICE_AVAILABLE:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid(devid,avail));

			break;

		case CL_DEVICE_COMPILER_AVAILABLE:

			__case_get_param(sizeof(cl_bool),
				&__resolve_devid(devid,compiler_avail));

			break;

		case CL_DEVICE_EXECUTION_CAPABILITIES:

			__case_get_param(sizeof(cl_device_exec_capabilities),
				&__resolve_devid(devid,supp_exec_cap));

			break;

		case CL_DEVICE_QUEUE_PROPERTIES:

			__case_get_param(sizeof(cl_command_queue_properties),
				&__resolve_devid(devid,cmdq_prop));

			break;

		case CL_DEVICE_PLATFORM:

			__case_get_param(sizeof(cl_platform_id),
				&__resolve_devid(devid,platformid));

			break;

		case CL_DEVICE_NAME:

			__case_get_param(
				1+strnlen(__resolve_devid(devid,name),__CLMAXSTR_BUFSZ),
				__resolve_devid(devid,name));

			break;

		case CL_DEVICE_VENDOR:

			__case_get_param(
				1+strnlen(__resolve_devid(devid,vendor),__CLMAXSTR_BUFSZ),
				__resolve_devid(devid,vendor));

			break;

		case CL_DRIVER_VERSION:

			__case_get_param(
				1+strnlen(__resolve_devid(devid,drv_version),__CLMAXSTR_BUFSZ),
				__resolve_devid(devid,drv_version));

			break;

		case CL_DEVICE_PROFILE:

			__case_get_param(
				1+strnlen(__resolve_devid(devid,profile),__CLMAXSTR_BUFSZ),
				__resolve_devid(devid,profile));

			break;

		case CL_DEVICE_VERSION:

			__case_get_param(
				1+strnlen(__resolve_devid(devid,version),__CLMAXSTR_BUFSZ),
				__resolve_devid(devid,version));

			break;

		case CL_DEVICE_EXTENSIONS:

			__case_get_param(
				1+strnlen(__resolve_devid(devid,extensions),__CLMAXSTR_BUFSZ),
				__resolve_devid(devid,extensions));

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



