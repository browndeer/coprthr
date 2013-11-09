/* thread.c
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
#include "thread.h"
#include "device.h"

#include "coprthr_mem.h"
#include "coprthr_program.h"

#define __malloc_struct(t) (struct t*)malloc(sizeof(struct t))

int coprthr_attr_init( coprthr_td_attr_t* attr ) 
{
	if (!attr)
		return EFAULT;

	if ( (*attr = __malloc_struct(coprthr_td_attr)) == 0) 
		return ENOMEM;
		
	(*attr)->detachstate = COPRTHR_A_CREATE_JOINABLE;
	(*attr)->initstate = COPRTHR_A_CREATE_EXECUTE;
	(*attr)->dd = -1;

	return 0;
}


int coprthr_attr_destroy( coprthr_td_attr_t* attr ) 
{
	if (attr==0 || *attr==0)
		return EINVAL;

	free(*attr);

	return 0;
}


int coprthr_attr_setdetachstate( coprthr_td_attr_t* attr, int state )
{
	if (attr==0 || *attr==0)
		return EINVAL;

	else if (state==COPRTHR_A_CREATE_DETACHED 
		|| state==COPRTHR_A_CREATE_JOINABLE)
			(*attr)->detachstate = state;
	else
		return EINVAL;

	return 0;
}


int coprthr_attr_setdevice( coprthr_td_attr_t* attr, int dd )
{
	if (dd<0 || dd>255)
		return EINVAL;

	(*attr)->dd = dd;

	return 0;
}


int coprthr_attr_setinitstate( coprthr_td_attr_t* attr, int state  )
{
	if (attr==0 || *attr==0) 
		return EINVAL;

	if (state==COPRTHR_A_CREATE_EXECUTE || state==COPRTHR_A_CREATE_SUSPEND)
		(*attr)->initstate = state;
	else
		return EINVAL;

	return 0;
}


int coprthr_create( coprthr_td_t* td, coprthr_td_attr_t* attr, 
	coprthr_sym_t thr, void* pargs )
{
	printcl( CL_DEBUG "coprthr_create");

	if (!td)
		return EFAULT;

	if (attr==0 || *attr==0 || thr==0) 
		return EINVAL;

	if ( (*td = __malloc_struct(coprthr_td)) == 0) 
		return ENOMEM;

	__do_set_kernel_arg_1( thr, 0, 0, pargs );

	__coprthr_init_event( &((*td)->ev) );

	size_t gwo = 0;
	size_t gws = 1;
	size_t lws = 1;

	__do_set_cmd_ndrange_kernel_1(*td,thr,1,&gwo,&gws,&lws);

	struct coprthr_device* dev = __ddtab[(*attr)->dd];

	if ((*attr)->initstate == COPRTHR_A_CREATE_EXECUTE) 
		__do_enqueue_cmd_1(dev,&((*td)->ev));
	
	return 0;	
}


int coprthr_join( coprthr_td_t td, void** val )
{
	if (td==0)
		return EINVAL;

//	int err = __do_thread_join_1();
	__do_wait_1( &(td->ev) );

	free(td);
	
	return 0;
}


#if(0)
int coprthr_ncreate( unsigned int nthr, coprthr_td_t* td, 
	coprthr_td_attr_t* attr, coprthr_sym_t thr, void* parg )
{
	printcl( CL_DEBUG "coprthr_ncreate");

	if (td==0)
		return EFAULT;

	if (attr==0 || *attr==0 || thr==0) 
		return EINVAL;

	if ( (*td = __malloc_struct(coprthr_td)) == 0) 
		return ENOMEM;

	__do_set_kernel_arg_1( thr, 0, 0, pargs );

	__coprthr_init_event(*td);

	size_t gwo = 0;
	size_t gws = nthr;
	size_t lws = 1;

	__do_set_cmd_ndrange_kernel_1(*td,thr,1,&gwo,&gws,&lws);

	struct coprthr_device* dev = __ddtab[*attr->dd];

	__do_enqueue_cmd_1(dev,*td);
	
	return 0;	
}


int coprthr_group( coprthr_td_t* tdgrp, unsigned int ntd, coprthr_td_t* v_td,
	int flags )
{
	int i;

	if (!ntd || !v_td)
		return EINVAL;

	*tdgrp = __malloc_struct(coprthr_td);

	for(i=0; i<ntd; i++)
		/* mark and chain */

	return 0;
}


int coprthr_ungroup( coprthr_td_t* tdgrp, int flags )
{
	free(*tdgrp);
}


