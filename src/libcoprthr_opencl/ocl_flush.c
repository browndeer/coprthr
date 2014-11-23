/* ocl_flush.c 
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

// Flush and Finish API calls


cl_int 
_clFlush(cl_command_queue cmdq)
{
	printcl( CL_WARNING "clFlush: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

//	__do_flush(cmdq);
	/* nothing to do, coprthr uses immediate command queues */

	return(CL_SUCCESS);
}


cl_int 
_clFinish(cl_command_queue cmdq )
{
	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	__do_finish(cmdq);

	return(CL_SUCCESS);
}


// Aliased Flush and Finish API Calls

cl_int
clFlush(cl_command_queue cmdq)
	__attribute__((alias("_clFlush")));

cl_int
clFinish(cl_command_queue cmdq )
	__attribute__((alias("_clFinish")));


