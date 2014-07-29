/* clmulti_array_interval_CLETE.h
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

/* This is a derivative work based on PETE-2.1.0, produced in compliance
 * with the original ACL license reproduced below, and which should not be
 * confused with the original software.
 */

// ACL:license
// ----------------------------------------------------------------------
// This software and ancillary information (herein called "SOFTWARE")
// called PETE (Portable Expression Template Engine) is
// made available under the terms described here.  The SOFTWARE has been
// approved for release with associated LA-CC Number LA-CC-99-5.
// 
// Unless otherwise indicated, this SOFTWARE has been authored by an
// employee or employees of the University of California, operator of the
// Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
// the U.S. Department of Energy.  The U.S. Government has rights to use,
// reproduce, and distribute this SOFTWARE. The public may copy, distribute,
// prepare derivative works and publicly display this SOFTWARE without 
// charge, provided that this Notice and any statement of authorship are 
// reproduced on all copies.  Neither the Government nor the University 
// makes any warranty, express or implied, or assumes any liability or 
// responsibility for the use of this SOFTWARE.
// 
// If SOFTWARE is modified to produce derivative works, such modified
// SOFTWARE should be clearly marked, so as not to confuse it with the
// version available from LANL.
// 
// For more information about PETE, send e-mail to pete@acl.lanl.gov,
// or visit the PETE web page at http://www.acl.lanl.gov/pete/.
// ----------------------------------------------------------------------
// ACL:license
// ----------------------------------------------------------------------
// $RCSfile: Eval.h,v $   $Author: swhaney $
// $Revision: 1.11 $   $Date: 2000/01/04 01:39:17 $
// ----------------------------------------------------------------------

/* DAR */

#ifndef _CLMULTI_ARRAY_INTERVAL_CLETE_H
#define _CLMULTI_ARRAY_INTERVAL_CLETE_H

#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <cstdlib>
#include <stdcl.h>
using namespace std;

#include <stdcl.h>
#include <clmulti_array.h>

#include "CLETE/PETE.h"
#include "CLETE/clmulti_array_interval_Operators.h"
#include "CLETE/PrintType.h"
#include "CLETE/PrintF.h"

#include <cmath>

#if defined(__CLVECTOR_FULLAUTO_STDACC)
#define __CLCONTEXT stdacc
#define __WGSIZE 16
#elif defined(__CLVECTOR_FULLAUTO_STDCPU)
#define __CLCONTEXT stdcpu
#define __WGSIZE 16
#elif defined(__CLVECTOR_FULLAUTO_STDGPU)
#define __CLCONTEXT stdgpu
#define __WGSIZE 64
#else
#define __CLCONTEXT stdgpu
#define __WGSIZE 64
#endif

#define clete_expr auto

//-----------------------------------------------------------------------------
// We need to specialize CreateLeaf<T> for our class, so that operators
// know what to stick in the leaves of the expression tree.
//-----------------------------------------------------------------------------

template<class T, std::size_t D>
struct CreateLeaf<clmulti_array_interval<T, D> >
{
  typedef Reference<clmulti_array_interval<T, D> > Leaf_t;
  inline static
  Leaf_t make(const clmulti_array_interval<T, D>& a) { return Leaf_t(a); }
};

//-----------------------------------------------------------------------------
// EvalLeaf1 is used to evaluate expression with vectors.
// (It's already defined for Scalar values.)
//-----------------------------------------------------------------------------


/* 
 * XXX the EvalLeaf functors are not used for offload, fix later -DAR

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, EvalLeaf1>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 1>& m, const EvalLeaf1& f)
  {
    return m[f.val1()];
  }
};


template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, EvalLeaf2>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 1>& m, const EvalLeaf2 &f)
  {
    return m[f.val1()];
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 2>, EvalLeaf2>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 2>& m, const EvalLeaf2 &f)
  {
    return m[f.val1()][f.val2()];
  }
};




template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, EvalLeaf3>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 1>& m, const EvalLeaf3 &f)
  {
    return m[f.val1()];
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 2>, EvalLeaf3>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 2>& m, const EvalLeaf3 &f)
  {
    return m[f.val1()][f.val2()];
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 3>, EvalLeaf3>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 3>& m, const EvalLeaf3 &f)
  {
    return m[f.val1()][f.val2()][f.val3()];
  }
};




template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, EvalLeaf4>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 1>& m, const EvalLeaf4 &f)
  {
    return m[f.val1()];
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 2>, EvalLeaf4>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 2>& m, const EvalLeaf4 &f)
  {
    return m[f.val1()][f.val2()];
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 3>, EvalLeaf4>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 3>& m, const EvalLeaf4 &f)
  {
    return m[f.val1()][f.val2()][f.val3()];
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 4>, EvalLeaf4>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, 4>& m, const EvalLeaf4 &f)
  {
    return m[f.val1()][f.val2()][f.val3()][f.val4()];
  }
};
*/


////
//// clmulti_array_interval<>, SizeLeaf* functors
////

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, SizeLeaf1>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 1>& m, const SizeLeaf1& s) 
  {
    return s(m.shape()[0]);
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, SizeLeaf2>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 1>& m, const SizeLeaf2& s) 
  {
    return s(m.shape()[0]);
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, SizeLeaf3>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 1>& m, const SizeLeaf3& s) 
  {
    return s(m.shape()[0]);
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 1>, SizeLeaf4>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 1>& m, const SizeLeaf4& s) 
  {
    return s(m.shape()[0]);
  }
};


