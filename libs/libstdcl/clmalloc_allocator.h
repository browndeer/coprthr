/* clmalloc_allocator.h
 */

// Allocator that wraps "C" malloc -*- C++ -*-

// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
// Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file ext/malloc_allocator.h
 *  This file is a GNU extension to the Standard C++ Library.
 */

/* DAR */

#ifndef _clmalloc_allocator_h
#define _clmalloc_allocator_h

#include <cstdlib>
#include <cstdio>
//#include <new>
//#include <bits/functexcept.h>
//#include <bits/move.h>

//#include "stdcl.h"
#include "stdcl.h"

using std::size_t;
using std::ptrdiff_t;

template<typename T>
class clmalloc_allocator
{
	public:

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

	template<typename T1>
	struct rebind
	{ typedef clmalloc_allocator<T1> other; };

	clmalloc_allocator() throw() { }

	clmalloc_allocator(const clmalloc_allocator&)  throw() { }

	template<typename T1>
	clmalloc_allocator(const clmalloc_allocator<T1>&) throw() { }

	~clmalloc_allocator() throw() { }

	pointer
	address(reference x) const { return &x; }

	const_pointer
	address(const_reference x) const { return &x; }

	pointer
	allocate(size_type n, const void* p = 0)
	{
//		printf("clmalloc_allocator::allocate(%d,%p)\n",n,p);

		if (__builtin_expect(n > this->max_size(), false)) {
			fprintf(stderr,"clmalloc_allocator::error 5\n"); 
			fflush(stderr);
			exit(-1);
		}

//		pointer ret = static_cast<T*>(std::malloc(n * sizeof(T)));
		pointer ret = static_cast<T*>(::clmalloc(0,n * sizeof(T), CL_MEM_DETACHED));

		if (!ret) {
			fprintf(stderr,"clmalloc_allocator::error 6\n"); 
			fflush(stderr);
			exit(-1);
		}

		return ret;
	}

	void
	deallocate(pointer p, size_type)
	{ 
//		printf("clmalloc_allocator::deallocate(%p)\n",p);

//		std::free(static_cast<void*>(p)); 
		clfree(static_cast<void*>(p)); 
	}

	size_type
	max_size() const throw() 
	{ return size_t(-1) / sizeof(T); }

	void 
	construct(pointer p, const T& val) 
	{ 
//		printf("clmalloc_allocator::construct %p\n",(void*)p);

		::new((void *)p) value_type(val); 
	}

	void 
	destroy(pointer p) 
	{ p->~T(); }

};

template<typename T>
inline bool
operator==(const clmalloc_allocator<T>&, const clmalloc_allocator<T>&)
{ return true; }
  
template<typename T>
inline bool
operator!=(const clmalloc_allocator<T>&, const clmalloc_allocator<T>&)
{ return false; }

#endif

