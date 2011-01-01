/* xcl_command_queue.c 
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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

/*  
 * XXX the error checking is weak.  many cases have merely a not-null test
 * XXX for objects as a placeholder for a true test of validity. -DAR
 */


#include <CL/cl.h>

#include "xcl_structs.h"


// Command Queue APIs

	 
cl_command_queue 
clCreateCommandQueue(
	cl_context ctx, cl_device_id devid,	
	cl_command_queue_properties prop, 
	cl_int* err_ret
)
{
	DEBUG(__FILE__,__LINE__,"clCreateCommandQueue");

	if (__invalid_context(ctx)) 
		__error_return(CL_INVALID_CONTEXT,cl_command_queue);

	if (__invalid_device_id(devid)) 
		__error_return(CL_INVALID_DEVICE,cl_command_queue);

//		if () { /* XXX this needs to check prop -DAR */
//			if (err) *err = CL_INVALID_VALUE;
//			return((cl_command_queue)0);
//		}

//		if () { /* XXX this needs to check prop against device -DAR */
//			if (err_ret) *err_ret = CL_INVALID_QUEUE_PROPERTIES;
//			return((cl_command_queue)0);
//		}


	struct _cl_command_queue* cmdq 
		= (struct _cl_command_queue*)malloc(sizeof(struct _cl_command_queue));

	
	if (cmdq) {

		__init_command_queue(cmdq);

		cmdq->refc = 1;
		cmdq->ctx = ctx;
		cmdq->devid = devid;
		cmdq->prop = prop;

		__do_create_command_queue(cmdq);

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_command_queue);

	__success();

	return(cmdq);
}


	 
cl_int 
clRetainCommandQueue( cl_command_queue cmdq )
{
	DEBUG(__FILE__,__LINE__,"clRetainCommandQueue");

	if (!cmdq) return(CL_INVALID_COMMAND_QUEUE);

	++cmdq->refc;

	return(CL_SUCCESS);
}


	 
cl_int 
clReleaseCommandQueue( cl_command_queue cmdq )
{
	DEBUG(__FILE__,__LINE__,"clReleaseCommandQueue");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (--cmdq->refc == 0) {
	
		__do_release_command_queue(cmdq);	

		__free_command_queue(cmdq);

	}

	return(CL_SUCCESS);
}


	 
cl_int 
clGetCommandQueueInfo(
	cl_command_queue cmdq,
	cl_command_queue_info param_name,
	size_t param_sz, 
	void* param_val,	 	
	size_t* param_sz_ret
)
{
	DEBUG(__FILE__,__LINE__,"clGetCommandQueueInfo");

	if (!cmdq) return(CL_INVALID_COMMAND_QUEUE);

	size_t sz;

	switch (param_name) {

		case CL_QUEUE_CONTEXT:

			__case_get_param(sizeof(cl_context),&cmdq->ctx);

			break;

		case CL_QUEUE_DEVICE:

			__case_get_param(sizeof(cl_device_id),&cmdq->devid);

			break;

		case CL_QUEUE_REFERENCE_COUNT:

			__case_get_param(sizeof(cl_uint),&cmdq->refc);

			break;

		case CL_QUEUE_PROPERTIES:

			__case_get_param(sizeof(cl_command_queue_properties),&cmdq->prop);

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


	 
cl_int 
clSetCommandQueueProperty(
	cl_command_queue cmdq, 
	cl_command_queue_properties prop, 		
	cl_bool enable, 
	cl_command_queue_properties* prop_old
)
{
	WARN(__FILE__,__LINE__,"clSetCommandQueueProperty: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

//		/* XXX this needs to check prop -DAR */
//		if () return(CL_INVALID_VALUE);

//		/* XXX this needs to check prop against device -DAR */
//		if () return(CL_INVALID_QUEUE_PROPERTIES);

	if (prop_old) *prop_old = cmdq->prop;

	if (enable)	cmdq->prop |= prop;
	else cmdq->prop &= ~prop;

	return(CL_SUCCESS);
}


