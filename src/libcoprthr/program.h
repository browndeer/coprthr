/* program.h
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include "coprthr_program.h"

void __do_release_program_1(struct coprthr1_program* prg1);

int __do_check_compiler_available(cl_device_id devid);

unsigned int __do_build_program_from_binary_1( struct coprthr1_program* prg1 );

int bind_ksyms_default( struct _coprthr_ksyms_struct* ksyms, void* h, 
	char* kname );

#endif