template<class T>
struct LeafFunctor<clmulti_array_interval<T, 2>, SizeLeaf2>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 2>& m, const SizeLeaf2& s) 
  {
    return s(m.shape()[0],m.shape()[1]);
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 2>, SizeLeaf3>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 2>& m, const SizeLeaf3& s) 
  {
    return s(m.shape()[0],m.shape()[1]);
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 2>, SizeLeaf4>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 2>& m, const SizeLeaf4& s) 
  {
    return s(m.shape()[0],m.shape()[1]);
  }
};


template<class T>
struct LeafFunctor<clmulti_array_interval<T, 3>, SizeLeaf3>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 3>& m, const SizeLeaf3& s) 
  {
    return s(m.shape()[0],m.shape()[1],m.shape()[2]);
  }
};

template<class T>
struct LeafFunctor<clmulti_array_interval<T, 3>, SizeLeaf4>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 3>& m, const SizeLeaf4& s) 
  {
    return s(m.shape()[0],m.shape()[1],m.shape()[2]);
  }
};


template<class T>
struct LeafFunctor<clmulti_array_interval<T, 4>, SizeLeaf4>
{
  typedef bool Type_t;
  inline static
  bool apply(const clmulti_array_interval<T, 4>& m, const SizeLeaf4& s) 
  {
    return s(m.shape()[0],m.shape()[1],m.shape()[2],m.shape()[3]);
  }
};


////
//// Interval, SizeLeaf* functors
////

template<>
struct LeafFunctor<Interval, SizeLeaf1>
{
  typedef bool Type_t;
  inline static
  bool apply(const Interval& m, const SizeLeaf1& s) 
  {
    return s(0);
  }
};

template<>
struct LeafFunctor<Interval, SizeLeaf2>
{
  typedef bool Type_t;
  inline static
  bool apply(const Interval& m, const SizeLeaf2& s) 
  {
    return s(0);
  }
};

template<>
struct LeafFunctor<Interval, SizeLeaf3>
{
  typedef bool Type_t;
  inline static
  bool apply(const Interval& m, const SizeLeaf3& s) 
  {
    return s(0);
  }
};

template<>
struct LeafFunctor<Interval, SizeLeaf4>
{
  typedef bool Type_t;
  inline static
  bool apply(const Interval& m, const SizeLeaf4& s) 
  {
    return s(0);
  }
};


////
//// more functors
////

template<class T, std::size_t D>
struct LeafFunctor<clmulti_array_interval<T, D>, PrintTmpLeaf>
{
  typedef std::string Type_t;
  inline static
  std::string apply(
		const clmulti_array_interval<T, D> & v, const PrintTmpLeaf & p
  ) 
  { return "tmp" + tostr(p((intptr_t)&v)); }
};


template<>
struct LeafFunctor<Interval, PrintTmpLeaf>
{
  typedef std::string Type_t;
  inline static
  std::string apply(
		const Interval& v, const PrintTmpLeaf & p
  ) 
  { return "/* ??? */ (gti+(" + tostr(v.shift) + "))"; }
};


template<class T, std::size_t D>
struct LeafFunctor<clmulti_array_interval<T, D>*, PtrListLeaf>
{
  typedef std::list<const void*> Type_t;
  inline static
  Type_t apply(
		clmulti_array_interval<T, D>* const & ptr, const PtrListLeaf & plist
  )
  { return Type_t(1,ptr); }
};


template<class T, std::size_t D>
struct LeafFunctor<clmulti_array_interval<T, D>*, RefListLeaf>
{
  typedef std::list<Ref> Type_t;
  inline static
  Type_t apply(clmulti_array_interval<T, D>* const & ptr, const RefListLeaf &r)
  {
		typedef PrintF<clmulti_array_interval<T,D> > tprint;
    return Type_t(1,Ref(
		ptr,0,
		-1,-1,-1,
		ptr->get_ptr(),D,
		tprint::type_str(),
		tprint::arg_str(tostr(r((intptr_t)ptr))),
		tprint::tmp_decl_str(r.mask((intptr_t)ptr),*ptr),
		tprint::tmp_ref_str(tostr(r((intptr_t)ptr))),
		tprint::store_str(tostr(r((intptr_t)ptr))) ));
  }
};

template<>
struct LeafFunctor<Interval*, RefListLeaf>
{
  typedef std::list<Ref> Type_t;
  inline static
  Type_t apply(Interval* const & ptr, const RefListLeaf &r)
  {
    return Type_t(1,Ref(
		ptr,0,ptr->first,ptr->end,ptr->shift,0,0,
		PrintF< Interval >::type_str(),
		PrintF< Interval >::arg_str(tostr(r((intptr_t)ptr))),
		PrintF< Interval >::tmp_decl_str(r.mask((intptr_t)ptr), *ptr ),
		PrintF< Interval >::tmp_ref_str(tostr(r((intptr_t)ptr))),
		PrintF<Interval >::store_str(tostr(r((intptr_t)ptr))) ));
  }
};


template<class T, std::size_t D>
struct LeafFunctor<clmulti_array_interval<T, D>, AddressOfLeaf>
{
  typedef clmulti_array_interval<T, D>* Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, D>& m, const AddressOfLeaf & f)
  {
    return Type_t(&m);
  }
};

