/* e32ser_mem_if.h
 *
 * Copyright (c) 2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _E32SER_MEM_IF_H
#define _E32SER_MEM_IF_H

#include <sys/types.h>
#include <stdint.h>

#include "e32_config.h"

#if defined(__x86_64__)
#define __host__
#endif

#if !defined(__host__)
#include <e_coreid.h>
#include <e_common.h>
#endif

//#define NCORES 16
#define E32_INT_SZ 4
#define E32_PTR_SZ 4

#define E32_SZ_CTRL_READY (E32_NCORES * E32_INT_SZ)
#define E32_SZ_CTRL_RUN (E32_NCORES * E32_INT_SZ)
#define E32_SZ_CTRL_RETVAL (E32_NCORES * E32_INT_SZ)
#define E32_SZ_CTRL_CALLP (E32_NCORES * E32_PTR_SZ)
#define E32_SZ_CTRL ( \
	E32_SZ_CTRL_READY \
	+E32_SZ_CTRL_RUN \
	+E32_SZ_CTRL_RETVAL \
	+E32_SZ_CTRL_CALLP )

#define E32_SZ_WE (19 * E32_INT_SZ)

#define E32_SZ_KDATA_FUNCP E32_PTR_SZ
#define E32_SZ_KDATA_NARG E32_INT_SZ
#define E32_SZ_KDATA_PTR_ARG_OFF E32_PTR_SZ
#define E32_SZ_KDATA_PTR_ARG_BUF E32_PTR_SZ
#define E32_SZ_KDATA ( \
	E32_SZ_KDATA_FUNCP \
	+ E32_SZ_KDATA_NARG \
	+ E32_SZ_KDATA_PTR_ARG_OFF \
	+ E32_SZ_KDATA_PTR_ARG_BUF )

/* XXX hardcoded for now, later use a dummy struct placed in .shared_dram */
//#define E32_DRAM_BASE 0x81000000

#define E32_ADDR_CTRL E32_DRAM_BASE
#define E32_ADDR_CTRL_READY (E32_ADDR_CTRL)
#define E32_ADDR_CTRL_RUN (E32_ADDR_CTRL_READY + E32_SZ_CTRL_READY)
#define E32_ADDR_CTRL_RETVAL (E32_ADDR_CTRL_RUN + E32_SZ_CTRL_RUN)
#define E32_ADDR_CTRL_CALLP (E32_ADDR_CTRL_RETVAL + E32_SZ_CTRL_RETVAL)

#define E32_ADDR_WE (E32_ADDR_CTRL + E32_SZ_CTRL)

#define E32_ADDR_KDATA (E32_ADDR_WE + E32_NCORES * E32_SZ_WE)
#define E32_ADDR_KDATA_FUNCP E32_ADDR_KDATA
#define E32_ADDR_KDATA_NARG (E32_ADDR_KDATA_FUNCP + E32_SZ_KDATA_FUNCP)
#define E32_ADDR_KDATA_PTR_ARG_OFF (E32_ADDR_KDATA_NARG + E32_SZ_KDATA_NARG)
#define E32_ADDR_KDATA_PTR_ARG_BUF \
	(E32_ADDR_KDATA_PTR_ARG_OFF + E32_SZ_KDATA_PTR_ARG_OFF)

#define E32_ZERO_PAGE_FREE (E32_ADDR_KDATA + E32_SZ_KDATA)

#if defined(__host__)
typedef int32_t e32_int_t;
typedef uint32_t e32_uint_t;
typedef uint32_t e32_ptr_t;
#else
typedef int e32_int_t;
typedef unsigned int e32_uint_t;
typedef void* e32_ptr_t;
#endif

typedef e32_uint_t e32_workp_entry_t[19];


__inline static unsigned int ncores() { return E32_NCORES; }


#if defined(__host__)

