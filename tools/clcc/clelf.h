/* clelf.h
 *
 * Copyright (c) 2008-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _CLELF_H
#define _CLELF_H

#define CLCC_TEST

/* DAR */

#define CL_CONTEXT_OFFLINE_DEVICES_AMD 0x403F

#define CLELF_PLATFORM_CODE_AMDAPP	1
#define CLELF_PLATFORM_CODE_NVIDIA	2
#define CLELF_PLATFORM_CODE_COPRTHR	3
#define CLELF_PLATFORM_CODE_INTEL	4

#define DEFAULT_STR_SIZE			1024
#define DEFAULT_BUF_ALLOC			1048576
#define DEFAULT_CLPRGTAB_NALLOC	16
#define DEFAULT_CLKRNTAB_NALLOC	16
#define DEFAULT_CLPRGS_NALLOC		16
#define DEFAULT_CLPRGB_NALLOC		128


#if defined(__x86_64__)

struct clprgtab_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Half e_prgs;
	Elf64_Half e_nprgs;
	Elf64_Half e_prgb;
	Elf64_Half e_nprgb;
	Elf64_Half e_krn;
	Elf64_Half e_nkrn;
};

struct clkrntab_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Half e_prg;
};

struct clprgs_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Word e_platform;
	Elf64_Word e_device;
	Elf64_Half e_shndx;
	Elf64_Addr	e_offset;
	Elf64_Xword e_size;
};

struct clprgb_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Word e_platform;
	Elf64_Word e_device;
	Elf64_Half e_shndx;
	Elf64_Addr	e_offset;
	Elf64_Xword e_size;
};

#elif defined(__i386__)

struct clprgtab_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_prgs;
	Elf32_Half e_nprgs;
	Elf32_Half e_prgb;
	Elf32_Half e_nprgb;
};

struct clprgs_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_platform;
	Elf32_Half e_device;
	Elf32_Half e_shndx;
	Elf32_Addr	e_offset;
	Elf32_Xword e_size;
};

struct clprgb_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_platform;
	Elf32_Half e_device;
	Elf32_Half e_shndx;
	Elf32_Addr	e_offset;
	Elf32_Xword e_size;
};

#endif

#define __clprgtab_entry_sz sizeof(struct clprgtab_entry)
#define __clkrntab_entry_sz sizeof(struct clkrntab_entry)
#define __clprgs_entry_sz sizeof(struct clprgs_entry)
#define __clprgb_entry_sz sizeof(struct clprgb_entry)


#define LANG_OPENCL  1
#define LANG_STDCL   2
#define LANG_CUDA    3

#endif

