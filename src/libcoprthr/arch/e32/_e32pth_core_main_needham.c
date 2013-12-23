/* _e32pth_core_main_needham.c
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

#define E_ROWS_IN_CHIP 4
#define E_COLS_IN_CHIP 4
#define E_FIRST_CORE_ROW 32
#define E_FIRST_CORE_COL 8

#include <stdlib.h>
#include <string.h>
#include <e_coreid.h>
#include <e_common.h>
#include <e_mutex.h>

#define XXX_RUN 0x7f90
#define XXX_DEBUG 0x7f94
#define XXX_INFO 0x7f98

#include "e32_config_needham.h"
#include "e32pth_mem_if_needham.h"
#include "e32pth_if_needham.h"
#include "e32_opencl_ext.h"

#include "esdk_missing.h"

#define TIMEOUT 100000

typedef void (*callp_t)(void*);

volatile struct core_local_data_struct core_local_data SECTION("section_core");
volatile e32_workp_entry_t core_we SECTION("section_core");
volatile struct thr_data_struct thr_data SECTION("section_core");
unsigned char coremap[E32_NCORES] SECTION("section_core");
unsigned char threadmap[E32_NCORES] SECTION("section_core");

int __giant SECTION("section_core") = 0;
int __active SECTION("section_core") = 0;

int _local_mem_base SECTION("section_core"); /* just a marker, no used */

__inline 
static unsigned core_num(e_coreid_t coreid)
{
   unsigned row, col;

   row = (coreid >> 6) & (E_ROWS_IN_CHIP - 1);
   col =  coreid       & (E_COLS_IN_CHIP - 1);
   return row * E_COLS_IN_CHIP + col;
}

typedef void (*func_t)(int*,int*);
extern func_t test_arg_1_1_kern;

