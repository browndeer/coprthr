/* context.c
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

//#ifdef ENABLE_ATIGPU
//#include "cal.h"
//#endif

#include "xcl_structs.h"
#include "printcl.h"
#include "context.h"

void __do_create_context(cl_context ctx) 
{
//#ifdef ENABLE_ATIGPU
//	int i,err;
//	ctx->imp.calctx = (CALcontext*)malloc(ctx->ndev*sizeof(CALcontext));
//	for(i=0;i<ctx->ndev;i++) {
//		if (__resolve_devid(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU) {
//			err = calCtxCreate(&ctx->imp.calctx[i],
//				__resolve_devid(ctx->devices[i],atigpu.caldev));
//			printcl( CL_DEBUG "calCtxCreate returned %d",err);
//		} else ctx->imp.calctx[i] = 0;
//	}
//#endif
}

void __do_release_context(cl_context ctx) 
{
//#ifdef ENABLE_ATIGPU
//	int i,err;
//	for(i=0;i<ctx->ndev;i++) {
//		if (__resolve_devid(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU) {
//			err = calCtxDestroy(ctx->imp.calctx[i]);
//			printcl( CL_DEBUG "calCtxDestroy returned %d",err);
//		}
//	}
//#endif
}

void __do_get_max_buffer_size_in_context(cl_context ctx, size_t* sz)
{
	int i;
	size_t tmp = 0;
	for(i=0;i<ctx->ndev;i++) 
//		tmp = max(tmp,__resolve_devid(ctx->devices[i],max_mem_alloc_sz));
		tmp = max(tmp,__resolve_devid_devinfo(ctx->devices[i],max_mem_alloc_sz));
	*sz = tmp;
}
