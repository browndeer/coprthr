/* clsched.c
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


/* XXX to do, add err code checks, other safety checks * -DAR */
/* XXX to do, clvplat_destroy should automatically release all txts -DAR */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <dlfcn.h>

#include <CL/cl.h>
//#include "stdcl.h"
#include "util.h"
#include "clsched.h"



#define __error(e) do { errno=e; return(-1); } while(0); 


/* XXX hack to work around the problem with clCreateCommandQueue -DAR */
//static inline void __cmdq__(CONTEXT* cp, cl_uint n)
//{
//   if (!cp->cmdq[n]) 
//      cp->cmdq[n] = clCreateCommandQueue(cp->ctx,cp->dev[n],0,0);
//}


cl_event clfork(
	CONTEXT* cp, cl_uint devnum, 
	cl_kernel krn, struct clndrange_struct* ndr, int flags
)
{
	
	if (cp->kev[devnum].nev==STDCL_EVENTLIST_MAX) return((cl_event)0);

	/* XXX here you should check flags to ensure sufficient and valid -DAR */

	cl_event* evp = (cp->kev[devnum].ev_first==cp->kev[devnum].ev_free)? 
		0 : cp->kev[devnum].ev + cp->kev[devnum].ev_free -1;

	cl_event ev;

	DEBUG(__FILE__,__LINE__,"clfork: devnum=%d\n",devnum);
	DEBUG(__FILE__,__LINE__,"clfork: kev,evp %p,%p\n",cp->kev[devnum].ev,evp);

//	__cmdq__(cp,devnum); // XXX this is a hack -DAR

	 DEBUG(__FILE__,__LINE__,"clfork: ndr.dim=%d\n",ndr->dim);
	 DEBUG(__FILE__,__LINE__,"clfork: ndr.gtid=%d %d %d %d\n",
		ndr->gtid[0],ndr->gtid[1],ndr->gtid[2],ndr->gtid[3]);
	 DEBUG(__FILE__,__LINE__,"clfork: ndr.ltid=%d %d %d %d\n",
		ndr->ltid[0],ndr->ltid[1],ndr->ltid[2],ndr->ltid[3]);

	if (flags & CL_FAST) {

	int err = clEnqueueNDRangeKernel(
		cp->cmdq[devnum],krn,ndr->dim,0,ndr->gtid,ndr->ltid,
		0,0,0
	);
	return((cl_event)0);


	} else {

	int err = clEnqueueNDRangeKernel(
		cp->cmdq[devnum],krn,ndr->dim,0,ndr->gtid,ndr->ltid,
		(evp)?1:0,evp,&ev
	);

	DEBUG(__FILE__,__LINE__,"clfork: clEnqueueNDRangeKernel err %d\n",err);

	if (flags & CL_EVENT_NOWAIT) {

		cp->kev[devnum].ev[cp->kev[devnum].ev_free++] = ev;
		cp->kev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->kev[devnum].nev;

//	} else { /* CL_EVENT_WAIT */
	} else if (flags & CL_EVENT_WAIT) { /* CL_EVENT_WAIT */

		DEBUG(__FILE__,__LINE__,"clfork: clWaitForEvents %d,%p",1,&ev);

		err = clWaitForEvents( 1, &ev);
		DEBUG(__FILE__,__LINE__,"clfork: clWaitForEvents %d",err);

		if (flags & CL_EVENT_RELEASE) {
		
			clReleaseEvent(ev);
			ev = (cl_event)0;

		}

	}

	}


	DEBUG(__FILE__,__LINE__,
		"clfork first,free %d,%d\n",cp->kev[devnum].ev_first,cp->kev[devnum].ev_free);
	
	return(ev);
}


