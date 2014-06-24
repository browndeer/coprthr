/* clvector_interval_CLETE.h
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

#ifndef _CLVECTOR_INTERVAL_CLETE_H
#define _CLVECTOR_INTERVAL_CLETE_H

#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <cstdlib>
#include <stdcl.h>
using namespace std;

#include <stdcl.h>
#include <clvector.h>

#include "CLETE/PETE.h"
#include "CLETE/clvector_interval_Operators.h"
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
// This file contains several class definitions that are used to evaluate
// expressions containing STL vectors.  The main function defined at the end
// is evaluate(lhs,op,rhs), which allows the syntax:
// vector<int> a,b,c;
// evaluate(a,OpAssign(),b+c);
//
// evaluate() is called by all the global assignment operator functions
// defined in VectorOperators.h
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// We need to specialize CreateLeaf<T> for our class, so that operators
// know what to stick in the leaves of the expression tree.
//-----------------------------------------------------------------------------

template<class T, class Allocator>
struct CreateLeaf<clvector_interval<T, Allocator> >
{
  typedef Reference<clvector_interval<T> > Leaf_t;
  inline static
  Leaf_t make(const clvector_interval<T, Allocator> &a) { return Leaf_t(a); }
};

//-----------------------------------------------------------------------------
// EvalLeaf1 is used to evaluate expression with vectors.
// (It's already defined for Scalar values.)
//-----------------------------------------------------------------------------

/*
 * XXX the EvalLeaf functors are not used for offload, fix later -DAR

template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>,EvalLeaf1>
{
  typedef T Type_t;
  inline static
  Type_t apply(const clvector_interval<T, Allocator>& vec,const EvalLeaf1 &f)
  {
    return vec[f.val1()];
  }
};

template <>
struct LeafFunctor<Interval,EvalLeaf1>
{
  typedef int Type_t;
  inline static
  Type_t apply(const Interval& ival, const EvalLeaf1 &f)
  {
    return ival.shift + f.val1();
  }
};
*/


template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>, SizeLeaf>
{
  typedef bool Type_t;
  inline static
  bool apply(const clvector_interval<T, Allocator> &v, const SizeLeaf &s) 
  {
    return s(v.size());
  }
};

template <>
struct LeafFunctor<Interval, SizeLeaf>
{
  typedef bool Type_t;
  inline static
  bool apply(const Interval &v, const SizeLeaf &s) 
  {
    return s(0);
  }
};


template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>, PrintTmpLeaf>
{
  typedef std::string Type_t;
  inline static
  std::string apply(
		const clvector_interval<T, Allocator> & v, const PrintTmpLeaf & p
	) { return "tmp" + tostr(p((intptr_t)&v)); }
};

template <>
struct LeafFunctor<Interval, PrintTmpLeaf>
{
  typedef std::string Type_t;
  inline static
  std::string apply(const Interval & v, const PrintTmpLeaf & p) 
  {
    return "(gti+(" + tostr(v.shift) + "))";
  }
};


template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>*, PtrListLeaf>
{
  typedef std::list<const void*> Type_t;
  inline static
  Type_t apply(clvector_interval<T, Allocator>* const & ptr, const PtrListLeaf &plist)
  {
    return Type_t(1,ptr);
  }
};


template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>*, RefListLeaf>
{
  typedef std::list<Ref> Type_t;
  inline static
  Type_t apply(clvector_interval<T, Allocator>* const & ptr, const RefListLeaf &r)
  {
    return Type_t(1,Ref(
		ptr,0,ptr->interval.first,ptr->interval.end,ptr->interval.shift,ptr->data(),1,
		PrintF< clvector_interval<T,Allocator> >::type_str(),
		PrintF< clvector_interval<T,Allocator> >::arg_str(tostr(r((intptr_t)ptr))),
		PrintF< clvector_interval<T,Allocator> >::tmp_decl_str(tostr(r((intptr_t)ptr)),
			tostr(ptr->interval.shift)),
		PrintF< clvector_interval<T,Allocator> >::tmp_ref_str(tostr(r((intptr_t)ptr))),
		PrintF< clvector_interval<T,Allocator> >::store_str(tostr(r((intptr_t)ptr))) ) );
  }
};

