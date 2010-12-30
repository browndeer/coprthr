/* clvector.h
 *
 * Copyright (c) 2010 Brown Deer Technology, LLC.  All Rights Reserved.
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

//#include <string.h>
#include <stdio.h>
//#include <sys/queue.h>

#include <string>

#include <CL/cl.h>
#include <stdcl.h>
#include "clmalloc_allocator.h"

#include <vector>


#ifdef __cplusplus

#define __stdclpp__

//namespace stdclpp {

/***
 *** clvector
 ***/

template<class T>
class Expression;

template < typename T, typename A = clmalloc_allocator<T> >
class clvector : public std::vector< T, clmalloc_allocator<T> >
{
	typedef clmalloc_allocator<T> allocator_t;

	public:

		void clmattach( CONTEXT* cp )
		{ 
			::clmattach(cp, (void*)this->_M_impl._M_start); 
		}
		
		void clmdetach()
		{ ::clmdetach((void*)this->_M_impl._M_start); }
	
		void clmsync( CONTEXT* cp, unsigned int devnum, int flags = 0 )
		{ ::clmsync(cp, devnum, (void*)this->_M_impl._M_start, flags); }
	
		void clarg_set_global( CONTEXT* cp, cl_kernel krn, unsigned int argnum )
		{ 
printf("clvector::clarg_set_global %p\n",(void*)this->_M_impl._M_start); fflush(stdout); 
::__clarg_set_global(cp, krn, argnum, 
			(void*)this->_M_impl._M_start); }


  template<class RHS>
  clvector<T,A>& operator=(const Expression<RHS> &rhs);

	
};

//#include "Eval.h"
#include "CLETE/clvector_CLETE.h"

  template < typename T, typename A> template<class RHS>
  clvector<T,A>& clvector<T,A>::operator=(const Expression<RHS> &rhs)
  {
		assign(*this,rhs);

    return *this;
  }


//} //// namespace stdclpp

#endif //// ifdef __cplusplus

#endif

