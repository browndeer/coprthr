/* clsched.c
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


/* XXX to do, add err code checks, other safety checks * -DAR */
/* XXX to do, clvplat_destroy should automatically release all txts -DAR */

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <CL/cl.h>
#include "util.h"
#include "printcl.h"
#include "clerrno.h"


#define __STDCL__
#include "clsched.h"


LIBSTDCL_API
cl_event clfork(
	CONTEXT* cp, cl_uint devnum, 
	cl_kernel krns, struct clndrange_struct* ndr, int flags
)
{
	cl_kernel krn = (cp->nctx > 1)? 
		((cl_kernel*)krns)[cp->devctxi[devnum]] : krns;
	
	if (cp->kev[devnum].nev==STDCL_EVENTLIST_MAX) return((cl_event)0);

	/* XXX here you should check flags to ensure sufficient and valid -DAR */

	cl_event* evp = (cp->kev[devnum].ev_first==cp->kev[devnum].ev_free)? 
		0 : cp->kev[devnum].ev + cp->kev[devnum].ev_free -1;

	cl_event ev;

	printcl( CL_DEBUG "clfork: devnum=%d\n",devnum);
	printcl( CL_DEBUG "clfork: kev,evp %p,%p\n",cp->kev[devnum].ev,evp);

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	 printcl( CL_DEBUG "clfork: ndr.dim=%d\n",ndr->dim);

#if defined(CL_VERSION_1_1)
	 printcl( CL_DEBUG "clfork: using CL_VERSION_1_1");
#endif

#if defined(CL_VERSION_1_1)
	 printcl( CL_DEBUG "clfork: ndr.gtid_offset=%d %d %d %d\n",
		ndr->gtid_offset[0],ndr->gtid_offset[1],
		ndr->gtid_offset[2],ndr->gtid_offset[3]);
#endif
	 printcl( CL_DEBUG "clfork: ndr.gtid=%d %d %d %d\n",
		ndr->gtid[0],ndr->gtid[1],ndr->gtid[2],ndr->gtid[3]);
	 printcl( CL_DEBUG "clfork: ndr.ltid=%d %d %d %d\n",
		ndr->ltid[0],ndr->ltid[1],ndr->ltid[2],ndr->ltid[3]);

	if (flags & CL_FAST) {

#if defined(CL_VERSION_1_1)
	 printcl( CL_DEBUG "clfork: using CL_VERSION_1_1");
#endif

	int err = clEnqueueNDRangeKernel(
#if defined(CL_VERSION_1_1)
		cp->cmdq[devnum],krn,ndr->dim,ndr->gtid_offset,ndr->gtid,ndr->ltid,
#else
		cp->cmdq[devnum],krn,ndr->dim,0,ndr->gtid,ndr->ltid,
#endif
		0,0,0
	);
	__set_oclerrno(err);

	return((cl_event)0);


	} else {

	int err = clEnqueueNDRangeKernel(
#if defined(CL_VERSION_1_1)
		cp->cmdq[devnum],krn,ndr->dim,ndr->gtid_offset,ndr->gtid,ndr->ltid,
#else
		cp->cmdq[devnum],krn,ndr->dim,0,ndr->gtid,ndr->ltid,
#endif
		(evp)?1:0,evp,&ev
	);
	__set_oclerrno(err);

	printcl( CL_DEBUG "clfork: clEnqueueNDRangeKernel err %d\n",err);

	if (flags & CL_EVENT_NOWAIT) {

		cp->kev[devnum].ev[cp->kev[devnum].ev_free++] = ev;
		cp->kev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->kev[devnum].nev;

	} else if (flags & CL_EVENT_WAIT) { /* CL_EVENT_WAIT */

		printcl( CL_DEBUG "clfork: clWaitForEvents %d,%p",1,&ev);

		err = clWaitForEvents( 1, &ev);
		__set_oclerrno(err);
		printcl( CL_DEBUG "clfork: clWaitForEvents %d",err);

//#ifdef USE_DEPRECATED_FLAGS
//		if (flags & CL_EVENT_RELEASE && !(flags & CL_EVENT_NORELEASE) ) {
//			clReleaseEvent(ev);
//			ev = (cl_event)0;
//		}
//#else
		if ( !(flags & CL_EVENT_NORELEASE) ) {
			err = clReleaseEvent(ev);
			__set_oclerrno(err);
			ev = (cl_event)0;
		}
//#endif

	}

	}


	printcl( CL_DEBUG "clfork first,free %d,%d\n",
		cp->kev[devnum].ev_first,cp->kev[devnum].ev_free);
	
	return(ev);
}


