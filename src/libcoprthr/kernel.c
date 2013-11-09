/* kernel.c
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

#include <string.h>
#include <errno.h>

#include "printcl.h"
#include "kernel.h"

#include "coprthr_mem.h"
#include "coprthr_program.h"

#define __invalid_memobj(memobj) (!memobj)

void __do_create_kernel_1(struct coprthr1_kernel* krn1 )
{
	int i;

	struct coprthr1_program* prg1 = krn1->prg1;
	unsigned int knum = krn1->knum;

	printcl( CL_DEBUG "__do_create_kernel: prg1=%p knum=%d",prg1,knum);

//	krn1 = (struct coprthr1_kernel*)malloc(sizeof(struct coprthr1_kernel));

	unsigned int narg = prg1->knarg[knum];

	printcl( CL_DEBUG "__do_create_kernel: narg=%d",narg);

	if (narg == 0) return;

	krn1->arg_off = malloc(narg*sizeof(uint32_t));
		
	size_t arg_buf_sz = krn1->arg_buf_sz = prg1->karg_buf_sz[knum];

	if (arg_buf_sz > 0) krn1->arg_buf = malloc(arg_buf_sz);

	size_t sz = 0;

	for(i=0;i<narg;i++) {
		krn1->arg_off[i] = sz;
		sz += krn1->prg1->karg_sz[knum][i];
		printcl( CL_DEBUG "CHECKING arg_sz[%d] %d",
			i,krn1->prg1->karg_sz[knum][i]);
	}

}


int __do_set_kernel_arg_1( 
	struct coprthr1_kernel* krn1, 
	unsigned int argn, size_t arg_sz, const void* arg_val 
)
{

	printcl( CL_DEBUG "krn1=%p argn=%d",krn1,argn);
	printcl( CL_DEBUG "arg_buf=%p",krn1->arg_buf);
	printcl( CL_DEBUG "arg_off=%d",krn1->arg_off[argn]);

	void* p = krn1->arg_buf + krn1->arg_off[argn];

	int knum = krn1->knum;

	unsigned int arg_kind = krn1->prg1->karg_kind[knum][argn];

	if (arg_sz==0) 
		arg_sz = krn1->prg1->karg_sz[knum][argn];

	/* XXX hack to allow user to strongly imply local address space -DAR */
	if (arg_sz > 0 && arg_val == 0) arg_kind = CLARG_KIND_LOCAL;

	switch (arg_kind) {

		case CLARG_KIND_VOID:

			printcl( CL_WARNING "setting void arg has no effect");

			break;

		case CLARG_KIND_DATA:

			printcl( CL_DEBUG "CLARG_KIND_DATA compare sz %d %d",
				arg_sz,krn1->prg1->karg_sz[knum][argn]);

			if (arg_sz != krn1->prg1->karg_sz[knum][argn]) 
				return(__CL_INVALID_ARG_SIZE);

			if (!arg_val) return (__CL_INVALID_ARG_VALUE);

			memcpy(p,arg_val,arg_sz);

			break; 

		case CLARG_KIND_GLOBAL:

			if (arg_sz != krn1->prg1->karg_sz[knum][argn]) 
				return(__CL_INVALID_ARG_SIZE);

			if (!arg_val) return (__CL_INVALID_ARG_VALUE);

			if (__invalid_memobj(arg_val)) return(__CL_INVALID_MEM_OBJECT);

//			if (arg_sz != sizeof(cl_mem)) return(__CL_INVALID_ARG_SIZE);
			if (arg_sz != sizeof(struct coprthr1_mem*)) return(__CL_INVALID_ARG_SIZE);

			printcl( CL_DEBUG "from set arg %p %p (res=%p)",
//				arg_val,*(cl_mem*)arg_val);
				arg_val,*(struct coprthr1_mem**)arg_val,(*(struct coprthr1_mem**)arg_val)->res);

			memcpy(p,arg_val,arg_sz);

			break;

		case CLARG_KIND_LOCAL:

			if (arg_val) return (__CL_INVALID_ARG_VALUE);

			if (arg_sz == 0) return(__CL_INVALID_ARG_SIZE);

			*(size_t*)p = arg_sz;

			break;

		case CLARG_KIND_CONSTANT:

			if (arg_sz != krn1->prg1->karg_sz[knum][argn]) 
				return(__CL_INVALID_ARG_SIZE);

			if (!arg_val) return (__CL_INVALID_ARG_VALUE);

			if (__invalid_memobj(arg_val)) return(__CL_INVALID_MEM_OBJECT);

//			if (arg_sz != sizeof(cl_mem)) return(__CL_INVALID_ARG_SIZE);
			if (arg_sz != sizeof(struct coprthr1_mem*)) return(__CL_INVALID_ARG_SIZE);

			printcl( CL_DEBUG "from set arg %p %p",
//				arg_val,*(cl_mem*)arg_val);
				arg_val,*(struct coprthr1_mem**)arg_val);

			memcpy(p,arg_val,arg_sz);

			break;

		case CLARG_KIND_SAMPLER:

//			if (arg_sz != sizeof(cl_sampler)) return(__CL_INVALID_ARG_SIZE);
			if (arg_sz != sizeof(void*)) return(__CL_INVALID_ARG_SIZE);

			printcl( CL_ERR "sampler arg not supported");
			return(ENOTSUP);

			break;

		case CLARG_KIND_IMAGE2D:

			printcl( CL_ERR "image2d arg not supported");
			return(ENOTSUP);

			break;

		case CLARG_KIND_IMAGE3D:

			printcl( CL_ERR "image3d arg not supported");
			return(ENOTSUP);

			break;

		default:
			break;
	}

	return(0);

}

