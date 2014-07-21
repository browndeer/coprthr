/* clmulti_array.h
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

#ifndef __CLMULTI_ARRAY_H
#define __CLMULTI_ARRAY_H

#include <stdio.h>
#include <string>

#include <stdcl.h>
#include <clmalloc_allocator.h>

#include <boost/multi_array.hpp>

#include "CLETE/Interval.h"

#ifdef __cplusplus

#define __stdclpp__

/***
 *** clmulti_array
 ***/

template<class T> class Expression;

template < typename T, std::size_t D > class clmulti_array;

template < typename T, std::size_t D >
struct clmulti_array_interval {};

template < typename T >
struct clmulti_array_interval<T,1>
{
   public:

      template<class RHS>
      clmulti_array_interval<T,1>& operator=(const Expression<RHS> &rhs);

      clmulti_array_interval<T,1>& operator=(const clmulti_array_interval<T,1>& rhs);

      T* get_ptr() { return (T*)xref.get_ptr(); }

      clmulti_array<T,1>& xref;
		const Interval& interval;

   	friend
      clmulti_array_interval<T,1> clmulti_array<T,1>::operator()( const Interval& I );

   private:

      clmulti_array_interval() {}

      clmulti_array_interval( clmulti_array<T,1>& xref, const Interval& interval) 
			: xref(xref), interval(interval) {}

};

template < typename T >
struct clmulti_array_interval<T,2>
{
   public:

      template<class RHS>
      clmulti_array_interval<T,2>& operator=(const Expression<RHS> &rhs);

      clmulti_array_interval<T,2>& operator=(const clmulti_array_interval<T,2>& rhs);

      T* get_ptr() { return (T*)xref.get_ptr(); }
		
		template < std::size_t d >
		size_t size() const { return xref.shape()[d]; }

      clmulti_array<T,2>& xref;
		const Interval& interval0;
		const Interval& interval1;

   	friend
      clmulti_array_interval<T,2> 
			clmulti_array<T,2>::operator()( const Interval& I, const Interval& J );

   private:

      clmulti_array_interval() {}

      clmulti_array_interval( clmulti_array<T,2>& xref, 
			const Interval& interval0, const Interval& interval1
		) : xref(xref), interval0(interval0), interval1(interval1) {}

};


template < typename T >
struct clmulti_array_interval<T,3>
{
   public:

      template<class RHS>
      clmulti_array_interval<T,3>& operator=(const Expression<RHS> &rhs);

      T* get_ptr() { return (T*)xref.get_ptr(); }
		
		template < std::size_t d >
		size_t size() const { return xref.shape()[d]; }

      clmulti_array<T,3>& xref;
		const Interval& interval0;
		const Interval& interval1;
		const Interval& interval2;

   	friend
      clmulti_array_interval<T,3> 
			clmulti_array<T,3>::operator()( 
				const Interval& I, const Interval& J, const Interval& K );

   private:

      clmulti_array_interval() {}

      clmulti_array_interval( clmulti_array<T,3>& xref, 
			const Interval& interval0, const Interval& interval1, const Interval& interval2
		) : xref(xref), interval0(interval0), interval1(interval1), interval2(interval2) {}

};


template < typename T, std::size_t D >
class clmulti_array : public boost::multi_array< T, D, clmalloc_allocator<T> >
{
	typedef clmalloc_allocator<T> allocator_t;

	public:

  explicit clmulti_array() : boost::multi_array<T,D,allocator_t>() {}

  template <class ExtentList>
  explicit clmulti_array(
      ExtentList const& extents
#ifdef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
      , typename mpl::if_<
      detail::multi_array::is_multi_array_impl<ExtentList>,
      int&,int>::type* = 0
#endif 
      ) : boost::multi_array<T,D,allocator_t>(extents) {}



		void clmattach( CONTEXT* cp )
		{ ::clmattach(cp, (void*)this->origin() ); }
		
		void clmdetach()
		{ ::clmdetach((void*)this->origin() ); }
	
		void clmsync( CONTEXT* cp, unsigned int devnum, int flags = 0 )
		{ ::clmsync(cp, devnum, (void*)this->origin(), flags); }
	
		void clarg_set_global( CONTEXT* cp, cl_kernel krn, unsigned int argnum )
		{ ::clarg_set_global(cp, krn, argnum, (void*)this->origin()); }

		void* get_ptr() { return (void*)this->origin(); }


		// up-cast to clmulti_array_interval<T,1>
		clmulti_array_interval<T,1> operator()(const Interval& interval )
      { return clmulti_array_interval<T,1>(*this,interval); }

		// XXX this explicit cast should be eliminated -DAR
		clmulti_array_interval<T,1> operator()( int i )
      { return clmulti_array_interval<T,1>(*this,Interval(i)); }


		// up-cast to clmulti_array_interval<T,2>
		clmulti_array_interval<T,2> operator()(
			const Interval& interval0, const Interval& interval1 
		) { return clmulti_array_interval<T,2>(*this,interval0,interval1); }

		clmulti_array_interval<T,2> operator()(
			int i0, const Interval& interval1 
		) { return operator()( Interval(i0,i0+1), interval1 ); }

		clmulti_array_interval<T,2> operator()(
			const Interval& interval0, int i1
		) { return operator()( interval0, Interval(i1,i1+1) ); }


		// up-cast to clmulti_array_interval<T,3>
		clmulti_array_interval<T,3> operator()(
			const Interval& interval0, const Interval& interval1, 
			const Interval& interval2
		) { 
			return 
				clmulti_array_interval<T,3>(*this,interval0,interval1, interval2); 
		}


  		template<class RHS>
  		clmulti_array<T,D>& operator=(const Expression<RHS> &rhs);
	
};

#ifndef _WIN64
#include "CLETE/clmulti_array_CLETE.h"
#include "CLETE/clmulti_array_interval_CLETE.h"
#endif


#endif //// ifdef __cplusplus

#endif

