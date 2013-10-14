/* coprthr.h
 *
 * Copyright (c) 2013 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _coprthr_h
#define _coprthr_h

#include "coprthr_device.h"
#include "coprthr_mem.h"
#include "coprthr_program.h"
#include "coprthr_sched.h"

//int coprthr_devctl( int dd, int request, ... );

//int coprthr_wait( int dd, int flags );
//int coprthr_kill( int dd, coprthr_event_t ev, int sig, int flags );

int coprthr_dopen( const char* path, int flags);
void coprthr_dclose(int dd);

void* coprthr_compile( int dd, char* src, size_t len, char* opt, char** log );
void* coprthr_link( int dd, struct coprthr1_program* prg1, const char* kname);

void* coprthr_dmalloc( int dd, size_t size, int flags );
void* coprthr_drealloc( int dd, void* dptr, size_t size, int flags );
void* coprthr_dfree( int dd, void* dptr, int flags );
void* coprthr_dmwrite(int dd, void* dptr, void* ptr, size_t len, int flags);
void* coprthr_dmread(int dd, void* dptr, void* ptr, size_t len, int flags);

void coprthr_dwaitev( struct coprthr_event* ev );

void* coprthr_dexec( int dd, unsigned int nthr, struct coprthr1_kernel* krn, 
	unsigned int nargs, void** p_args);

//coprthr_kernel_t coprthr1_ksym( int dd, coprthr_program_t prg );

//int coprthr_sched( int dd, coprthr_event_t ev, int action, ... );

#endif

