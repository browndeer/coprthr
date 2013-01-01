
#include <string.h>
#include <e_coreid.h>
#include <e_common.h>

#include "e32_config.h"
#include "e32ser_mem_if.h"
#include "e32ser_if.h"

typedef void (*callp_t)(void*);

volatile struct core_local_data_struct core_local_data SECTION("section_core");
volatile e32_workp_entry_t core_we SECTION("section_core");
volatile struct thr_data_struct thr_data SECTION("section_core");
int _local_mem_base SECTION("section_core"); /* just a marker, not used */


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
	int status;

	status = 0;

   core_local_data.coreID = e_get_coreid();
   core_local_data.corenum = core_num(core_local_data.coreID);
   e_coords_from_coreid(core_local_data.coreID, 
		(unsigned*)&core_local_data.row, (unsigned*)&core_local_data.col);
   core_local_data.row = core_local_data.row - E_FIRST_CORE_ROW;
   core_local_data.col = core_local_data.col - E_FIRST_CORE_COL;

	e32_ctrl_run[core_local_data.corenum] = 0;
	e32_ctrl_ready[core_local_data.corenum] = 1;

   core_local_data.count = 0;

	e32_ctrl_run[core_local_data.corenum] = 0;

   while (1) {
      while (e32_ctrl_run[core_local_data.corenum] == 0) {};

		e32_ctrl_run[core_local_data.corenum] = 2;

		memcpy((void*)core_we,(void*)&e32_we[core_local_data.corenum],
			sizeof(e32_workp_entry_t));

		callp_t callp = (callp_t)e32_ctrl_callp[core_local_data.corenum];

		int count = 0;

		volatile e32_uint_t* pi0 = (e32_uint_t*)&thr_data.ltdidx[0];
		volatile e32_uint_t* pi1 = (e32_uint_t*)&thr_data.ltdidx[1];
		volatile e32_uint_t* pi2 = (e32_uint_t*)&thr_data.ltdidx[2];
		volatile e32_uint_t* pj0 = (e32_uint_t*)&thr_data.gtdidx[0];
		volatile e32_uint_t* pj1 = (e32_uint_t*)&thr_data.gtdidx[1];
		volatile e32_uint_t* pj2 = (e32_uint_t*)&thr_data.gtdidx[2];

//		if (callp && callp < (callp_t)0x7000) {
		if (callp && callp < (callp_t)E32_CORE_LOCAL_MEM_HI) {

			thr_data.blkidx[2] = we_ndp_blk_first(core_we,2);
			while( thr_data.blkidx[2] < we_ndp_blk_end(core_we,2) ) {
				thr_data.blkidx[1] = we_ndp_blk_first(core_we,1);
				while( thr_data.blkidx[1] < we_ndp_blk_end(core_we,1) ) {
					thr_data.blkidx[0] = we_ndp_blk_first(core_we,0);
					while( thr_data.blkidx[0] < we_ndp_blk_end(core_we,0) ) {

						for(*pi2=0,*pj2=thr_data.blkidx[2]*we_ndr_ltdsz(core_we,2);
							*pi2<we_ndr_ltdsz(core_we,2);(*pi2)++,(*pj2)++) 
						for(*pi1=0,*pj1=thr_data.blkidx[1]*we_ndr_ltdsz(core_we,1);
							*pi1<we_ndr_ltdsz(core_we,1);(*pi1)++,(*pj1)++) 
						for(*pi0=0,*pj0=thr_data.blkidx[0]*we_ndr_ltdsz(core_we,0);
							*pi0<we_ndr_ltdsz(core_we,0);(*pi0)++,(*pj0)++) {

							++e32_ctrl_run[core_local_data.corenum];
							callp(0);
							--e32_ctrl_run[core_local_data.corenum];
							++count;

						}

					++thr_data.blkidx[0];
					}
				
				++thr_data.blkidx[1];
				}

			++thr_data.blkidx[2];
			}

		} else status = -1;

		status = count;

		e32_ctrl_retval[core_local_data.corenum] = status;
      e32_ctrl_run[core_local_data.corenum] = 0;
      core_local_data.count++;
   }

	return status;
}

