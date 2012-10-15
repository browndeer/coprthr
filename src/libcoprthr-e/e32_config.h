/* e32_config.h
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


#ifndef _E32_CONFIG_H
#define _E32_CONFIG_H

/*** 
 *** IMPORTANT *** This header contains information about the e32 device.
 *** IMPORTANT *** This infortmation MUST be synchronized manually to match
 *** IMPORTANT ***   the target device and Epiphany device headers. -DAR
 ***/


/* define physical core array and number of cores */
#define E32_COL_FIRST		36
#define E32_COL_END			40
#define E32_ROW_FIRST 		32
#define E32_ROW_END			36
#define E32_COLS_IN_CHIP (E32_COL_END - E32_COL_FIRST)
#define E32_ROWS_IN_CHIP (E32_ROW_END - E32_ROW_FIRST)
#define E32_NCORES \
	((E32_ROW_END-E32_ROW_FIRST)*(E32_COL_END-E32_COL_FIRST))


#define E32_DRAM_BASE 			0x81000000
#define E32_GLOBAL_MEM_BASE 	(E32_DRAM_BASE + 0x4000)
#define E32_GLOBAL_MEM_HI		0x82000000 /* XXX is this correct? -DAR */


#define E32_CORE_LOCAL_MEM_HI	0x8000

#define E32_CORE0_LOCAL_MEM_BASE	0x82400000


#endif


