/* ocl_command_queue.c 
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

/*  
 * XXX the error checking is weak.  many cases have merely a not-null test
 * XXX for objects as a placeholder for a true test of validity. -DAR
 */


#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"


void __do_create_command_queue( cl_command_queue cmdq );
void __do_release_command_queue( cl_command_queue cmdq );
void __do_enqueue_cmd( cl_command_queue cmdq, cl_event ev );
void __do_finish( cl_command_queue cmdq );


// Command Queue API Calls
	 
cl_command_queue 
_clCreateCommandQueue(
	cl_context ctx, cl_device_id devid,	
	cl_command_queue_properties prop, 
	cl_int* err_ret
)
{
	printcl( CL_DEBUG "clCreateCommandQueue");

	if (__invalid_context(ctx)) 
		__error_return(CL_INVALID_CONTEXT,cl_command_queue);

	printcl( CL_DEBUG "clCreateCommandQueue %p",devid);

	if (__invalid_device_id(devid)) 
		__error_return(CL_INVALID_DEVICE,cl_command_queue);

	printcl( CL_DEBUG "clCreateCommandQueue");

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
_clRetainCommandQueue( cl_command_queue cmdq )
{
	printcl( CL_DEBUG "clRetainCommandQueue");

	if (!cmdq) return(CL_INVALID_COMMAND_QUEUE);

	++cmdq->refc;

	return(CL_SUCCESS);
}


	 
cl_int 
_clReleaseCommandQueue( cl_command_queue cmdq )
{
	printcl( CL_DEBUG "clReleaseCommandQueue");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (--cmdq->refc == 0) {
	
		__do_release_command_queue(cmdq);	

//printcl( CL_DEBUG "before __free_command_queue");
		__free_command_queue(cmdq);
//printcl( CL_DEBUG "after __free_command_queue");

	}

	return(CL_SUCCESS);
}


	 
cl_int 
_clGetCommandQueueInfo(
	cl_command_queue cmdq,
	cl_command_queue_info param_name,
	size_t param_sz, 
	void* param_val,	 	
	size_t* param_sz_ret
)
{
	printcl( CL_DEBUG "clGetCommandQueueInfo");

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
_clSetCommandQueueProperty(
	cl_command_queue cmdq, 
	cl_command_queue_properties prop, 		
	cl_bool enable, 
	cl_command_queue_properties* prop_old
)
{
	printcl( CL_WARNING "clSetCommandQueueProperty: warning: unsupported");

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



// Aliased Command Queue API Calls

cl_command_queue
clCreateCommandQueue( cl_context, cl_device_id, cl_command_queue_properties,
   cl_int* )
   __attribute__((alias("_clCreateCommandQueue")));

cl_int
clRetainCommandQueue( cl_command_queue )
   __attribute__((alias("_clRetainCommandQueue")));

cl_int
clReleaseCommandQueue( cl_command_queue )
   __attribute__((alias("_clReleaseCommandQueue")));

cl_int
clGetCommandQueueInfo( cl_command_queue, cl_command_queue_info, size_t, void*,
   size_t*)
   __attribute__((alias("_clGetCommandQueueInfo")));

cl_int
clSetCommandQueueProperty( cl_command_queue, cl_command_queue_properties,
   cl_bool, cl_command_queue_properties*)
   __attribute__((alias("_clSetCommandQueueProperty")));


/*
 * Internal command queueu implementation calls
 */


void __do_create_command_queue( cl_command_queue cmdq ) 
{
	printcl( CL_DEBUG "__do_create_command_queue: dev=%p",cmdq->devid->codev);

	__do_create_command_queue_1( cmdq->devid->codev );
	cmdq->ptr_imp = cmdq->devid->codev->devstate->cmdq;
	
   cl_context ctx = cmdq->ctx;
   unsigned int ndev = ctx->ndev;
   cl_device_id* devices = ctx->devices;
   unsigned int n = 0;
   while (n < ndev && devices[n]->codev != cmdq->devid->codev) ++n;
	cmdq->devnum = n;

	printcl( CL_DEBUG "__do_create_command_queue: devnum=%d",n);
}



void __do_release_command_queue( cl_command_queue cmdq ) 
{
	__do_release_command_queue_1(cmdq->devid->codev);
}



void __do_enqueue_cmd( cl_command_queue cmdq, cl_event ev ) 
{
	ev->ev1->cmd = ev->cmd;
	__do_enqueue_cmd_1( cmdq->devid->codev, ev->ev1);
	ev->cmdq = cmdq;
	ev->dev = cmdq->devid->codev;
	__retain_event(ev);
}


void __do_finish( cl_command_queue cmdq )
{
	__do_finish_1( cmdq->devid->codev );
}


