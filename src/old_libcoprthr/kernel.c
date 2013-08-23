/* kernel.c
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

#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "kernel.h"


void __do_create_kernel(cl_kernel krn, cl_uint k) 
{
	int i;
//	void* p;

	cl_program prg = krn->prg;

	printcl( CL_DEBUG "__do_create_kernel: knum=%d",k);

	krn->name = prg->imp.kname[k];

	printcl( CL_DEBUG "__do_create_kernel: kname=%s",krn->name);

/* XXX this is odd way of assigning kernel, fix it by contracting v_* -DAR */

	krn->imp.v_kbin = prg->imp.v_kbin;
//	krn->imp.v_ksym = prg->imp.v_ksym;
//	krn->imp.v_kcall = prg->imp.v_kcall;
	krn->imp.v_ksyms = prg->imp.v_ksyms;
	krn->imp.knum = k;

	cl_uint narg = krn->narg = prg->imp.knarg[k];

	printcl( CL_DEBUG "__do_create_kernel: narg=%d",narg);

	if (narg == 0) return;

	if (prg->imp.karg_kind[k]) 
		__clone(krn->imp.arg_kind,prg->imp.karg_kind[k],narg,cl_uint);
	else krn->imp.arg_kind = (cl_uint*)malloc(narg*sizeof(cl_uint));

	if (prg->imp.karg_sz[k]) 
		__clone(krn->imp.arg_sz,prg->imp.karg_sz[k],narg,size_t);
	else krn->imp.arg_sz = (size_t*)malloc(narg*sizeof(size_t));

	krn->imp.arg_off = malloc(narg*sizeof(uint32_t));
		
	size_t arg_buf_sz = krn->imp.arg_buf_sz = prg->imp.karg_buf_sz[k];

	if (arg_buf_sz > 0) krn->imp.arg_buf = malloc(arg_buf_sz);

	size_t sz = 0;

	for(i=0;i<narg;i++) {
		krn->imp.arg_off[i] = sz;
		sz += krn->imp.arg_sz[i];
		printcl( CL_DEBUG "CHECKING arg_sz[%d] %d",i,krn->imp.arg_sz[i]);
	}

}

void __do_release_kernel(cl_kernel krn) {}


int __do_set_kernel_arg( 
	cl_kernel krn, cl_uint argn, size_t arg_sz, const void* arg_val 
)
{

//	if (arg_sz != krn->imp.arg_sz[argn]) return(CL_INVALID_ARG_SIZE);

	void* p = krn->imp.arg_buf + krn->imp.arg_off[argn];

	cl_uint arg_kind = krn->imp.arg_kind[argn];

	/* XXX hack to allow user to strongly imply local address space -DAR */
	if (arg_sz > 0 && arg_val == 0) arg_kind = CLARG_KIND_LOCAL;

	switch (arg_kind) {

		case CLARG_KIND_VOID:

			printcl( CL_WARNING "setting void arg has no effect");

			break;

		case CLARG_KIND_DATA:

			printcl( CL_DEBUG "CLARG_KIND_DATA compare sz %d %d",
				arg_sz,krn->imp.arg_sz[argn]);

			if (arg_sz != krn->imp.arg_sz[argn]) return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			memcpy(p,arg_val,arg_sz);

			break; 

		case CLARG_KIND_GLOBAL:

			if (arg_sz != krn->imp.arg_sz[argn]) return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			if (__invalid_memobj(arg_val)) return(CL_INVALID_MEM_OBJECT);

			if (arg_sz != sizeof(cl_mem)) return(CL_INVALID_ARG_SIZE);

			printcl( CL_DEBUG "from set arg %p %p",
				arg_val,*(cl_mem*)arg_val);

			memcpy(p,arg_val,arg_sz);

			break;

		case CLARG_KIND_LOCAL:

			if (arg_val) return (CL_INVALID_ARG_VALUE);

			if (arg_sz == 0) return(CL_INVALID_ARG_SIZE);

			*(size_t*)p = arg_sz;

			break;

		case CLARG_KIND_CONSTANT:

//			printcl( CL_ERR "constant arg not supported");
//			return(CL_ENOTSUP);
//
//			break;

			if (arg_sz != krn->imp.arg_sz[argn]) return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			if (__invalid_memobj(arg_val)) return(CL_INVALID_MEM_OBJECT);

			if (arg_sz != sizeof(cl_mem)) return(CL_INVALID_ARG_SIZE);

			printcl( CL_DEBUG "from set arg %p %p",
				arg_val,*(cl_mem*)arg_val);

			memcpy(p,arg_val,arg_sz);

			break;

		case CLARG_KIND_SAMPLER:

			if (arg_sz != sizeof(cl_sampler)) return(CL_INVALID_ARG_SIZE);

			printcl( CL_ERR "sampler arg not supported");
			return(CL_ENOTSUP);

			break;

		case CLARG_KIND_IMAGE2D:

			printcl( CL_ERR "image2d arg not supported");
			return(CL_ENOTSUP);

			break;

		case CLARG_KIND_IMAGE3D:

			printcl( CL_ERR "image3d arg not supported");
			return(CL_ENOTSUP);

			break;

		default:
			break;
	}

	return(CL_SUCCESS);

}

