/* compiler.h
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


#ifndef _COMPILER_H
#define _COMPILER_H

#include <stddef.h>
#include "xcl_structs.h"

//typedef int(*compiler_t)(cl_device_id,char*,size_t,char*,size_t,char*,char*);
typedef int(*compiler_t)(cl_device_id,char*,size_t,char**,size_t*,char*,char**);
//typedef int(*ircompiler_t)(cl_device_id,char*,size_t,char*,size_t,char*,char*);


int compile_x86_64(
	cl_device_id devid,
   unsigned char* src, size_t src_sz,
   unsigned char** p_bin, size_t* p_bin_sz,
   char* opt, char** p_log
);

int compile_arm(
	cl_device_id devid,
   unsigned char* src, size_t src_sz,
   unsigned char** p_bin, size_t* p_bin_sz,
   char* opt, char** p_log
);

int compile_e32(
	cl_device_id devid,
   unsigned char* src, size_t src_sz,
   unsigned char** p_bin, size_t* p_bin_sz,
   char* opt, char** p_log
);

int compile_e32_needham( cl_device_id devid, unsigned char* src, 
	size_t src_sz, unsigned char** p_bin, size_t* p_bin_sz, char* opt, 
	char** p_log);

int compile_e32_needhampro( cl_device_id devid, unsigned char* src, 
	size_t src_sz, unsigned char** p_bin, size_t* p_bin_sz, char* opt, 
	char** p_log);

int compile_e32_blank( cl_device_id devid, unsigned char* src, 
	size_t src_sz, unsigned char** p_bin, size_t* p_bin_sz, char* opt, 
	char** p_log);


#endif


