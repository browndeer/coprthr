/* memobj.h
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

#ifndef _memobj_h
#define _memobj_h

void* __coprthr_memalloc( size_t sz, int flags );
void* __coprthr_memrealloc( void* ptr, size_t sz, int flags);
void __coprthr_memfree( void* memptr, int flags );
size_t __coprthr_memread( void* memptr, void* buf, size_t sz );
size_t __coprthr_memwrite( void* memptr, void* buf, size_t sz );
size_t __coprthr_memcopy( void* memptr_src, void* memptr_dst, size_t sz);


#endif