template<>
struct LeafFunctor<Interval, AddressOfLeaf>
{
  typedef Interval* Type_t;
  inline static
  Type_t apply(const Interval& ival, const AddressOfLeaf & f)
  {
    return Type_t(&ival);
  }
};


template<class T, std::size_t D>
struct LeafFunctor<clmulti_array_interval<T, D>, IAddressOfLeaf>
{
  typedef intptr_t Type_t;
  inline static
  Type_t apply(const clmulti_array_interval<T, D>& m, const IAddressOfLeaf & f)
  {
    return Type_t((intptr_t)&m);
  }
};

template<>
struct LeafFunctor<Interval, IAddressOfLeaf>
{
  typedef intptr_t Type_t;
  inline static
  Type_t apply(const Interval& ival, const IAddressOfLeaf & f)
  {
    return Type_t((intptr_t)&ival);
  }
};


/*
template < class T, std::size_t D >
struct PrintF< clmulti_array_interval<T, D> > {

	typedef clmulti_array_interval<T, D> xtype_t;

   inline static std::string type_str() 
   { return "__global " + PrintType<T>::type_str() + "*"; }

   inline static std::string arg_str( std::string x) 
   { return "a" + x; }

   inline static std::string tmp_decl_str( std::string x, const xtype_t& xobj )
	{ return PrintType<T>::type_str() + " tmp" + x + " = a" + x + "[gti+(" + tostr(xobj.interval.shift) + ")] [[D]] "; }

	inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x )
	{ return PrintType<T>::type_str() + " tmp" + tostr(refid) + " = a" + tostr(refid) + "[gti+(" + tostr(x.interval.shift) + ")] [[D]] "; }

   inline static std::string tmp_ref_str( std::string x )
   { return "tmp" + x; }

   inline static std::string store_str( std::string x )
   { return "a" + x + "[gti]"; }
	//// XXX should this account for shift?  LHS is always not shifted -DAR
};
*/


template < class T >
struct PrintF< clmulti_array_interval<T, 1> > {

	typedef clmulti_array_interval<T, 1> xtype_t;

   inline static std::string type_str() 
   { return "__global " + PrintType<T>::type_str() + "*"; }

   inline static std::string arg_str( std::string x) 
   { return "a" + x; }

   inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x )
	{ return PrintType<T>::type_str() + " tmp" + tostr(refid) + " = a" + tostr(refid) + "[gti0+(" + tostr(x.interval.shift) + ")]"; }

   inline static std::string tmp_ref_str( std::string x )
   { return "tmp" + x; }

   inline static std::string store_str( std::string x )
   { return "a" + x + "[gti]"; }
	//// XXX should this account for shift?  LHS is always not shifted -DAR
};

template < class T >
struct PrintF< clmulti_array_interval<T, 2> > {

	typedef clmulti_array_interval<T, 2> xtype_t;

   inline static std::string type_str() 
   { return "__global " + PrintType<T>::type_str() + "*"; }

   inline static std::string arg_str( std::string x) 
   { return "a" + x; }

   inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x )
	{ 
		return PrintType<T>::type_str() + " tmp" + tostr(refid) + " = a" 
//			+ tostr(refid) + "[(gti1+(" + tostr(x.interval1.shift) + "))*size1 + gti0 + (" + tostr(x.interval0.shift) + ")]"; 
			+ tostr(refid) + "[(gti0+(" + tostr(x.interval0.shift) + "))*size1 + gti1 + (" + tostr(x.interval1.shift) + ")]"; 
	}

   inline static std::string tmp_ref_str( std::string x )
   { return "tmp" + x; }

   inline static std::string store_str( std::string x )
   { return "a" + x + "[gti]"; }
	//// XXX should this account for shift?  LHS is always not shifted -DAR
};

template < class T >
struct PrintF< clmulti_array_interval<T, 3> > {

	typedef clmulti_array_interval<T, 3> xtype_t;

   inline static std::string type_str() 
   { return "__global " + PrintType<T>::type_str() + "*"; }

   inline static std::string arg_str( std::string x) 
   { return "a" + x; }

   inline static std::string tmp_decl_str( intptr_t refid, const xtype_t& x )
	{ 
		return PrintType<T>::type_str() + " tmp" + tostr(refid) + " = a" 
			+ tostr(refid) + "[(gti0+(" + tostr(x.interval0.shift) + "))*size1*size2 + (gti1 + (" + tostr(x.interval1.shift) + "))*size2 + gti2 + (" + tostr(x.interval2.shift) + ")]"; 
	}

   inline static std::string tmp_ref_str( std::string x )
   { return "tmp" + x; }

   inline static std::string store_str( std::string x )
   { return "a" + x + "[gti]"; }
	//// XXX should this account for shift?  LHS is always not shifted -DAR
};

//// XXX use macros as workaround for incorrect behavior of gcc 4.1 -DAR

#define log_kernel(srcstr) do { \
   if (__log_automatic_kernels_filename) { \
      std::ofstream ofs( \
         __log_automatic_kernels_filename, \
         std::ios_base::out|std::ios_base::app); \
      ofs<<srcstr<<"\n"; \
      ofs.close(); \
	} \
   } while (0)




/* this is where the real magic happens ... the evaluate functions */

