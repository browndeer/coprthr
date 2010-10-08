/* clarg.c
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


#include <string.h>
#include <stdio.h>
//#include <sys/queue.h>

#include <CL/cl.h>

//#include "clcontext.h"
//#include "clinit.h"
//#include "clsched.h"
#include "clmalloc.h"
//#include "clfcn.h"
#include "util.h"


#ifdef __cplusplus
extern "C" {
#endif

size_t 
__clarg_set_global(CONTEXT* cp, cl_kernel krn, unsigned int argnum, void* ptr) 
{
	struct _memd_struct* memd = 0;
	size_t offset = -1;

//   struct _memd_struct* memd;
//   for (
//      txt = cp->txt_listhead.lh_first; txt != 0;
//      txt = txt->txt_list.le_next
//   ) {
//      if (prgs==0 || txt->prgs == prgs) break;
//   }

	if (__test_memd_magic(ptr)) {
	
		memd = (struct _memd_struct*)((intptr_t)ptr-sizeof(struct _memd_struct)); 
		offset = 0;

	} else {
		for (
			memd = cp->memd_listhead.lh_first; memd != 0;
			memd = memd->memd_list.le_next
			) {
				intptr_t p1 = (intptr_t)memd + sizeof(struct _memd_struct);
				intptr_t p2 = p1 + memd->sz;
				if (p1 < (intptr_t)ptr && (intptr_t)ptr < p2) {
					DEBUG(__FILE__,__LINE__,"memd match"); 
					offset = (intptr_t)ptr - p1;
					break; 
				}
		}
	}

	if (memd) clSetKernelArg(krn,argnum,sizeof(cl_mem),(void*)&memd->clbuf); 

	return(offset);	
}

#ifdef __cplusplus
}
#endif


