/* clarray.h
 *
 * Copyright (c) 2010-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef __CLARRAY_H
#define __CLARRAY_H

//#include <string.h>
#include <stdio.h>
//#include <sys/queue.h>

#include <string>

#include <CL/cl.h>
#include <stdcl.h>
#include <clmalloc_allocator.h>

#include <vector>
//#include <clvector.h>

template < class T, class A > class clvector;

#include "CLETE/Interval.h"
//#include "CLETE/Wrapper.h"

#ifdef __cplusplus

#define __stdclpp__

//namespace stdclpp {

//struct Interval {
//	int first, end, shift;
//};


/***
 *** clarray
 ***/

template<class T>
class Expression;

/* address the difference in implementations -DAR */
#ifdef _WIN64
#define _clarray_ptr (this->_Myfirst)
#else
#define _clarray_ptr (this->_M_impl._M_start)
#endif

//#ifndef _WIN64
//#include "CLETE/clarray_CLETE.h"
//////////////////#endif

template < typename T, typename A = clmalloc_allocator<T> >
class clarray : public std::vector< T, clmalloc_allocator<T> >
{
	typedef clmalloc_allocator<T> allocator_t;

	public:

		clarray()
			: std::vector< T, clmalloc_allocator<T> >() {}

		clarray( size_t n, const T& value = T() )
			: std::vector< T, clmalloc_allocator<T> >( n, value ) {}

		template < class InputIterator >
		clarray( InputIterator first, InputIterator last )
			: std::vector< T, clmalloc_allocator<T> >( first, last ) {}

		void clmattach( CONTEXT* cp )
		{ if (_clarray_ptr) ::clmattach(cp, (void*)_clarray_ptr); }
		
		void clmdetach()
		{ if (_clarray_ptr) ::clmdetach((void*)_clarray_ptr); }
	
		void clmsync( CONTEXT* cp, unsigned int devnum, int flags = 0 )
		{ if (_clarray_ptr) ::clmsync(cp, devnum, (void*)_clarray_ptr, flags); }
	
		void clarg_set_global( CONTEXT* cp, cl_kernel krn, unsigned int argnum )
		{ 
			if (_clarray_ptr) 
				::clarg_set_global(cp, krn, argnum,(void*)_clarray_ptr); 
		}

		void* get_ptr() { return (void*)_clarray_ptr; }

//		clarray<T,A>& operator () ( const Interval& ii ) 
//			{ first=ii.first; end=ii.end; shift=ii.shift; return *this; }

//		Wrapper< clarray<T,A> > operator () ( const Interval& ii )
//			{ return Wrapper< clarray<T,A> >(this,ii); } 

  template<class RHS>
  clarray<T,A>& operator=(const Expression<RHS> &rhs);


		clarray( clvector<T,A>* vec, int first, int end, int shift )
			: vec(vec), first(first), end(end), shift(shift) {}

//	protected:
	public:
	clvector<T,A>* vec;
	int first;
	int end;
	int shift;
	
};

/*
#ifndef _WIN64
#include "CLETE/clarray_CLETE.h"
#endif
*/

/*
  template < typename T, typename A> template<class RHS>
  clarray<T,A>& clarray<T,A>::operator=(const Expression<RHS> &rhs)
  {
		assign(*this,rhs);

    return *this;
  }
*/

//#ifndef _WIN64
//#include "CLETE/clarray_CLETE.h"
//#endif

//} //// namespace stdclpp

#endif //// ifdef __cplusplus

#endif