template<class T, class Op, class RHS>
inline void evaluate(
	clmulti_array_interval<T, 1> &lhs, const Op &op, 
	const Expression<RHS> &rhs
)
{
	typedef clmulti_array_interval<T, 1> xtype_t;

	typedef PrintF< clmulti_array_interval<T, 1> > tprint;

//  if (forEach(rhs, SizeLeaf1(lhs.size()), AndCombine())) {
	if (1) {

#if defined(__CLMULTI_ARRAY_SEMIAUTO) || defined(__CLMULTI_ARRAY_FULLAUTO)

	typedef typename 
		ForEach<Expression<RHS>, AddressOfLeaf, TreeCombine >::Type_t New_t;

	New_t rhs2 = forEach(rhs,AddressOfLeaf(),TreeCombine());

	typedef std::list<Ref> rlist_t;

	intptr_t mask = ~((intptr_t)
		forEach(rhs,IAddressOfLeaf(),AndBitsCombine()) & (intptr_t)&lhs);
 
	rlist_t rlist = forEach(rhs2,RefListLeaf(mask),ListCombine<Ref>());

	rlist.sort(ref_is_ordered);
	rlist.unique(ref_is_equal);

	rlist_t rlista = rlist;
	rlista.push_back(
		Ref(&lhs,0,
			lhs.interval.first,lhs.interval.end,lhs.interval.shift,
			lhs.get_ptr(),1,
			tprint::type_str(),
			tprint::arg_str(tostr(mask & (intptr_t)&lhs)),
			tprint::tmp_decl_str(mask & ((intptr_t)&lhs), lhs ),
			tprint::tmp_ref_str(tostr(mask & (intptr_t)&lhs)),
			tprint::store_str(tostr(mask & (intptr_t)&lhs))
		)
	);
	rlista.sort(ref_is_ordered);
	rlista.unique(ref_is_equal);

//fprintf(stderr,"rlist.size()=%d rlista.size()=%d\n",rlist.size(),rlista.size());

	int size = lhs.xref.size();
	size_t r = size;
	if (r%256 > 0) r += 256 - r%256;

	int first = lhs.interval.first;
   int end = lhs.interval.end;

	static cl_kernel krn = (cl_kernel)0;

	if (!krn) {

		std::string srcstr = "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n";

		srcstr += "__kernel void\nkern( int first, int end, \n";

		int n = 2;	
		for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
			std::string argstr = (*it).arg_str;
			if (argstr != "INTERVAL")
				srcstr += (*it).type_str + " " + (*it).arg_str + ",\n";
	
		}
		srcstr += "int size\n";

		srcstr += "){\n";
		srcstr += "int gti = get_global_id(0);\n";
		srcstr += "int gti0 = gti;\n";

		srcstr += "if (gti >= first && gti < end ) {\n";

		if (size != r) srcstr += "if (gti<size) {\n";

		for( rlist_t::iterator it = rlist.begin(); it!=rlist.end(); it++,n++) {
			srcstr += (*it).tmp_decl_str;
//			if ((*it).dim == 1) srcstr += "[gti]";
			srcstr += ";\n";
		}


		std::string expr = forEach(rhs,PrintTmpLeaf(mask),PrintCombine());

		srcstr += op.strexpr(
			PrintF< clmulti_array_interval<T,1> >::store_str(
				tostr(mask & (intptr_t)&lhs)), expr ) + ";\n" ;
		
		if (size != r) srcstr += "}\n";

		srcstr += "}\n";
		srcstr += "}\n";

		log_kernel(srcstr);

		void* clh = clsopen(__CLCONTEXT,srcstr.c_str(),CLLD_NOW);
		krn = clsym(__CLCONTEXT,clh,"kern",CLLD_NOW);
	}

	if (!krn) {
		fprintf(stderr, "CLETE failed to build kernel, crashing now\n");
		exit(-1);
	}

	clndrange_t ndr = clndrange_init1d(0,r,__WGSIZE);

	clSetKernelArg(krn,0,sizeof(int),&first);
  	clSetKernelArg(krn,1,sizeof(int),&end);

	int n = 2;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++) {

		size_t sz = (*it).sz;

		if (sz > 0) {

			clSetKernelArg(krn,n++,sz,(*it).ptr);

		} else if ( (*it).memptr ) { 

			void* data_ptr = ((xtype_t*)(*it).ptr)->xref.get_ptr();

#if defined(__CLMULTI_ARRAY_FULLAUTO)
			clmattach(__CLCONTEXT, data_ptr);
			clmsync(__CLCONTEXT,0, data_ptr, CL_MEM_DEVICE|CL_EVENT_NOWAIT);
#endif

			clarg_set_global(__CLCONTEXT,krn,n++, data_ptr );

		}
	}

	clarg_set(__CLCONTEXT,krn,n,size);


	clfork(__CLCONTEXT,0,krn,&ndr,CL_EVENT_NOWAIT);

#if defined(__CLMULTI_ARRAY_FULLAUTO)

	clmsync(__CLCONTEXT,0,lhs.xref.get_ptr(),CL_MEM_HOST|CL_EVENT_NOWAIT);

	clwait(__CLCONTEXT,0,CL_KERNEL_EVENT|CL_MEM_EVENT);

	n = 0;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
		size_t sz = (*it).sz;
		if (sz > 0) {
		} else { 

			if ((*it).memptr != 0)
				clmdetach((void*)((xtype_t*)(*it).ptr)->xref.get_ptr());

		}
	}

#elif defined(__CLMULTI_ARRAY_SEMIAUTO)

	clwait(__CLCONTEXT,0,CL_KERNEL_EVENT);

#endif