template <>
struct LeafFunctor<Interval*, RefListLeaf>
{
  typedef std::list<Ref> Type_t;
  inline static
  Type_t apply(Interval* const & ptr, const RefListLeaf &r)
  {
    return Type_t(1,Ref(
		ptr,0,ptr->first,ptr->end,ptr->shift,0,1,
		PrintF< Interval >::type_str(),
		PrintF< Interval >::arg_str(tostr(r((intptr_t)ptr))),
		PrintF< Interval >::tmp_decl_str(tostr(r((intptr_t)ptr)),
			tostr(ptr->shift)),
		PrintF< Interval >::tmp_ref_str(tostr(r((intptr_t)ptr))),
		PrintF< Interval >::store_str(tostr(r((intptr_t)ptr))) ) );
  }
};


template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>,AddressOfLeaf>
{
  typedef clvector_interval<T, Allocator>* Type_t;
  inline static
  Type_t apply(const clvector_interval<T, Allocator>& vec,const AddressOfLeaf &f)
  {
    return Type_t(&vec);
  }
};

template <>
struct LeafFunctor<Interval,AddressOfLeaf>
{
  typedef Interval* Type_t;
  inline static
  Type_t apply(const Interval& ival,const AddressOfLeaf &f)
  {
    return Type_t(&ival);
  }
};


template<class T, class Allocator>
struct LeafFunctor<clvector_interval<T, Allocator>,IAddressOfLeaf>
{
  typedef intptr_t Type_t;
  inline static
  Type_t apply(const clvector_interval<T, Allocator>& vec,const IAddressOfLeaf &f)
  {
    return Type_t((intptr_t)&vec);
  }
};

template <>
struct LeafFunctor<Interval,IAddressOfLeaf>
{
  typedef intptr_t Type_t;
  inline static
  Type_t apply(const Interval& ival,const IAddressOfLeaf &f)
  {
    return Type_t((intptr_t)&ival);
  }
};


template < class T, class Allocator >
struct PrintF< clvector_interval<T, Allocator> > {

   inline static std::string type_str() 
   { return "__global " + PrintType<T>::type_str() + "*"; }

   inline static std::string arg_str( std::string x) 
   { return "a" + x; }

   inline static std::string tmp_decl_str( std::string x, std::string s )
   { 
		return PrintType<T>::type_str() + " tmp" + x 
			+ " = a" + x + "[gti+(" + s + ")]"; 
	}

   inline static std::string tmp_ref_str( std::string x )
   { return "tmp" + x; }

   inline static std::string store_str( std::string x )
   { return "a" + x + "[gti] "; }

};


#define log_kernel(srcstr) do { \
	if (__log_automatic_kernels_filename) { \
		std::ofstream ofs( \
			__log_automatic_kernels_filename, \
			std::ios_base::out|std::ios_base::app); \
		ofs<<srcstr<<"\n"; \
		ofs.close(); \
	} \
	} while (0)



