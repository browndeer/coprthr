/* ocl_event.c
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
#include "printcl.h"
#include "event.h"


// Event Object API Calls


cl_int 
_clWaitForEvents(
	 cl_uint nev,
	 const cl_event* evlist
)
{
	printcl( CL_DEBUG "clWaitForEvents");

	if (nev == 0) return(CL_INVALID_VALUE);

	int i;

	for(i=1;i<nev;i++) 
		if (evlist[0]->ctx != evlist[i]->ctx) return(CL_INVALID_CONTEXT);

	for(i=0;i<nev;i++) if(__invalid_event(evlist[i])) return(CL_INVALID_EVENT);

	__do_wait_for_events(nev,evlist);

	for(i=0;i<nev;i++)
		if (evlist[i]->ev1->cmd_stat < 0) {
			printcl( CL_ERR "event execution status error");
			return(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
		}

	return(CL_SUCCESS);
}


cl_int 
_clGetEventInfo(
	 cl_event ev,
	 cl_event_info param_name,
	 size_t param_sz,
	 void* param_val,
	 size_t* param_sz_ret
)
{
	printcl( CL_WARNING "clGetEventInfo: warning: unsupported");

	if (__invalid_event(ev)) return(CL_INVALID_EVENT);

	size_t sz;

	switch (param_name) {

		case CL_EVENT_COMMAND_QUEUE:

			__case_get_param(sizeof(cl_command_queue),&ev->cmdq);

			break;

		case CL_EVENT_COMMAND_TYPE:

			__case_get_param(sizeof(cl_command_type),&ev->cmd);

			break;

		case CL_EVENT_COMMAND_EXECUTION_STATUS:

			__case_get_param(sizeof(cl_int),&ev->ev1->cmd_stat);

			break;

		case CL_EVENT_REFERENCE_COUNT:

			__case_get_param(sizeof(cl_uint),&ev->refc);

			break;

		default:

			return(CL_INVALID_VALUE);
	}

	return(CL_SUCCESS);
}


cl_int
_clRetainEvent( cl_event ev )
{
	printcl( CL_DEBUG "clRetainEvent");

	if (__invalid_event(ev)) return(CL_INVALID_EVENT);

	__lock_event(ev);
	__retain_event(ev);
	__unlock_event(ev);

	return(CL_SUCCESS);
}


cl_int
_clReleaseEvent( cl_event ev )
{
	printcl( CL_DEBUG "clReleaseEvent");

	if (__invalid_event(ev)) return(CL_INVALID_EVENT);

	/* added to prevent release before completion, that is programmer error */
	if (ev->ev1->cmd_stat != CL_COMPLETE) return -1;

	__lock_event(ev);
	__release_event(ev);
	/* unlock implicit in __release_event() -DAR */

	return(CL_SUCCESS);
}


// Aliased Event object API Calls

cl_int
clWaitForEvents( cl_uint nev, const cl_event* evlist)
	__attribute__((alias("_clWaitForEvents")));

cl_int
clGetEventInfo( cl_event ev, cl_event_info param_name, size_t param_sz,
    void* param_val, size_t* param_sz_ret)
	__attribute__((alias("_clGetEventInfo")));

cl_int
clRetainEvent( cl_event ev )
	__attribute__((alias("_clRetainEvent")));

cl_int
clReleaseEvent( cl_event ev )
	__attribute__((alias("_clReleaseEvent")));