#else

	fprintf(stderr,"CLETE clvector_interval only implemented for offload\n");
	exit(-1);

	for (int i = 0; i < lhs.size(); ++i) {
		op(lhs[i], forEach(rhs, EvalLeaf1(i), OpCombine()));
	}

#endif


	} else {
      cerr << "Error: LHS and RHS don't conform." << endl;
      exit(1);
   }
}


template<class T, class Op, class RHS>
inline void evaluate(
	clmulti_array_interval<T, 2> &lhs, const Op &op, 
	const Expression<RHS> &rhs
)
{
	typedef clmulti_array_interval<T, 1> xtype1_t;
	typedef clmulti_array_interval<T, 2> xtype_t;
	typedef PrintF< clmulti_array_interval<T, 2> > tprint;

//  if (forEach(rhs, SizeLeaf2(lhs.shape()[0],lhs.shape()[1]), AndCombine())) {
	if(1) {

#if defined(__CLMULTI_ARRAY_SEMIAUTO) || defined(__CLMULTI_ARRAY_FULLAUTO)

	typedef typename 
		ForEach<Expression<RHS>, AddressOfLeaf, TreeCombine >::Type_t New_t;

	New_t rhs2 = forEach(rhs,AddressOfLeaf(),TreeCombine());

	typedef std::list<Ref> rlist_t;

	intptr_t mask = ~((intptr_t)
		forEach(rhs,IAddressOfLeaf(),AndBitsCombine()) & (intptr_t)&lhs);
 
	rlist_t rlist = forEach(rhs2,RefListLeaf(mask),ListCombine<Ref>());

	rlist.sort(ref_is_ordered);
	rlist.unique(ref_is_equal);

	rlist_t rlista = rlist;
	rlista.push_back(
		Ref(&lhs,0,
			lhs.interval0.first,lhs.interval0.end,lhs.interval0.shift,
			lhs.get_ptr(),2,
			tprint::type_str(),
			tprint::arg_str(tostr(mskptr(mask,&lhs))),
			tprint::tmp_decl_str( mskptr(mask,&lhs),lhs),
			tprint::tmp_ref_str(tostr( mskptr(mask,&lhs))),
			tprint::store_str(tostr(mskptr(mask,&lhs)))
		)
	);

	rlista.sort(ref_is_ordered);
	rlista.unique(ref_is_equal);

	int size0 = lhs.xref.shape()[0];
	int size1 = lhs.xref.shape()[1];
	int size = size0*size1;
	int r = size;
	if (r%256 > 0) r += 256 - r%256;

	printf("size0=%d size1=%d\n",size0,size1);

	int first0 = lhs.interval0.first;
	int end0 = lhs.interval0.end;
	int first1 = lhs.interval1.first;
	int end1 = lhs.interval1.end;

	static cl_kernel krn = (cl_kernel)0;

	if (!krn) {

		std::string srcstr = "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n";

		srcstr += "__kernel void\nkern(\n";
		srcstr += " int first0, int end0,\n";
		srcstr += " int first1, int end1,\n";

		int n = 4;	
		for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
			srcstr += (*it).type_str + " " + (*it).arg_str + ",\n";
	
		}
		srcstr += "int size, int size0, int size1\n";

		srcstr += "){\n";
		srcstr += "int gti = get_global_id(0);\n";
		srcstr += "int gti0 = gti/size1;\n";
		srcstr += "int gti1 = gti%size1;\n";

		srcstr += "if (gti0>=first0 && gti0<end0 && gti1>=first1 && gti1<end1) {\n";

		if (size != r) srcstr += "if (gti<size) {\n";

		for( rlist_t::iterator it = rlist.begin(); it!=rlist.end(); it++,n++) {
			srcstr += (*it).tmp_decl_str;
			srcstr += ";\n";
		}


		std::string expr = forEach(rhs,PrintTmpLeaf(mask),PrintCombine());

		srcstr += op.strexpr(
			tprint::store_str( tostr(mask & (intptr_t)&lhs)), expr ) + ";\n" ;
		
		
		if (size != r) srcstr += "}\n";

		srcstr += "}\n";

		srcstr += "}\n";

		log_kernel(srcstr);

		void* clh = clsopen(__CLCONTEXT,srcstr.c_str(),CLLD_NOW);
		krn = clsym(__CLCONTEXT,clh,"kern",CLLD_NOW);
	}

	if (!krn) {
		fprintf(stderr, "CLETE failed to build kernel, crashing now\n");
		exit(-1);
	}
			
	clndrange_t ndr = clndrange_init1d(0,r,__WGSIZE);

	clSetKernelArg(krn,0,sizeof(int),&first0);
  	clSetKernelArg(krn,1,sizeof(int),&end0);
	clSetKernelArg(krn,2,sizeof(int),&first1);
  	clSetKernelArg(krn,3,sizeof(int),&end1);
	int n = 4;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++) {

		size_t sz = (*it).sz;

		if (sz > 0) {

			clSetKernelArg(krn,n++,sz,(*it).ptr);

		} else if ( (*it).memptr ) { 

			void* data_ptr = 0;
			switch((*it).dim) {
				case 1:
					data_ptr = ((xtype1_t*)(*it).ptr)->xref.get_ptr();
					break;
				default:
					data_ptr = ((xtype_t*)(*it).ptr)->xref.get_ptr();
			}

//printf( "dim %d\n",(*it).dim); fflush(stdout);

#if defined(__CLMULTI_ARRAY_FULLAUTO)
			clmattach(__CLCONTEXT,data_ptr);
			clmsync(__CLCONTEXT,0,data_ptr,CL_MEM_DEVICE|CL_EVENT_NOWAIT);
#endif

			clarg_set_global(__CLCONTEXT,krn,n++,data_ptr);

		}
	}

	clarg_set(__CLCONTEXT,krn,n,size);
	clarg_set(__CLCONTEXT,krn,n+1,size0);
	clarg_set(__CLCONTEXT,krn,n+2,size1);


	clfork(__CLCONTEXT,0,krn,&ndr,CL_EVENT_NOWAIT);

