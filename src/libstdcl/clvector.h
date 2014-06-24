/* clvector.h
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

#ifndef __CLVECTOR_H
#define __CLVECTOR_H

#include <stdio.h>
#include <string>

#include <stdcl.h>
#include <clmalloc_allocator.h>

#include <vector>

#include "CLETE/Interval.h"

#ifdef __cplusplus

#define __stdclpp__


/***
 *** clvector
 ***/

template<class T> class Expression;

template < class T, class A > class clvector;

template < typename T, typename A = clmalloc_allocator<T> >
struct clvector_interval
{
	private:

		clvector_interval() {}

		clvector_interval( clvector<T,A>& vecref, const Interval& interval )
			: vecref(vecref), interval(interval) {}

	public:

		template<class RHS>
		clvector_interval<T,A>& operator=(const Expression<RHS> &rhs);

		T* data() { return vecref.data(); }

	public:

		clvector<T,A>& vecref;
		const Interval& interval;

	friend
		clvector_interval<T,A> 
		clvector<T,A>::operator()(const Interval& interval );

};


/* address the difference in implementations -DAR */
#ifdef _WIN64
#define _clvector_ptr (this->_Myfirst)
#else
#define _clvector_ptr (this->_M_impl._M_start)
#endif

template < typename T, typename A = clmalloc_allocator<T> >
class clvector : public std::vector< T, clmalloc_allocator<T> >
{
	typedef clmalloc_allocator<T> allocator_t;

	public:

		clvector()
			: std::vector< T, clmalloc_allocator<T> >() {}

		clvector( size_t n, const T& value = T() )
			: std::vector< T, clmalloc_allocator<T> >( n, value ) {}

		template < class InputIterator >
		clvector( InputIterator first, InputIterator last )
			: std::vector< T, clmalloc_allocator<T> >( first, last ) {}

		void clmattach( CONTEXT* cp )
		{ if (_clvector_ptr) ::clmattach(cp, (void*)_clvector_ptr); }
		
		void clmdetach()
		{ if (_clvector_ptr) ::clmdetach((void*)_clvector_ptr); }
	
		void clmsync( CONTEXT* cp, unsigned int devnum, int flags = 0 )
		{ if (_clvector_ptr) ::clmsync(cp, devnum, (void*)_clvector_ptr, flags); }
	
		void clarg_set_global( CONTEXT* cp, cl_kernel krn, unsigned int argnum )
		{ 
			if (_clvector_ptr) 
				::clarg_set_global(cp, krn, argnum,(void*)_clvector_ptr); 
		}

		void* get_ptr() { return (void*)_clvector_ptr; }

		clvector_interval<T,A> operator()(const Interval& interval ) 
		{ return clvector_interval<T,A>(*this,interval); }


  template<class RHS>
  clvector<T,A>& operator=(const Expression<RHS> &rhs);

};

#endif

#include <CLETE/clvector_CLETE.h>
#include <CLETE/clvector_interval_CLETE.h>

#endif

