/* icd.c
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

//#include "xcl_structs.h"

#include "oclcall.h"

/*
cl_int
clSetCommandQueueProperty(
   cl_command_queue cmdq,
   cl_command_queue_properties prop,
   cl_bool enable,
   cl_command_queue_properties* prop_old
);

extern cl_int
_clGetPlatformInfo(
   cl_platform_id platformid,
   cl_platform_info param_name,
   size_t param_sz,
   void* param_val,
   size_t* param_sz_ret
);
*/

__DECL_API_CALLS(_,)

void* __icd_call_vector[] = __set_icd_call_vector(_,);

