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

void __do_create_kernel(cl_kernel krn, cl_uint k) 
{
	int i;

	cl_program prg = krn->prg;

	printcl( CL_DEBUG "__do_create_kernel: knum=%d",k);

	krn->name = prg->prg1[0]->kname[k];

	printcl( CL_DEBUG "__do_create_kernel: kname=%s",krn->name);

	krn->krn1 = (struct coprthr1_kernel**)
		malloc(prg->ndev * sizeof(struct coprthr1_kernel*));

	for(i=0; i<prg->ndev; i++) {
		krn->krn1[i] = (struct coprthr1_kernel*)
			malloc(sizeof(struct coprthr1_kernel));
		krn->krn1[i]->prg1 = prg->prg1[i];
		krn->krn1[i]->knum = k;
	}	

	cl_uint narg = krn->narg = prg->prg1[0]->knarg[k];

	printcl( CL_DEBUG "__do_create_kernel: narg=%d",narg);

	if (narg == 0) return;

	krn->krn1[0]->arg_off = malloc(narg*sizeof(uint32_t));
		
	size_t arg_buf_sz = krn->krn1[0]->arg_buf_sz = prg->prg1[0]->karg_buf_sz[k];

	if (arg_buf_sz > 0) krn->krn1[0]->arg_buf = malloc(arg_buf_sz);

	size_t sz = 0;

	for(i=0;i<narg;i++) {
		krn->krn1[0]->arg_off[i] = sz;
		sz += krn->krn1[0]->prg1->karg_sz[k][i];
		printcl( CL_DEBUG "CHECKING arg_sz[%d] %d",
			i,krn->krn1[0]->prg1->karg_sz[k][i]);
	}

}

void __do_release_kernel(cl_kernel krn) {}


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
				return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			memcpy(p,arg_val,arg_sz);

			break; 

		case CLARG_KIND_GLOBAL:

			if (arg_sz != krn1->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

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

			if (arg_sz != krn1->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

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


int __do_set_kernel_arg( 
	cl_kernel krn, cl_uint argn, size_t arg_sz, const void* arg_val 
)
{
	__do_set_kernel_arg_1( krn->krn1[0], argn, arg_sz, arg_val );

	struct coprthr1_mem* mem1;

	/* resolve arg_val for all devices to compensate for opencl design mistake */

	unsigned int ndev = krn->ctx->ndev;

	int idev;

	for(idev=0;idev<ndev;idev++) {

		int knum = krn->krn1[idev]->knum;

		cl_uint arg_kind = krn->krn1[idev]->prg1->karg_kind[knum][argn];

		/* XXX hack to allow user to strongly imply local address space -DAR */
		if (arg_sz > 0 && arg_val == 0) arg_kind = CLARG_KIND_LOCAL;

		int err;
		struct coprthr1_mem* mem1;
		switch (arg_kind) {

			case CLARG_KIND_GLOBAL:
			case CLARG_KIND_CONSTANT:

				mem1 = (*(cl_mem*)arg_val)->mem1[idev];
				err = __do_set_kernel_arg_1(krn->krn1[idev], argn, arg_sz, &mem1);
				break;

			default:

				err = __do_set_kernel_arg_1(krn->krn1[idev], argn, arg_sz, arg_val);
				break;

		}

		if (err) return err;

	}

#if(0)
	void* p = krn->krn1[0]->arg_buf + krn->krn1[0]->arg_off[argn];

	int knum = krn->krn1[0]->knum;

	cl_uint arg_kind = krn->krn1[0]->prg1->karg_kind[knum][argn];

	/* XXX hack to allow user to strongly imply local address space -DAR */
	if (arg_sz > 0 && arg_val == 0) arg_kind = CLARG_KIND_LOCAL;

	switch (arg_kind) {

		case CLARG_KIND_VOID:

			printcl( CL_WARNING "setting void arg has no effect");

			break;

		case CLARG_KIND_DATA:

			printcl( CL_DEBUG "CLARG_KIND_DATA compare sz %d %d",
				arg_sz,krn->krn1[0]->prg1->karg_sz[knum][argn]);

			if (arg_sz != krn->krn1[0]->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			memcpy(p,arg_val,arg_sz);

			break; 

		case CLARG_KIND_GLOBAL:

			if (arg_sz != krn->krn1[0]->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

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

			if (arg_sz != krn->krn1[0]->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

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
#endif

	return(CL_SUCCESS);

}

