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

typedef struct coprthr_device* coprthr_dev_t;
typedef struct coprthr1_mem* coprthr_mem_t;
typedef struct coprthr1_program* coprthr_program_t;
typedef struct coprthr1_kernel* coprthr_kernel_t;
typedef struct coprthr_event* coprthr_event_t;

void* coprthr_devread( coprthr_dev_t dev, coprthr_mem_t dptr, void* buf,
	size_t len, int flags);
void* coprthr_devwrite( coprthr_dev_t dev, coprthr_mem_t dptr, void* buf,
	size_t len, int flags);
void* coprthr_devexec( coprthr_dev_t dev, int nthr, 
	struct coprthr1_kernel* krn1, unsigned int narg, void** args);



//int coprthr_devctl( int dd, int request, ... );

//int coprthr_wait( int dd, int flags );
//int coprthr_kill( int dd, coprthr_event_t ev, int sig, int flags );

#define COPRTHR_O_DEFAULT 0

int coprthr_dopen( const char* path, int flags);
int coprthr_dclose(int dd);

void* coprthr_compile( int dd, char* src, size_t len, char* opt, char** log );
void* coprthr_link( int dd, struct coprthr1_program* prg1, const char* kname);

void* coprthr_dmalloc( int dd, size_t size, int flags );
void* coprthr_drealloc( int dd, void* dptr, size_t size, int flags );
void coprthr_dfree( int dd, void* dptr );
void* coprthr_dmwrite(int dd, void* dptr, void* ptr, size_t len, int flags);
void* coprthr_dmread(int dd, void* dptr, void* ptr, size_t len, int flags);

void coprthr_dwaitev( int dd, struct coprthr_event* ev );

void* coprthr_dexec( int dd, unsigned int nthr, struct coprthr1_kernel* krn, 
	unsigned int nargs, void** p_args);

//coprthr_kernel_t coprthr1_ksym( int dd, coprthr_program_t prg );

//int coprthr_sched( int dd, coprthr_event_t ev, int action, ... );

#define COPRTHR_DEVMEM_TYPE_BUFFER		0x0001
#define COPRTHR_DEVMEM_TYPE_MUTEX		0x0002
#define COPRTHR_DEVMEM_TYPE_SIGNAL		0x0004
#define COPRTHR_DEVMEM_TYPE_REGISTER	0x0008
#define COPRTHR_DEVMEM_TYPE_FIFO			0x0010
#define COPRTHR_DEVMEM_TYPE_STACK		0x0020

#define COPRTHR_DEVMEM_PROT_ENABLED		0x0800
#define COPRTHR_DEVMEM_PROT_READ			(0x0100|COPRTHR_DEVMEM_PROT_ENABLED)
#define COPRTHR_DEVMEM_PROT_WRITE		(0x0200|COPRTHR_DEVMEM_PROT_ENABLED)

#define COPRTHR_DEVMEM_TYPEMASK			0x0ff
#define COPRTHR_DEVMEM_PROTMASK			0xf00

#define COPRTHR_DEVMEM_FIXED				0x1000

void* coprthr_devmemalloc( coprthr_dev_t dev, void* addr, size_t nmemb, 
	size_t size, int flags );

void coprthr_devmemfree( struct coprthr_device* dev, struct coprthr1_mem* mem );

#define COPRTHR_DEVLOCK_NOWAIT		0x1
#define COPRTHR_DEVLOCK_NOINIT		0x2
#define COPRTHR_DEVUNLOCK_PERSIST	0x4

int coprthr_devlock( coprthr_dev_t dev, int flags );
int coprthr_devunlock( coprthr_dev_t dev, int flags );

void* coprthr_getdev( const char* path, int flags );

void* coprthr_devcompile( struct coprthr_device* dev, char* src, size_t len, char* opt, char** log );
void* coprthr_devlink( struct coprthr_device* dev, struct coprthr1_program* prg1, const char* kname);

#endif