#define  __SCALAR_BUILTINS(name,NAME,elem_t,elem_sz) \
__inline static void e32_read_##name( elem_t* pval ) \
{ 	e_read(E32_ADDR_##NAME, (void*)pval, elem_sz ); } \
__inline static void e32_write_##name( elem_t val ) \
{ 	elem_t tmp = val;  \
	e_write(E32_ADDR_##NAME, (void*)&tmp, elem_sz ); } \

#define  __ARRAY_BUILTINS(name,NAME,elem_t,elem_sz,nlim) \
__inline static void e32_read_##name( elem_t* pval ) \
{ e_read(E32_ADDR_##NAME, (void*)pval, elem_sz*nlim ); } \
__inline static void e32_write_##name( elem_t* pval ) \
{ e_write(E32_ADDR_##NAME, (void*)pval, elem_sz*nlim ); } \
__inline static void e32_read_##name##_n( elem_t* pval, unsigned int n ) \
{ 	if (n < nlim) e_read(E32_ADDR_##NAME + n*elem_sz, (void*)pval, elem_sz ); }\
__inline static void e32_write_##name##_n( elem_t val, unsigned int n ) \
{ 	elem_t tmp = val;  \
	if (n < nlim) e_write(E32_ADDR_##NAME + n*elem_sz, (void*)&tmp, elem_sz ); }

__ARRAY_BUILTINS(ctrl_ready,CTRL_READY,e32_int_t,E32_INT_SZ,ncores())
__ARRAY_BUILTINS(ctrl_run,CTRL_RUN,e32_int_t,E32_INT_SZ,ncores())
__ARRAY_BUILTINS(ctrl_retval,CTRL_RETVAL,e32_int_t,E32_INT_SZ,ncores())
__ARRAY_BUILTINS(ctrl_callp,CTRL_CALLP,e32_ptr_t,E32_PTR_SZ,ncores())

/* XXX gcc complained about this but make sure it was not wrong -DAR */
__ARRAY_BUILTINS(we,WE,e32_uint_t,sizeof(e32_workp_entry_t),ncores())

__SCALAR_BUILTINS(kdata_funcp,KDATA_FUNCP,e32_ptr_t,E32_PTR_SZ)
__SCALAR_BUILTINS(kdata_narg,KDATA_NARG,e32_int_t,E32_INT_SZ)
__SCALAR_BUILTINS(kdata_ptr_arg_off,KDATA_PTR_ARG_OFF,e32_ptr_t,E32_PTR_SZ)
__SCALAR_BUILTINS(kdata_ptr_arg_buf,KDATA_PTR_ARG_BUF,e32_ptr_t,E32_PTR_SZ)

#define e32_init_we(symbol,we,size) do { \
	int i; \
	for(i=0;i<size;i++) { \
		symbol[i][0] = we[i].ndr_dim; \
		memcpy(&symbol[i][1],we[i].ndr_gtdoff,3*sizeof(e32_uint_t)); \
		memcpy(&symbol[i][4],we[i].ndr_gtdsz,3*sizeof(e32_uint_t)); \
		memcpy(&symbol[i][7],we[i].ndr_ltdsz,3*sizeof(e32_uint_t)); \
		memcpy(&symbol[i][10],we[i].ndp_blk_first,3*sizeof(e32_uint_t)); \
		memcpy(&symbol[i][13],we[i].ndp_blk_end,3*sizeof(e32_uint_t)); \
		memcpy(&symbol[i][16],we[i].ndp_ltd0,3*sizeof(e32_uint_t)); \
		switch (we[i].ndr_dim) { \
			case 1: symbol[i][8] = 1; symbol[i][11] = 0; symbol[i][14] = 1; \
			case 2: symbol[i][9] = 1; symbol[i][12] = 0; symbol[i][15] = 1; \
			default: break; \
		} \
	} } while(0)

#else

#define __SCALAR(NAME,elem_t) (*(elem_t*)E32_ADDR_##NAME)
#define __ARRAY(NAME,elem_t) ((elem_t*)E32_ADDR_##NAME)

#define e32_ctrl_ready 	__ARRAY(CTRL_READY,e32_int_t)
#define e32_ctrl_run 	__ARRAY(CTRL_RUN,e32_int_t)
#define e32_ctrl_retval	__ARRAY(CTRL_RETVAL,e32_int_t)
#define e32_ctrl_callp 	__ARRAY(CTRL_CALLP,e32_ptr_t)

#define e32_we 					__ARRAY(WE,e32_workp_entry_t)

#define e32_kdata_funcp 		__SCALAR(KDATA_FUNCP,e32_ptr_t)
#define e32_kdata_narg 			__SCALAR(KDATA_NARG,e32_int_t)
#define e32_kdata_ptr_arg_off	__SCALAR(KDATA_PTR_ARG_OFF,e32_int_t*)
#define e32_kdata_ptr_arg_buf	__SCALAR(KDATA_PTR_ARG_BUF,e32_ptr_t)

#define we_ndr_dim(we) 				(we[0])
#define we_ndr_gtdoff(we,d) 		(we[1+d])
#define we_ndr_gtdsz(we,d) 		(we[4+d])
#define we_ndr_ltdsz(we,d) 		(we[7+d])
#define we_ndp_blk_first(we,d)	(we[10+d])
#define we_ndp_blk_end(we,d) 		(we[13+d])
#define we_ndp_ltd0(we,d) 			(we[16+d])

#define e32_inc_ctrl_run(n) \
	do { ++e32_ctrl_run[core_local_data.corenum] } while(0)
#define e32_dec_ctrl_run(n) \
	do { --e32_ctrl_run[core_local_data.corenum] } while(0)


#endif

#endif

