/* clinit.h
 *
 * Copyright (c) 2009-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _CLINIT_H
#define _CLINIT_H

#ifndef __STDCL__
#error Do not include clinit.h directly, include stdcl.h instead.
#endif

#include <string.h>
#include <stdio.h>

#ifdef _WIN64
#include "fix_windows.h"
#else
#define LIBSTDCL_API 
#include <sys/queue.h>
#include <elf.h>
#endif


#include <CL/cl.h>

//#include "../libclelf/clelf.h"
#include "clelf.h"

#include "clcontext.h"


//struct _proc_cl_struct {
//
//	unsigned int clprgs_n;
//   struct clprgs_entry* clprgs;
//
//	size_t cltexts_sz;
//   char* cltexts;
//
//	unsigned int clprgb_n;
//   struct clprgb_entry* clprgb;
//
//	size_t cltextb_sz;
//   char* cltextb;
//
//	size_t clstrtab_sz;
//   char* clstrtab;
//
//};

//#if defined(__x86_64__)
//
//struct clprgs_entry {
//   Elf64_Word e_name;
//   Elf64_Word e_info;
//   Elf64_Half e_shndx;
//   Elf64_Addr  e_offset;
//   Elf64_Xword e_size;
//};
//
//struct clprgb_entry {
//   Elf64_Word e_name;
//   Elf64_Word e_info;
//   Elf64_Half e_shndx;
//   Elf64_Addr  e_offset;
//   Elf64_Xword e_size;
//};
//
//#elif defined(__i386__)
//
//struct clprgs_entry {
//   Elf32_Word e_name;
//   Elf32_Word e_info;
//   Elf32_Half e_shndx;
//   Elf32_Addr  e_offset;
//   Elf32_Xword e_size;
//};
//
//struct clprgb_entry {
//   Elf32_Word e_name;
//   Elf32_Word e_info;
//   Elf32_Half e_shndx;
//   Elf32_Addr  e_offset;
//   Elf32_Xword e_size;
//};
//
//#endif
//
//#define __clprgs_entry_sz sizeof(struct clprgs_entry)
//#define __clprgb_entry_sz sizeof(struct clprgb_entry)


//extern struct _proc_cl_struct _proc_cl;
extern struct clelf_sect_struct* _proc_clelf_sect;


extern LIBSTDCL_API CONTEXT* stddev;
extern LIBSTDCL_API CONTEXT* stdcpu;
extern LIBSTDCL_API CONTEXT* stdgpu;
extern LIBSTDCL_API CONTEXT* stdrpu;

extern char* __log_automatic_kernels_filename;

#ifdef __cplusplus
extern "C" {
#endif

extern void _assert_proto_stub(void);
//LIBSTDCL_API void _libstdcl_init();
#ifdef _WIN64
LIBSTDCL_API void _libstdcl_init();
#define stdcl_init() _libstdcl_init()
#else 
#define stdcl_init()
#endif


#ifdef __cplusplus
}
#endif


#endif

