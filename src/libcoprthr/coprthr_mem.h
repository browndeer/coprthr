/* coprthr_mem.h
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

#ifndef _coprthr_mem_h
#define _coprthr_mem_h


struct coprthr_mem {
	int devnum;
	void** res;
	void** resmap;
	unsigned int host_cached;
	void* cache_ptr;
};

#define __coprthr_init_memobj(imp) do { \
	(imp) = malloc(sizeof(struct coprthr_mem)); \
	(imp)->res = 0; \
	(imp)->resmap = 0; \
	} while(0)

#define __coprthr_free_memobj(imp) do { \
	__free((imp)->res); \
	__free((imp)->resmap); \
	__free((imp)); \
	} while(0)

#endif

