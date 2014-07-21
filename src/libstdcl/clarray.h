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

#ifndef __CLARRAY_H 
#define __CLARRAY_H

#include "clmulti_array.h"

template < typename T, std::size_t D = 1 >
class clarray : public clmulti_array<T,D> { };

template < typename T >
class clarray<T,1> : public clmulti_array<T,1> { 
	public: 
		clarray( std::size_t size0, T val = (T)0 ) 
			: clmulti_array<T,1>(boost::extents[size0]) {
			for(int i=0; i < size0; i++) *(T*)this->origin() = val;
		}
		std::size_t size( int d ) const { return this->shape()[d]; }
		std::size_t length( int d ) const { return size(d); }
};

template < typename T >
class clarray<T,2> : public clmulti_array<T,2> { 
	public: 
		clarray( std::size_t size0, std::size_t size1, T val = (T)0 ) 
			: clmulti_array<T,2>(boost::extents[size0][size1]) {
			for(int i=0; i < size0*size1; i++) *(T*)this->origin() = val;
		}
		std::size_t size( int d ) const { return this->shape()[d]; }
		std::size_t length( int d ) const { return size(d); }
};

template < typename T >
class clarray<T,3> : public clmulti_array<T,3> { 
	public: 
		clarray( std::size_t size0, std::size_t size1, std::size_t size2, 
			T val = (T)0 
		) : clmulti_array<T,3>(boost::extents[size0][size1][size2]) {
			for(int i=0; i < size0*size1*size2; i++) *(T*)this->origin() = val;
		}
		std::size_t size( int d ) const { return this->shape()[d]; }
		std::size_t length( int d ) const { return size(d); }
};

#endif