LIBSTDCL_API
cl_event clwait(CONTEXT* cp, unsigned int devnum, int flags)
{
	int err;
	int n;
	cl_event* evp;

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

//	if (flags&CL_FAST) {
//		err = clFinish(cp->cmdq[0]);
//		__set_oclerrno(err);
//		return((cl_event)0);
//	}

	/* XXX clwait should be responsive to these flags, fix -DAR */

//#ifdef USE_DEPRECATED_FLAGS
//	if ( !flags&CL_EVENT_RELEASE ) {
//		WARN(__FILE__,__LINE__,"clwait: forcing CL_EVENT_RELEASE");
//	}
//#else
//	if ( flags&CL_EVENT_NORELEASE ) {
//		printcl( CL_WARNING "clwait: ignoring CL_EVENT_NORELEASE");
//	}
//#endif

	int release = (flags&CL_EVENT_NORELEASE)? 0 : 1;

	/* provide warning if no event list has been chosen */

	if (!flags&(CL_KERNEL_EVENT|CL_MEM_EVENT)) {
		printcl( CL_WARNING "clwait: no event list specified");
	}


	printcl( CL_DEBUG "clwait: devnum=%d kev.ndev=%d mev.nev=%d",
		devnum,cp->kev[devnum].nev,cp->mev[devnum].nev);

	/*
	 * wait on kernel events
	 */

	if (flags&CL_KERNEL_EVENT && cp->kev[devnum].nev > 0) {

		printcl( CL_DEBUG "clwait first,free=%d,%d knev=%d\n",
			cp->kev[devnum].ev_first,cp->kev[devnum].ev_free,cp->kev[devnum].nev);

//		if (!cp->kev[devnum].nev) return((cl_event)0);

		if (cp->kev[devnum].ev_first < cp->kev[devnum].ev_free) {

			printcl( CL_DEBUG "clwait one event list");

			printcl( CL_DEBUG "clwait: clWaitForEvents(%d,%p) %p",
				cp->kev[devnum].ev_free - cp->kev[devnum].ev_first,
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first],cp->kev[devnum].ev);

			err = clWaitForEvents(
				cp->kev[devnum].ev_free - cp->kev[devnum].ev_first, 
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first] );
			__set_oclerrno(err);

			for(evp = cp->kev[devnum].ev + cp->kev[devnum].ev_first; 
				evp < cp->kev[devnum].ev + cp->kev[devnum].ev_free; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				} 
			}

			cp->kev[devnum].nev = cp->kev[devnum].ev_first 
				= cp->kev[devnum].ev_free = 0;

		} else if (cp->kev[devnum].ev_free > cp->kev[devnum].ev_first) {

			printcl( CL_DEBUG "clwait two event lists");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX - cp->kev[devnum].ev_first, 
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first]);
			__set_oclerrno(err);

			err = clWaitForEvents(cp->kev[devnum].ev_free, cp->kev[devnum].ev);
			__set_oclerrno(err);

			for(evp = cp->kev[devnum].ev + cp->kev[devnum].ev_first; 
				evp < cp->kev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			for(evp = cp->kev[devnum].ev; 
				evp < cp->kev[devnum].ev + cp->kev[devnum].ev_free; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			cp->kev[devnum].nev 
				= cp->kev[devnum].ev_first = cp->kev[devnum].ev_free = 0;

		} else {

			printcl( CL_DEBUG "clwait full even list");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX,cp->kev[devnum].ev);
			__set_oclerrno(err);

			for(evp = cp->kev[devnum].ev; 
				evp < cp->kev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			cp->kev[devnum].nev 
				= cp->kev[devnum].ev_first = cp->kev[devnum].ev_free = 0;

		}

	}

	if (flags&CL_MEM_EVENT && cp->mev[devnum].nev > 0) {

		printcl( CL_DEBUG "clwait first,free=%d,%d mnev=%d\n",
			cp->mev[devnum].ev_first,cp->mev[devnum].ev_free,cp->mev[devnum].nev);

//		if (!cp->mev[devnum].nev) return((cl_event)0);

		if (cp->mev[devnum].ev_first < cp->mev[devnum].ev_free) {

			printcl( CL_DEBUG "clwait one event list");

			printcl( CL_DEBUG "clwait: clWaitForEvents(%d,%p) %p",
				cp->mev[devnum].ev_free - cp->mev[devnum].ev_first,
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first],cp->mev[devnum].ev);

			err = clWaitForEvents(
				cp->mev[devnum].ev_free - cp->mev[devnum].ev_first, 
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first]);
			__set_oclerrno(err);

			for(evp = cp->mev[devnum].ev + cp->mev[devnum].ev_first; 
				evp < cp->mev[devnum].ev + cp->mev[devnum].ev_free; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		} else if (cp->mev[devnum].ev_free > cp->mev[devnum].ev_first) {

			printcl( CL_DEBUG "clwait two event lists");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX - cp->mev[devnum].ev_first, 
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first]);
			__set_oclerrno(err);

			err = clWaitForEvents(cp->mev[devnum].ev_free, cp->mev[devnum].ev);
			__set_oclerrno(err);

			for(evp = cp->mev[devnum].ev + cp->mev[devnum].ev_first; 
				evp < cp->mev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) {
				if (release) {
					clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			for(evp = cp->mev[devnum].ev; 
				evp < cp->mev[devnum].ev + cp->mev[devnum].ev_free; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		} else {

			printcl( CL_DEBUG "clwait full even list");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX,cp->mev[devnum].ev);
			__set_oclerrno(err);

			for( evp = cp->mev[devnum].ev; 
				evp < cp->mev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) {
				if (release) {
					err = clReleaseEvent(*evp);
					__set_oclerrno(err);
				}
			}

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		}

	}
	
	return((cl_event)0);

}