#if defined(__CLMULTI_ARRAY_FULLAUTO)

	clmsync(__CLCONTEXT,0,lhs.get_ptr(),CL_MEM_HOST|CL_EVENT_NOWAIT);

	clwait(__CLCONTEXT,0,CL_KERNEL_EVENT|CL_MEM_EVENT);

	n = 0;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
		size_t sz = (*it).sz;
		if (sz > 0) {
		} else { 

			switch((*it).dim) {
				case 1:
					clmdetach(((xtype1_t*)(*it).ptr)->xref.get_ptr());
					break;
				default:
					clmdetach(((xtype_t*)(*it).ptr)->xref.get_ptr());
			}

		}
	}

#elif defined(__CLMULTI_ARRAY_SEMIAUTO)

	clwait(__CLCONTEXT,0,CL_KERNEL_EVENT);

#endif


#else

	fprintf(stderr,"CLETE clvector_interval only implemented for offload\n");
	exit(-1);

   for (int i = 0; i < lhs.shape()[0]; ++i) 
   for (int j = 0; j < lhs.shape()[1]; ++j) {
       op(lhs[i][j], forEach(rhs, EvalLeaf2(i,j), OpCombine()));
   }

#endif


   } else {
      cerr << "Error: LHS and RHS don't conform." << endl;
      exit(1);
   }
}


template<class T, class Op, class RHS>
inline void evaluate(
	clmulti_array_interval<T, 3> &lhs, const Op &op, 
	const Expression<RHS> &rhs
)
{
	typedef clmulti_array_interval<T, 3> xtype_t;
	typedef PrintF< clmulti_array_interval<T, 3> > tprint;

//  if (forEach(rhs, SizeLeaf3(lhs.shape()[0],lhs.shape()[1],lhs.shape()[2]), AndCombine())) {
	if (1) {

#if defined(__CLMULTI_ARRAY_SEMIAUTO) || defined(__CLMULTI_ARRAY_FULLAUTO)

	typedef typename 
		ForEach<Expression<RHS>, AddressOfLeaf, TreeCombine >::Type_t New_t;

	New_t rhs2 = forEach(rhs,AddressOfLeaf(),TreeCombine());

	typedef std::list<Ref> rlist_t;

	intptr_t mask = ~((intptr_t)forEach(rhs,IAddressOfLeaf(),AndBitsCombine()) & (intptr_t)&lhs);
 
	rlist_t rlist = forEach(rhs2,RefListLeaf(mask),ListCombine<Ref>());

	rlist.sort(ref_is_ordered);
	rlist.unique(ref_is_equal);

	rlist_t rlista = rlist;
	rlista.push_back(
		Ref(&lhs,0,
			lhs.interval0.first,lhs.interval0.end,lhs.interval0.shift,
			lhs.get_ptr(),3,
			tprint::type_str(),
			tprint::arg_str(tostr(mask & (intptr_t)&lhs)),
			tprint::tmp_decl_str(mask & ((intptr_t)&lhs),lhs),
			tprint::tmp_ref_str(tostr(mask & (intptr_t)&lhs)),
			tprint::store_str(tostr(mask & (intptr_t)&lhs))
		)
	);

	rlista.sort(ref_is_ordered);
	rlista.unique(ref_is_equal);

	int size0 = lhs.xref.shape()[0];
	int size1 = lhs.xref.shape()[1];
	int size2 = lhs.xref.shape()[2];
	int size = size0*size1*size2;
	int r = size;
	if (r%256 > 0) r += 256 - r%256;

	int first0 = lhs.interval0.first;
	int end0 = lhs.interval0.end;
	int first1 = lhs.interval1.first;
	int end1 = lhs.interval1.end;
	int first2 = lhs.interval2.first;
	int end2 = lhs.interval2.end;

	static cl_kernel krn = (cl_kernel)0;

	if (!krn) {

		std::string srcstr = "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n";

		srcstr += "__kernel void\nkern(\n";
		srcstr += "int first0, int end0,\n";
		srcstr += "int first1, int end1,\n";
		srcstr += "int first2, int end2,\n";

		int n = 6;	
		for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
			srcstr += (*it).type_str + " " + (*it).arg_str + ",\n";
	
		}
		srcstr += "int size, int size0, int size1, int size2\n";

		srcstr += "){\n";
		srcstr += "int gti = get_global_id(0);\n";
		srcstr += "int gti0 = gti/(size1*size2);\n";
		srcstr += "int gti1 = (gti%(size1*size2))/size2;\n";
		srcstr += "int gti2 = gti%size2;\n";

		srcstr += "if (gti0>=first0 && gti0<end0 && gti1>=first1 && gti1<end1 && gti2>=first2 && gti2<end2) {\n";
		if (size != r) srcstr += "if (gti<size) {\n";

		for( rlist_t::iterator it = rlist.begin(); it!=rlist.end(); it++,n++) {
			srcstr += (*it).tmp_decl_str;
			srcstr += ";\n";
		}


		std::string expr = forEach(rhs,PrintTmpLeaf(mask),PrintCombine());

		srcstr += op.strexpr(
			tprint::store_str( tostr(mask & (intptr_t)&lhs)), expr ) + ";\n" ;
		
		if (size != r) srcstr += "}\n";

		srcstr += "}\n";

		srcstr += "}\n";

		log_kernel(srcstr);

		void* clh = clsopen(__CLCONTEXT,srcstr.c_str(),CLLD_NOW);
		krn = clsym(__CLCONTEXT,clh,"kern",CLLD_NOW);
	}

	if (!krn) {
		fprintf(stderr,"CLETE failed to build kernel, crashing now\n");
		exit(-1);
	}


	clndrange_t ndr = clndrange_init1d(0,r,__WGSIZE);

	clSetKernelArg(krn,0,sizeof(int),&first0);
	clSetKernelArg(krn,1,sizeof(int),&end0);
	clSetKernelArg(krn,2,sizeof(int),&first1);
	clSetKernelArg(krn,3,sizeof(int),&end1);
	clSetKernelArg(krn,4,sizeof(int),&first2);
	clSetKernelArg(krn,5,sizeof(int),&end2);
	int n = 6;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++) {

		size_t sz = (*it).sz;

		if (sz > 0) {

			clSetKernelArg(krn,n++,sz,(*it).ptr);

		} else if ( (*it).memptr ) { 

			void* data_ptr = ((xtype_t*)(*it).ptr)->xref.get_ptr();

#if defined(__CLMULTI_ARRAY_FULLAUTO)
			clmattach(__CLCONTEXT,data_ptr);
			clmsync(__CLCONTEXT,0,data_ptr,CL_MEM_DEVICE|CL_EVENT_NOWAIT);
#endif

			clarg_set_global(__CLCONTEXT,krn,n++,data_ptr);

		}
	}

	clarg_set(__CLCONTEXT,krn,n,size);
	clarg_set(__CLCONTEXT,krn,n+1,size0);
	clarg_set(__CLCONTEXT,krn,n+2,size1);
	clarg_set(__CLCONTEXT,krn,n+3,size2);


	clfork(__CLCONTEXT,0,krn,&ndr,CL_EVENT_NOWAIT);

