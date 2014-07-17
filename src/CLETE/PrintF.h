/* PrintF.h
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

#ifndef _PRINTF_H
#define _PRINTF_H

#include <iostream>
#include <list>
#include <string>
#include <sstream>
using namespace std;

#include <stdcl.h>

#include "CLETE/PrintType.h"

template < class T >
static inline std::string tostr(const T& val )
{ std::stringstream ss; ss << val; return ss.str(); }

template <class T>
struct PrintF {

	typedef T xtype_t;

	inline static std::string type_str( std::string x ) 
	{ return "@arg-str-unknown@:" + x; }

	inline static std::string arg_str( std::string x ) 
	{ return "@arg-str-unknown@:" + x; }

//	inline static std::string tmp_decl_str( std::string x, std::string s = "0" ) 
	inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x ) 
//	{ return "@tmp-decl-str-unknown@:" + x; }
	{ return "@tmp-decl-str-unknown@:" + tostr(refid); }

	inline static std::string tmp_ref_str( std::string x ) 
	{ return "@tmp-ref-str-unknown@:" + x; }

	inline static std::string store_str( std::string x ) 
	{ return "@tmp-store-str-unknown@:" + x; }

};


template < class T > 
struct PrintF< Scalar<T> > { 

	typedef Scalar<T> xtype_t;

	inline static std::string type_str() 
	{ return PrintType<T>::type_str(); }

	inline static std::string arg_str( std::string x) 
	{ return "a" + x; }

//	inline static std::string tmp_decl_str( std::string x, std::string s = "0" ) 
//	inline static std::string tmp_decl_str( std::string x, xtype_t& xobj ) 
//	inline static std::string tmp_decl_str( std::string x ) 
	inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x ) 
	{ return PrintType<T>::type_str() + " tmp" + tostr(refid) + " = a" + tostr(refid); }

	inline static std::string tmp_ref_str( std::string x ) 
	{ return "tmp" + x; }

	inline static std::string store_str( std::string x ) 
	{ return "a" + x; }

};


// XXX this specialization must be here to avoid specialization before 
// XXX instantiation error -DAR
template <>
struct PrintF< Interval > {

	typedef Interval xtype_t;

   inline static std::string type_str() 
   { return "INTERVAL " + PrintType<int>::type_str(); }
  
   inline static std::string arg_str( std::string x)
   { return "INTERVAL"; }
  
//   inline static std::string tmp_decl_str( std::string x, std::string s )
//   inline static std::string tmp_decl_str( std::string x, xtype_t& xobj )
   inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x )
   { return ""; }

   inline static std::string tmp_ref_str( std::string x )
   { return "INTERVAL" + x; }
  
   inline static std::string store_str( std::string x )
   { return "INTERVAL" + x ; }
  
};  


#endif

