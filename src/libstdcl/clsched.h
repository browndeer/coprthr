/* clsched.h
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

#ifndef _CLSCHED_H
#define _CLSCHED_H

#if defined(__APPLE__)
#include "OpenCL/opencl.h"
#else
#include "CL/cl.h"
#endif

#include "clcontext.h"

#define CL_FAST      0x08

#define CL_EVENT_WAIT      0x01
#define CL_EVENT_NOWAIT    0x02
#define CL_EVENT_RELEASE   0x04

#define CL_KERNEL_EVENT		0x10
#define CL_MEM_EVENT			0x20
#define CL_ALL_EVENT			(CL_KERNEL_EVENT|CL_MEM_EVENT)

struct clndrange_struct {
   size_t dim;
   size_t gtid_offset[4];
   size_t gtid[4];
   size_t ltid[4];
};


#ifdef __cplusplus
extern "C" {
#endif

cl_event clfork(CONTEXT* cp, cl_uint devnum, cl_kernel krn, struct clndrange_struct* ndr, int flags);

cl_event clwait(CONTEXT* cp, unsigned int devnum, int flags);
cl_event clwaitev(
   CONTEXT* cp, unsigned int devnum, const cl_event ev, int flags
);

int clflush(CONTEXT* cp, unsigned int devnum, int flags);

#ifdef __cplusplus
}
#endif

#endif