LIBSTDCL_API
cl_event clwaitev(
  CONTEXT* cp, unsigned int devnum, const cl_event ev, int flags
)
{
	int err;

	/* XXX clwait should be responsive to these flags, fix -DAR */

//#ifdef USE_DEPRECATED_FLAGS
//	if ( !flags&CL_EVENT_RELEASE ) {
//		WARN(__FILE__,__LINE__,"clwait: forcing CL_EVENT_RELEASE");
//	}
//#else
//	if ( flags&CL_EVENT_NORELEASE ) {
//		WARN(__FILE__,__LINE__,"clwait: ignoring CL_EVENT_NORELEASE");
//	}
//#endif

	err = clWaitForEvents(1,&ev);
	__set_oclerrno(err);

	if ( !(flags&CL_EVENT_NORELEASE) ) {
		err = clReleaseEvent(ev);
		__set_oclerrno(err);
	}

	/* XXX here should force ev to 0 however, require evp -DAR */

	return(ev);
}


LIBSTDCL_API
int clflush(CONTEXT* cp, unsigned int devnum, int flags)
{

//	if (flags&CL_FAST) {
//		clFinish(cp->cmdq[0]);
//		return((cl_event)0);
//	}

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	int err = clFlush(cp->cmdq[devnum]);
	__set_oclerrno(err);

	return(0);

}

