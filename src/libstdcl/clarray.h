/* clarray.h
 *
 * Copyright (c) 2010-2014 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include <stdio.h>

#include <string>

#include <CL/cl.h>
#include <stdcl.h>
#include <clmalloc_allocator.h>

#include <vector>

template < class T, class A > class clvector;

#include "CLETE/Interval.h"

#ifdef __cplusplus

#define __stdclpp__


/***
 *** clarray
 ***/

template<class T>
class Expression;

/* address the difference in implementations -DAR */
//#ifdef _WIN64
//#define _clarray_ptr (this->_Myfirst)
//#else
//#define _clarray_ptr (this->_M_impl._M_start)
//#endif

template < typename T, typename A = clmalloc_allocator<T> >
class clarray 
{
	typedef clmalloc_allocator<T> allocator_t;

	public:

		clarray() {}

//		clarray( size_t n, const T& value = T() ) {}

		template<class RHS>
		clarray<T,A>& operator=(const Expression<RHS> &rhs);


		clarray( clvector<T,A>* vec, int first, int end, int shift )
			: vec(vec), first(first), end(end), shift(shift) {}

		T* data() { return vec->data(); }

//	protected:
	public:
	clvector<T,A>* vec;
	int first;
	int end;
	int shift;
	
};

#endif

#endif