#if defined(__CLMULTI_ARRAY_FULLAUTO)

	clmsync(__CLCONTEXT,0,lhs.get_ptr(),CL_MEM_HOST|CL_EVENT_NOWAIT);

	clwait(__CLCONTEXT,0,CL_KERNEL_EVENT|CL_MEM_EVENT);

	n = 0;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
		size_t sz = (*it).sz;
		if (sz > 0) {
		} else { 

			clmdetach(((xtype_t*)(*it).ptr)->xref.get_ptr());

		}
	}

#elif defined(__CLMULTI_ARRAY_SEMIAUTO)

		clwait(__CLCONTEXT,0,CL_KERNEL_EVENT);

#endif



#else

	fprintf(stderr,"CLETE clvector_interval only implemented for offload\n");
	exit(-1);

      for (int i = 0; i < lhs.shape()[0]; ++i) 
      for (int j = 0; j < lhs.shape()[1]; ++j) 
      for (int k = 0; k < lhs.shape()[2]; ++k) {
          op(lhs[i][j][k], forEach(rhs, EvalLeaf3(i,j,k), OpCombine()));
        }

#endif


    } else {
      cerr << "Error: LHS and RHS don't conform." << endl;
      exit(1);
    }
}


template<class T, class Op, class RHS>
inline void evaluate(
	clmulti_array_interval<T, 4> &lhs, const Op &op, 
	const Expression<RHS> &rhs
)
{
  if (forEach(rhs, SizeLeaf4(lhs.shape()[0],lhs.shape()[1],lhs.shape()[2],lhs.shape()[3]), AndCombine())) {

#if defined(__CLMULTI_ARRAY_SEMIAUTO) || defined(__CLMULTI_ARRAY_FULLAUTO)

	typedef typename 
		ForEach<Expression<RHS>, AddressOfLeaf, TreeCombine >::Type_t New_t;

	New_t rhs2 = forEach(rhs,AddressOfLeaf(),TreeCombine());

	typedef std::list<Ref> rlist_t;

	intptr_t mask = ~((intptr_t)forEach(rhs,IAddressOfLeaf(),AndBitsCombine()) & (intptr_t)&lhs);
 
	rlist_t rlist = forEach(rhs2,RefListLeaf(mask),ListCombine<Ref>());

	rlist.sort(ref_is_ordered);
	rlist.unique(ref_is_equal);

	rlist_t rlista = rlist;
	rlista.push_back(
		Ref(&lhs,0,lhs.data(),4,
			PrintF< clmulti_array<T, 4> >::type_str(),
			PrintF< clmulti_array<T, 4> >::arg_str(tostr(mask & (intptr_t)&lhs)),
//			PrintF< clmulti_array<T, 4> >::tmp_decl_str(tostr(mask & (intptr_t)&lhs)),
			PrintF< clmulti_array<T, 4> >::tmp_decl_str(mask & ((intptr_t)&lhs),lhs),
			PrintF< clmulti_array<T, 4> >::tmp_ref_str(tostr(mask & (intptr_t)&lhs)),
			PrintF< clmulti_array<T, 4> >::store_str(tostr(mask & (intptr_t)&lhs))
		)
	);
	rlista.sort(ref_is_ordered);
	rlista.unique(ref_is_equal);

	int size1 = lhs.shape()[0];
	int size2 = lhs.shape()[1];
	int size3 = lhs.shape()[2];
	int size4 = lhs.shape()[3];
	int size = size1*size2*size3*size4;
	int r = size;
	if (r%256 > 0) r += 256 - r%256;

	static cl_kernel krn = (cl_kernel)0;

	if (!krn) {

		std::string srcstr = "__kernel void\nkern(\n";

		int n = 0;	
		for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
			srcstr += (*it).type_str + " " + (*it).arg_str + ",\n";
	
		}
		srcstr += "int size, int size2, int size3, int size4\n";

		srcstr += "){\n";
		srcstr += "int gti = get_global_id(0);\n";
		srcstr += "int gti1 = gti/size2/size3/size4;\n";
		srcstr += "int gti2 = gti/size3/size4;\n";
		srcstr += "int gti3 = gti/size4;\n";

		if (size != r) srcstr += "if (gti<size) {\n";

		for( rlist_t::iterator it = rlist.begin(); it!=rlist.end(); it++,n++) {
			srcstr += (*it).tmp_decl_str;
			switch( (*it).dim ) {
				case 4: srcstr += "[gti]"; break;
				case 3: srcstr += "[gti3]"; break;
				case 2: srcstr += "[gti2]"; break;
				case 1: srcstr += "[gti1]"; break;
				default: break;
			}
			srcstr += ";\n";
		}


		srcstr += PrintF< clmulti_array<T, 1> >::store_str(tostr(mask & (intptr_t)&lhs)) + "[gti] = ";

		std::string expr = forEach(rhs,PrintTmpLeaf(mask),PrintCombine());
		srcstr += expr + ";\n" ;
		
		if (size != r) srcstr += "}\n";

		srcstr += "}\n";

		log_kernel(srcstr);

		void* clh = clsopen(__CLCONTEXT,srcstr.c_str(),CLLD_NOW);
		krn = clsym(__CLCONTEXT,clh,"kern",CLLD_NOW);
	}

	if (krn) {

		clndrange_t ndr = clndrange_init1d(0,r,__WGSIZE);

	int n = 0;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
		size_t sz = (*it).sz;
		if (sz > 0) {
			int dummy;
			clSetKernelArg(krn,n,sz,(*it).ptr);
		} else { 


#if defined(__CLMULTI_ARRAY_FULLAUTO)
			clmattach(__CLCONTEXT,(void*)(*it).memptr);
			clmsync(__CLCONTEXT,0,(void*)(*it).memptr,CL_MEM_DEVICE|CL_EVENT_NOWAIT);
#endif

			clarg_set_global(__CLCONTEXT,krn,n,(void*)(*it).memptr);
//			(*it).ptr->clarg_set_global(__CLCONTEXT,krn,n);

		}
	}

	clarg_set(__CLCONTEXT,krn,n,size);
	clarg_set(__CLCONTEXT,krn,n+1,size2);
	clarg_set(__CLCONTEXT,krn,n+2,size3);
	clarg_set(__CLCONTEXT,krn,n+3,size4);


		clfork(__CLCONTEXT,0,krn,&ndr,CL_EVENT_NOWAIT);

#if defined(__CLMULTI_ARRAY_FULLAUTO)

		clmsync(__CLCONTEXT,0,lhs.data(),CL_MEM_HOST|CL_EVENT_NOWAIT);

		clwait(__CLCONTEXT,0,CL_KERNEL_EVENT|CL_MEM_EVENT);

	n = 0;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
		size_t sz = (*it).sz;
		if (sz > 0) {
		} else { 

			clmdetach((void*)(*it).memptr);

		}
	}

