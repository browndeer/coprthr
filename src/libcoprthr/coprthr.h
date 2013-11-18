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

#include "coprthr_arch.h"
#include "coprthr_device.h"
#include "coprthr_mem.h"
#include "coprthr_program.h"
#include "coprthr_sched.h"


/***
 *** typdefs for opaque types
 ***/

typedef struct coprthr_device* coprthr_dev_t;
typedef struct coprthr1_mem* coprthr_mem_t;
typedef struct coprthr1_program* coprthr_program_t;
typedef struct coprthr1_kernel* coprthr_kernel_t;
typedef struct coprthr_event* coprthr_event_t;


/***
 *** coprthr_dev API (low)
 ***/

#define COPRTHR_DEVMEM_TYPE_BUFFER		0x0001
#define COPRTHR_DEVMEM_TYPE_MUTEX		0x0002
#define COPRTHR_DEVMEM_TYPE_SIGNAL		0x0004
#define COPRTHR_DEVMEM_TYPE_REGISTER		0x0008
#define COPRTHR_DEVMEM_TYPE_FIFO		0x0010
#define COPRTHR_DEVMEM_TYPE_STACK		0x0020

#define COPRTHR_DEVMEM_PROT_ENABLED		0x0800
#define COPRTHR_DEVMEM_PROT_READ		(0x0100|COPRTHR_DEVMEM_PROT_ENABLED)
#define COPRTHR_DEVMEM_PROT_WRITE		(0x0200|COPRTHR_DEVMEM_PROT_ENABLED)

#define COPRTHR_DEVMEM_TYPEMASK			0x0ff
#define COPRTHR_DEVMEM_PROTMASK			0xf00

#define COPRTHR_DEVMEM_FIXED			0x1000

void* coprthr_devmemalloc( coprthr_dev_t dev, void* addr, size_t nmemb, 
	size_t size, int flags );

void coprthr_devmemfree( struct coprthr_device* dev, struct coprthr1_mem* mem );

size_t coprthr_devmemsize( struct coprthr1_mem* mem );
void* coprthr_devmemptr( struct coprthr1_mem* mem );

void* coprthr_devread( coprthr_dev_t dev, coprthr_mem_t dptr, void* buf,
	size_t len, int flags);
void* coprthr_devwrite( coprthr_dev_t dev, coprthr_mem_t dptr, void* buf,
	size_t len, int flags);
void* coprthr_devcopy( coprthr_dev_t dev, coprthr_mem_t dptr_src,
	coprthr_mem_t dptr_dst, size_t len, int flags);
void* coprthr_devexec( coprthr_dev_t dev, int nthr, 
	struct coprthr1_kernel* krn1, unsigned int narg, void** args);

//int coprthr_devctl( int dd, int request, ... );

#define COPRTHR_DEVLOCK_NOWAIT		0x1
#define COPRTHR_DEVLOCK_NOINIT		0x2
#define COPRTHR_DEVUNLOCK_PERSIST	0x4

int coprthr_devlock( coprthr_dev_t dev, int flags );
int coprthr_devunlock( coprthr_dev_t dev, int flags );

void* coprthr_getdev( const char* path, int flags );

void* coprthr_devcompile( struct coprthr_device* dev, char* src, size_t len, char* opt, char** log );
void* coprthr_devlink( struct coprthr_device* dev, struct coprthr1_program* prg1, const char* kname);


/***
 *** coprthr API (direct)
 ***/

#define COPRTHR_DEVICE_X86_64 	((char*)COPRTHR_ARCH_ID_X86_64)
#define COPRTHR_DEVICE_I386 	((char*)COPRTHR_ARCH_ID_I386)
#define COPRTHR_DEVICE_ARM32 	((char*)COPRTHR_ARCH_ID_ARM32)
#define COPRTHR_DEVICE_E32_EMEK	((char*)COPRTHR_ARCH_ID_E32_EMEK)
#define COPRTHR_DEVICE_E32 	((char*)COPRTHR_ARCH_ID_E32)

#define COPRTHR_O_NONBLOCK	0x0001
#define COPRTHR_O_STREAM 	0x0010
#define COPRTHR_O_THREAD 	0x0020
#define COPRTHR_O_EXCLUSIVE 	0x0040

#define COPRTHR_O_DEFAULT COPRTHR_O_STREAM

int coprthr_dopen( const char* path, int flags);
int coprthr_dclose(int dd);

void* coprthr_compile( int dd, char* src, size_t len, char* opt, char** log );
void* coprthr_link( int dd, struct coprthr1_program* prg1, const char* kname);

//#define COPRTHR_E_WAIT 		0x0001
//#define COPRTHR_E_NOWAIT 	0x0002
//#define COPRTHR_E_NOW 		0x0004

#define COPRTHR_E_DEFAULT COPRTHR_E_NOWAIT

void* coprthr_dmalloc( int dd, size_t size, int flags );
void* coprthr_drealloc( int dd, void* dptr, size_t size, int flags );
void coprthr_dfree( int dd, void* dptr );
void* coprthr_dwrite(int dd, void* dptr, void* ptr, size_t len, int flags);
void* coprthr_dread(int dd, void* dptr, void* ptr, size_t len, int flags);
void* coprthr_dcopy(int dd, void* dptr_src, void* dptr_dst, size_t len, 
	int flags);

void coprthr_dwaitev( int dd, struct coprthr_event* ev );

coprthr_event_t coprthr_dexec( int dd, 
	coprthr_kernel_t krn, unsigned int nargs, void** args,
	unsigned int nthr, int flags );

coprthr_event_t coprthr_dnexec( int dd, unsigned int nkrn, 
	coprthr_kernel_t* v_krn, unsigned int* v_nargs, void*** v_args,
	unsigned int* v_nthr, int flags );

//int coprthr_kill( int dd, coprthr_event_t ev, int sig, int flags );

#define COPRTHR_S_EXECUTE	0x0001
#define COPRTHR_S_YIELD		0x0002
#define COPRTHR_S_SUSPEND	0x0004
#define COPRTHR_S_EXIT		0x0008

#define COPRTHR_S_PREEMPT	0x0100
#define COPRTHR_S_NOPREEMPT	0x0200
#define COPRTHR_S_CLEANUP	0x0400

//int coprthr_sched( int dd, coprthr_event_t ev, int action, ... );

#define COPRTHR_CTL_TESTSUPP	10

#endif