int main(void)
{

	volatile int* xxx_run = (int*)XXX_RUN;
	volatile int* xxx_debug = (int*)XXX_DEBUG;
	volatile int* xxx_info = (int*)XXX_INFO;

	int status = 0;

	*xxx_debug = 50;
	*xxx_info = 0;

	int timeout = 0;
   while ((*xxx_run) == 0 && timeout < TIMEOUT) {
		timeout++;
	}
	if (timeout == TIMEOUT) {
		*xxx_run = -2;
		exit(-1);
	}

	*xxx_run = 2;

	*xxx_debug = 51;

//	callp_t callp = (callp_t)e32_ctrl_callp[core_local_data.corenum];
//	*xxx_info = (int)callp;
//
//		e32_ctrl_retval[core_local_data.corenum] = -99;
//	*xxx_info = core_local_data.corenum;
//
//	*xxx_debug = 100+core_local_data.corenum;
//
//*xxx_run = 0;
//exit(0);

/*
	timeout = 0;
   while ((*xxx_run) == 2 && timeout < TIMEOUT) {
		timeout++;
	}
	if (timeout == TIMEOUT) {
		*xxx_run = -2;
		exit(-1);
	}
		*xxx_run = 3;
*/

	e_coreid_t tmpcid;
   core_local_data.coreID = e_get_coreid();
   core_local_data.corenum = core_num(core_local_data.coreID);
   e_coords_from_coreid(core_local_data.coreID, 
		(unsigned*)&core_local_data.row, (unsigned*)&core_local_data.col);
   core_local_data.row = core_local_data.row - E_FIRST_CORE_ROW;
   core_local_data.col = core_local_data.col - E_FIRST_CORE_COL;
   core_local_data.sigb;
   core_local_data.recvaddr;
   core_local_data.thrs_self = (threadspec_t)e_address_from_coreid(
		core_local_data.coreID, (void*)&core_local_data);

	memcpy((void*)coremap,(void*)&e32_coremap[0],E32_NCORES);
	memcpy((void*)threadmap,(void*)&e32_threadmap[0],E32_NCORES);
	memcpy((void*)core_we,(void*)&e32_we[0],sizeof(e32_workp_entry_t));

callp_t callp = (callp_t)e32_ctrl_callp[core_local_data.corenum];
//*xxx_info = (int)callp;
//*xxx_info = (int)&e32_ctrl_callp[core_local_data.corenum];
//*xxx_info = *(int*)0x8e1000c0;
//*xxx_run = 0;
//exit(0);

		*xxx_debug = 52;

		int count = 0;

		int dim = we_ndr_dim(core_we);	
		int ltsz0 = we_ndr_ltdsz(core_we,0);
		int ltsz1 = we_ndr_ltdsz(core_we,1);
		int nthr = ltsz0 * ltsz1;
	
      int i0,i1,i2;

		i2 = 0;
		int m = (int)threadmap[core_local_data.corenum];
		i1 = m / ltsz0;
		i0 = m % ltsz0;

		thr_data.ltdidx[0] = i0;
		thr_data.ltdidx[1] = i1;
		thr_data.ltdidx[2] = i2;

		if (m < 255) {

		if (dim == 1) {

			unsigned int id = (i0 + ltsz0 - 1) % ltsz0;
			core_local_data.thrs_prev[0] = get_thread( 1, &id );
			core_local_data.thrs_prev[1] = 0;

			id = (i0 + 1) % ltsz0;
			core_local_data.thrs_next[0] = get_thread( 1, &id );
			core_local_data.thrs_next[1] = 0;

		} else if (dim == 2) {

			unsigned int id[2];

			id[0] = (i0 + ltsz0 - 1) % ltsz0;
			id[1] = i1;
			core_local_data.thrs_prev[0] = get_thread( 2, id );
			id[0] = (i0 + 1) % ltsz0;
			core_local_data.thrs_next[0] = get_thread( 2, id );

			id[0] = i0;
			id[1] = (i1 + ltsz1 - 1) % ltsz1;
			core_local_data.thrs_prev[1] = get_thread( 2, id );
			id[1] = (i1 + 1) % ltsz1;
			core_local_data.thrs_next[1] = get_thread( 2, id );

		}

		*xxx_debug = 62;

		{
			int mm = (m+nthr-1) % nthr;	
			int n = (int)coremap[mm];
			e_coreid_t coreid = __corenum_to_coreid(n);
			core_local_data.psigb_prev = (threadspec_t)e_address_from_coreid( 
				coreid, (void*)&core_local_data.sigb);
			
			mm = (m+1) % nthr;	
			n = (int)coremap[mm];
			coreid = __corenum_to_coreid(n);
			core_local_data.psigb_next = (threadspec_t)e_address_from_coreid( 
				coreid, (void*)&core_local_data.sigb);
		}


		}

   	core_local_data.count = 0;

		*xxx_debug = 53;

//	*xxx_run = 2;

/*
	timeout = 0;
   while ((*xxx_run) == 2 && timeout < TIMEOUT) {
		timeout++;
	}
	if (timeout == TIMEOUT) {
		*xxx_run = -2;
		exit(-1);
	}
		*xxx_run = 3;
*/

//		*xxx_debug = 72;

///////////////////////////////////////////////////////////////////////

//	callp_t callp = (callp_t)e32_ctrl_callp[core_local_data.corenum];
//	*xxx_info = (int)callp;
//	*xxx_info = (int)&e32_ctrl_callp[core_local_data.corenum];


//*xxx_run = 0;
//exit(0);

		if (callp && callp > (callp_t)0x8000) {

			*xxx_debug = 91;
			*xxx_info = (int)callp;

			status = -1;

		} else if (m == 255) {

			*xxx_debug = 92;

			status = 0;

		} else {

			*xxx_debug = 54;

			thr_data.blkidx[2] = we_ndp_blk_first(core_we,2);

			while( thr_data.blkidx[2] < we_ndp_blk_end(core_we,2) ) {

				thr_data.gtdidx[2] 
					= thr_data.blkidx[2]*we_ndr_ltdsz(core_we,2) + i2;

				thr_data.blkidx[1] = we_ndp_blk_first(core_we,1);

				while( thr_data.blkidx[1] < we_ndp_blk_end(core_we,1) ) {

					thr_data.gtdidx[1] 
						= thr_data.blkidx[1]*we_ndr_ltdsz(core_we,1) + i1;

					thr_data.blkidx[0] = we_ndp_blk_first(core_we,0);

					while( thr_data.blkidx[0] < we_ndp_blk_end(core_we,0) ) {

						thr_data.gtdidx[0] 
							= thr_data.blkidx[0]*we_ndr_ltdsz(core_we,0) + i0;

//							++e32_ctrl_run[core_local_data.corenum];
							++(*xxx_run);
							callp(0);
//							--e32_ctrl_run[core_local_data.corenum];
							--(*xxx_run);
							++count;

							*xxx_debug = 55;

//							barrier(0);
//							e_barrier(bar_array,tgt_bar_array);
							barrier_thread_all(0);

					++thr_data.blkidx[0];
					}
				
				++thr_data.blkidx[1];
				}

			++thr_data.blkidx[2];
			}

			status = count;

			*xxx_debug = 56;

		}

		e32_ctrl_retval[core_local_data.corenum] = status;
//      e32_ctrl_run[core_local_data.corenum] = 0;
		*xxx_run = 0;
      core_local_data.count++;

		__active = 0;

//		break;

//   }

//	return status;
	exit(0);

}

