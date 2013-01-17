/* program.h
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

#ifndef _program_h
#define _program_h

#include "xcl_structs.h"

void __do_create_program(cl_program prg);

void __do_release_program(cl_program prg);

int __do_check_compiler_available(cl_device_id devid);

void __do_compile_and_link(cl_program prg, cl_device_id devid, int devnum);

cl_int __do_build_program_from_source(
	cl_program prg, cl_device_id devid, cl_uint devnum);

cl_int __do_build_program_from_binary(
	cl_program prg, cl_device_id devid, cl_uint devnum);

int __do_find_kernel_in_program( cl_program, const char* );

int bind_ksyms_default( struct _imp_ksyms_struct* ksyms, void* h, char* kname );

#endif