#elif defined(__CLMULTI_ARRAY_SEMIAUTO)

		clwait(__CLCONTEXT,0,CL_KERNEL_EVENT);

#endif


	}


#else

      for (int i = 0; i < lhs.shape()[0]; ++i) 
      for (int j = 0; j < lhs.shape()[1]; ++j) 
      for (int k = 0; k < lhs.shape()[2]; ++k) 
      for (int l = 0; l < lhs.shape()[3]; ++l) {
          op(lhs[i][j][k][l], forEach(rhs, EvalLeaf4(i,j,k,l), OpCombine()));
        }

#endif


    } else {
      cerr << "Error: LHS and RHS don't conform." << endl;
      exit(1);
    }
}


template < typename T >  template<class RHS>
clmulti_array_interval<T,1>&
clmulti_array_interval<T,1>::operator=(const Expression<RHS> &rhs)
{
    assign(*this,rhs);
    return *this;
}

template < typename T > 
clmulti_array_interval<T,1>&
clmulti_array_interval<T,1>::operator=(const clmulti_array_interval<T,1>& rhs)
{
    assign(*this,Expression<clmulti_array_interval<T,1> >(rhs));
    return *this;
}


template < typename T >  template<class RHS>
clmulti_array_interval<T,2>&
clmulti_array_interval<T,2>::operator=(const Expression<RHS> &rhs)
{
    assign(*this,rhs);
    return *this;
}


template < typename T >  template<class RHS>
clmulti_array_interval<T,3>&
clmulti_array_interval<T,3>::operator=(const Expression<RHS> &rhs)
{
    assign(*this,rhs);
    return *this;
}


#endif

