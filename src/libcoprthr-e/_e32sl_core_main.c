
#include <string.h>
#include <setjmp.h>
#include <e_coreid.h>
#include <e_common.h>

#include "e32_config.h"
#include "e32sl_mem_if.h"
#include "e32sl_if.h"

#define __callsp(sp,pf,argp) __asm volatile ( \
   "mov r31,sp\n\t"			      \
   "mov sp,%1\n\t"			      \
	"str r31,[sp]\n\t" \
   "mov r0,%0\n\t"  \
   "jalr %2\n"   \
	"ldr r31,[sp]\n\t" \
	"mov sp,r31\n\t" \
   : : "r" (argp), "r" (sp), "r" (pf)  \
   )

typedef void (*callp_t)(void*);

volatile struct core_local_data_struct core_local_data SECTION("section_core");
volatile e32_workp_entry_t core_we SECTION("section_core");
volatile struct thr_data_struct thr_data SECTION("section_core");
int _local_mem_base SECTION("section_core"); /* just a marker, no used */

__inline 
static unsigned core_num(e_coreid_t coreid)
{
   unsigned row, col;

   row = (coreid >> 6) & (E_ROWS_IN_CHIP - 1);
   col =  coreid       & (E_COLS_IN_CHIP - 1);
   return row * E_COLS_IN_CHIP + col;
}


jmp_buf main_jbuf;


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

//		if (callp && callp < (callp_t)0x7000) {
		if (callp && callp < (callp_t)E32_CORE_LOCAL_MEM_HI) {

			thr_data.blkidx[2] = we_ndp_blk_first(core_we,2);
			while( thr_data.blkidx[2] < we_ndp_blk_end(core_we,2) ) {
				thr_data.blkidx[1] = we_ndp_blk_first(core_we,1);
				while( thr_data.blkidx[1] < we_ndp_blk_end(core_we,1) ) {
					thr_data.blkidx[0] = we_ndp_blk_first(core_we,0);
					while( thr_data.blkidx[0] < we_ndp_blk_end(core_we,0) ) {

						int gtdoff0 = thr_data.blkidx[0]*we_ndr_ltdsz(core_we,0);
						int gtdoff1 = thr_data.blkidx[1]*we_ndr_ltdsz(core_we,1);
						int gtdoff2 = thr_data.blkidx[2]*we_ndr_ltdsz(core_we,2);

						jmp_buf thr_jbuf[16];

						void* stackp = (void*)THR_STACK_MEMHI;

						int i0,i1,i2;
						int j0,j1,j2;
						int nthr = 0;
						for(i2=0,j2=gtdoff2;i2<we_ndr_ltdsz(core_we,2);i2++,j2++)
						for(i1=0,j1=gtdoff1;i1<we_ndr_ltdsz(core_we,1);i1++,j1++)
						for(i0=0,j0=gtdoff0;i0<we_ndr_ltdsz(core_we,0);i0++,j0++) {
							struct thr_data_struct* pthr_data 
								= (struct thr_data_struct*)(stackp - THR_STACK_SZ);
							pthr_data->gtdidx[0] = j0;
							pthr_data->gtdidx[1] = j1;
							pthr_data->gtdidx[2] = j2;
							pthr_data->ltdidx[0] = i0;
							pthr_data->ltdidx[1] = i1;
							pthr_data->ltdidx[2] = i2;
							++nthr;
							stackp -= THR_STACK_SZ;
						}

						stackp = (void*)THR_STACK_MEMHI;

						int i;
						for(i=0;i<nthr;i++) {
							struct thr_data_struct* pthr_data 
								= (struct thr_data_struct*)(stackp - THR_STACK_SZ);
							pthr_data->this_jbufp = thr_jbuf + i;
							pthr_data->next_jbufp = thr_jbuf + (i+1)%nthr;
							stackp -= THR_STACK_SZ;
						}

						stackp = (void*)THR_STACK_MEMHI;

						++e32_ctrl_run[core_local_data.corenum];

						for(i=0;i<nthr;i++) {

							if (!setjmp(main_jbuf)) {

								++count;

								++e32_ctrl_run[core_local_data.corenum];

								__callsp(stackp-16,callp,0);

							}

							stackp -= THR_STACK_SZ;

						}

						for(i=0;i<nthr;i++) {
							if(!setjmp(main_jbuf)) longjmp(thr_jbuf[i],i+1);
							--e32_ctrl_run[core_local_data.corenum];
						}

						--e32_ctrl_run[core_local_data.corenum];

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