int coprthr_sched( coprthr_td_t* td, int action, int flags )
{
	if (td) {

		if (action==COPRTHR_S_EXECUTE) {
		} else if (action==COPRTHR_S_YIELD) {
		} else if (action==COPRTHR_S_SUSPEND) {
		} else if (action==COPRTHR_S_EXIT) {
		} else return EINVAL;

	} else return EINVAL;

}


/***
 *** mutex
 ***/

int coprthr_mutex_attr_init( coprthr_mutex_attr_t* mtxattr)
{
	if (mtxattr==0)
		return EFAULT;

	if ( (*mtxattr = __malloc_struct(coprthr_mutex_attr)) == 0)
		return ENOMEM;

	return 0;
}


int coprthr_mutex_attr_destroy( coprthr_mutext_attr_t* mtxattr)
{
	if (mtxattr==0)
		return EFAULT;

	if (*mtxattr==0)
		return EINVAL;

	free(*mtxattr);

	return 0;
}


int coprthr_mutex_attr_setdevice( coprthr_mutext_attr_t* mtxattr, int dd )
{
	if (mtxattr==0) 
		return EFAULT;

	if (*mtxattr==0 || dd<0)
		return EINVAL;

	*mtxattr->dd = dd;

	return 0;
}


int coprthr_mutex_init( coprthr_mutex_t* mtx, coprthr_mutext_attr_t* mtxattr )
{
	if (!mtx || !mtxattr)
		return EINVAL;

	if ( (*mtx = __malloc_struct(coprthr_mutex)) == 0) 
		return ENOMEM;

	struct coprthr_device* dev = __ddtab[mtxattr->dd];

	coprthr_mem_t mem 
		= coprthr_devmemalloc(dev,0,1,0,COPRTHR_DEVMEM_TYPE_MUTEX);

	if (!mem)
		return ENOMEM;

	/* XXX it is possible that error shold be ENOSUP
	return 0;
}


int coprthr_mutex_destroy( coprthr_mutex_t* mtx )
{
	if (mtx==0)
		return EFAULT;

	coprthr_devmemfree(mem);

	free(*mtx);

	return 0;
}


int coprthr_mutex_lock( coprthr_mutex_t* mtx)
{
	if (!mtx)
		return EINVAL;

	__do_mutex_lock_1(mtx);

	/* from mtx, get dev, and then perform the device-specific lock
	 * using call from devops
	 */

	return 0;
}


int coprthr_mutex_unlock( coprthr_mutex_t* mtx)
{
	if (!mtx)
		return EINVAL;

	return 0;
}


/***
 *** cond variable
 ***/

int coprthr_cond_attr_init( coprthr_cond_attr_t* condattr)
{
	if (condattr=0)
		return EFAULT;

	if (*condattr = __malloc_struct(coprthr_cond_attr)) == 0)
		return ENOMEM;
	
	return 0;
}


int coprthr_cond_attr_destroy( coprthr_cond_attr_t* condattr)
{
	if (condattr=0)
		return EFAULT;
	
	if (*condattr=0)
		return EINVAL;

	free(*condattr);

	return 0;
}


int coprthr_cond_attr_setdevice( coprthr_cond_attr_t* condattr, int dd)
{
	if (!condattr)
		return EINVAL;

	if (dd<0) 
		return EINVAL;

	condattr->dd = dd;

	return 0;
}


int coprthr_cond_init( coprthr_cond_t* cond, coprthr_cond_attr_t* condattr )
{
	if (!cond || !condattr)
		return EINVAL; /* XXX this should really be EFAULT */

	/* XXX check if condattr is bad, this is an EINVAL */

	if ( (*cond = __malloc_struct(coprthr_cond)) == 0)
		return ENOMEM;

	struct coprthr_device* dev = __ddtab[condattr->dd];

	coprthr_mem_t mem 
		= coprthr_devmemalloc(dev,0,1,0,COPRTHR_DEVMEM_TYPE_SIGNAL);

	if (!mem)
		return ENOMEM;

	return 0;
}


int coprthr_cond_destroy( coprthr_cond_t* cond )
{
	if (!cond)
		return EINVAL;

	if (*cond->mem)
		coprthr_devmemfree((*cond)->mem);

	free(*cond);

	return 0;
}


int coprthr_cond_wait( coprthr_cond_t* cond, coprthr_mutex_t* mtx)
{
	
}


int coprthr_cond_signal(coprthr_cond_t* cond)
{
}


int coprthr_kill( coprthr_td_t td, int sig )
{
}

#endif