cl_event clwait(CONTEXT* cp, unsigned int devnum, int flags)
{
	int err;
	int n;
	cl_event* evp;

	if (flags&CL_FAST) {
		clFinish(cp->cmdq[0]);
		return((cl_event)0);
	}

	/* XXX clwait should be responsive to this flag, fix -DAR */

	if (!flags&CL_EVENT_RELEASE) {
		WARN(__FILE__,__LINE__,"clwait: forcing CL_EVENT_RELEASE");
	}

	/* provide warning if no event list has been chosen */

	if (!flags&(CL_KERNEL_EVENT|CL_MEM_EVENT)) {
		WARN(__FILE__,__LINE__,"clwait: no event list specified");
	}


	/*
	 * wait on kernel events
	 */

	if (flags&CL_KERNEL_EVENT) {

		DEBUG(__FILE__,__LINE__,
			"clwait first,free=%d,%d knev=%d\n",
			cp->kev[devnum].ev_first,cp->kev[devnum].ev_free,cp->kev[devnum].nev);

		if (!cp->kev[devnum].nev) return((cl_event)0);

		if (cp->kev[devnum].ev_first < cp->kev[devnum].ev_free) {

			DEBUG(__FILE__,__LINE__, "clwait one event list");

			DEBUG(__FILE__,__LINE__, "clwait: clWaitForEvents(%d,%p) %p",
				cp->kev[devnum].ev_free - cp->kev[devnum].ev_first,
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first],cp->kev[devnum].ev);

DEBUG(__FILE__,__LINE__, "clwait: here");

			err = clWaitForEvents(
				cp->kev[devnum].ev_free - cp->kev[devnum].ev_first, 
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first]
			);

DEBUG(__FILE__,__LINE__, "clwait: here");

			for(evp = cp->kev[devnum].ev + cp->kev[devnum].ev_first; 
				evp < cp->kev[devnum].ev + cp->kev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->kev[devnum].nev = cp->kev[devnum].ev_first 
				= cp->kev[devnum].ev_free = 0;

		} else if (cp->kev[devnum].ev_free > cp->kev[devnum].ev_first) {

			DEBUG(__FILE__,__LINE__, "clwait two event lists");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX - cp->kev[devnum].ev_first, 
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first]
			);

			err = clWaitForEvents(cp->kev[devnum].ev_free, cp->kev[devnum].ev);

			for(evp = cp->kev[devnum].ev + cp->kev[devnum].ev_first; 
				evp < cp->kev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) clReleaseEvent(*evp);

			for(evp = cp->kev[devnum].ev; 
				evp < cp->kev[devnum].ev + cp->kev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->kev[devnum].nev = cp->kev[devnum].ev_first = cp->kev[devnum].ev_free = 0;

		} else {

			DEBUG(__FILE__,__LINE__, "clwait full even list");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX,cp->kev[devnum].ev);

			for(evp = cp->kev[devnum].ev; evp < cp->kev[devnum].ev + STDCL_EVENTLIST_MAX; evp++)
				clReleaseEvent(*evp);

			cp->kev[devnum].nev = cp->kev[devnum].ev_first = cp->kev[devnum].ev_free = 0;

		}

	}

	if (flags&CL_MEM_EVENT) {

		DEBUG(__FILE__,__LINE__,
			"clwait first,free=%d,%d mnev=%d\n",
			cp->mev[devnum].ev_first,cp->mev[devnum].ev_free,cp->mev[devnum].nev);

		if (!cp->mev[devnum].nev) return((cl_event)0);

		if (cp->mev[devnum].ev_first < cp->mev[devnum].ev_free) {

			DEBUG(__FILE__,__LINE__, "clwait one event list");

			DEBUG(__FILE__,__LINE__, "clwait: clWaitForEvents(%d,%p) %p",
				cp->mev[devnum].ev_free - cp->mev[devnum].ev_first,
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first],cp->mev[devnum].ev);

			err = clWaitForEvents(
				cp->mev[devnum].ev_free - cp->mev[devnum].ev_first, 
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first]
			);

			for(evp = cp->mev[devnum].ev + cp->mev[devnum].ev_first; 
				evp < cp->mev[devnum].ev + cp->mev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		} else if (cp->mev[devnum].ev_free > cp->mev[devnum].ev_first) {

			DEBUG(__FILE__,__LINE__, "clwait two event lists");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX - cp->mev[devnum].ev_first, 
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first]);

			err = clWaitForEvents(cp->mev[devnum].ev_free, cp->mev[devnum].ev);

			for(evp = cp->mev[devnum].ev + cp->mev[devnum].ev_first; 
				evp < cp->mev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) clReleaseEvent(*evp);

			for(evp = cp->mev[devnum].ev; 
				evp < cp->mev[devnum].ev + cp->mev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		} else {

			DEBUG(__FILE__,__LINE__, "clwait full even list");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX,cp->mev[devnum].ev);

			for(evp = cp->mev[devnum].ev; evp < cp->mev[devnum].ev + STDCL_EVENTLIST_MAX; evp++)
				clReleaseEvent(*evp);

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		}

	}
	
	return((cl_event)0);

}


cl_event clwaitev(
  CONTEXT* cp, unsigned int devnum, const cl_event ev, int flags
)
{
	int err;

	/* XXX clwait should be responsive to this flag, fix -DAR */

	if (!flags&CL_EVENT_RELEASE) {
		WARN(__FILE__,__LINE__,"clwait: forcing CL_EVENT_RELEASE");
	}


	err = clWaitForEvents(1,&ev);
	clReleaseEvent(ev);

	/* XXX here should force ev to 0 however, require evp -DAR */

	return(ev);
}


int clflush(CONTEXT* cp, unsigned int devnum, int flags)
{

//	if (flags&CL_FAST) {
//		clFinish(cp->cmdq[0]);
//		return((cl_event)0);
//	}

	clFlush(cp->cmdq[devnum]);

	return(0);

}