template<class T, class Allocator, class Op, class RHS>
inline void evaluate(
	clvector_interval<T, Allocator> &lhs, const Op &op, 
	const Expression<RHS> &rhs
)
{
//  if (forEach(rhs, SizeLeaf(lhs.size()), AndCombine())) {
  if (1) {

#if defined(__CLVECTOR_SEMIAUTO) || defined(__CLVECTOR_FULLAUTO)

	typedef typename 
		ForEach<Expression<RHS>, AddressOfLeaf, TreeCombine >::Type_t New_t;

	New_t rhs2 = forEach(rhs,AddressOfLeaf(),TreeCombine());

	typedef std::list<Ref> rlist_t;

	intptr_t mask = ~((intptr_t)forEach(rhs,IAddressOfLeaf(),
		AndBitsCombine()) & (intptr_t)&lhs);
 
	rlist_t rlist = forEach(rhs2,RefListLeaf(mask),ListCombine<Ref>());

	rlist.sort(ref_is_ordered);
	rlist.unique(ref_is_equal);

	rlist_t rlista = rlist;
	rlista.push_back(
		Ref(&lhs,0,lhs.interval.first,lhs.interval.end,lhs.interval.shift,lhs.data(),1,
			PrintF< clvector_interval<T, Allocator> >::type_str(),
			PrintF< clvector_interval<T, Allocator> >::arg_str(tostr(mask & (intptr_t)&lhs)),
			PrintF< clvector_interval<T, Allocator> >::tmp_decl_str(
				tostr(mask & (intptr_t)&lhs), tostr(lhs.interval.shift) ),
			PrintF< clvector_interval<T, Allocator> >::tmp_ref_str(
				tostr(mask & (intptr_t)&lhs)),
			PrintF< clvector_interval<T, Allocator> >::store_str(
				tostr(mask & (intptr_t)&lhs))
		)
	);
	rlista.sort(ref_is_ordered);
	rlista.unique(ref_is_equal);

	int size = lhs.vecref.size();

	size_t r = size;
	if (r%256 > 0) r += 256 - r%256;

	int first = lhs.interval.first;
	int end = lhs.interval.end;
	
	static cl_kernel krn = (cl_kernel)0;

	if (!krn) {

		std::string srcstr = "__kernel void\nkern( int first, int end, \n";

		int n = 0;	
		for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
			std::string argstr = (*it).arg_str;
			if (argstr != "INTERVAL") 
				srcstr += (*it).type_str + " " + argstr + ",\n";
	
		}
		srcstr += "int size\n";

		srcstr += "){\n";
		srcstr += "int gti = get_global_id(0);\n";

		srcstr += "if (gti >= first && gti < end ) {\n";

		if (size != r) srcstr += "if (gti<size) {\n";

		for( rlist_t::iterator it = rlist.begin(); it!=rlist.end(); it++,n++) {
			srcstr += (*it).tmp_decl_str + ";\n";
		}

		std::string expr = forEach(rhs,PrintTmpLeaf(mask),PrintCombine());

		srcstr += op.strexpr( 
			PrintF< clvector_interval<T, Allocator> >::store_str(
				tostr(mask & (intptr_t)&lhs)), expr ) + ";\n" ;
		
		if (size != r) srcstr += "}\n";

		srcstr += "}\n";
		srcstr += "}\n";

		log_kernel(srcstr);

		void* clh = clsopen(__CLCONTEXT,srcstr.c_str(),CLLD_NOW);
		krn = clsym(__CLCONTEXT,clh,"kern",CLLD_NOW);
	}


	if (krn) {

		clndrange_t ndr = clndrange_init1d(0,r,__WGSIZE);

	clSetKernelArg(krn,0,sizeof(int),&first);
	clSetKernelArg(krn,1,sizeof(int),&end);
	int n = 2;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++) {

		size_t sz = (*it).sz;

		if (sz > 0) {

			fprintf( stderr,"%d clSetKernelArg sz>0",n);
			
			clSetKernelArg(krn,n++,sz,(*it).ptr);

		} else { 

			if ((*it).memptr != 0) {

#if defined(__CLVECTOR_FULLAUTO)

				clmattach(__CLCONTEXT,
					(void*)((clvector_interval<T,Allocator>*)(*it).ptr)->vecref.data());
				clmsync(__CLCONTEXT,0,
					(void*)((clvector_interval<T,Allocator>*)(*it).ptr)->vecref.data(),
					CL_MEM_DEVICE|CL_EVENT_NOWAIT);
#endif

			fprintf( stderr,"%d clarg_set_global sz==0",n);

				clarg_set_global(__CLCONTEXT,krn,n++,
					(void*)((clvector_interval<T,Allocator>*)(*it).ptr)->vecref.data());
			}

		}
	}

	clarg_set(__CLCONTEXT,krn,n,size);


	clfork(__CLCONTEXT,0,krn,&ndr,CL_EVENT_NOWAIT);

#if defined(__CLVECTOR_FULLAUTO)

		clmsync(__CLCONTEXT,0,lhs.vecref.data(),CL_MEM_HOST|CL_EVENT_NOWAIT);

		clwait(__CLCONTEXT,0,CL_KERNEL_EVENT|CL_MEM_EVENT);

	n = 0;	
	for( rlist_t::iterator it = rlista.begin(); it!=rlista.end(); it++,n++) {
		size_t sz = (*it).sz;
		if (sz > 0) {
		} else { 

			if ((*it).memptr != 0)
				clmdetach((void*)((clvector_interval<T,Allocator>*)(*it).ptr)->vecref.data());

		}
	}

#elif defined(__CLVECTOR_SEMIAUTO)

		clwait(__CLCONTEXT,0,CL_KERNEL_EVENT);

#endif

	}


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

done:
	return;

}


  template < typename T, typename A> template<class RHS>
  clvector_interval<T,A>& clvector_interval<T,A>::operator=(const Expression<RHS> &rhs)
  {
      assign(*this,rhs);

    return *this;
  }



#endif

