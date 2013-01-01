/* e32_config_blank.h
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


#ifndef _E32_CONFIG_BLANK_H
#define _E32_CONFIG_BLANK_H

/*** 
 *** IMPORTANT *** This header contains information about the e32 device.
 *** IMPORTANT *** This infortmation MUST be synchronized manually to match
 *** IMPORTANT ***   the target device and Epiphany device headers. -DAR
 ***/

#define E32_COLS_IN_CHIP 3
#define E32_ROWS_IN_CHIP 3

#define E32_NCORES (E32_COLS_IN_CHIP*E32_ROWS_IN_CHIP)


#if defined(__coprthr_host__)

#include "printcl.h"
#include "e_host.h"

#define xxx_e_read_zeropage( src, dst, len) do { \
	printcl( CL_DEBUG "xxx_e_read_zeropage %p - %x",src,E32_DRAM_ZEROPAGE); \
	e_mread_buf( &e_dram, (src), dst, len); \
	} while(0)

#define xxx_e_write_zeropage( dst, src, len) do { \
	printcl( CL_DEBUG "xxx_e_write_zeropage %p - %x",dst,E32_DRAM_ZEROPAGE); \
	e_mwrite_buf( &e_dram, (dst), src, len); \
	} while(0)

#elif defined(__coprthr_device__)

#else
#error must be compiled with either __coprthr_host__ or __coprthr_device__
#endif



#endif

